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
 
#include "usubscribe.h"
#include "usource.h"
#include "utime.h"
#include "stdio.h"

void USubscribe::sendStatus(USource * client)
{
  const int MSL = 200;
  char s[MSL];
  snprintf(s, MSL, "#                   subscriber %d %s interval %.3f, all %d\r\n", cli->sourceNum, cli->sourceID, interval, all);
  client->sendLock.lock();
  client->sendStr(s);
  client->sendLock.unlock();
}

void USubscribe::printStatus()
{
  printf("                                  subscriber %d %s interval %.3f, all %d\r\n", cli->sourceNum, cli->sourceID, interval, all);
}

bool USubscribe::tick(const char ** todo)
{
  bool isTime;
  if (all)
    isTime = true;
  else
  {
    float dt = itemSend.getTimePassed();
  //   printf("# USubscribe::tick\n");
    isTime = dt > interval;
  }
  if (isTime)
  { // send or action taken time
    itemSend.now();
  }
  if (onUpdate.size() > 0)
  {
    printf("# USubscribe::tick: an onUpdate tick: '%s'\n", onUpdate.c_str());
    *todo = onUpdate.c_str();
  }
  return isTime;
}
