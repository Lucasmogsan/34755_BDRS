/*
 * Handling of data items - i.e. messages
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

#include "userverport.h"
#include "userverclient.h"
#include "udataitem.h"
#include "ulogfile.h"
#include "ubridge.h"
#include "uhandler.h"
#include "uteensy.h"
#include "usubscribe.h"

UDataItem::UDataItem(const char * key, USource * from, UServerPort * servPtr, const char * createdLogPath, UHandler * responceHandler)
{
  itemValid = false;
  source = from;
  updateCnt = 0;
  serv = servPtr;
  handler = responceHandler;
  strncpy(itemKey, key, MAX_ITEM_KEY_LENGTH);
  updateInterval = 0.0; // in seconds
  itemDescription = "(no description)";
  dataPath = createdLogPath;
  itemTime.now();
}
  
UDataItem::~UDataItem()
{
  for (int i = 0; i < (int)subs.size(); i++)
  {
    delete subs[i];
  }
}

///////////////////////////////////////////////////////////////////////////

void UDataItem::handleData(const char * params, USource * msgSource)
{
  // check first parameter for sub-command
//   if (strcmp(itemKey, "dname") == 0)
//     printf("# UDataItem::handleData: dname, params=%s\n", params);
  if (params == nullptr or strlen(params) == 0)
  { // no params, so just request for data - send it right away
    lock.lock();
    sendTo(msgSource);
    lock.unlock();
  }
  else if (isalpha(params[0]) and reservedKeyword(params))
  { // there is more - this is a management messages
    if (strncmp(params, "subscribe ", 10) == 0)
    { // subscribe message get priority
      const char * p1 = &params[10];
      USource * toWhere = msgSource;
      int p = strtol(p1, (char**)&p1, 10);
      while (*p1 != '\0' and *p1 <= ' ')
        p1++;
//       printf("# UDataItem::handleData params='%s', dest name= '%s'\n", params, p1);
      if (isalpha(*p1))
      { // find the named source
        toWhere = bridge.findSource(p1);
        if (toWhere == nullptr)
        {
          toWhere = msgSource;
//           printf("# UDataItem::handleData: message destination %s not found as a data source\n", p1);
        }
      }
      if (strncmp(toWhere->sourceID, "ini", 3) != 0)
        // the configuration file (called 'ini') will never subscribe to itself, so ignore
        setMinimumInterval(toWhere, p); //, respond->getRegbot());
    }
    else if (strncmp(params, "status", 6) == 0)
    { // send item status to requester
      lock.lock();
      sendStatus(msgSource);
      lock.unlock();
    }
    else if (strncmp(params, "name ", 5) == 0)
    { // set item name
      const char * p1 = &params[5];
      // copy to name/description
      itemDescription = p1;
    }
    else if (strncmp(params, "meta", 4) == 0 /*and msgSource->sourceNum >= 0*/)
    { // not for robot or joy
      lock.lock();
      sendMetaTo(msgSource);
      lock.unlock();
    }
    else if (strncmp(params, "get", 3) == 0 /*and msgSource->sourceNum >= 0*/)
    { // not for robot or joy
      lock.lock();
      sendTo(msgSource);
      lock.unlock();
    }
    else if (strncmp(params, "onupdate ", 9) == 0)
    { // not for robot or joy
      const char * p1 = &params[10];
      int p = strtol(p1, (char **)&p1, 10);
      while (isspace(*p1))
        p1++;
      if (*p1 > ' ')
        setOnUpdate(msgSource, p, p1);
      else
        printf("UDataItem::updateItem - failed onupdate command '%s'\n", p1);
    }
    else if (strncmp(params, "logopen", 7) == 0)
    { //
      const int MSL = 200;
      char s[MSL];
      snprintf(s, MSL, "%s_%s", source->sourceID, itemKey);
      lock.lock();
      setLogName(s);
      openLog(true);
      // debug
      fprintf(stderr, "# log for %s is open=%d to %s\n", itemKey, isOpen(), getLogFileName());
      // debug end
      if (isOpen())
      { // add description as first line in log (with matlab comment)
        fprintf(getF(), "%% logfile for item %s:%s\n", source->sourceID, itemKey);
        fprintf(getF(), "%% %s\n", itemDescription.c_str());
        printf("UDataItem::updateItem - opened logfile %s\n", getLogFileName());
      }
      snprintf(s, MSL, "# opened logfile for %s:%s as %s\r\n", source->sourceID, itemKey, getLogFileName());
      lock.unlock();
    }
    else if (strncmp(params, "logclose", 8) == 0)
    { // not for robot or joy
      // debug
      printf("# log for %s is closing\n", itemKey);
      // debug end
      lock.lock();
      if (isOpen())
      {
        closeLog();
        printf("UDataItem::updateItem - closed logfile %s\n", getLogFileName());
      }
      lock.unlock();
    }
    else if (strncmp(params, "help", 4) == 0)
    { // not for robot or joy
      serv->sendString("# help for special second parameter:\r\n", msgSource);
      serv->sendString("#   get        \tGets the value of the data item\r\n", msgSource);
      serv->sendString("#   meta       \tGets 'key meta r name vs s name p description': r=responder, vs: 0=val 1=seq, s=source, p=priority\r\n", msgSource);
      serv->sendString("#   subscribe p \tSet subscription 0=none, -1=all updates, else interval in ms\r\n", msgSource);
      serv->sendString("#   onupdate p c \tSet onUpdate action 0=none, -1=all updates, else interval in ms; c=action string\r\n", msgSource);
      serv->sendString("#   status     \tSends status 'key status c T n p p p p ...' c: update count, T: since last (sec), n=clients slots, p client priority\r\n", msgSource);
      serv->sendString("#   name xxx   \tSets name or description for data item\r\n", msgSource);
      serv->sendString("#   logopen    \tOpens a (new) logfile and log all updates with timestamp (key.txt)\r\n", msgSource);
      serv->sendString("#   logclose   \tCloses logfile (if open)\r\n", msgSource);
      serv->sendString("#   help       \tThis help\r\n", msgSource);
      //
      // debug
      printf("# UDataItem::updateItem: send item help to %s\n", msgSource->sourceID);
      // debug end
    }
  }
  else
  { // normal data message - update fields
//     if (strcmp(itemKey, "dname") == 0)
//       printf("# UDataItem::handleData: else , params=%s\n", params);
    // key and source is set already
    lock.lock();
    //    printf("UDataItem::updateItem: not handled debug 2\n");
    float dt = itemTime.getTimePassed();
    // average update time a bit - or maybe not
    updateInterval = dt; // updateInterval * 3/4 + dt/4;
    itemTime.now();
    itemParams = params;
    // remove linefeed and cr
    int n = itemParams.size() - 1;
    while (itemParams[n] < ' ' and n > 0)
      n--;
    itemParams.resize(n+1);
    itemValid = true;
    if (isOpen())
    {
      toLog(itemTime, params);
    }
    lock.unlock();
    // run a tick to get as
    // fresh data as possible
    // to clients
    updateCnt++;
    tick(itemTime);
  }
}
  
  
  
void UDataItem::sendStatus(USource * client)
{
  const int MSL = 300;
  char s[MSL];
  snprintf(s, MSL, "# status for data item %s:%s is %d upds %.3fs, %u subscribers\r\n", source->sourceID, itemKey, updateCnt, updateInterval, subs.size());
  serv->sendString(s, client);
  for (int i = 0; i < (int)subs.size(); i++)
  {
    subs[i]->sendStatus(client);
  }
}

void UDataItem::printStatus(bool justSubs)
{
  if (not justSubs)
    printf("# status for data item %s:%s is %d upds %.3fs, %u subscribers\r\n", source->sourceID, itemKey, updateCnt, updateInterval, subs.size());
  for (int i = 0; i < (int)subs.size(); i++)
  {
    subs[i]->printStatus();
  }
}


/**
  * Set new subscribe priority from this client
  * \param client is client number from server
  * \param intervalMs is interval in ms, 0=stop, negative is all.
  * */
void UDataItem::setMinimumInterval(USource * client, int intervalMs/*, UTeensy * rob*/)
{
  //printf("# DataItem::setMinimumInterval 0\n");
  USubscribe * cp = findSubscriber(client);
  if (cp == nullptr)
  { // create new subscription
    //printf("# DataItem::setMinimumInterval new subscription\n");
    cp = new USubscribe();
    cp->cli = client;
    subs.push_back(cp);
  }
  if (cp != nullptr)
  {
    if (intervalMs < 0)
    {
      cp->all = true;
      cp->interval = 1;
    }
    else
    {
      cp->interval = intervalMs * 0.001;
      cp->all = false;
    }
    //printf("# DataItem::setMinimumInterval interval=%fs, all=%d\n", cp->interval, cp->all);
  }
}


void UDataItem::setOnUpdate(USource* client, int intervalMs, const char* action)
{
  USubscribe * cp = nullptr;
  for (int i = 0; i < (int)subs.size(); i++)
  { // look for existing action
    if (strcmp(subs[i]->onUpdate.c_str(), action) == 0)
    {
      printf("UDataItem::setOnUpdate: found existing on-update subscription (%d)\n", i);
      cp = subs[i];
      break;
    }
  }
  if (cp == nullptr)
  { // no existing onAction subscribtion found
    if (intervalMs != 0)
    { // not a delete, so create
      cp = new USubscribe();
      cp->cli = client;
      cp->onUpdate = action;
      subs.push_back(cp);
      printf("UDataItem::setOnUpdate: created new on-update subscription\n");
    }
  }
  if (cp != nullptr)
  { // client is actually the source of the last onUpdate command
    cp->cli = client;
    if (intervalMs < 0)
    {
      cp->all = true;
      cp->interval = 1;
    }
    else
    {
      cp->interval = intervalMs * 0.001;
      cp->all = false;
    }
  }        
}


/**
  * Check time to send this message */
void UDataItem::tick(UTime now)
{
  lock.lock();
  for (int i = 0; i < (int)subs.size(); i++)
  {
    if (subs[i]->interval > 0.0001)
    {
      const char * todo = nullptr;
      bool timeToSend = subs[i]->tick(&todo);
      if (timeToSend)
      {
        if (todo == nullptr)
          sendTo(subs[i]->cli);
        else
        { // this is an onUpdate event
          // so issue the command
          handler->handleCommand(subs[i]->cli, todo, true);
        }
      }
    }
  }
  lock.unlock();
}

/**
  * Send message to client
  * */
void UDataItem::sendTo(USource * client)
{
  if (itemValid)
  {
    const int MSL = 300;
    char s[MSL];
    
    snprintf(s, MSL, "%s:%s %s\r\n", source->sourceID, itemKey, itemParams.c_str());
    bool isOK = serv->sendString(s, client);
    if (not isOK)
      stopClientSubscription(client);
  }
  else
  {
    const int MSL = 60;
    char s[MSL];
    snprintf(s, MSL, "# data item with key='%s:%s' has no data\r\n", source->sourceID, itemKey);
    serv->sendString(s, client);
  }
}


void UDataItem::sendMetaTo(USource * client)
{
  bool isOK = false;
  const int MSL = 3000;
  char s[MSL];
  snprintf(s, MSL, "meta %s %s %d %s %d %s\r\n", source->sourceID, itemKey, source->sourceNum, itemDescription.c_str(), isOpen(), getLogFileName());
  isOK = serv->sendString(s, client);
  if (not isOK)
    stopClientSubscription(client);
}


USubscribe * UDataItem::findSubscriber(USource* client)
{
  for (int i = 0; i < (int)subs.size(); i++)
  {
    if (subs[i]->cli == client)
      return subs[i];
  }
  return nullptr;
}

void UDataItem::stopClientSubscription(USource* client)
{
  printf("# UDataItem::stopClientSubscription\n");
  for (int i = 0; i < (int)subs.size(); i++)
  {
    if (subs[i]->cli == client)
    {
      delete subs[i];
      subs.erase(subs.begin()+i);
    }
  }
}


bool UDataItem::reservedKeyword(const char* params)
{
  bool isKeyword = false;
  const char * p = " subscribe onupdate status name get meta logopen logclose logpath help ";
  char s[32];
  const char * p1 = params;
  const char * p2 = strchrnul(p1, ' ');
  // remove potential \r\n
  while (*p2 <= ' ' and p2 > p1)
    p2--;
  // get length of keyword
  int n = p2 - p1 + 1;
  if (n > 0 and n < 32)
  { // add space before and after
    s[0] = ' ';
    strncpy(&s[1], p1, n);
    s[n+1] = ' ';
    s[n+2] = '\0';
    p1 = strstr(p, s);
    if (p1 != nullptr)
      isKeyword = true;
    // debug
//     printf("# UDataItem::reservedKeyword: is=%d, '%s' (from '%s') in %s\n",
//            isKeyword, s, params, p);
    // debug end
  }
  return isKeyword;
}

bool UDataItem::match(const char* item, USource* itemSource)
{
  bool isMatch = source == itemSource;
  if (isMatch)
    isMatch = strcmp(item, itemKey) == 0;
  return isMatch;
}

void UDataItem::stopSubscription(int client)
{
//   if (client == 0)
//     printf("# UDataItem::stopSubscription client=0 (debug)\n");
  for (int i = 0; i < (int)subs.size(); i++)
  {
    USubscribe * c = subs[i];
    if (c->cli->sourceNum == client)
    {
      c->interval = 0;
      c->all = false;
    }
  }
}
