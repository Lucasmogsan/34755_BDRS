/***************************************************************************
 *   Copyright (C) 2022 by DTU                             *
 *   jca@elektro.dtu.dk                                                    *
 *
 *   Simple sound based on PWM frequency
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

//#define IR13_50CM 1

#ifndef USOUND_H
#define USOUND_H

#include "usubss.h"

class USound : public USubss
{
public:

  /**
   * setup */
  void setup();
  /**
   * decode data command */
  bool decode(const char * buf);
  /**
   * send help on messages */
  void sendHelp();
  /**
   * Do sensor processing - at tick time */
  void tick();  
  
  void eePromSave();

  void eePromLoad();
  
  void play(int song);

protected:
  /**
   * send data to subscriber or requester over USB 
   * @param item is the item number corresponding to the added subscription during setup. */
  void sendData(int item);
  
  void sendSoundInfo();
  void sendMuteInfo();
  int played = -1;  
  bool unmute = true;
  int tickCnt = 0;
  //
  friend class ULogger;
};

extern USound sound;

#endif
