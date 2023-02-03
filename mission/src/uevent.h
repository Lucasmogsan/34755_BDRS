/*  
 * 
 * Copyright © 2022 DTU, Christian Andersen jcan@dtu.dk
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


#ifndef UEVENT_H
#define UEVENT_H

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

class UEvent{
  
public:
  /** setup and request data */
  void setup();
  /** decode an unpacked incoming messages
   * \returns true if the message us used */
  bool decode(char * msg);
  /**
   * clear all events */
  void clearEvents();
  /**
   * Test if event is set 
   * \param i is event index (0 to 33)
   * event 33 is mission started
   * event 0 is mission ended
   * event 1..32 is user events
   * \returns true if event is received */
  bool gotEvent(int i);
  /**
   * wait for event or return if a mission is not running
   * \param n is the event to wait for.
   * \returns true if the event has arrived and false if no mission is running */
  bool waitForEvent(int n);

private:
  static const int MAX_EVENT = 34;
  bool events[MAX_EVENT] = {false};
  mutex dataLock;
};

/**
 * Make this visible to the rest of the software */
extern UEvent event;

#endif
