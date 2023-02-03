/***************************************************************************
 *   Copyright (C) 2014-2022 by DTU
 *   jca@elektro.dtu.dk            
 * 
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

#ifndef UCURRENT_H
#define UCURRENT_H

#include <stdint.h>
#include "main.h"
#include "ucontrol.h"
#include "usubss.h"
#include "pins.h"

class UCurrent : public USubss
{
public:
  /**
   * Setup */
  void setup();
  /**
   * send help */
  void sendHelp();
  /**
   * decode command for this unit */
  bool decode(const char * buf);  
  /**
   * Checks for state change
   * to ESC values */
  void tick();
  /**
   * load configuration from flash */
  void eePromLoad();
  /**
   * save current configuration to flash */
  void eePromSave();
  /**
   * get motor current for motor 0 or 1 in amps.
   * NB! no test for valid index.
   * \returns current in amps */
  float getMotorCurrentM(int m, int32_t value);
  void logIntervalChanged();
//   void motorDisabled();
  float motorCurrentA[2];
  
  
protected:
  /**
   * send data to subscriber or requester over USB 
   * @param item is the item number corresponding to the added subscription during setup. */
  void sendData(int item);
  
private:
  void sendMotorCurrent();
  void sendMotorCurrentOffset();
  //   void sendStatusCurrentVolt();
  int32_t motorCurrentMLowPass[2];
  
//   uint16_t motorCurrentM[2];
  int32_t motorCurrentMOffset[2];
  uint16_t lowPassFactor;
  //
  bool currentOffsetting = false;
  int postpondCalibration = 0;
};

extern UCurrent current;

#endif
