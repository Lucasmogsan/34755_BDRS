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
#include "ubridge.h"
#include "ustate.h"

// create the class with received info
UState state;


// Bridge class:
void UState::setup()
{ /// subscribe to pose information
  bridge.tx("regbot:hbt subscribe -1\n");
}


bool UState::decode(char* msg)
{ // like: regbot:hbt 37708.7329 74 1430 5.01 0 6
  bool used = true;
  const char * p1 = strchrnul(msg, ':');
  if (strncmp(p1, ":hbt ", 5) == 0)
  { // decode pose message
    // advance to first parameter
    if (strlen(p1) > 5)
      p1 += 5;
    else
      return false;
    // get data
    dataLock.lock();
    // time in seconds
    t = strtof64(p1, (char**)&p1);
    idx = strtol(p1, (char**)&p1, 10); // index (serial)
    version = strtol(p1, (char**)&p1, 10); // index (serial)
    batteryVoltage = strtof(p1, (char**)&p1); // y
    controlState = strtol(p1, (char**)&p1, 10); // control state 0=no control, 2=user mission
    type = strtol(p1, (char**)&p1, 10); // control state 0=no control, 2=user mission
    dataLock.unlock();
  }
  else
    used = false;
  return used;
}


