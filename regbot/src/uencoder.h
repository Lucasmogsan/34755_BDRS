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

#ifndef UENCODER_H
#define UENCODER_H

#include <stdint.h>
#include <math.h>
#include "main.h"
// #include "command.h"
#include "pins.h"
#include "usubss.h"



/**
 * interrupt for motor encoder */
void m1EncoderA();
void m2EncoderA();
void m1EncoderB();
void m2EncoderB();


class UEncoder : public USubss
{
public:
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
   * sample update */
  void tick();
  /**
   * save configuration to (EE)disk */
  void eePromSave();
  /**
   * load configuration from EE-prom */
  void eePromLoad();
  /**
   * encoder status */
  void sendEncStatus();
  /**
   * Send robot config,
   * wheel radius (x2), gear ratio, encoder tics, wheelbase */
  void sendRobotConfig();
  
  /**
   * estimated pose status */
  void sendPose();
  /**
   * estimated pose status */
  void sendVelocity();
  
  void updatePose(uint32_t loop);
  
  void encoderInterrupt(int m, bool a);
  /**
   * clear pose and distance traveled. */
  void clearPose();
  
protected:
  /**
   * send data to subscriber or requester over USB 
   * @param item is the item number corresponding to the added subscription during setup. */
  void sendData(int item);  
  

public:
  bool encCCV[2];
  uint32_t encStartTime_cpu[2];
  bool encTimeOverload_cpu[2];
  uint32_t encoder[2];
  uint32_t encPeriod_cpu[2];
  
  /// estimated velocity - same as wheelVelocity times wheel radius
  float wheelVelocityEst[2] = {0, 0}; // in meter per second
  float pose[4] = {0,0,0,0}; // x,y,h,tilt
  float distance = 0.0; // distance in meters (forward) reverse is negative.
  //float robot_delta_velocity = 0; // velocity difference between wheels
  float robotTurnrate = 0.0; // is 1/turn_radius
  float robotVelocity = 0; // linear velocity
  //
  float odoWheelBase = 0.41; // distance between wheels
  //
  int intCnt = 0; // debug
  
private:
  int tickCnt = 0;
  // 1ms = frq/12bit
  static const int max_pwm = 4095; // 12 bit
  /// pwm value to give 1ms
  int msPulseCount;
  /// pwm at center position 1.5ms for signed or 1ms for one way only (trust)
  int pulseCountOffset;
  uint32_t encoderLast[2];
  //
  //
  float odoWheelRadius[2] = {0.19/2.0, 0.19/2.0};
  //
  float wheelVelocity[2];
  /// should velocity at low speed be estimated (observer)
  //bool regul_vel_est = true;
//   float wheelPosition[2] = {0, 0}; // in radians since start
  //
  float gear = 51;
  uint16_t pulsPerRev = 40; // using all edges
  float anglePerPuls = 2.0 * M_PI / (pulsPerRev * gear);
  // debug
  static const int MDV = 100;
  uint8_t eport[MDV][4];
  float edt[MDV];
  int eportCnt = 0;
  int nanCnt = 0;
  // subscribe
//   int subEncCnt = 0;
//   int subEncN = 0;
//   int subPoseCnt = 0;
//   int subPoseN = 0;
//   int subVelCnt = 0;
//   int subVelN = 0;
//   int subRobotCnt = 0;
//   int subRobotN = 0;
//   static const int SUBS_CNT = 4;
//   USubs * subs[SUBS_CNT];
};

extern UEncoder encoder;

#endif // MOTOR_CONTROLLER_H
