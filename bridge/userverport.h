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
#ifndef USERVERPORT_H
#define USERVERPORT_H

#include <pthread.h>
#include <arpa/inet.h>

#include "userverclient.h"
#include "urun.h"

/**
Number of clients beeing served at one time */
#define MAX_SOCKET_CLIENTS    5


/**
Max length of attributes in name-space message */
#define MAX_NAMESPACE_ATTRIBUTE_LENGTH 200

class UHandler;

/**
Class used by a socket server to control and accept connections to one port number.
The port number are determined during initialization.

@author Christian Andersen
*/
class UServerPort : public URun
{
public:
  /**
  Constructor */
//     UServerPort();//, UBridge * bridge);
  /**
  Destructor */
  virtual ~UServerPort();
  /**
   * Send string to client 
   * \param data is the string to be send
   * \param client is index to client list */
  bool sendString(const char * data, USource * client);
  /**
   * Send string to client 
   * \param data is the string to be send
   * \param idx is index to client list */
//   bool sendString(const char * data, int idx)
//   {
//     sendString(data, getClient(idx));
//   }
  /**
   *  Set server port number */
  void setup(int port);
  /**
   *  Get number of  active clients */
  int getActiveClientCnt();
  /**
   * debug print status to console */
  void printStatus();
  /**
   * Print status to string buffer
   * \param cli is client to receive response */
  void printStatus(USource * cli); 
  /**
   *  Get handle to connection - returns NULL if out of range */
  UServerClient * getClient(const int i);
  /**
  Get server port number */
  inline int getPort() { return serverPort; };
  /**
  Get hostname - just a wrap of the normal 'gethostname' call.
  Returns a pointer to the provided string. */
  char * getHostName();
  /**
   * Stop all connections */
  void stopAllConnections();

protected:
  /**
  The thread that handles all port tasks.
  Runs until stop-flag is set, or if port can not be bound.
  Should not be called!, call start() to create the thread
  calling this function. */
  void run();
  /**
  Print server status to console,
  preseded by this string (preStr). */
  void print(const char * preStr);
  /**
  Return number of initialized clients.
  The clients may be active or inactive (closed) */
  inline int getClientCnt() { return clientCnt; };
  /**
  Function to trigger events, when new data is available.
  Should be overwritten by decendent classes. */
//   virtual void messageReceived();
  /**
  This function is called, when a new client is connected */
  virtual void gotNewClient(UServerClient * cnn);
  /**
  Is server open for connections - i.e. port number is valid */
  bool isOpen4Connections()
  { return open4Connections; };
  /**
   * Alive call from server thread */
  void serverIsAlive()
  { serverAlive.now(); };
  /**
   * Get time passed since the server was last reported alive.
   * \returns time in seconds since that server thread last set the alive timestamp. */
  double serverAliveLast();
  /**
   * Set the allow connections flag
   * Existing connections are not closed
   * \param value set to true, if to allow new connections */
  void setAllowConnections(bool value)
  { allowConnections = value; };
  /**
   * Get the allow connections flag
   * if false, then server do not allow (new) connections
   * \param value set to true, if to allow connections */
  bool getAllowConnections()
  { return allowConnections; };
  /**
   * Get client number of latest clinet connected - mostly for debug */
  int getLastClient()
  { return lastClientNumber; };
  /**
   * Should be called whenever a client is disconnected - to allow cleanup of obligations for this client */
  void connectionLost(int client);
  /**
   * Get last used client serial number */
  inline int getLastClientSerial()
  { return lastClientSerial; };
  
  
  
public:
  static const int MAX_HOSTNAME_LEN = 100;
  char hostname[MAX_HOSTNAME_LEN];
  /**
   *  Is the selected port valid, i.e. open for connections */
  bool open4Connections;
  /**
   * server port number */
  int serverPort;
  
protected:
  /**
  Get a free client handle */
  int getFreeClientHandle();
  /**
  Service receive channel of all clients.
  Returns true is fata were processed.
  Returns false if nothing to process, either
  after poll-timeout or just no connections. */
  bool serviceClients(int msTimeout);
  /**
   * Should server allow connections, if not, then close server port - until changed. */
  bool allowConnections;

private:
  /**
  Running is true when accepting-calls thread is running */
  bool running;
  /**
  Structure for the accepted connections */
  UServerClient * client[MAX_SOCKET_CLIENTS];
  /**
  Last used client connection */
  int clientCnt;
  /**
  Number of active clients */
  int clientCntActive;
  /**
  Print more to info connection / console */
  bool verboseMessages;
  /**
  Statistics - receive loops */
  int recvLoops;
  /**
   * The last time the server thread was reported alive */
  UTime serverAlive;
  /**
   * client number to be used used for the last connection (-1 = no clients yet) */
  int lastClientNumber;
  /**
   * last client serial number used */
  int lastClientSerial;
  /**
   * message handler */
//    UHandler * handler;
//   UBridge * bridge;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

extern UServerPort server;

#endif
