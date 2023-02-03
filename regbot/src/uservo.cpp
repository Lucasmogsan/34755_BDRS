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
#include "main.h"
#include "uservo.h"
#include "umission.h"
#include "ucontrol.h"
// #include "robot.h"
// #include "motor_controller.h"
#include "ueeconfig.h"
#include "pins.h"

#include "ustate.h"


UServo servo;

UServo::UServo()
{
//   initServo();
}


void UServo::setup()
{
  // resolution set by motor controller
  //analogWriteRes(10); /// resolution (10 bit)
  if (state.robotHWversion >= 3 and state.robotHWversion != 5)
  { // frequency is common for a motor-pin too - on HW version 3
    for (unsigned int i = 0; i < MAX_SERVO_CNT; i++)
    {
      int pin = getServoPin(i);
      pinMode(pin, OUTPUT);
      analogWriteFrequency(pin, PWMfrq); /// frequency (Hz)
      analogWrite(pin, 0);
    }
    // used by DAConverter - but may influence motor controller
    // disable servo (analog value = 0)
    for (int i = 0; i < MAX_SERVO_CNT; i++)
    {
      servoEnabled[i] = false;
      servoValue[i] = 0;
      servoVel[i] = 0;
      servoRef[i] = 0;
      servoEnaRef[i] = false;
    }
    addPublistItem("svo", "get servo configuration 'svo {enabeled pos[0...1000] vel}*5'");
  }
}

bool UServo::decode(char* buf)
{
  if (state.robotHWversion >= 3 and state.robotHWversion != 5)
  { // frequency is common for a motor-pin too - on HW version 3
    bool used = true;
    if (strncmp(buf, "svos ", 5) == 0)
    {  // new servo values
        servo.setServoConfig(&buf[5]);
    }
    else if (strncmp(buf, "servo ", 6) == 0)
    {
      servo.setOneServo(&buf[6]);
    }
    else 
      used = subscribeDecode(buf);
    return used;
  }
  else
    return false;
}

void UServo::sendHelp()
{
  if (state.robotHWversion >= 3 and state.robotHWversion != 5)
  {
    usb.send("# Servo ------\n");
    usb.send("# \tsvos  e p v  e p v ... \tset all servos e=enable, p=position +/-1024, v=velocity -1..100,-1=max\r\n");
    usb.send("# \tservo i p v\tset one servo i=index 1..5, pos +/-1024, v=velocity 0..4000,0=max, 1=sloooow\r\n");
    subscribeSendHelp();
  }
}

/**
 * send subscribed messages */
void UServo::sendData(int item)
{
  switch (item)
  {
    case 0: sendServoStatus(); break;
    default: break;
  }
}

/** 
 * \param pin allowed pin is 0,1. 
 * \param value input true is 1
 * \param enable if false, then port in set as input
 * */
void UServo::setServoPin(int8_t pin, int16_t value, bool enable)
{ // not valid for HW 41
#ifndef REGBOT_HW41
  switch (pin)
  {
    case 0:
      if (enable)
      { // output
        pinMode(SERVO_PIN_0, OUTPUT);
        pinIsOutput[0] = true;
        analogWrite(SERVO_PIN_0, value);
        pin4Value = value;
      }
      else
      {
        pinMode(SERVO_PIN_0, INPUT);
        pinIsOutput[0] = false;
        pin4Value = analogRead(SERVO_PIN_0);
//           usb.send("# pin 4 (A14) set\r\n");
      }
//         usb.send("# pin 4 (A14) set\r\n");
      break;
    case 1:
      if (enable)
      {
        pinMode(SERVO_PIN_1, OUTPUT);
        pinIsOutput[1] = true;
        digitalWriteFast(SERVO_PIN_1, value != 0);
        pin5Value = value != 0; 
//         usb.send("# pin 5 (24) set\r\n");
      }
      else
      {
        pinMode(SERVO_PIN_1, INPUT);
        pinIsOutput[1] = false;
        pin5Value = digitalRead(SERVO_PIN_1);
      }
      break;
    default:
      usb.send("# unknown pin not set\r\n");
      break;
  }
#endif
}


void UServo::setServo ( int8_t idx, int16_t value, bool enable, int8_t vel )
{
#ifdef REGBOT_HW41
  setServoPWM(idx-1, value, enable, vel);
#else
  if (idx <=3)
    setServoPWM(idx-1, value, enable, vel);
  else
    setServoPin(idx-4, value, enable);
#endif
}

void UServo::releaseServos()
{
//   usb.send("# releasing servos\r\n");
  for (int i = 0; i < MAX_SERVO_CNT; i++)
  {
    setServo(i+1, 0, false, 1);
  }
  if (MAX_SERVO_CNT < 5)
  {
    // servo 4 pin (analog)
    setServoPin(0, 0, false);
    // servo 5 pin (digital)
    setServoPin(1, 0, false);
  }
}



void UServo::sendServoStatus()
{ // return servo status
  const int MSL = 100;
  char s[MSL];
  // changed to svs rather than svo, the bridge do not handle same name 
  // both to and from robot - gets relayed back to robot (create overhead)
//   snprintf(s, MSL, "svo %d %d %d %d %d %d %d %d %d %d %d %d %d "
//   " %d %d %g %g\r\n", 
#ifdef REGBOT_HW41
  snprintf(s, MSL, "svo %d %d %d  %d %d %d  %d %d %d  %d %d %d  %d %d %d"
  "\r\n", 
  servo.servoEnabled[0], int(servo.servoValue[0]/100), servo.servoVel[0],
           servo.servoEnabled[1], int(servo.servoValue[1]/100), servo.servoVel[1],
           servo.servoEnabled[2], int(servo.servoValue[2]/100), servo.servoVel[2],
           servo.servoEnabled[3], int(servo.servoValue[3]/100), servo.servoVel[3],
           servo.servoEnabled[4], int(servo.servoValue[4]/100), servo.servoVel[4]
  );
#else
  // teensy 3.5 boards
  snprintf(s, MSL, "svo %d %d %d %d %d %d %d %d %d %d %d %d %d "
  "\r\n", 
  servo.servoEnabled[0], int(servo.servoValue[0]/100), servo.servoVel[0],
           servo.servoEnabled[1], int(servo.servoValue[1]/100), servo.servoVel[1],
           servo.servoEnabled[2], int(servo.servoValue[2]/100), servo.servoVel[2],
           servo.pinIsOutput[0], servo.pin4Value, 
           servo.pinIsOutput[1], servo.pin5Value 
           //            0, 0, distToSteerWheel, anglePer_ms
  );
#endif
  usb.send(s);
  
}


void UServo::setServoConfig(const char* line)
{
  int16_t v, e;
  uint8_t a;
  const char * p1 = line;
  for (int i = 1; i <= 5; i++)
  { // get all set of values
    e = strtol(p1, (char**)&p1, 10);
    v = strtol(p1, (char**)&p1, 10);
    a = strtol(p1, (char**)&p1, 10);
    servo.setServo(i, v, e != 0, a);
  }
}


void UServo::setOneServo(const char* line)
{
  if (state.robotHWversion >= 3 and state.robotHWversion != 5)
  {
    const char * p1 = line;
    int8_t idx = strtol(p1, (char**)&p1, 10);
    if (idx >= 1 and idx <= 6)
    {
      int us = strtol(p1, (char**)&p1, 10);
      int vel = strtol(p1, (char**)&p1, 10);
      bool enable;
      enable = us >= -1024 and us <= 1024;
      switch (idx)
      {
        case 1: 
          // servo 1 is steering, so may be active, if remote control is used - this is a conflict
          // set back to true, if rc=1, ... is used
          control.remoteControl = false;
          servo.setServoPWM(0, us, enable, vel); 
          break;
        case 2: servo.setServoPWM(1, us, enable, vel); break;
        case 3: servo.setServoPWM(2, us, enable, vel); break;
#ifdef REGBOT_HW41
        case 4: servo.setServoPWM(3, us, enable, vel); break; 
        case 5: servo.setServoPWM(4, us, enable, vel); break; 
#else
        case 4: servo.setServoPin(0, us, enable); break; // us=-1 means input (analog read)
        case 5: servo.setServoPin(1, us != 0, enable); break; // us=-1 means input
#endif
        default: break;
      }
      // debug
//       const int MSL = 75;
//       char s[MSL];
//       snprintf(s, MSL, "# %g servo %d (en=%d), '%s'\n", time,idx, enable, line);
//       usb.send(s);
      // debug end
    }
    else
      usb.send("# unknown servo: 1-3 is servo 4,5 is pins\r\n");
  }
  else
    usb.send("# supported on hardware version 3\r\n");
}


int UServo::getServoPin(int i)
{
  int pin;
  switch (i)
  {
    case 0: pin = PIN_SERVO1; break;
    case 1: pin = PIN_SERVO2; break;
    case 2: pin = PIN_SERVO3; break;
    case 3: pin = PIN_SERVO4; break;
    case 4: pin = PIN_SERVO5; break;
    default: pin = PIN_SERVO1; break;
  }
  return pin;
}


void UServo::tick()
{ // speed limit on servo
  // pt. servo 2 (index 1) handled only 
  //
  // disable servo nicely
  for (int i = 0; i < MAX_SERVO_CNT; i++)
  {
    int pin = getServoPin(i);
    if (servoEnabled[i] != servoEnaRef[i])
    { 
      if (not servoEnaRef[i])
      { // closing
        if (not digitalReadFast(pin))
        { // output is zero, time to disable PWM for servo (konstant 0)
          analogWrite(pin, 0);
          servoEnabled[i] = false;
        }
      }
      else
        servoEnabled[i] = true;
    }
  }
  //
  for (unsigned int i = 0; i < MAX_SERVO_CNT; i++)
  {
    int pin = getServoPin(i);
    // set servo position - if enabled
    if (servoEnabled[i] and (hbTimerCnt % 10 == i))
    { // often, this is a fast servo
      // velocitu
  //     0 = Fastest (servo decide)
  //     1 = 1 value per sec 
  //     2 = 2 values per sec
  //     ...
  //     999 = 999 values per second
      int e = servoRef[i]*100 - servoValue[i];
      int dw = e;
      if (abs(dw) > servoVel[i] and servoVel[i] > 0)
      {
        if (dw > 0)
          dw = servoVel[i];
        else
          dw = -servoVel[i];
      }
      if (dw != 0)
      { // implement new value
          servoValue[i] += dw;
        // midt v= 2040, min=1240, max= 2840
        // valid for HiTec HS7235-MH in high angle mode
        // v is pulsewidth in us
        int v = ((servoValue[i]/100) * msPulse/2) / 512 + midt;
        if (v < 200)
          v = 200;
        else if (v > 3250)
          v = 3250;
        analogWrite(pin, v);
        //
//         const int MSL = 100;
//         char s[MSL];
//         snprintf(s, MSL, "# setvo %d, ref=%d, e=%d, value=%ld, vel=%d, dw=%d, v=%d\n", i, servoRef[i], e, servoValue[i], servoVel[i], dw, v);
//         usb.send(s);
      }
    }
  }
  
  subscribeTick();
}

///////////////////////////////////////////////////////

void UServo::eePromSave()
{
  uint8_t flag = 0;
  // flags - no flags to save here
  if (false)
    flag +=  1 << 0;
  eeConfig.pushByte(flag);
  eeConfig.pushWord(0); // not used anymore (steering servo)
  eeConfig.pushFloat(0.0);
  eeConfig.pushFloat(0.0);
}

void UServo::eePromLoad()
{
  if (eeConfig.isStringConfig())
  {
    eeConfig.skipAddr(1 + 2 + 4 + 4);
  }
  else
  {
    /*uint8_t flag =*/ eeConfig.readByte();
    // no flags used
    // enabeled
//     servo1isSteering = (flag & (1 << 0)) != 0;
    // offset
    eeConfig.readWord();
    // distance to front wheel
    eeConfig.readFloat();
    // angle servo turns when PW change from 1 to 2 ms
    eeConfig.readFloat();
  }
}


