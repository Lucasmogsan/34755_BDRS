/***************************************************************************
 *   Copyright (C) 2019-2022 by DTU                             *
 *   jca@elektro.dtu.dk                                                    *
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

#ifndef UAD_H
#define UAD_H

#include <stdint.h>
#include <ADC.h>
#include "main.h"
#include "ucontrol.h"
#include "usubss.h"
#include "pins.h"

class UAd : public USubss
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
   * NB! called by timer interrupt */
  void tickHalfTime();
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
//   float getMotorCurrentM(int m, int32_t value);
  
  
protected:
  /**
   * send data to subscriber or requester over USB 
   * @param item is the item number corresponding to the added subscription during setup. */
  void sendData(int item);
  
public:
  /** called by interrupt 
   * \param a is the AD hardware number */
  void adInterrupt(int a);
  /// raw AD data
  /**
   * Sharp analog distance sensor */
  uint16_t irRawAD[2]; 
  /**
   * battery voltage pin */
  uint16_t batVoltRawAD;
  /**
   * analog current sensor */
  uint16_t motorCurrentRawAD[2];
  /**
   * Line sensor x 8 */
  int16_t adcLSL[8]; // when LED os off
  int16_t adcLSH[8]; // when lLED is on
  /// 
  uint16_t adcStartCnt = 0, adcHalfCnt = 0;
  uint32_t adcConvertTime;
  uint32_t adcHalfConvertTime;
  
private:
  /**
   * send raw AD - except line sensor */
  void sendADraw();
  /**
   * send raw AD for linesensor */
  void sendStatusLSRaw();
  /**
   * send */
//   void sendStatusCurrentVolt();
  
  
  // AD conversion
  // ADC pins
  #define ADC_NUM_IR_SENSORS      2
  #define ADC_NUM_NO_LS           5
  #define ADC_NUM_ALL             (ADC_NUM_NO_LS + 8)
  
  uint16_t adcInt0Cnt = 0;
  uint16_t adcInt1Cnt = 0;
  //
  int adcResetCnt = 1;
  int debug1adcIntCntLast = 0;
  int adcIntErrCnt = 0;
  // Destination for the first 5 ADC conversions. Value is set in ADC interrupt routine
  uint16_t * adcDest[ADC_NUM_NO_LS] =
  {
    &irRawAD[0],
    &irRawAD[1],
    &batVoltRawAD,
    &motorCurrentRawAD[0],
    &motorCurrentRawAD[1]
    
  };
  // List of AD numbers. First 5 values are ID, Battery and motor current. Remaining are the 8 line-sensor values
  int adcPin[ADC_NUM_ALL] =
  {
    PIN_IR_RAW_1,
    PIN_IR_RAW_2,
    PIN_BATTERY_VOLTAGE,
    PIN_LEFT_MOTOR_CURRENT,
    PIN_RIGHT_MOTOR_CURRENT,
    PIN_LINE_SENSOR_0,
    PIN_LINE_SENSOR_1,
    PIN_LINE_SENSOR_2,
    PIN_LINE_SENSOR_3,
    PIN_LINE_SENSOR_4,
    PIN_LINE_SENSOR_5,
    PIN_LINE_SENSOR_6,
    PIN_LINE_SENSOR_7
  };
  uint32_t adcStartTime;
  uint32_t adcHalfStartTime;
  
  
  ADC adc;       // ADC class
  int adcSeq;      // current ADC measurement index - shifted in interrupt-routine
  bool adcHalf;    // for double ADC conversion for LS
};

extern UAd ad;

#endif
