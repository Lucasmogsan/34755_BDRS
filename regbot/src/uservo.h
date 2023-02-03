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

#ifndef SERVO_ON_REGBOT_H
#define SERVO_ON_REGBOT_H

#include <stdint.h>
#include "main.h"
#include "pins.h"
#include "usubss.h"

class UServo : public USubss
{
public:
  /** 
   * constructor (niit) */
  UServo();
  /** else disabled (no pulse) */
#ifdef REGBOT_HW41
  static const int MAX_SERVO_CNT = 5;
#else
  static const int MAX_SERVO_CNT = 3;
#endif
  bool servoEnabled[MAX_SERVO_CNT];
  /** last commanded value */
  int32_t servoValue[MAX_SERVO_CNT];
  int16_t servoVel[MAX_SERVO_CNT];
  int16_t servoRef[MAX_SERVO_CNT];
  bool    servoEnaRef[MAX_SERVO_CNT];
  /** servoboard pin - is output */
  bool pinIsOutput[2]; // not used for HW41
  /** servoboard pin - set value if output and read value if input
   * pin 4 is analog, pin 5 is digital */
  int16_t pin4Value; // not used for HW41
  bool    pin5Value;
  /** PWM frequency (400 Hz is probably maximum) */
  static const int PWMfrq = 333; // Hz - 3ms update rate
  /**
  * set PWM port of frekvens */
  void setup();
  /**
   * decode servo related commands
   * \param buf is the command string 
   * \returns true is used.
   * */
  bool decode(char * buf);
  
  void sendHelp();
  /**
   * send subscribed data - called from subscription */
  void sendData(int item);
  /**
   * Get actual pin number for servo */
  int getServoPin(int i);
  
  /**
  * Set servo 1 PWM value
  * \param pwm allowed input is +/- 512, where
  * 0 is center (1.5ms)
  *  \param enable if false, then PWM pulse is stopped (servo disables),
  * but port is still an output port
  * \param vel is max velocity for servo 0=no limit 1 is slow 2 faster, ~10 is full speed.
  * */  
  inline void setServoPWM(int serviIdx, int16_t pwm, bool enable, int16_t vel)
  {
    servoRef[serviIdx] = pwm;
    servoEnaRef[serviIdx] = enable;
//     if (vel >= 0)
    servoVel[serviIdx] = vel;
//     if (not enable)
//       usb_send_str("# setServo1PWM off\n");
  }
  /** 
   * \param pin allowed pin is 0,1. 
   * \param value input true is 1
   * \param enable if false, then port in set as input
   * */
  void setServoPin(int8_t pin, int16_t value, bool enable);
  /**
   * set any servo or IO pin/value */
  void setServo(int8_t idx, int16_t value, bool enable, int8_t vel);
  /**
   * stop PWM to servos and set servo pins to input */
  void releaseServos();
  /**
   * make servo 1 act as steering */
//   float setServoSteering();
  /**
   * send servo status to client */
  void sendServoStatus();
  /**
   * send status for steering using servo 1 
   *  servo 1 as steering wheel
   *  format: 'sv1' use offset dist angle
   *  e.g.: sv1 1 0 0.135 90.0
   *  use is a boolean - activate steering
   *  offset is value +/- 512 for straight forward (close to 0)
   *  dist is distance to steering wheel
   *  angle is servo angle from 1ms to 2ms
   * */
//   void sendServo1Status();
  /**
   * set servo values from string */
  void setServoConfig(const char * line);
  /**
   *  servo 1 as steering wheel - settings from command line
   *  format: 'sv1' use offset dist angle
   *  e.g.: sv1 1 0 0.135 90.0
   *  use is a boolean - activate steering
   *  offset is value +/- 512 for straight forward (close to 0)
   *  dist is distance to steering wheel
   *  angle is servo angle from 1ms to 2ms
   * */
//   void setServo1Config(const char * line);
  /**
   * set one servo or pin (mostly for debug) */
  void setOneServo(const char * line);
  /**
   * 5ms update of servo */
  void tick();
  
  /**
   * save steering configuration to (EE)disk */
  void eePromSave();
  /**
   * load configuration from EE-prom */
  void eePromLoad();
  
  
private:
  // 1ms = frq/12bit
  static const int max_pwm = 4096;
  /// pwm value to give 1ms
  static const int msPulse = (max_pwm * PWMfrq) / 1000;
  /// center position (1.5ms)
  static const int midt = (msPulse * 3) / 2;
};

extern UServo servo;

#endif // USERVO_H
