/***************************************************************************
 *   Copyright (C) 2022 by DTU (Christian Andersen)                        *
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
#ifndef USOURCE_H
#define USOURCE_H

#include "mutex"
#include "utime.h"

class UHandler;
class UBridge;


class USource
{
public:
  USource(/*UBridge * bridge*/);
  /**
   * source ID used to explain to message receiver from this client */
  static const int MAX_ID_LENGTH = 32;
  char sourceID[MAX_ID_LENGTH];
  /**
   * source ID used as index to client name as source */
  int sourceNum = -1;
  /**
   * error count */
  int errCnt;
  UTime terr;
  std::mutex sendLock;
  /**
   * handler of messages from this source */
//   inline void setHandler(UHandler * hnd)
//   { 
//     handler = hnd;
//   }
  /**
   * Set source name */
  void setSourceID(const char * id);
  /**
   * is the message for this device */
  virtual bool decode(const char * key, const char * params, USource * client);
  /**
   * send device details */
  virtual void sendDeviceDetails(USource * toClient);
  /**
   * is data source active (is device open) */
  virtual bool isActive()
  {
    return false;
  }
  /**
   * Send this string to the associated connection
   * \param cmd is string to send
   * \param msTimeout is timeout, if string can not be send */
  void sendStr(const char * cmd);
  /**
   * Send this string to the associated connection
   * \param key is keyword for the message
   * \param params is the parameters to send after the keyword
   * \param msTimeout is timeout, if string can not be send */
//   void sendMsg(const char * key, const char * params);

protected:
  /**
   * pointer to handler of all messages */
//   UHandler* handler = nullptr;
  /**
   * timeout in ms for tx */
  int timeoutMs = 50;

private:
  /**
   * Send this string to the associated connection
   * \param cmd is string to send
   * \param msTimeout is timeout, if string can not be send */
  virtual void sendString(const char * cmd, int msTimeout);
};
#endif
