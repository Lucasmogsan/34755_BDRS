/*
 * Handling messages to and from clients
 * and to and from regbot
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

#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "uteensy.h"
#include "ujoy.h"
#include "uhandler.h"
#include "udataitem.h"
#include "uoled.h"
#include "ubridge.h"

UHandler handler;

// UHandler::UHandler(UData* data/*, UBridge * brg*/)
// {
//   items = data;
// //   bridge = brg;
// }

// UHandler::~UHandler()
// {
//   printf("Handler destroyed\n");
// }


void UHandler::setup()
{
  items = &dataa;
}


void UHandler::handleCommand(USource * source, const char* msgOrg, bool noCRC)
{ // test for shutting down
  if (stopHandle)
    return;
  //
  const char * msg = msgOrg;
  bool dataOK = noCRC;
  if (not noCRC)
  {
    if (msg[0] == ';')
    { // there is a CRC check code
      if (isdigit(msg[1]) and isdigit(msg[2]))
      {
        const char * p1 = &msg[3];
        int sum = 0;
        int m = strlen(p1);
        for (int i = 0; i < m; i++)
        {
          if (*p1 >= ' ')
            sum += *p1;
          p1++;
        }
        int q1 = (sum % 99) + 1;
        int q2 = (msg[1] - '0') * 10 + msg[2] - '0';
        if (q1 != q2)
          printf("# UHandler::handleCommand: CRC check failed (from %d %s) q1=%d != q2=%d (msg=%s\n", source->sourceNum, source->sourceID, q1, q2, msg);
        msg += 3; // skip CRC
        dataOK = true;
      }
    }
    else
    { // message without CRC - should be avoided
      printf("message without CRC!: '%s'\n", msg);
    }
  }
  if (dataOK)
  {
    regularDataUpdate(source, msg);
  }
  else
  {
    printf("# UHandler::handleCommand ignored (bad CRC) '%s'\r\n", msg);
  }
}
  
void UHandler::regularDataUpdate(USource * source, const char* msg)
{
  // check for explicit source
  // find out if this is about a data item or a destination
  // 1: 'drive gyroi ...'   is data to the device 'drive', if 'drive' is a data source, 'gyroi' is then the message
  // 2: 'drive:gyro ...'    is to request data item 'gyro' from data source 'drive'
  // 2: ':hbt ...'          is to request data item 'hbt' from all data sources with this data
  // 3: 'gyro ppp uuu etc'  is new data item 'gyro' from current source ('gyro' must not be a data source)
  // 5: '#...'              is new free text message from current source or reply on meta data
  // 6: '%...'              is new log data header from current source
  // 7: '9...'              is new numeric log data from current source

  const char * params = msg;
  char keystr[MAX_ITEM_KEY_LENGTH + 1];
  char sourceStr[32] = "";
  bool isOK = false;
  bool anySource = false;
  // isolate key and parameters
  // to use key as index - together with source
  if (isdigit(msg[0]) or msg[0] == '%')
  { // log data from robot ram starts with a number (timestamp)
    // or a '%' as is a comment in matlab
    isOK = true;
    strncpy(keystr, "logdata", MAX_ITEM_KEY_LENGTH);
    params = &msg[0];
  }
  else if (isalpha(msg[0]) or strchr("<:*#", msg[0]) != nullptr)
  { // data key is included that seems valid
    int n = strlen(msg);
    int m = n;
    int a = 0; // explicit source
    for (int i = 0; i < n; i++)
    {
      if (msg[i] <= ' ')
      {
        m = i;
        break;
      }
      else if (not isalnum(msg[i]))
      {
        if (strchr("+-=#", msg[i]) != nullptr)
        { // part of key
          m = i + 1;
          break;
        }
        if (msg[i] == ':')
        { // explicit source found
          a = i + 1;
        }
      }
    }
    if (a > 0)
    {
      anySource = a == 1 or msg[0] == '*';
      if (not anySource)
      {
        strncpy(sourceStr, msg, a-1);
        sourceStr[a-1] = '\0';
      }
    }
    // then get the key
    strncpy(keystr, &msg[a], m - a);
    keystr[m - a] = '\0';
    params = &msg[m];
    while (isspace(*params) and *params != '\0')
    {
      params++;
    }
    int ln = strlen(keystr);
    isOK = ln > 0 and ln < MAX_ITEM_KEY_LENGTH;
  }
  if (not isOK)
  { // Not a valid keyword
    printf("UHandler::regularDataUpdate: key failed: %s, must start with an alpha, # or a number.\n", keystr);
  }
  else
  { 
    bridge.decodeSourceCmds(keystr, params, source);
    USource * explicitSource = nullptr;
    if (strlen(sourceStr) > 0)
    { // key is of type source:key
      explicitSource = bridge.findSource(sourceStr);
    }
    items->updateItem(keystr, params, source, explicitSource, anySource);
  }
}

