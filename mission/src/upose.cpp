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
#include "upose.h"
#include "ubridge.h"

// create value
UPose pose;


// Bridge class:
void UPose::setup()
{ /// subscribe to pose information
  bridge.tx("regbot:pose subscribe -1\n");
}


bool UPose::decode(char* msg)
{
  bool used = true;
  const char * p1 = strchrnul(msg, ':');
  if (strncmp(p1, ":pose ", 6) == 0)
  { // decode pose message
    // advance to first parameter
    if (strlen(p1) > 6)
      p1 += 6;
    else
      return false;
    // get data
    dataLock.lock();
    // time in seconds
    t = strtof64(p1, (char**)&p1);
    x = strtof(p1, (char**)&p1); // x
    y = strtof(p1, (char**)&p1); // y
    h = strtof(p1, (char**)&p1); // heading (rad)
    tilt = strtof(p1, (char**)&p1); // tilt in radians around robot y-axis
    dataLock.unlock();
  }
  else
    used = false;
  
  
  return used;
}


