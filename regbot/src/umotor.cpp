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

#include <stdlib.h>
// #include <../teensy3/kinetis.h>
#include "main.h"
#include "umotor.h"
#include "ueeconfig.h"
#include "pins.h"
#include "ustate.h"
// #include "logger.h"
#include "uusb.h"
// #include "sensor.h"
#include "ucontrol.h"
#include "usubss.h"
#include "ulog.h"

UMotor motor;

void UMotor::setup()
{
  analogWriteResolution(12); /// resolution (12 bit)
  // find offset for motor current
  setPWMfrq(PWMfrq);
  motorPreEnabled = true;
  //
  motorSetEnable(0,0);
  #ifdef REGBOT_HW41
  pinMode(PIN_LEFT_DIR,OUTPUT); // motor 1 IN1 (LEFT_IN_+)
  pinMode(PIN_RIGHT_DIR,OUTPUT); // motor 2 IN1 (RIGHT_IN_+)
  pinMode(PIN_LEFT_PWM,OUTPUT); // motor 1 IN2 (LEFT_IN_-)
  pinMode(PIN_RIGHT_PWM,OUTPUT); // motor 2 IN2 (RIGHT_IN_-)
  // PWM is set to output when 
  analogWriteFrequency(PIN_LEFT_DIR, PWMfrq); 
  analogWriteFrequency(PIN_RIGHT_DIR, PWMfrq);
  analogWriteFrequency(PIN_LEFT_PWM, PWMfrq); 
  analogWriteFrequency(PIN_RIGHT_PWM, PWMfrq);
  motorSetPWM(0,0);
  #else
  switch (state.robotHWversion)
  {
    case 1: // no used anymore
      break;
    case 2: // probably not used, but same motherboard as 5
    case 5: // with new satellite boards for power and IR
      pinMode(SLEW,OUTPUT); // slewrate hight
      digitalWrite(SLEW,HIGH); // slewrate - always high
      pinMode(M1PWM,OUTPUT); // motor 1 PWM 
      pinMode(M1DIR,OUTPUT); // motor 1 direction
      pinMode(M12DIS,OUTPUT); // disable motor 1 and 2
      //       pinMode(M2DIR1,OUTPUT); // direction (hardware < 3 only)
      pinMotor2Pwm = M2PWM1;
      pinMode(pinMotor2Pwm, OUTPUT);
      // slow motor controller
      analogWriteFrequency(pinMotor2Pwm, 17000); // motor 2 (right)
      analogWriteFrequency(M1PWM, 17000); // motor 1 (left)
      pinMotor2Dir = M2DIR1;
      pinMode(pinMotor2Dir, OUTPUT);
      // write to motor controller pins
      pinMode(M1DIS1, OUTPUT);
      pinMode(M2DIS1, OUTPUT);
      digitalWrite(M1DIS1, motorEnable[0]); 
      digitalWrite(M2DIS1, motorEnable[1]);
      break;
    case 3:
      //       pinMode(M1DIS1,OUTPUT); // disable motor 1
      pinMode(M1PWM,OUTPUT); // motor 1 PWM 
      pinMode(M1DIR,OUTPUT); // motor 1 direction
      pinMode(M12DIS,OUTPUT); // disable motor 1 and 2
//       pinMode(M2DIR1,OUTPUT); // direction (hardware < 3 only)
      pinMotor2Pwm = M2PWM3;
      pinMode(pinMotor2Pwm, OUTPUT);
      // slow motor controller
      analogWriteFrequency(pinMotor2Pwm, 17000); // motor 2 (right)
      analogWriteFrequency(M1PWM, 17000); // motor 1 (left)
      pinMotor2Dir = M2DIR3;
      pinMode(pinMotor2Dir, OUTPUT);
      // write to motor controller pins
//       if (false)
//       { // wrong - pre version 3.3?
//         digitalWrite(M1DIS1, motorEnable[0]); 
//         digitalWrite(M2DIS1, motorEnable[1]);
//       }
//       else
      digitalWrite(M12DIS, motorEnable[0]);
      break;
      
      usb.send("# setup motor controller 1,2,3,5\n");
    case 4:
    case 6:
      pinMode(M1PWM,OUTPUT); // motor 1 PWM 
      pinMode(M1DIR,OUTPUT); // motor 1 direction
      pinMode(M12DIS,OUTPUT); // disable both motors pin
      pinMode(M2DIR3,OUTPUT); // direction (hardware 3 only) pin
      pinMode(M2PWM3,OUTPUT); //OUTPUT); // PWM signal right motor (hardware 3 only) pin
//       if (state.robotHWversion == 4)
//         PWMfrq 
//         analogWriteFrequency(M2PWM3, 25000); // small motor controller (both)
//       else
//         analogWriteFrequency(M2PWM3, 17000); // big motor controller (both)
      pinMotor2Dir = M2DIR3;
      pinMotor2Pwm = M2PWM3;
      // write to motor controller pins
      digitalWrite(M12DIS, motorEnable[0]);
      break;
    default:
      break;
  }
  #endif
  // PWM is set to output when 
  analogWriteFrequency(PIN_LEFT_DIR, PWMfrq); 
  analogWriteFrequency(PIN_RIGHT_DIR, PWMfrq);
  analogWriteFrequency(PIN_LEFT_PWM, PWMfrq); 
  analogWriteFrequency(PIN_RIGHT_PWM, PWMfrq);
  motorSetPWM(0,0);
  //
  if (setupCnt == 0)
  { // only once
    addPublistItem("mot", "Get motor voltage 'mot m1(V) m2(V) vel_ref1(m/s) vel_ref2(m/s) reversed'");
    addPublistItem("motpwm", "Get motor direction and PWM 'motpwm dir1 pwm1 dir2 pwm2'");
  }
  setupCnt++;
}


/**
 * e2 used on hardware < 3 only */
void UMotor::motorSetEnable(uint8_t e1, uint8_t e2)
{
  if (motorPreEnabled and (e1 or e2) and not (motorEnable[0] or motorEnable[1]))
  { // switch off current zero offset calculation
    motorPreEnabled = false;
    motorPreEnabledRestart = true;
  }
  // reset overload
  if (e1 and not motorEnable[0])
    overloadCount = 0;
  // enable motors (or disable)
  motorEnable[0] = e1;
  motorEnable[1] = e2;
  //
#ifdef REGBOT_HW41
  if (not motorEnable[0])
  {// disable motor - set to sleep
    pinMode(PIN_LEFT_DIR,OUTPUT); 
    pinMode(PIN_LEFT_PWM,OUTPUT); 
    digitalWriteFast(PIN_LEFT_DIR, LOW);
    digitalWriteFast(PIN_LEFT_PWM, LOW);
    motorSleeping[0] = true;
//     usb.send("# motor 1 sleep\n");    
  }
  if (not motorEnable[1])
  { // disable motor - set to sleep
    pinMode(PIN_RIGHT_DIR,OUTPUT); 
    pinMode(PIN_RIGHT_PWM,OUTPUT); 
    digitalWriteFast(PIN_RIGHT_DIR, LOW);
    digitalWriteFast(PIN_RIGHT_PWM, LOW);
    motorSleeping[1] = true;
//     usb.send("# motor 2 sleep\n");    
  }
#else
  switch (state.robotHWversion)
  {
    case 2:
    case 5:
      digitalWrite(M1DIS1, motorEnable[0]);
      digitalWrite(M2DIS1, motorEnable[1]);
      break;
    default:
      digitalWrite(M12DIS, motorEnable[0]);
      break;
  }
#endif
}


void UMotor::motorSetAnchorVoltage()
{
  float batteryNominalVoltage = state.batteryVoltage;
  if (batteryNominalVoltage < 5.0)
    // not on battery - just for test and avoid divide by zero
    batteryNominalVoltage = 11.1;
  const float max_pwm = 4096;
  float scaleFactor = max_pwm / batteryNominalVoltage;
  // overload check
  if (overloadCount > 500 and motorEnable[0])
  { // disable motor (after 0.5 second)
    motorSetEnable(0, 0);
    //
    usb.send("# UMotor::motorSetAnchorVoltage: overload, disabled motors\n");
    //
  }
  // convert to PWM values (at 20khz)
  int w1, w2;
  if (motorReversed)
    // the big 12V Chinese motors
    scaleFactor *= -1.0;
  // else
  //   normal for pololu motors
  w1 = int16_t(motorVoltage[0] * scaleFactor);
  // the right motor must run the other way
  w2 = int16_t(-motorVoltage[1] * scaleFactor);
  // implement
  motorSetPWM(w1, w2);
}

/** 
 * allowed input is +/- 2048, where 2048 is full battery voltage
 * */
void UMotor::motorSetPWM(int m1PWM, int m2PWM)
{ // PWM is 12 bit
  const int max_pwm = 4095;
  // for logging
  motorAnkerPWM[0] = m1PWM;
  motorAnkerPWM[1] = m2PWM;
  motorAnkerDir[0] = m1PWM >= 0;
  motorAnkerDir[1] = m2PWM >= 0;
  
//   usb.send("# setting PWM\n");
  //
#ifdef REGBOT_HW41
  // LEFT motor
  if (not motorSleeping[0])
  {
    if (m1PWM >= 0)
    {
      pinMode(PIN_LEFT_DIR,OUTPUT); 
      digitalWriteFast(PIN_LEFT_DIR, HIGH);
      analogWrite(PIN_LEFT_PWM, max_pwm - m1PWM);
    }
    else /*if (m1PWM < 0)*/
    { // move PWM to other pin
      pinMode(PIN_LEFT_PWM, OUTPUT); // motor 1 PWM 
      digitalWriteFast(PIN_LEFT_PWM, HIGH);
      analogWrite(PIN_LEFT_DIR, max_pwm + m1PWM);
    }
//     else
//     { // disable motor
//       pinMode(PIN_LEFT_DIR,OUTPUT); 
//       pinMode(PIN_LEFT_PWM,OUTPUT); 
//       digitalWriteFast(PIN_LEFT_DIR, LOW);
//       digitalWriteFast(PIN_LEFT_PWM, LOW);
//       motorSleeping[0] = true;
//       usb.send("# motor 1 sleep\n");
//     }
  }
  else if (m1PWM != 0)
  { // set to not sleeping by 1,1 for > 400us
    // one sample time should be sufficient
    digitalWriteFast(PIN_LEFT_DIR, HIGH);
    digitalWriteFast(PIN_LEFT_PWM, HIGH);
    motorSleeping[0] = false;
//     usb.send("# motor 1 out of sleep\n");
  }
  // RIGHT motor
  if (not motorSleeping[1])
  {
    if (m2PWM >= 0)
    {
      pinMode(PIN_RIGHT_DIR,OUTPUT); 
      digitalWriteFast(PIN_RIGHT_DIR, HIGH);
      analogWrite(PIN_RIGHT_PWM, max_pwm - m2PWM);
    }
    else /*if (m2PWM < 0)*/
    { // move PWM to other pin
      pinMode(PIN_RIGHT_PWM,OUTPUT); 
      digitalWriteFast(PIN_RIGHT_PWM, HIGH);
      analogWrite(PIN_RIGHT_DIR, max_pwm + m2PWM);
    }
//     else
//     { // disable motor
//       pinMode(PIN_RIGHT_DIR,OUTPUT); 
//       pinMode(PIN_RIGHT_PWM,OUTPUT); 
//       digitalWriteFast(PIN_RIGHT_DIR, LOW);
//       digitalWriteFast(PIN_RIGHT_PWM, LOW);
//       motorSleeping[1] = true;
//       usb.send("# motor 2 sleep\n");
//     }
  }
  else if (m2PWM != 0)
  { // set to not sleeping by 1,1 for >400us
    // one sample time should be sufficient
    digitalWriteFast(PIN_RIGHT_DIR, HIGH);
    digitalWriteFast(PIN_RIGHT_PWM, HIGH);
    motorSleeping[1] = false;
//     usb.send("# motor 2 out of sleep\n");
  }
//     else
      
    
#else
  // big motor controller only
  const int pwmOffsetFwd = 50; // compensation for slow rise time
  const int pwmOffsetRev = 100; // motor controller around 12 V is a bit slower
  int pwm1=-1, pwm2=-1, m1d=-1, m2d=-1;
  switch (state.robotHWversion)
  {
    case 1:
    case 2:
    case 3:
    case 5:
      // pololu 33926 dual controller (big controller with FB and overload detect)
      // motor 1 (left)
      if (m1PWM >= 0)
      { // put H-bridge side 2 to high
        m1d = HIGH;
        if (m1PWM == 0)
        { // make side 1 switch high
          pwm1 = max_pwm;
        }
        else
        { // make side 1 switch with low pulses down to fully low 
          pwm1 = max_pwm - m1PWM - pwmOffsetFwd;
        }
      }
      else
      { // put H-bridge side 2 to low
        m1d = LOW;
        // make side 1 switch with high pulses up to fully high 
        pwm1 = -m1PWM + pwmOffsetRev;
      }
      digitalWrite(M1DIR, m1d);
      analogWrite(M1PWM, pwm1);
      //
      // motor 2 (rignt)
      if (m2PWM >= 0)
      { // put H-bridge side 2 to high
        m2d = HIGH;
        if (m2PWM == 0)
        { // make side 1 switch high
          pwm2 = max_pwm;
        }
        else
        { // make side 1 switch with low pulses down to fully low 
          pwm2 = max_pwm - m2PWM - pwmOffsetFwd;
        }
      }
      else
      { // put H-bridge side 2 to low
        m2d = LOW;
        // make side 1 switch with high pulses up to fully high 
        pwm2 = -m2PWM + pwmOffsetRev;
      }
      digitalWrite(pinMotor2Dir, m2d);
      analogWrite(pinMotor2Pwm, pwm2);
      // debug
      if (false and tickCnt % 100 == 0)
      {
        const int MSL = 150;
        char s[MSL];
        snprintf(s, MSL, "# UMotor::motorSetPWM PWM=%d %d, dir=%d %d, pwmpin=%d %d, motDir = %d %d\n", pwm1, pwm2, m1d, m2d, M1PWM, pinMotor2Pwm, M1DIR, pinMotor2Dir);
        usb.send(s);
      }
      // debug end
      break;
      //
    case 4:
    case 6:
      // pololu TB6612 dual motor controller (small controller)
      if (m1PWM >= 0)
      { // put H-bridge side 2 to high
        digitalWrite(M1DIR, LOW);
        // make side 1 switch with low pulses down to fully low 
        analogWrite(M1PWM, max_pwm - m1PWM);
      }
      else
      { // put H-bridge side 2 to low
        digitalWrite(M1DIR, HIGH);
        // make side 1 switch with high pulses up to fully high 
        analogWrite(M1PWM, max_pwm + m1PWM);
      }
      if (m2PWM >= 0)
      { // put H-bridge side 2 to high
        digitalWrite(pinMotor2Dir, LOW);
        // make side 1 switch with low pulses down to fully low 
        analogWrite(pinMotor2Pwm, max_pwm - m2PWM);
      }
      else
      { // put H-bridge side 2 to low
        digitalWrite(pinMotor2Dir, HIGH);
        // make side 1 switch with high pulses up to fully high 
        analogWrite(pinMotor2Pwm, max_pwm + m2PWM);
      }
      break;
    default:
      break;
  }
#endif
}


void UMotor::sendHelp()
{
  const int MRL = 300;
  char reply[MRL];
  usb.send("# Motor -------\r\n");
  snprintf(reply, MRL, "# \tmotr V \tSet motor reversed; V=0 for small motors, V=1 for big motors\r\n");
  usb.send(reply);
  snprintf(reply, MRL, "# \tmotv m1 m2 \tSet motor voltage -24.0..24.0 - and enable motors\r\n");
  usb.send(reply);
  snprintf(reply, MRL, "# \tmotfrq \tSet motor PWM frequency [100..50000], is %d\r\n", PWMfrq);
  usb.send(reply);
  subscribeSendHelp();
}


bool UMotor::decode(const char* buf)
{
  bool used = true;
  if (strncmp(buf, "motr", 4) == 0)
  {
    const char * p1 = &buf[4];
    // get two values - if no value, then 0 is returned
    bool rev = strtol(p1, (char**)&p1, 10);
    if (p1 == &buf[4] or rev)
      motorReversed = true;
    else
      motorReversed = false;
  }
  else if (strncmp(buf, "motv", 4) == 0)
  {
    float m1, m2;
    const char * p1 = &buf[4];
    // get two values - if no value, then 0 is returned
    m1 = strtof(p1, (char**)&p1);
    m2 = strtof(p1, (char**)&p1);
    motorVoltage[0] = m1;
    motorVoltage[1] = m2;
    if ((fabs(m1) < 0.01) and (fabs(m2) < 0.01))
      motorSetEnable(0, 0);
    else
      motorSetEnable(1,1);
    // debug
    // should be in tick() only
    motorSetAnchorVoltage();
    usb.send("# setting motor voltage\n");
    // debug end 
  }
  else if (strncmp(buf, "motfrq ", 7) == 0)
  {
    const char * p1 = &buf[7];
    int frq = strtol(p1, nullptr, 10);
    if (frq < 100)
      frq = 100;
    if (frq > 50000)
      frq = 50000;
    setPWMfrq(frq);
  }
  else if (subscribeDecode(buf)) {}
  else
    used = false;
  return used;
}

void UMotor::sendData(int item)
{
  if (item == 0)
    sendMotorValues();
  if (item == 1)
    sendMotorPWM();
}

void UMotor::sendMotorValues()
{
  const int MSL = 150;
  char s[MSL];
  snprintf(s, MSL, "mot %.2g %.2g %.3g %.4g %d\r\n", 
           motorVoltage[0], motorVoltage[1], control.vel_ref[0], control.vel_ref[1], motorReversed);
  usb.send(s);
}

void UMotor::sendMotorPWM()
{
  const int MSL = 150;
  char s[MSL];
#ifdef REGBOT_HW41
  int f1 = digitalReadFast(PIN_LEFT_FAULT);
  int f2 = digitalReadFast(PIN_RIGHT_FAULT);
#else
  int f1=0;
  int f2=0;
#endif
  snprintf(s, MSL, "motpwm %d %d %d %d %d %d %d\r\n", 
           motorAnkerDir[0], motorAnkerPWM[0], motorAnkerDir[1], motorAnkerPWM[1], PWMfrq, f1, f2);
  usb.send(s);
}

void UMotor::setPWMfrq(const char* line)
{
  // pwmfrq 400\n
  const char * p1 = line;
  int frq = strtol(p1, (char**)&p1, 10);
  if (p1 != line)
    // valid number
    setPWMfrq(frq);
}

void UMotor::setPWMfrq(int frq)
{
  // pwmfrq 400\n
  PWMfrq = frq;
  analogWriteFrequency(PIN_LEFT_DIR, PWMfrq); /// frequency (Hz)
  analogWriteFrequency(PIN_LEFT_PWM, PWMfrq); /// frequency (Hz)
  analogWriteFrequency(PIN_RIGHT_DIR, PWMfrq); /// frequency (Hz)
  analogWriteFrequency(PIN_RIGHT_PWM, PWMfrq); /// frequency (Hz)
  // debug
  if (false)
  {
    const int MSL = 130;
    char s[MSL];
    snprintf(s, MSL, "# setting motor PWM frq to %d Hz\n", PWMfrq);
    usb.send(s);
  }
}

void UMotor::tick()
{ //
  tickCnt++;
  if (true)
    motorSetAnchorVoltage();
  // subscription
  subscribeTick();
}

///////////////////////////////////////////////////////

void UMotor::eePromSave()
{
  // save desired PWM FRQ
  uint16_t flags = 0;
  flags |= motorReversed << 0;
  eeConfig.pushWord(flags);
  eeConfig.pushWord(PWMfrq);
}

void UMotor::eePromLoad()
{
  uint16_t flags = eeConfig.readWord();
  motorReversed = (flags & 0x01) > 0; 
  PWMfrq = eeConfig.readWord();
  setup();
}

void UMotor::stopAllMotors()
{ // 
  motorVoltage[0] = 0;
  motorVoltage[1] = 0;
}


