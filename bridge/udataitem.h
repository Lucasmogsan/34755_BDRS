/*
 * Handling of data items - i.e. messages
 * 
 ***************************************************************************
 *   Copyright (C) 2017-2022 by DTU (Christian Andersen)                        *
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

#ifndef UDATAITEM_H
#define UDATAITEM_H

#include <mutex>
#include <vector>

#include "userverport.h"
#include "usubscribe.h"
#include "ulogfile.h"

#define MAX_PRIORITY 6
#define MAX_ITEM_KEY_LENGTH 16
// extra client is command line
#define EXTRA_CLIENTS       1
#define MAX_CLIENTS         (MAX_SOCKET_CLIENTS + EXTRA_CLIENTS)

class UBridge;
class UHandler;
class UTeensy;

class UDataItem : public ULogFile
{
public:
  /** constructor */
  UDataItem(const char * key, USource * from, UServerPort * servPtr, const char * createdLogPath, UHandler * responceHandler);
  /** destructor */
  ~UDataItem();
  // descriptive name for this data item
  std::string itemDescription;
  // keyword (tag) for this message,
  // and also make space for string terminator  
  char itemKey[MAX_ITEM_KEY_LENGTH + 1];
  // value string
  std::string itemParams;
  // source update time for item
  UTime itemTime;
  // has data ever been updated
  bool itemValid;
  /**
   * simple update count */
  int updateCnt;
  /**
   * Update interval, time since last update (in seconds) */
  float updateInterval;
  // source of last update
  USource * source = nullptr;
  
  
protected:
  /**
   * pointer so socket server */
  UServerPort * serv;
  // avoid sending and updating at the same time
  std::mutex lock;
  // Relay to client default priority
  int clientDefPriority;
  // logfile
//   ULogFile * logfile;
  // pointer to log-path stored elsewhere
//   const char * logPath;
  // handler if an update involves handling by this bridge itself
  UHandler * handler;
  // list of subscribers to this item
  std::vector<USubscribe*> subs;   
  
  
public:
  /**
   * Setting new source data 
   * \param newData is the data string
   * \param cmdSource is the source of the message
   */
//   void updateItem(const char * key, const char * params, USource * cmdSource);
//   /**
//    * getting stored data or modify subscription or log
//    * \param params are potential commands for this data item
//    * \param msgSource is the source of the message
//    * \param newItem if true, then also test for meta commands - relevant for data source -2 only (Teensy)
//    */
  void handleData(const char * params, USource * msgSource);
  /**
   * returns true, if a reserved keyword for data management is found (e.g. openLog */
  static bool reservedKeyword(const char * msg);
  /**
   * Set new priority from this client
   * \param client is client number from server
   * \param interval set update interval 0=stop, -1=all, 1.. is shortest interval in ms.
   * */
  void setMinimumInterval(USource * client, int intervalMs); //, UTeensy * rob);
  /**
   * Set new priority from this client
   * \param client is client number from server
   * \param interval set update interval 0=stop, -1=all, 1.. is shortest interval in ms.
   * */
  void setOnUpdate(USource * client, int intervalMs, const char * action); //, UTeensy * rob);
  /**
   * Check time to send this message */
  void tick(UTime now);
  /**
   * Get data item status */
  bool match(const char * item, USource * itemSource);
  /**
   * print status to console */
  void printStatus(bool justSubs);
  /**
   * Stop any subscriptions from this client */
  void stopSubscription(int client);
protected:
  /**
   * get reference to client */
  USubscribe * findSubscriber(USource * client);
  /**
   * Send message to client
   * */
  void sendTo(USource * client);
  /**
   * Send meta information of data item */
  void sendMetaTo(USource * clientIdx);
  
  private:
  /** send item status to this client
   * \param client is index to client to get the status */
  void sendStatus(USource * client);
  /**
   * stop any subscriptions from this client (client lost) */
  void stopClientSubscription(USource * client);
  
};

#endif
