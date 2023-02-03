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
#include "ujoy.h"
#include "ubridge.h"

// create value
UJoy joy;


// Bridge class:
void UJoy::setup()
{ /// subscribe to pose information
  bridge.tx("regbot:joy subscribe -1\n");
}


bool UJoy::decode(char* msg)
{
  /*
   *  snprintf(s, MSL, "joy %d %d %d %d  %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d %d\r\n", 
   *          available, manOverride, number_of_axes, number_of_buttons,
   *          (psAxis[0] - 127) << 7, 
   *          -(psAxis[1] - 127) << 7, 
   *          (psAxis[3] - 127) << 7, 
   *          (psAxis[2] - 127) << 7, 
   *          -(psAxis[5] - 127) << 7, 
   *          (psAxis[4] - 127) << 7, 
   *          (psAxis[6] - 127) << 7, 
   *          (psAxis[7] - 127) << 7,
   *          (buttons & (1 << 2)) > 0,
   *          (buttons & (1 << 1)) > 0,
   *          (buttons & (1 << 3)) > 0,
   *          (buttons & (1 << 0)) > 0,
   *          (buttons & (1 << 4)) > 0,
   *          (buttons & (1 << 5)) > 0,
   *          (buttons & (1 << 6)) > 0,
   *          (buttons & (1 << 9)) > 0,
   *          (buttons & (1 << 8)) > 0,
   *          (buttons & (1 << 10)) > 0,
   *          (buttons & (1 << 7)) > 0
   *  );
   * */
  bool used = true;
  const char * p1 = strchrnul(msg, ':');
  if (strncmp(p1, ":joy ", 5) == 0)
  { // Decode gamepad joysticks and buttons
    // advance to first parameter
    if (strlen(p1) > 5)
      p1 += 5;
    else
      return false;
    // get data
    dataLock.lock();
    // time in seconds
    available = strtol(p1, (char **)&p1, 10);
    joystickControl = strtol(p1, (char **)&p1, 10);
    axisCnt = strtol(p1, (char **)&p1, 10);
    buttonCnt = strtol(p1, (char **)&p1, 10);
    if (buttonCnt > MAX_BUTTON_CNT)
      buttonCnt = MAX_BUTTON_CNT;
    if (axisCnt > MAX_AXIS_CNT)
      axisCnt = MAX_AXIS_CNT;
    for (unsigned int a = 0; a < axisCnt; a++)
    { // read all axis values (+/- 32000 values)
      axiss[a] = strtol(p1, (char**)&p1, 10);
    }
    for (unsigned int b = 0; b < axisCnt; b++)
    { // read all axis values (+/- 32000 values)
      buttons[b] = strtol(p1, (char**)&p1, 10);
    }
    dataLock.unlock();
  }
  else
    used = false;
  
  
  return used;
}

bool UJoy::waitForButton(int n)
{
  bool result = true;
  if (n > MAX_BUTTON_CNT or n < 1)
    n = 1;
  while (not buttons[n-1])
  { // wait 5ms
    usleep(5000);
  }
  return result;
}

bool UJoy::axis(int n)
{
  if (n > 0 and n <= (int)axisCnt)
    return axiss[n - 1];
  else
    return 0;
}

bool UJoy::button(int n)
{
  if (n > 0 and n <= (int)buttonCnt)
    return buttons[n - 1];
  else
    return 0;
}

