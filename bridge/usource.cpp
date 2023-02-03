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

#include <iostream>
#include "usource.h"
#include "uhandler.h"
#include "ubridge.h"

USource::USource()
{
//   bridge.addSource(this);
  terr.now();
  errCnt = 0;
//   printf("# added source\n");
}


void USource::setSourceID(const char* id)
{
  strncpy(sourceID, id, MAX_ID_LENGTH);
  bridge.addSource(this);
}

void USource::sendString(const char* cmd, int msTimeout)
{ // this is the default, all is send to the console
  std::cout << cmd;
}

bool USource::decode(const char* key, const char* params, USource* client)
{
//   std::cout << "# this source has no decode function " << sourceID << "\n";
  return false;
}

// void USource::sendMsg(const char * key, const char * params)
// { // construct string and send to robot
//   const int MSL = 500;
//   char s[MSL];
//   int n = strlen(key) + strlen(params) + 5;
//   if (n > MSL)
//   {
//     printf("UTeensy::send: msg to robot too long (%d>%d) truncated\r\n", n, MSL);
//     n = MSL-1;
//     s[n] = '\0';
//   }
//   if (strlen(params) == 0)
//   {
//     snprintf(s, n, "%s\r\n", key);
//   }
//   else
//   {
//     snprintf(s, n, "%s %s\r\n", key, params);
//   }
//   sendString(s, timeoutMs);
// }

void USource::sendStr(const char* cmd)
{
//   txLock.lock();
  sendString(cmd, timeoutMs);
//   txLock.unlock();
}

void USource::sendDeviceDetails(USource* toClient)
{
  const int MSL = 100;
  char s[MSL];
  snprintf(s, MSL, "source %d %s:dev %d has no data yet\n", sourceNum, sourceID, isActive());
  toClient->sendStr(s);
}
