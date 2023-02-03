/*  
 * 
 * Copyright © 2022 DTU, 
 * Author:
 * Christian Andersen jcan@dtu.dk
 * 
 * The MIT License (MIT)  https://mit-license.org/
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software 
 * is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies 
 * or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE. */

#include <string>
#include <string.h>
#include "uevent.h"
#include "ubridge.h"
#include "ustate.h"

// create value
UEvent event;


// Bridge class:
void UEvent::setup()
{ /// subscribe to pose information
  bridge.tx("regbot:event subscribe -1\n");
}


bool UEvent::decode(char* msg)
{
  bool used = true;
  const char * p1 = strchrnul(msg, ':');
  if (strncmp(p1, ":event ", 7) == 0)
  { // decode pose message
    // advance to first parameter
    if (strlen(p1) > 7)
      p1 += 7;
    else
      return false;
    // decode data
    dataLock.lock();
    // time in seconds
    int e = strtol(p1, (char**)&p1, 10);
    if (e >= 0 and e < MAX_EVENT)
    {
      if (e == 33)
        // start mission
        clearEvents();
      events[e] = true;
    }
    dataLock.unlock();
  }
  else
    used = false;
  
  
  return used;
}


void UEvent::clearEvents()
{
  for (int i = 0; i < MAX_EVENT; i++)
    events[i] = false;
}

bool UEvent::gotEvent(int i)
{
  bool result = false;
  if (bridge.terminate)
  { // return true if we are termination this app (ctrl-c)
    result = true;
  }
  else
  { // we are not shutting down, 
    if (i >= 0 and i < MAX_EVENT)
      result = events[i];
  }
  return result;
}

bool UEvent::waitForEvent(int n)
{
  int m = 0;
  bool result = true;
  while (not gotEvent(n))
  { // wait 50ms
    usleep(50000);
    m++;
    if (state.controlState == 0 and m > 20)
    { // mission is not started (waited a second for heartbeat status)
      // so an event will never happen
      // so stop waiting
      printf("# No mission is started! - stopped waiting for event.\n");
      result = false;
      break;
    }
  }
  return result;
}

