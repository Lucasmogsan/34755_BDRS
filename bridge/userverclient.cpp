/***************************************************************************
 *   Copyright (C) 2006 by DTU (Christian Andersen)                        *
 *   jca@oersted.dtu.dk                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Lesser General Public License for more details.                   *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

//#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <string>

#include "userverclient.h"
#include "userverport.h"
#include "uhandler.h"
#include "usource.h"

///////////////////////////////////////////////

void UServerClient::setup(int index)
{
  const int MSL = 32;
  char s[MSL];
  snprintf(s, MSL, "socket%02d", index);
  sourceNum = index;
  setSourceID(s);
  conn = -1;
  connected = false;
  msgReceived = 0;
  msgSend = 0;
  tcpipReceived = 0;
  msgBufCnt = 0;
  msgSkippedBytes = 0;
  serverAlive = NULL;
//   handler = NULL;
  msgBufTime.now();
  msgSendTime.now();
}

///////////////////////////////////////////////

UServerClient::~UServerClient()
{
  stopConnection(true, false);
}


///////////////////////////////////////////////

void UServerClient::justConnected()
{ // we just got a new client
  sendLock.lock();
  sendStr("# Welcome to robot bridge (HAL) - send 'help' for more info\r\n");
  sendStr("bridge 1 crc mod99\r\n");
  sendLock.unlock();
  errCnt = 0;
  terr.Now();
  connectTime.now();
}

///////////////////////////////////////////////

bool UServerClient::blockSend(const char * buffer, int length, int msTimeout)
{ // returns true if send and false if connection timeout
  const int pollTime = 5; // miliseconds
  bool result = true;
  int d = 0, n, t = 0;
  // status out is not used, if no error, then just try
  if (connected)
  { // still connected (no error yet)  
    // Add CRC 
    int sum = 0;
    for (int i=0; i < length; i++)
      if (buffer[i] >= ' ')
        sum += buffer[i];
    const int MSL = 4;
    char s[MSL];
    snprintf(s, MSL, ";%02d", (sum % 99)+1);
    std::string ss = s;
    ss.append(buffer);    
    // send length bytes
    const char * sc = ss.c_str();
    while ((d < length) and (t < msTimeout) and result)
    { // get status
      n = send(conn, &sc[d], length + 3 - d, MSG_DONTWAIT |
                                              MSG_NOSIGNAL);
      if (n < 0)
      { // error - an error occurred while sending
        switch (errno)
        {
          case EAGAIN:
            //not all send - just continue
            //printf("UServerClient::blockSend: waiting - nothing send %d/%d\n", d, length);
            usleep(5000);
            break;
          case EFAULT:
            perror("UServerClient::blockSend: EFAULT: ");
            connected = false;
            break;
          case EINTR:
            perror("UServerClient::blockSend: EINTR: ");
            connected = false;
            break;
          case EINVAL:
            perror("UServerClient::blockSend: EINVAL: ");
            connected = false;
            break;
          case EPIPE:
            perror("UServerClient::blockSend: EPIPE: ");
            connected = false;
            break;
          default:
            perror("UServerClient::blockSend (continues): ");
            result = false;
            break;
        }
        // dump the rest on most errors
        if (not connected)
          result = false;
        if (not result)
          break;
      }
      else
        // count bytes send
        d += n;
      t += pollTime;
    }
    if (not connected)
    {
      errCnt++;
      terr.now();
      const int MSL = 100;
      char s[MSL], s2[MSL];
      snprintf(s, MSL, "UServerClient::blockSend: err %d send failed at %s\r\n", errCnt, terr.getDateTimeAsString(s2));
      handler.handleCommand(this, s, true);
      shutdown(conn, SHUT_RDWR);
      close(conn);
    }
  }
  else
    result = false;
  // count messages
  if (result)
    result = t < msTimeout;
  if (result)
  {
    msgSend++;
    msgSendTime.now();
  }
  if (not result)
  {
    const int MSL = 40;
    char s[MSL];
    int n = mini(MSL - 1, length);
    strncpy(s, buffer, n);
    s[n] = '\0';
    printf("UServerClient::blockSend: failed to send %d chars: '%s'\n", length, s);
  }
  // debug end
  //
  return result;
}

////////////////////////////////////////////////////////

bool UServerClient::initConnection(int clientNumber, int clnt, struct sockaddr_in from,
                                  UTime * aliveTime)
{ // connection info
  clientInfo = from;
  conn = clnt;
  connected = true;
  // statistics init
  msgSend = 0;
  tcpipReceived = 0;
  msgReceived = 0;
  msgBufCnt = 0;
  sourceNum = clientNumber;
  // reset timers
  msgBufTime.now();
  msgSendTime.now();
  connectTime.now();
  serverAlive = aliveTime;
  noCRC = false;
  // send a welcome message
//   justConnected();
  //
  //
  return connected;
}

////////////////////////////////////////////////////////

char * UServerClient::getClientName()
{
  char * result = NULL;
  //
  if (connected)
    result = inet_ntoa(clientInfo.sin_addr);
  //
  return result;
}

///////////////////////////////////////////////

void UServerClient::stopConnection(bool sendHUP, bool stopSubscriptions)
{
  if (connected)
  {
    if (sendHUP)
    { // send hup to client
      shutdown(conn, SHUT_RDWR);
    }
    close(conn);
    connected = false;
    //
    if (stopSubscriptions)
    { // to this client (source)
  //     printf("# UServerClient::stopConnection  1\n");
      handler.items->stopSubscription(sourceNum);
  //     printf("# UServerClient::stopConnection 99\n");
    }
  }
}

////////////////////////////////////////////////

bool UServerClient::receiveData()
{ // data is available
  bool result = true;
  const float UNUSED_MESSAGE_PART_TIMEOUT = 10.0; // seconds
  int len;
  UTime t;
  //
  // test for old
  if (msgBufCnt > 0)
  { // test if buffer is too old to use
    t.Now();
    if ((t - msgBufTime) > UNUSED_MESSAGE_PART_TIMEOUT)
    { // skip old remainings
      msgSkippedBytes += msgBufCnt;
      msgBufCnt = 0;
    }
  }
  // get position in buffer to fill new data
  len = MAX_MSG_LENGTH - msgBufCnt;
  // get data
  len = recv(conn, &msgBuff[msgBufCnt], len, 0);
  if (len > 0)
  { // data received
    msgBufCnt += len;
    // terminate if string
    msgBuff[msgBufCnt] = '\0';
    // set receive-time
    msgBufTime.Now();
    //
    while (true)
    { // there must be a newline to end a message
      // find valid start
      char * p2 = msgBuff;
      while ((*p2 <= ' ' or *p2 > 'z') and (p2 - msgBuff) < msgBufCnt)
        p2++;
      // look for first message
      const char * pnl = strchr(p2, '\n');
      if (pnl != NULL)
      { // find first message delimited with either a \n or a \r
        char * pe;
        char * p1 = strtok_r(p2, "\r\n", &pe);
        if (p1 != NULL)
        { //
          // process message
          gotNewMessage(p1);
          //
          //
          // statistics count
          tcpipReceived++;
          connectedTime = connectTime.getTimePassed();
          // find end or start of new message
          while (*pe <= ' ' and *p1 != 0)
            pe++;
          // test if more data left
          int used = pe - msgBuff;
          if (used >= msgBufCnt)
            // no more valid data
            msgBufCnt = 0;
          else
          { // there could be more valid data, move it to start of buffer
            memmove(msgBuff, pe, msgBufCnt - used + 1); // move also string terminator
            msgBufCnt -= used;
          }
        }
      }
      else if (p2 > msgBuff)
      { // remove leading garbage
        int used = p2 - msgBuff;
        memmove(msgBuff, p2, msgBufCnt - used);
        msgBufCnt -= used;
      }
      if (pnl == NULL)
        // no more valid messages
        break;
    }
  }
  else if (len == 0)
  { // closed, e.g. HUP received (don't send HUP)
    usleep(1000);
  }
  else // len = -1
  { // not a message -  other signal that retry
    perror("UServerClient::receiveData: there should be data iaw poll - ignoring\n");
    //stopConnection(false); // don't send HUP
    result = false;
  }
  return result;
}


////////////////////////////////////////////////////////

void UServerClient::gotNewMessage(char * msg)
{ // Got new message from client for parsing
  if (msg != NULL)
  { // got a line of data
    if (msgReceived == 0)
    { // first sign of data
      justConnected();
    }
    if ((msg[0] == 'h' and msg[1] < ' ') or strncmp(msg, "help", 4) == 0)
    {
      handler.handleCommand(this, "help\n", true);
      sendLock.lock();
      sendStr("# send i=1 for no CRC mode\r\n");
      sendLock.unlock();
    }
    else 
    {
      if (strncmp(msg, "i=1", 3) == 0)
      {
        noCRC = true;
        sendLock.lock();
        sendStr("# now in no CRC mode\r\n");
        sendLock.unlock();
      }
      else
      handler.handleCommand(this, msg, noCRC);
    }
    msgReceived++;
  }
}

//////////////////////////////////////////////////////////////


void UServerClient::sendAliveReply()
{
  const int MRL = 50;
  char reply[MRL];
  snprintf(reply, MRL, "alive\n");
  sendLock.lock();
  sendStr(reply);
  sendLock.unlock();
}

/////////////////////////////////////////////



