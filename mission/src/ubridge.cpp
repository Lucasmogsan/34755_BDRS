/*  
 * 
 * Copyright © 2022 DTU, 
 * Author:
 * Christian Andersen jcan@dtu.dk
 * Jacob Bechmann Pedersen
 * 
 * The MIT License (MIT)  https://mit-license.org/
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software 
 * is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies 
 * or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE. */

#include <string>
#include <string.h>
#include "ubridge.h"
#include "upose.h"
#include "ucomment.h"
#include "ustate.h"
#include "uvision.h"
#include "uevent.h"
#include "ujoy.h"

// create the bridge connection
UBridge bridge;


void UBridge::decode(char* msg)
{ /// ask all if they can use this message
  if (comment.decode(msg)) {}
  else if (pose.decode(msg)) {}
  else if (state.decode(msg)) {}
  else if (event.decode(msg)) {}
  else if (joy.decode(msg)) {}
  else
    printf("Received, but not used: %s\n", msg);
}

void shutdown(int signal)
{ // shutdown due to signal - especially CTRL-C
  printf("# Shutting down due to signal %d\n", signal);
  bridge.terminate=true;
  vision.terminate = true;
}

// Bridge class:
void UBridge::setup(const char ip[], const char port[], int argc, char **argv)
{ // setup handling of CTRL-C
  sigIntHandler.sa_handler = shutdown;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0; 
  sigaction(SIGINT, &sigIntHandler, NULL);
  //
  // decode extra parameters
  for (int i = 1; i < argc; i++)
  { // check for command line parameters
    // for process debug
    if (strcmp(argv[i], "nobridge") == 0)
      usebridge = false;
  }
  /// Setup socket to use
  if (usebridge)
  {
    int res;
    host = ip;
    hostport = port;
    if ((res = getaddrinfo(ip, port, nullptr, &servinfo)) != 0)
    { // failed
      fprintf(stderr,"# Getting address info failed: %s\n", gai_strerror(res));
    }
    if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
    { // failed
      fprintf(stderr,"# Failed socket creation to %s:%s\n", ip, port);
    }
    // try to connect
    if (connect(sockfd,servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    { // failed
      perror("# Bridge connect failed");
      connected = false;
      close(sockfd);
    }
    else
    { // connection established
      connected = true;
    }
    /// Start the listen thread:
    if (connected)
      listener = new thread(startloop, this);
  }
  else
  {
    printf("# Running without bridge connection\n");
    // set the connected flag anyhow
    connected = true;
  }
}

UBridge::~UBridge()
{
  stop();
  if (servinfo != NULL){
    freeaddrinfo(servinfo);
  }
}

void UBridge::stop()
{
  if (connected)
  { // tell bridge we are done
    connected = false;
    // wait for it to suck in
    usleep(300000);
    tx("# Bridge disconnected\n");
    listener->join();
    close(sockfd);
  }
}


void UBridge::tx(const char * msg)
{ // adding CRC check
  int n = strlen(msg);
  const char * p1 = msg;
  const char * p2;
  sendMtx.lock();
  if (connected and not terminate and usebridge)
  {
    while (*p1 != '\0' and n > 1)
    { // there is more data
      int sum = 0;
      while ((*p1 == ' ' or *p1 == '\t') and *p1 != '\0')
        // skip space and tabulator characters
        p1++;
      // save start of message line
      p2 = p1;
      while (*p1 != '\n' and *p1 != '\0')
      { // sum all non-white characters
        if (*p1 >= ' ')
          sum += *p1;
        p1++;
        // count remaining characters
        n--;
      }
      const int MCL = 4;
      char crc[MCL];
      /// calculate a number in range [01..99] as CRC after a ';' key
      snprintf(crc, MCL, ";%02d", sum % 99 + 1);
      //
      if (connected)
      { /// send CRC first
        send(sockfd, crc, 3, 0);
      }
      if (connected)
      { /// then the message - ending with a new-line (\n)
        p1++; // include also the '\n' character
        int len = p1 - p2; // including 
        send(sockfd, msg, len, 0);
      }
//       printf("# user mission send (%d):  '%s%s'\n", connected, crc, msg);
      // should not be needed
      usleep(4000);
    }
  }
  sendMtx.unlock();
  if (not usebridge)
  {
    printf("# bridge would send:%s", msg);
  }
}

void UBridge::startloop(UBridge * bridge)
{ // this is a static method for the class,
  // transfer control to the used class object
  bridge->loop();
}

void UBridge::loop()
{  
  const int MAX_RX_CNT = 500;
  char rxBuf[MAX_RX_CNT] = {0}; // fixed receive buffer (array of characters)
  unsigned int rxCnt = 0; /// received string length
  /// listen loop
  while (connected and not terminate)
  { // receive 1 character
    char recvChar;
    int e = recv(sockfd, &recvChar, 1, MSG_DONTWAIT);
    if (e == 1)
    { /// got a character
      // Check accepted chars
      if (recvChar >=' ' or recvChar == '\n' or recvChar == '\t')
      { // collect to a string (a fixed array of characters for speed)
        rxBuf[rxCnt] = recvChar;
        // If the line ends, terminate string and decode
        if (rxBuf[rxCnt] == '\n')
        { // terminate string (replacing the newline '\n')
          rxBuf[rxCnt] = '\0';
          // unpack this line
          unpackMessage(rxBuf);
          // ready for next message
          rxCnt = 0;
        }
        else if (rxCnt < MAX_RX_CNT)
        { // Increment string length
          rxCnt++;
        }
        else
        { // Buffer overflow
          printf("Bridge listen loop overflow (discards the buffer)\n");
          rxCnt = 0;
        }
      }
    }
    else if (e < 0 and errno != EAGAIN)
    { // lost connection with hardware
      // shut down
      printf("### lost hardware connection (errno=%d) ###\n", errno);
      connected = false;
      close(sockfd);
    }
    else
    { // no data, wait a bit
      usleep(900);
    }
  }
}

void UBridge::unpackMessage(char * msg)
{ // strip CRC and newline
  if (msg[0] == ';')
  { // two next characters are CRC, ASCII coded
    char * p1 = &msg[1];
    int crc = (*p1++ - '0') * 10;
    crc += *p1++ - '0';
    int charSum = 0;
    while (*p1 != '\n' and *p1 != '\0')
    {
      if (*p1 >= ' ')
        charSum += *p1;
      p1++;
    }
    // remove the newline character
    *p1 = '\0';
    // check result
    int crc2 = charSum % 99 + 1;
    if (crc == crc2)
    {
      decode(&msg[3]);
    }
    else
      printf("# CRC error (crc=%d sum%%99+1=%d): '%s'\n", crc, crc2, msg);
  }
  else
    printf("# message with wrong format discarded: '%s'\n", msg);
}
