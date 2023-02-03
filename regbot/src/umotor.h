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

#ifndef UMOTOR_H
#define UMOTOR_H

#include <stdint.h>
#include "main.h"
// #include "command.h"
#include "pins.h"
#include "usubss.h"

//#define ACT1_FREQ 25000

class UMotor : public USubss
{
public:
  // motor voltage
  float motorVoltage[2];
  bool motorEnable[2] = {false};
  /** PWM frequency (400 Hz is probably maximum) */
  int PWMfrq = 27300; // 16kHz is OK default and a must for big Pololu controllers
  /**
   * if motor gets full power in more than 1 second, then stop */
  int overloadCount;
  bool motorPreEnabled;
  bool motorPreEnabledRestart;
  // motor reversed
  // Pololu motors not, but big ones from China is opposite (reversed)
  bool motorReversed = false;
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
   * implement motor voltage (escValue) as PWM signal */
//   void implementMotorVoltage();
  /**
   * set one esc or pin (mostly for debug) */
  void setPWMfrq(const char * line);
  void setPWMfrq(int frq);
  /**
   * 2.5ms update of esc */
  void tick();
  /**
   * save configuration to (EE)disk */
  void eePromSave();
  /**
   * load configuration from EE-prom */
  void eePromLoad();
  /**
   * emergency stop */
  void stopAllMotors();
  /**
   * send current motor values (voltage and reference settings)
   * */
  void sendMotorValues();
  /**
   * Send motor direction and PWM (MPW in range [-4096 .. 4096]) */
  void sendMotorPWM();
  /**
   * e2 used on HW < 3 only */
  void motorSetEnable(uint8_t e1, uint8_t e2);
  
  
protected:
  
  void sendData(int item);
  
  void motorSetPWM(int m1PWM, int m2PWM);
  
  void motorSetAnchorVoltage();
  
  
private:
  //
  int16_t motorAnkerPWM[2];
  int16_t motorAnkerDir[2];
  int16_t motorSleeping[2];
  int pinMotor2Dir;
  int pinMotor2Pwm;
  // subscribe
//   static const int SUBS_CNT = 1;
//   USubs * subs[SUBS_CNT];
//   int subMotN = 0;
//   int subMotCnt = 0;
//   bool aaa = false;
  int tickCnt = 0;
  int setupCnt = 0;
};

extern UMotor motor;

#endif // UMOTOR_H
