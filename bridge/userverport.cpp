/***************************************************************************
 *   Copyright (C) 2017 by DTU (Christian Andersen)                        *
 *   jca@elektro.dtu.dk                                                    *
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

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include "userverport.h"
#include "uhandler.h"


UServerPort server;

/////////////////////////////////////////////////

// UServerPort::UServerPort() //, UBridge * bridgePtr)
// {
//   serverPort = 24001;
//   for (int i = 0; i < MAX_SOCKET_CLIENTS; i++)
//     client[i] = nullptr;
//   clientCnt = 0;
//   clientCntActive = 0;
//   //port = 24920; // default
//   open4Connections = false;
//   verboseMessages = false;
//   recvLoops = 0;
//   serverAlive.now();
//   lastClientNumber = -1;
//   getHostName();
//   allowConnections = true;
// //   bridge = bridgePtr;
// }

/////////////////////////////////////////////////

UServerPort::~UServerPort()
{
  // free client objects
  for (int i = 0; i < clientCnt; i++)
    delete client[i];
}

////////////////////////////////////////////////////////

void UServerPort::setup(int port)
{
  serverPort = port;
  for (int i = 0; i < MAX_SOCKET_CLIENTS; i++)
    client[i] = nullptr;
  clientCnt = 0;
  clientCntActive = 0;
  //port = 24920; // default
  open4Connections = false;
  verboseMessages = false;
  recvLoops = 0;
  serverAlive.now();
  lastClientNumber = -1;
  getHostName();
  allowConnections = true;
  // start service thread, now that a handler is available
  start();
}

////////////////////////////////////////////////////////

void UServerPort::run()
{ // running the server
  bool result = true;
  int err = 0;
  struct sockaddr_in name, from;
  int sock = -1; // server socket
  int clnt; // client connection
//   const int NSL = 100;
//   char hostname[NSL];
  int i, j = 0;
  socklen_t addr_len;
  int retryCnt = 0;
  int lastErrNo = 0;
  //
  printf("# UServerPort::run: Socket server started %d on %s\n", serverPort, hostname);
  //
  while (isRunning())
  { // thread that service clients is now running
    running = true;
    if (sock >= 0)
    { // test if new port is to be usedAHEAD
      if (name.sin_port != htons(serverPort) or not allowConnections)
      { // new port - close (but leave existing clients)
        close(sock);
        sock = -1;
        open4Connections = false;
      }
    }
    if (sock < 0 and (serverPort >= 1000) and allowConnections)
    {
//       printf("port server opening %d on %s  (handle=%d)\n", serverPort, hostname, sock);
      sock = socket(AF_INET, SOCK_STREAM, 0);
      result = (sock >= 0);
      if (not result)
        perror("*** UServerPort::runServerThread (socket)");
      if (result)
      { // set socket port reuse option
        i = 1; // true
        err = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
        result = (err == 0);
        if (not result)
          perror(strerror(errno));
      }
      // get specific socket
      if (result)
      { // prepare bind to protocol and port
        name.sin_family = AF_INET;
        name.sin_port = htons(serverPort);
        name.sin_addr.s_addr =INADDR_ANY;
        /**/
        err = bind(sock, (struct sockaddr *) &name, sizeof(name));
        if (err != 0)
        { // bind failed - port in use by another server
          if (retryCnt == 0)
            perror("*** UServerPort::runServerThread (bind port)");
          result = false;
          retryCnt++;
        }
        else
          retryCnt = 0;
      }
      //
      if (result)
      { // listen to queue, and allow only 1 pending connections
        err = listen(sock, 3);
        if (err != 0)
        { // listen failed, - no more space?
          perror("*** UServerPort::runServerThread (listen)");
          result = false;
        }
        else
        {
          open4Connections = true;
//           printf("port %d is open for connections\n", serverPort);
        }
      }
      //
      if (result)
      { // listen is sucessfull, now accept connections
        if (verboseMessages)
        { // print a connect message
//           i = gethostname(hostname, NSL - 1);
//           hostname[NSL - 1] = 0; // just in case of a very long hostname
          printf("      --- %s is open on port %d for requests.\n", hostname, serverPort);
        }
        //
        addr_len = sizeof(from);
        // set server to non-blocking
        fcntl(sock, F_SETFL, O_NONBLOCK);
      }
      if (not result)
      { // cound not bind socket port - close
        if (sock >= 0)
        {
          close(sock);
          sock = -1;
        }
      }
    }
    if (sock >= 0)
    { // accept connections
      clnt = accept(sock, (struct sockaddr*) & from, &addr_len);
      if (clnt == -1)
      { // not a connect - may be a ctrl-c
        i = errno;
/*        j = EOPNOTSUPP; // 95
        j = ENOTSOCK;   // 88
        j = EBADF;      //  9
        j = EWOULDBLOCK;// 11
        Too many open files: errno = 24
        k = EAGAIN;     // 11*/
        if (i !=EWOULDBLOCK)
        { // other signal that retry
          if (i != lastErrNo)
            perror("*** UServerPort::runServerThread (accept)");
          // debug
          else if (i == 24)
          { // too many open files - how is this possible??
            // ulimit say max is 1024, so either an error or someone eats file handles
            usleep(50000);
            lastErrNo = errno;
            j++;
            if (j > 100)
            { // time to report
              j = 0;
              lastErrNo = 0;
            }
          }
          // debug end
        }
      }
      else
      { // connection available
        // get name of requester
        err = getpeername(clnt,
                          (struct sockaddr *)&from,
                          &addr_len);
        if ((err == 0) and verboseMessages)
          // name is available
          printf("Connection request from IP: %s\n", inet_ntoa(from.sin_addr));
        //
        // activate a client object
        i = getFreeClientHandle();
        if (i >= 0)
        { // set client
          client[i]->initConnection(i, clnt, from, &serverAlive);
          gotNewClient(client[i]);
        }
        else
        { // close client connection again - no more space
          if (verboseMessages)
            fprintf(stderr, "*** UServerPort::runServerThread: out of client handles => HUP\n");
          shutdown(clnt, SHUT_RDWR);
          close(clnt);
        }
      }
      // service clients
      if (getActiveClientCnt() > 0)
        // poll clients with a 200 ms timeout
        serviceClients(200);
      else
        // idle - wait a bit (e.g. 0.2 sec)
        usleep(200000);
    }
    else
      // socket not open - wait for available port number
      sleep(1);
  }
  // close socket and terminate
  stopAllConnections();
  if (sock >= 0)
    close(sock);
  //
  running = false;
}

////////////////////////////////////////////////////////


void UServerPort::gotNewClient(UServerClient * cnn)
{
//   const int MRL = 60;
//   char reply[MRL];
  //
  printf("Got new client (%s)\n", cnn->getClientName());
}

////////////////////////////////////////////////

int UServerPort::getFreeClientHandle()
{ // returns unused handle or -1 if no handle is avaulable
  int result = -1;
  int i;
  UServerClient * cnn;
  int cln;
  //
  for (i = 0; i < MAX_SOCKET_CLIENTS; i++)
  {
    lastClientNumber = (lastClientNumber + 1) % MAX_SOCKET_CLIENTS;
    cln = lastClientNumber;
    cnn = client[cln];
    lastClientSerial++;
    if (cnn == NULL)
    {
      cnn = new UServerClient;
      cnn->setup(cln);
      client[cln] = cnn;
    }
    if (cnn != NULL)
      if (not cnn->isActive())
      { // available connection is found
        result = cln;
        cnn->sourceNum = lastClientSerial;
        if (cln >= clientCnt)
          clientCnt = cln + 1;
        break;
      }
  }
  //
  return result;
}

////////////////////////////////////////////////

bool UServerPort::serviceClients(int msTimeout)
{
  bool result = false;
  int err = 0;
  int i;
  int nActive;
  UServerClient * cnn;
  bool dataAvailable;
  struct pollfd pollStatus[MAX_SOCKET_CLIENTS];
  int clientNum[MAX_SOCKET_CLIENTS];
  int revents;
  //
  nActive = 0;
  for (i = 0; i < clientCnt; i++)
  { // prepare poll structures for all clients
    cnn = client[i];
    if (cnn->isActive())
    { // active, so
      pollStatus[nActive].fd = cnn->getCnn();
      pollStatus[nActive].revents = 0;
      pollStatus[nActive].events = POLLIN  |   /*  0x0001  There is data to read */
                                   POLLPRI;   /*  0x0002  There is urgent data to read */
                                /* POLLOUT 0x0004  Writing now will not block */
      // mark index to client structure
      clientNum[nActive] = i;
      // count active clients
      nActive++;
    }
  }
  //
  if (nActive != clientCntActive)
  {
    clientCntActive = nActive;
  }
  //
  if (nActive > 0)
  { // poll for status on this socket (timeout 200 ms)
    err = poll(pollStatus, nActive, msTimeout);
    if (err < 0)
    { // not a valid call (may be debugger interrrupted)
      perror("UServerPort::serviceClients (poll)");
    }
    else if (err > 0)
    { // at least one connection has data (or status change)
      for (i = 0; i < nActive; i++)
      { // test all connections
        dataAvailable = false;
        revents = pollStatus[i].revents;
        cnn = client[clientNum[i]];
        if (((revents & POLLIN) != 0) or
            ((revents & POLLPRI) != 0))
          dataAvailable = true;
        else if (((revents & POLLERR) > 0) or
                 ((revents & POLLHUP) > 0) or
                 ((revents & POLLNVAL) > 0))
        { // error situation
          printf("UServerPort::serviceClients Client %s (%d) has left the building (poll)\n",
                 cnn->getClientName(), clientNum[i]);
          // connection is lost - send no HUP
          cnn->stopConnection(false, true);
        }
        // other conditions are ignored
        //
        if (dataAvailable)
        { // get available data from this client
          result = cnn->receiveData();
          if (verboseMessages and not result)
          {
            printf("UServerPort::serviceClients Client %s (%d) has left the building (recv)\n",
              cnn->getClientName(), clientNum[i]);
            cnn->stopConnection(true, true);
          }
        }
        if (false)
        { // check for time since last message
          float dt = cnn->getTimeSinceLast();
          if (dt > 15.0)
          { // 5 seconds is a long time
            printf("UServerPort::serviceClients client %d is silent in %f seconds - closing\n", clientNum[i], dt);
            cnn->stopConnection(true, true);
          }
        }
      }
    }
  }
  // count loops
  recvLoops++;
  return result;
}

////////////////////////////////////////////////


int UServerPort::getActiveClientCnt()
{
   int result = 0;
   int i;
   //
   for (i = 0; i < clientCnt; i++)
     if (client[i]->isActive())
       result++;
   //
   return result;
}


/////////////////////////////////////////////////

UServerClient * UServerPort::getClient(const int i)
{
  UServerClient * result = NULL;
  //
  if ((i >= 0) and (i < clientCnt))
    result = client[i];
  //
  return result;
}

/////////////////////////////////////////////////


char * UServerPort::getHostName()
{
  int err;
  err = gethostname(hostname, MAX_HOSTNAME_LEN);
  if (err == -1)
    perror("hostname error ");
  else
    printf("Hostname: %s\n", hostname);
  return hostname;
}

/////////////////////////////////////////////

double UServerPort::serverAliveLast()
{
  return serverAlive.getTimePassed();
}


bool UServerPort::sendString(const char * data, USource * client)
{
  bool isOK = false;
  if (data != nullptr and client != nullptr)
  {
    if (client->sourceNum >= 0 and client->sourceNum < MAX_SOCKET_CLIENTS and strncmp(client->sourceID, "socket", 6) == 0)
    {
      UServerClient * clnt = (UServerClient *) client;
      isOK = clnt->isActive();
      if (isOK)
      { // wait up to 10ms to send, else
        // assume dead
        clnt->sendLock.lock();
        clnt->sendStr(data);
        clnt->sendLock.unlock();
      }
      if (not isOK)
      { // client lost - remove priotity
        clnt->stopConnection(true, true);
      }
    }
    else
    {
      client->sendLock.lock();
      client->sendStr(data);
      client->sendLock.unlock();
      isOK = true;
    }
  }
  return isOK;  
}


void UServerPort::printStatus()
{
  printf("Socket server: port: %d, running=%d, clientsCnt=%d, activeCnt=%d, loops=%d\r\n", serverPort, running, clientCnt, clientCntActive, recvLoops);
  for (int i = 0; i < clientCnt; i++)
  {
    UServerClient * c = client[i];
    printf("#  \tclient %d, active=%d, name=%s, rx=%d, tx=%d, err=%d, secs=%.3f\r\n",
           i, c->isActive(), c->getClientName(), c->msgReceived, c->msgSend, c->errCnt, c->connectedTime);
  }
}

void UServerPort::printStatus(USource * cli)
{
  const int m = 200;
  char p1[m];
  snprintf(p1, m, "# client list (running=%d with %d clients where %d is active, poll loops=%d)\r\n", running, clientCnt, clientCntActive, recvLoops);
//   m -= strlen(p1);
//   p1 = &buffer[bufferSize - m];
  cli->sendLock.lock();
  cli->sendStr(p1);
  for (int i = 0; i < clientCnt and m > 0; i++)
  {
    UServerClient * c = client[i];
    if (c->isActive())
    {
      snprintf(p1, m, "#     client %d, active=%d, IP=%s, rx=%d, tx=%d, err=%d, secs=%.3f\r\n",
            i, c->isActive(), c->getClientName(), c->msgReceived, c->msgSend, c->errCnt, c->connectedTime);
      cli->sendStr(p1);
    }
  }
  cli->sendLock.unlock();
}

void UServerPort::stopAllConnections()
{
  for (int i = 0; i < clientCnt; i++)
  {
    UServerClient * c = client[i];
    if (c->isActive())
    {
      c->stopConnection(true, true);
    }
  }
}

