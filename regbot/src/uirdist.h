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

#ifndef IR_DIST_H
#define IR_DIST_H

#include "usubss.h"

class UIrDist : public USubss
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
  
  /**
  * Calculated sensor distance in meters */
  float irDistance[2];
  /**
  * should time be spend on dist sensor calculation */
  bool useDistSensor = true;
  /**
  * Is dist sensor installed on this robot */
//   bool distSensorInstalled = true;

  /**
  * Send a "dip" message */
  void sendStatusDistIR();

  /**
  * Sets the two calibrate values for each IR sensor
  * "irc 20cm ir1, 80cm ir1, 20cm ir2, 80cm ir2", like
  * "irc 3011 480 2990 480"
  * \param buf line with calibrate values
  * \returns true if buffer was used
  *  */
//   bool setIrCalibrate(const char * buf);

  /**
  * Estimate distance in meters
  * uses straight hyperbolic estimate
  * source irRaw 
  * irDistance = irA + irB/irRaw;
  * where irA and irB is calibration values:
  * irA = 0.2 (irCal20cm - 4 * irCal80cm) / (irCal20cm - irCal80cm)
  * irB = 0.6 * irCal20cm * irCal80cm / (irCal20cm - irCal80cm)
  * \returns value in irDistance */
  void estimateIrDistance();
  /**
   * power on/off the IR sensor. */
  void setIRpower(bool power);
  
  void eePromSave();

  void eePromLoad();

protected:
  /**
   * send data to subscriber or requester over USB 
   * @param item is the item number corresponding to the added subscription during setup. */
  void sendData(int item);
  
  
private:
  uint32_t irCal13cm[2] = {72300, 72300}; 
  uint32_t irCal50cm[2] = {12500, 12500};

  
  bool initIrFilter = true;
  // filtered
public:
  uint32_t irRaw[2]; // move back to private, once ULogger is done
  /**
   * Raw sensor conversion values from 2 IR sensors (Sharp 2Y0A21 F) */
  float irA[2];
  float irB[2];
private:
  //
  // subscribe
  static const int SUBS_CNT = 1;
  USubs * subs[SUBS_CNT];
//  int subIrN = 0;
//  int subIrCnt = 0;
  
  void calibrateIr();
  //
  friend class ULogger;
};

extern UIrDist irdist;

#endif
