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


#ifndef ULINESENSOR_H
#define ULINESENSOR_H

#include "usubss.h"



class ULineSensor : public USubss
{
public:
  bool lsIsWhite;
  bool lsPowerHigh;
  bool lsTiltCompensate;

  /**
  * set PWM port of frekvens */
  void setup();
  /**
   * send command help */
  void sendHelp();
  /**
   * decode commands */
  bool decode(const char * buf);
  /**
   * update at sample time */
  void tick();

  /** save line sensor calibration */
  void eePromSaveLinesensor();
  /** load line sensor calibration */
  void eePromLoadLinesensor();

  /**
  * reset filters and stored values
  * as a follow line mission line is finished */
  void lsPostProcess();

public:
  /* Line sensor result */
  float lsLeftSide;
  float lsRightSide;
  bool lsEdgeValid;
  int8_t crossingLineCnt;
  int8_t lsEdgeValidCnt;
  /**
  * Use line sensor */
  bool lineSensorOn;
  int16_t whiteLevel[8];
  int16_t blackLevel[8];
  float lsGain[8] = {0.0};
  /**
   * next 2 probably not relevant */
  float findCrossingLineVal;
  float edgeAngle;
  /// maybe just debug for logging
  int lsIdxLow, lsIdxHi;

  
protected:
  /**
   * send subscripted data 
   * \param item is publish index (called by subscription class) */
  void sendData(int item);
  /**
   * edge detection function */
  void findEdgeV4();

  /**
  * Send line sensor difference values,
  * either directly from source, or kompensated for calibration (and tilt if in balance)
  */
  void sendStatusLineSensor(bool normalized);
  /**
  * send normalize gain values */
  void sendLineSensorGain();
  void sendStatusLineSensorLimitsWhite();
  void sendStatusLineSensorLimitsBlack();

  /**
  * Estimate line edge position - if relevant.
  * Called at every control interval */
  void estimteLineEdgePosition();
  /**
   * Find crossing line */
  void findCrossingLine();

  /**
  * Send status for aAD converter values directly
  * \param idx = 1 : ADC, 2: limits, 3:values and edges, 4: gain */
  void sendADLineSensor(int8_t idx);
  /**
  * set linesensor parameters from GUI.
  * \param buf is the command string
  * \returns true if used */
//   bool setLineSensor(const char * buf);
  /**
  * estimate edges of line */
  void findLineEdge(void);
  /**
  * estimate line crossing and edge - designed for balance use,
  * slightly more complicated */
  void findEdgeV2(void);
  /**
   * compensate for different channel-gain */
  void normalize(void);

  /**
  * Send linesensor findings */
  void sendLineSensorPosition();
  /**
  * difference between illuminated and not, normalized between calibration values */
  float lineSensorValue[8];

//   float blackVal;    // black value when line is black

  // debug
//   int lsIdxLow, lsIdxHi;
  int lsIdxxl, lsIdxxr;
  float lsTl, lsTr; // position index with decimals
  uint8_t whiteQ, blackQ; // quality of white line and black line detect
  // debug end
private:
  const float kd = 10;
  bool crossing = false;
  int oldDistTime = 0;
  /**
   * Normalized difference */
  int16_t adcLSD[8] =   {600,611,622,633,644,655,666,677};

  bool swapLeftRight = false;
  const int8_t crossingCntLimit = 20;
  const int edgeDetectCntLimit = 20; /// number of times a line has to be detected to be valid

  float crossingDetect = 4.2; // criteria for sensor count (as float) estimated as line
  bool wideSensor = false; // either 6cm (false) or 10 cm wide (true)

  int invalidCnt;
  float oldPos2mm = 0, oldPos10mm = 0;
  const bool logLineSensorExtra = false;
  float edgePos2mm[2];
  float edgePos10mm[2];
  const float lsMidtIndex = 3.5; /// led 0..3 left and LED 4..7 right, so center is 3.5
  const float lsLEDdistance = 0.76; /// distance between LED sensors in cm.};
  
  /// High or low power pin mode
  bool pinModeLed = INPUT;
};

extern ULineSensor ls;

#endif
