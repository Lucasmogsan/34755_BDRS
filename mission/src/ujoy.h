/*  
 * 
 * Copyright © 2022-2023 DTU, Christian Andersen jcan@dtu.dk
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


#ifndef UJOY_H
#define UJOY_H

#include <iostream>
#include <sys/time.h>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <thread>
#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

using namespace std;
// forward declaration

/**
 * Class for gamepad status - buttons and axis */
class UJoy{
  
public:
  /** setup and request data */
  void setup();
  /** decode an unpacked incoming messages
   * \returns true if the message us used */
  bool decode(char * msg);
  /**
   * Wait in a lopp for this button to be 1.
   * \param n is button number. First button is called button 1 
   * \returns true (or wait forever) */
  bool waitForButton(int n);
  /**
   * Is button pressed
   * \param n is button number. First button is called button 1 
   * \returns true if pressed */
  bool button(int n);
  /**
   * Axis value
   * \param n is axis number. First axis is called button 1 
   * \returns value of axis */
  bool axis(int n);
  
private:
  // data storage for
  static const int MAX_BUTTON_CNT = 20;
  static const int MAX_AXIS_CNT = 20;
  bool buttons[MAX_BUTTON_CNT] = {false};
  int axiss[MAX_AXIS_CNT] = {0};
  /// number of actual buttons and axis
  unsigned int buttonCnt = 0;
  unsigned int axisCnt = 0;
  // joystick available (and has got some info)
  bool available = false;
  bool joystickControl = false;  
  //
  mutex dataLock;
};

/**
 * Make this visible to the rest of the software */
extern UJoy joy;

#endif
