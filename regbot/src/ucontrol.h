 /***************************************************************************
 * definition file for the regbot.
 * The main function is the controlTick, that is called 
 * at a constant time interval (of probably 1.25 ms, see the intervalT variable)
 * 
 * The main control functions are in control.cpp file
 * 
 * ***************************************************************************
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
 
#ifndef REGBOT_CONTROL_H
#define REGBOT_CONTROL_H

#include <stdint.h>
#include "main.h"
#include <math.h>
#include "umission.h" 
#include "ucontrolbase.h"
#include "ucontrolturn.h"
 


class UControl
{
public:
  /**
   * constructor */
//   UControl();
  /**
   * general control setup */
  void setup();
  /**
   * send command help */
  void sendHelp();
  /**
   * decode command */
  bool decode(const char * line);
  /**
   * subscribe tick */
  void subscribeTick(void);
  /**
   * stop sending on all subscriptions */
  void stopSubscriptions();
  /**
   * Initialize input and output of controllers */
  void setRegulatorInOut();
  /**
   * save controller configuration to EE Prom */
  void eePromSaveCtrl();
  /**
   * load controller configuration from EE Prom
   * same order as when saved
   */
  void eePromLoadCtrl();
  /**
   * Reset all controllers, i.e. set all passed values to zero. */
  void resetControl();
  /**
   * Init control - calculate discrete control parameters
   * */
  void initControl();
  /**
   * Stop turning */
  void resetTurn();
  /**
   * Set remote control parameters 
   * \param ctrlState set to control master, 10 = auto, 1 = bridgeRC, 4=mission RC (using rc command), 10=mission line
   * \param vel is velocity reference in m/s
   * \param velDif is difference in wheel speed when turning
   * \param balance should robot try to acheive balance
   * */
  void setRemoteControl(int ctrlState, float vel, float velDif, bool balance);  
  /**
   * This function is called every time it is time to do a new control calculation
   * */
  void tick(void); 
  /**
   * set rate limit i.e. acceleration limit (m/s^2) */
  inline void setRateLimit(float limit)
  {
    rateLimitUse = limit < 100.0;
    rateLimit = fabs(limit);
  }
  /**
   * mission start state for hard coded missions */
  bool mission_hard_coded();
  /**
   * state update for user mission */
  void user_mission();
  /**
   * send mission status if any of the data has changed
   * \param forced send status anyhow - changed or not. */
  void sendMissionStatusChanged(bool forced);
  /**
   * is rate limit used - e.g. to stop integrator windup */
  inline bool isRateLimitUsed()
  { // eg for 
    return rateLimitUsed;
  }
  /**
   * Zeroset chirp variables, and set number of
   * logged values every full circle (360 degrees) 
   * \param logInterval is set from normal log interval,
   * but with chirp active is changes to samples per chirp period (of 360 deg) */
  void chirpReset(int logInterval);
  /**
   * Start chirp now, from highest frequency, as set with log interval.
   * This is called from mission line (first time chirp is found)
   * \param chirp is the amplitude of the chirp in cm/s or deci-radians (both 1..255) (0 is off) 
   * \param useHeading if false, then control velocity */
  void chirpStart(uint8_t chirp, bool useHeading);

public:  
  UControlBase * ctrlVelLeft;
  UControlBase * ctrlVelRight;
  UControlTurn * ctrlTurn;
  UControlBase * ctrlWallVel;
  UControlWallTurn * ctrlWallTurn;
  UControlPos * ctrlPos;
  UControlEdge * ctrlEdge;
  UControlBase * ctrlBal;
  UControlBalVel * ctrlBalVel;  
  UControlPos * ctrlBalPos;  
  
public:
  // mission based settings
  bool debugMissionEnd = false;
  int8_t missionLineNum;
  float mission_vel_ref;
  float mission_turn_ref;
  bool  mission_line_LeftEdge;
  bool mission_wall_turn;
  int8_t mission_irSensor_use;
  float mission_line_ref;
  float mission_turn_radius;
  float mission_wall_ref;
  float mission_wall_vel_ref;
  bool mission_turn_do;
  float mission_tr_turn;
  UMissionLine * misLine;
  uint8_t misThread;
  bool mission_pos_use;    // should position regulator be used
  float mission_pos_ref;    // should position regulator be used
  float misStartDist;         // start distance for this line
  bool balance_active;
  bool regul_line_use;  // use heading control from line sensor
  // debug values for controller
  float regVelELeft[], regVelUILeft[], regVelUDLeft[];
  //
  // remote control (rc command)
  int controlState = 0; // 0 = no control
  //
  bool remoteControl;
  float remoteControlVel; /// in m/s
  float remoteControlVelDif; /// in m/s
  bool remoteControlNewValue;
  
private:
  /// 
  bool backToNormal = false; // revert to flash default after hard mission
  /// adjusted and interim reference values
  /// other than those set by mission
//   float vel_ref_add_wall; // 0=left 1=right 
  float vel_ref_red_turn[2]; // 0=left 1=right
  float ctrl_vel_ref; // velocity ref for position control
  float ctrl_turn_ref; // turn ref used in for control input
  float balanceTiltRef; // reference tilt to balance regulator (from balance velocity)
  float balanceVelRef; // reference velocity as determined by balance controller
  /** rate limited wheel velocity - except for balance */
  bool rateLimitUse;
  bool rateLimitUsed;
  // mission control variables
  int16_t missionStateLast;
  uint8_t misThreadLast;
  //
//   int tickCnt;
  /// mission end time (theEnd got true)
  float endTime;
  ///
  bool savedBalanceActive = false;
  bool savedControlActive = false;
  /// frequency scale each sec (or ms?)
  double chirpFrqRate = 0; // scale factor per ms
  /// current chirp quadrant
  uint8_t chirpLogQuadrant;
  /// how many "quadrants" should be logged every revolution
  uint8_t chirpLogInterval;
  /// chirp destination
  /// 0 is velocity, 1 is heading
  uint8_t chirpDestination;
  // 
  /// test with turn-control on motor voltage (rather than on vel-ref)
  bool turnCtrlOnMotorDirect = true;

public:
  // used by data logger
  float vel_ref_pre_bal[2]; // reference to wheel velocity before ballance adjust
  float vel_ref[2]; // reference to wheel velocity controller
  float rateLimit; // current acceleration limit
  /// mission to start (with start or button)
  int mission;
  int16_t missionState;
  bool controlActive;
  // chirp settings - 
  bool  chirpRun = false;
  // amplitude in m/s (velocity) or radians (turn)
  float chirpAmplitude = 0;
  /// log flag - set by chirp, reset by log function
  bool chirpLog;
  /// current chirp frequency
  double chirpFrq = 0;
  /// current phase angle
  double chirpAngle = 0;
  /// current chirp value (amplitude at this time)
  float chirpValue;
  // do not go lower than this frequency
  float chirpMinimumFrequency;
  // chirp amplitude limitation
  /**
   * for every cycle find max motor voltage.
   * should limit chirp amplitude to reduce motor voltage
   * limit is hard coded to 6V */
  float chirpMotorVoltMax = 0;
  /**
   * for every cycle find max motor velocity.
   * should limit chirp amplitude to reduce velocity
   * limit is hard coded to 1m/s */
  float chirpVelMax = 0;
};

extern UControl control;

#endif
