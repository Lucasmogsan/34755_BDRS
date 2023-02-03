/*
 * Data storage (message storage)
 * 
 ***************************************************************************
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

#ifndef UDATA_H
#define UDATA_H

#include <string>
#include <mutex>

#include "urun.h"
#include "userverport.h"
#include "userverclient.h"
#include "ulogfile.h"

//#define MAX_PRIORITY 5

// maximum allowed message number
#define MAX_MESSAGE_COUNT 500

//#define MAX_ITEM_KEY_LENGTH 5

class UDataItem;
class UBridge;

class UData : public URun
{
public:
  /**
   * constructor */
//   UData(UServerPort * port, char filepath[]);
//   /**
//    * destructor */
  ~UData();
  
  void setup(char filepath[]);
  /**
   * new message for a data item from this client (source of this message)
   * \param key ID for this message,
   * \param params string with remaining parameters
   * \param source is the source for the message
   * */
  void updateItem(const char * key, const char * params, USource * source, USource * explicitSource, bool anySource);
//   void regularData(USource* source, const char* skey, const char* msg, USource* dataSrc);
  
  void setItem(int index, std::string itemName, const char * itemKey);
  /**
   * print data status to console */
  void printStatus();
  /**
   * send same status to a client */
  void sendStatus(USource * client, bool verbose);
  /**
   * send all data items to a client */
  void sendAll(USource * client);
  /**
   * get number of items in database */
  inline int getItemCnt()
  {
    return itemsCnt;
  }
  /**
   * set pointer to responce handler */
//   void setHandler(UHandler * messageHandler, UBridge * responsHandler)
//   {
//     handler = messageHandler;
//     bridge = responsHandler;
//   }
  /**
   * stop subscription from this client */
  void stopSubscription(int client);

  char logPath[MAX_FILENAME_SIZE];
  /**
   * Find a data item (see also findDataItem() that returns the data index)
   * \param dataSource if not nullptr, then only for this source
   * \param item the item keyword to look for.
   * \returns nullptr if not found */
  UDataItem * findData(const char * dataSource, const char * item);
  
private:
  /**
   * pointer to socket server - to get client connection */
  UServerPort* portServer;
  
  UDataItem * items[MAX_MESSAGE_COUNT];
  int itemsCnt = 0;
  std::mutex findDataItemLock;
  /**
   * Find a data item in the item database (see also findData() that returns a pointer)
   * \param item is the item key
   * \param requestedDataSource is the source for the data item we are looking for, NULL is any source.
   * \param firstItem is the index to start data item (used if any source)
   * \returns index to the item, or -1 if no match */
  int findDataItem(const char * item, USource * requestedDataSource, int firstItem);
  UBridge * bridge;
  UHandler * handler;
  UTime tStart;
};

extern UData dataa;

#endif
