/*
 * Control of data transfer
 * from socket client 
 * Services data from regbot and joystick - and possibly oled display
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

#include <mutex>
#include <string.h>
#include <streambuf>

#include "udata.h"
#include "udataitem.h"
#include "ubridge.h"

#define REV_ID "$Id: command.cpp 791 2018-01-01 19:44:32Z jcan $" 

UData dataa;


void UData::setup(char filepath[])
{
  portServer = &server;
  itemsCnt = 0;
  for (int i = 0; i < MAX_MESSAGE_COUNT; i++)
  {
    items[i] = NULL;
  }
  // create directory for logfiles
  tStart.now();
  const int MSL = MAX_FILENAME_SIZE + 20;
  char s[MSL];
  tStart.getForFilename(s);
  snprintf(logPath, MAX_FILENAME_SIZE, "%s/log_%s", filepath, s);
  // remove old empty dirs
  system ("rmdir log_20* 2>/dev/null\n");
  // make a new dir for this session
  snprintf(s, MSL, "mkdir -p %s\n", logPath);
  system(s);
}

UData::~UData()
{
  for (int i = 0; i < MAX_MESSAGE_COUNT; i++)
  {
    if (items[i] != NULL)
    {
      delete items[i];
      items[i] = NULL;
    }
  }
}


int UData::findDataItem(const char * item, USource * requestedDataSource, int firstItem)
{
  int idx = -1;
  findDataItemLock.lock();
//   bool aaa = false;
//   if (requestedDataSource != nullptr)
//   {
//     if (requestedDataSource->sourceNum == -2)
//     {
//       printf("# UData::findDataItem: from %s item %s from %d\n", requestedDataSource->sourceID, item, firstItem);
//       aaa = true;
//     }
//   }
  for (int i = firstItem; i < itemsCnt; i++)
  {
    UDataItem * d = items[i];
//     if (aaa)
//       printf("# UData::findDataItem: item %d\n", i);
    if (strcmp(item, d->itemKey) == 0)
    { // same key length - first fast check
      if (requestedDataSource == nullptr)
      { // any source (stop at first match)
        idx = i;
        break;
      }
      else if (d->source == requestedDataSource)
      { // data source is important
        idx = i;
        break;
      }
    }
  }
  findDataItemLock.unlock();
  return idx;
}


void UData::updateItem(const char * key, const char * params, USource * source, USource * explicitSource, bool anySource)
{ // find database item from this source
  int itemIndex;
  USource * src;
  if (explicitSource == nullptr)
  {
    if (anySource)
      src = nullptr;
    else
      // data from this source
      src = source;
  }
  else
  { // explicit source
    src = explicitSource;
  }
  itemIndex = findDataItem(key, src, 0);
  UDataItem * d = NULL;
  if (itemIndex < 0 and strlen(params) > 0)
  {
    if (itemsCnt < MAX_MESSAGE_COUNT)
    { // create a new data subject
      if (strlen(params) > 1)
      { // there is data for this key, so create new data item
        if (explicitSource and isalpha(params[0]))
          d = new UDataItem(key, explicitSource, portServer, logPath, handler);
        else
          d = new UDataItem(key, source, portServer, logPath, handler);
        items[itemsCnt++] = d;
        // handle as normal update/meta command
        d->handleData(params, source);
      }
    }
    else
      printf("UData::updateItem: no more space for new data items\n");
  }
  while (itemIndex >= 0)
  { // update/handle existing data
    d = items[itemIndex];
//     if (strcmp(key, "dname") == 0)
//       printf("# UData::updateItem: dname, item=%d, item*=%lx\n", itemIndex, (unsigned long)d);
    d->handleData(params, source);
    if (not anySource)
      break;
    itemIndex = findDataItem(key, src, itemIndex + 1);
  }
}


void UData::setItem(int index, std::string itemName, const char * itemKey)
{
  if (index >= 0 and index < MAX_MESSAGE_COUNT)
  {
    items[index]->itemDescription = itemName;
    strncpy(items[index]->itemKey, itemKey, MAX_ITEM_KEY_LENGTH);
    items[index]->itemKey[MAX_ITEM_KEY_LENGTH] = '\0';
  }
}


void UData::printStatus()
{
  printf("Bridge got %d items:\n", this->itemsCnt);
  for (int i = 0; i< itemsCnt; i++)
  {
    UDataItem * d = items[i];
    printf("   item %d is %10s:%-6s \tupds=%3d, dt=%.3fs, valid=%d, time=%.3fs, log=%d : %s\r\n",
           i, 
           d->source->sourceID, 
           d->itemKey, 
           d->updateCnt, 
           d->updateInterval, 
           d->itemValid,  
           d->itemTime - tStart, 
           d->isOpen(), 
           d->itemParams.c_str());
    printf("                             \t%s\r\n", 
           d->itemDescription.c_str());
    d->printStatus(true);
  }
}

void UData::sendStatus(USource * client, bool verbose)
{
//   printf("# UData::sendStatus 1\n");
  const int MSL = 300;
  char s[MSL];
  snprintf(s, MSL,"# Bridge got %d items:\n", itemsCnt);
  client->sendLock.lock();
  client->sendStr(s);
  for (int i = 0; i< itemsCnt; i++)
  {
    UDataItem * d = items[i];
    snprintf(s, MSL, "#   item %2d is %s:%-6s \tupds=%4d, time=%8.3fs, log=%d,'%s'\r\n",
           i, 
           d->source->sourceID, 
           d->itemKey, 
           d->updateCnt, 
//            d->updateInterval, 
//            d->itemValid,  
           d->itemTime - tStart, 
           d->isOpen(), 
           d->itemParams.c_str());
    client->sendStr(s);
    if (verbose)
    {
      snprintf(s, MSL, "#                        \t%s\r\n", d->itemDescription.c_str());
      client->sendStr(s);
    }
  }
  client->sendLock.unlock();
}

void UData::sendAll(USource * client)
{ // send all items to client
//   printf("# UData::sendStatus 1\n");
  const int MSL = 300;
  char s[MSL];
  client->sendLock.lock();
  client->sendStr("%% Item list: nr, updates, log open, source:keyword [params]* [description]\r\n");
  for (int i = 0; i< itemsCnt; i++)
  {
    UDataItem * d = items[i];
    snprintf(s, MSL, "item %d %6d %d %s:%s %s '%s'\r\n", i, d->updateCnt, d->isOpen(),
           d->source->sourceID, d->itemKey, d->itemParams.c_str(), d->itemDescription.c_str());
    client->sendStr(s);
  }
  client->sendLock.unlock();
}

void UData::stopSubscription(int client)
{
  for (int i = 0; i< itemsCnt; i++)
  {
    UDataItem * d = items[i];
    d->stopSubscription(client);
  }
}


UDataItem * UData::findData(const char * dataSource, const char* item)
{
  UDataItem * d = nullptr;
  findDataItemLock.lock();
  for (int i = 0; i < itemsCnt; i++)
  {
    UDataItem * b = items[i];
    if (strcmp(item, b->itemKey) == 0)
    { // same key 
      if (dataSource == nullptr)
      { // any source (stop at first match)
        d = b;
        break;
      }
      else if (strcmp(b->source->sourceID, dataSource) == 0)
      { // data source is important
        d = b;
        break;
      }
    }
  }
  findDataItemLock.unlock();
  return d;
}
