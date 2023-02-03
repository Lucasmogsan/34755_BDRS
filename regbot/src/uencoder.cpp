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
#include "uencoder.h"
#include "ueeconfig.h"
#include "pins.h"
#include "ustate.h"
// #include "logger.h"
#include "uusb.h"
// #include "sensor.h"
#include "ucontrol.h"
#include "umotor.h"

UEncoder encoder;


void UEncoder::setup()
{ // input should be default, but pin PIN_RIGHT_ENCODER_B on HW41 fails
//   pinMode(PIN_LEFT_ENCODER_A, INPUT);
//   pinMode(PIN_LEFT_ENCODER_B, INPUT);
//   pinMode(PIN_RIGHT_ENCODER_A, INPUT);
//   pinMode(PIN_RIGHT_ENCODER_B, INPUT);
  
  attachInterrupt ( M1ENC_A, m1EncoderA, CHANGE );
  attachInterrupt ( M2ENC_A, m2EncoderA, CHANGE );
  attachInterrupt ( M1ENC_B, m1EncoderB, CHANGE );
  attachInterrupt ( M2ENC_B, m2EncoderB, CHANGE );
  // data subscription
  addPublistItem("enc", "Get encoder value 'enc encoder interrupts' (integer)");
  addPublistItem("pose", "Get current pose 'pose t x y h' (sec,m,m,rad)");
  addPublistItem("vel", "Get velocity 'left right' (m/s)");
  addPublistItem("conf", "Get robot conf (radius, radius, gear, pulsPerRev, wheelbase, sample-time, reversed)");
}

/*  snprintf(s, MSL, "conf %.4f %.4f %.3f %u %4f %4f\r\n", odoWheelRadius[0], odoWheelRadius[1], 
    gear, pulsPerRev, odoWheelBase, SAMPLETIME
*/

void UEncoder::sendHelp()
{
  const int MRL = 300;
  char reply[MRL];
  usb.send("# Encoder settings -------\r\n");
  snprintf(reply, MRL, "# \tenc0 \tReset pose to (0,0,0)\r\n");
  usb.send(reply);
  snprintf(reply, MRL, "# \tconfw rl rr g t wb \tSet configuration (radius gear encTick wheelbase)\r\n");
  usb.send(reply);
  subscribeSendHelp();  
}


bool UEncoder::decode(const char* buf)
{
  const int MSL = 200;
  char s[MSL];
  bool used = true;
  if (subscribeDecode(buf)) {}
  else if (strncmp(buf, "enc0", 4) == 0)
  { // reset pose
    clearPose();
  }
  else if (strncmp(buf, "encd", 4) == 0)
  { // debug encoder print
    int n = eportCnt;
    snprintf(s, MSL, "encp %d\n", n);
    usb.send(s);
    for (int i = 0; i < n; i++)    
    {
      snprintf(s, MSL, "enct %d %d %d %d %d %f\n", i, eport[i][3], eport[i][0], eport[i][1], eport[i][2], edt[i]);
      usb.send(s);
    }
    eportCnt = 0;
  }
  else if (strncmp(buf, "confw ", 5) == 0)
  { // robot configuration
    const char * p1 = &buf[5];
    odoWheelRadius[0] = strtof(p1, (char**) &p1);
    odoWheelRadius[1] = strtof(p1, (char**) &p1);
    gear = strtof(p1, (char**) &p1);
    pulsPerRev = strtol(p1, (char**) &p1, 10);
    odoWheelBase = strtof(p1, (char**) &p1);
    // debug
    snprintf(s, MSL, "# got confw: r1=%g, r2=%g, G=%g, PPR=%d, WB=%g\n", odoWheelRadius[0],
             odoWheelRadius[1], gear, pulsPerRev, odoWheelBase);
    usb.send(s);
    // debug end
    if (pulsPerRev < 1)
      pulsPerRev = 1;
    if (gear < 1)
      gear = 1;
    if (odoWheelBase < 0.02)
      odoWheelBase = 0.02;
    anglePerPuls = 2.0 * M_PI / (pulsPerRev * gear);
  }
//   else if (strncmp(buf, "posec", 5) == 0)
//     clearPose();
  //   else if (strncmp(buf, "sub ", 4) == 0 /* and strstr(cmd, "gyro") != NULL*/)
//   { // subscribe service order
//     const char * p1 = &buf[4];
//     while (isspace(*p1))
//       p1++;
//     if (subs[0]->decodeSub(p1)) {}
//     else if (subs[1]->decodeSub(p1)) {}
//     else if (subs[2]->decodeSub(p1)) {}
//     else if (subs[3]->decodeSub(p1)) {}
//     else
//       used = false;
//   }
  else
    used = false;
  return used;
}

void UEncoder::sendData(int item)
{
  if (item == 0)
    sendEncStatus();
  else if (item == 1)
    sendPose();
  else if (item == 2)
    sendVelocity();
  else if (item == 3)
    sendRobotConfig();
}


void UEncoder::sendEncStatus()
{ // return esc status
  const int MSL = 100;
  char s[MSL];
  // changed to svs rather than svo, the bridge do not handle same name 
  // both to and from robot - gets relayed back to robot (create overhead)
  snprintf(s, MSL, "enc %lu %lu %d %d\r\n", encoder[0], encoder[1], intCnt, nanCnt);
  usb.send(s);
}

void UEncoder::sendRobotConfig()
{ // return esc status
  const int MSL = 100;
  char s[MSL];
  // changed to svs rather than svo, the bridge do not handle same name 
  // both to and from robot - gets relayed back to robot (create overhead)
  snprintf(s, MSL, "conf %.4f %.4f %.3f %u %.4f %.4f %d\r\n", odoWheelRadius[0], odoWheelRadius[1], 
    gear, pulsPerRev, odoWheelBase, state.SAMPLETIME, motor.motorReversed
  );
  usb.send(s);
}

void UEncoder::sendPose()
{
  const int MSL = 200;
  char s[MSL];
  snprintf(s, MSL, "pose %lu.%04lu %.3f %.3f %.4f %.4f\n",
           tsec, tusec/100, 
           pose[0], pose[1], pose[2], pose[3]);
  usb.send(s);
}

void UEncoder::sendVelocity()
{
  const int MSL = 130;
  char s[MSL];
  snprintf(s, MSL, "vel %lu.%03lu %.3f %.3f %.4f %.3f\n",
           tsec, tusec/1000, 
           wheelVelocityEst[0], wheelVelocityEst[1], robotTurnrate, robotVelocity);
  usb.send(s);
}

void UEncoder::tick()
{ // Update pose estimates
  tickCnt++;
  if (isnan(pose[0]) or isnan(pose[1]) or isnan(pose[2]))
  {
    clearPose();
    nanCnt++;
  }
  updatePose(tickCnt);
  //
  subscribeTick();
}

///////////////////////////////////////////////////////

void UEncoder::eePromSave()
{
  // save desired PWM FRQ
  //eeConfig.pushWord(PWMfrq);
  eeConfig.pushFloat(odoWheelRadius[0]);
  eeConfig.pushFloat(odoWheelRadius[1]);
  eeConfig.pushFloat(gear);
  eeConfig.pushWord(pulsPerRev);
  eeConfig.pushFloat(odoWheelBase);
  { // debug
//     const int MSL = 150;
//     char s[MSL];
//     snprintf(s, MSL, "# eeLoad %f %f %f %d %f\n", odoWheelRadius[0], odoWheelRadius[1],
//              gear, pulsPerRev, odoWheelBase);
//     usb.send(s);
  }
}

void UEncoder::eePromLoad()
{
  //  PWMfrq = eeConfig.readWord();
  odoWheelRadius[0] =  eeConfig.readFloat();
  odoWheelRadius[1] =  eeConfig.readFloat();
  gear = eeConfig.readFloat();
  pulsPerRev = eeConfig.readWord();
  odoWheelBase = eeConfig.readFloat();
  { // debug
//     const int MSL = 150;
//     char s[MSL];
//     snprintf(s, MSL, "# eeLoad %f %f %f %d %f\n", odoWheelRadius[0], odoWheelRadius[1],
//              gear, pulsPerRev, odoWheelBase);
//     usb.send(s);
  }
  if (pulsPerRev < 1)
    pulsPerRev = 1;
  if (gear < 1)
    gear = 1;
  if (odoWheelRadius[0] < 0.001)
    odoWheelRadius[0] = 0.001;
  if (odoWheelRadius[1] < 0.001)
    odoWheelRadius[1] = 0.001;
  if (odoWheelBase < 0.01)
    odoWheelBase = 0.01;
  anglePerPuls = 2.0 * M_PI / (pulsPerRev * gear);
}

void UEncoder::clearPose()
{
  pose[0] = 0;
  pose[1] = 0;
  pose[2] = 0;
  // pose[3] = 0; NB! tilt should NOT be reset
  encoder[0] = 0;
  encoder[1] = 0;
  encoderLast[0] = 0;
  encoderLast[1] = 0;
  distance = 0.0;
//   wheelPosition[0] = 0;
//   wheelPosition[1] = 0;
}


void UEncoder::updatePose(uint32_t loop)
{
  float v1, v2; // encoder tick speed (ticks/sec)
  const float    one_sec_in_cpu  = F_CPU; 
  const uint32_t half_sec_in_cpu = F_CPU/2;
  // motor 1 velocity
  uint32_t dt_cpu = ARM_DWT_CYCCNT - encStartTime_cpu[0];
  if (dt_cpu > half_sec_in_cpu)
  { // more than 0.5 sec is passed since last 
    encTimeOverload_cpu[0] = true;
  }
  if (not encTimeOverload_cpu[0] and encPeriod_cpu[0] > 0)
  {
    if (dt_cpu > encPeriod_cpu[0])
      // we are slowing down, and last full period is more than 1ms old
      v1 = one_sec_in_cpu/dt_cpu; // in pulse per sec
    else
      // last period is the most resent data and less then ta old
      // (may be older than 1ms, but the best we have)
      v1 = one_sec_in_cpu/encPeriod_cpu[0];
  }
  else 
    v1 = 0.0;
  //
  if (encCCV[0])
    wheelVelocity[0] = v1 * anglePerPuls;
  else
    wheelVelocity[0] = -v1 * anglePerPuls;
  //
  // motor 2 velocity
  // use ns (or CPU) values
  dt_cpu = ARM_DWT_CYCCNT - encStartTime_cpu[1];
  //uint32_t dt_ns = (dt_cpu * (one_sec_in_ns / centi_cec_in_cpu))/100;
  if (dt_cpu > half_sec_in_cpu)
  { // more than 0.5 sec is passed since last encoder tick
    encTimeOverload_cpu[1] = true;
  }
  if (not encTimeOverload_cpu[1] and encPeriod_cpu[1] > 0)
  {
    if (dt_cpu > encPeriod_cpu[1])
      // we are slowing down, and last full period is more than 1ms old
      v2 = one_sec_in_cpu/dt_cpu;
    else
      // last period is the most resent data and less then ta old
      // (may be older than 1ms, but the best we have)
      v2 = one_sec_in_cpu/encPeriod_cpu[1];
  }
  else
    v2 = 0;
  if (encCCV[1])
    wheelVelocity[1] = -v2 * anglePerPuls;
  else
    wheelVelocity[1] = v2 * anglePerPuls;
  //
  wheelVelocityEst[0] = wheelVelocity[0] * odoWheelRadius[0];
  wheelVelocityEst[1] = wheelVelocity[1] * odoWheelRadius[1];
  robotTurnrate = (wheelVelocityEst[1] - wheelVelocityEst[0])/odoWheelBase ;
  // 
  robotVelocity = (wheelVelocityEst[0] + wheelVelocityEst[0])/2.0;
  //
  // calculate movement and pose based on encoder count
  // encoder count is better than velocity based on time.
  // encoder position now
  uint32_t p1 = encoder[0];
  uint32_t p2 = encoder[1];
  // position change in encoder tics since last update
  int dp1 = (int32_t)p1 - (int32_t)encoderLast[0];
  int dp2 = (int32_t)p2 - (int32_t)encoderLast[1];
  // save current tick position to next time
  encoderLast[0] = p1;
  encoderLast[1] = p2;
//   if (reverseMotorVoltage)
//   {
//     dp1 *= -1;
//     dp2 *= -1;
//   }
  // angle movement with forward as positive
  float d1 =  -dp1 * anglePerPuls * odoWheelRadius[0];
  float d2 =   dp2 * anglePerPuls * odoWheelRadius[1];
  // integrate wheel position for each wheel
//   wheelPosition[0] += d1;
//   wheelPosition[1] += d2;
  // heading change in radians
  float dh = (d2 - d1) / odoWheelBase;
  // distance change in meters
  float ds = (d1 + d2) / 2.0;
  distance += ds;
  // add half the angle
  pose[2] += dh/2.0;
  // update pose position
  pose[0] += cosf(pose[2]) * ds;
  pose[1] += sinf(pose[2]) * ds;
  // add other half angle
  pose[2] += dh/2.0;
  // fold angle
  if (pose[2] > M_PI)
    pose[2] -= M_PI * 2;
  else if (pose[2] < -M_PI)
    pose[2] += M_PI * 2;
  
}


void m1EncoderA()
{ // motor 1 encoder A change
    encoder.encoderInterrupt(0,true);
    encoder.intCnt++;
//   // get timestamp now
}

void UEncoder::encoderInterrupt(int m, bool encA)
{
  uint32_t ecpu = ARM_DWT_CYCCNT;
  uint8_t pA, pB;
  if (m == 0)
  { // get port-numbers for this motor
//     pA = digitalReadFast(ACT1_ENCA_PIN);
//     pB = digitalReadFast(ACT1_ENCB_PIN);
    pA = digitalReadFast(M1ENC_A);
    pB = digitalReadFast(M1ENC_B);
  }
  else
  {
    pA = digitalReadFast(M2ENC_A);
    pB = digitalReadFast(M2ENC_B);
  }
  // read if this sensor is high or low
  if (motor.motorReversed)
  { // motor encoder and motor polarity reversed
    // the big 12V motors to RoboBot
    if (encA)
    { // encode pin A interrupt
      encCCV[m] = pA != pB;
    }
    else
    { // encode pin B interrupt
      encCCV[m] = pA == pB;
    }
  }
  else
  { // "normal" Regbot motors 
    if (encA)
    { // encode pin A interrupt
      encCCV[m] = pA == pB;
    }
    else
    { // encode pin B interrupt
      encCCV[m] = pA != pB;
    }
  }
  // dt
  // Time since last tick in cpu clocks
  uint32_t te = ecpu - encStartTime_cpu[m];
  //
  if (eportCnt < MDV)
  {
    eport[eportCnt][0] = pA;
    eport[eportCnt][1] = pB;
    eport[eportCnt][2] = encCCV[m];
    eport[eportCnt][3] = m;
    edt[eportCnt] = float(te) / (F_CPU/1000.0);
    eportCnt++;
  }    
  // save start of next encoder period
  encStartTime_cpu[m] = ecpu;
  // encoder tick time
  if (encTimeOverload_cpu[m])
  {
    encPeriod_cpu[m] = 0;
    encTimeOverload_cpu[m] = false;
  }
  else
  { // valid timing
    if (te < (F_CPU / 1000000 * 250))
      // shorter than 250 us between ticks
      // average over 4 samples
      encPeriod_cpu[m] = (encPeriod_cpu[m] * 3 + te) / 4;
    else if (te < (F_CPU / 1000000 * 500))
      // shorter than 0.5 ms average over 2 samples
      encPeriod_cpu[m] = (encPeriod_cpu[m] + te) / 2;
    else
      // longer than 0.5ms, use timing as is
      encPeriod_cpu[m] = te;
  }
  if (encCCV[m])
    encoder[m]--;
  else
    encoder[m]++;
  
}


//////////////////////////////////////////////////////////////

void m2EncoderA()
{ // motor 2 encoder A
    encoder.encoderInterrupt(1, true);
    encoder.intCnt++;
}

void m1EncoderB()
{ // motor 1 encoder pin B
    encoder.encoderInterrupt(0, false);
    encoder.intCnt++;
}

void m2EncoderB()
{ // motor 2 encoder pin B
    encoder.encoderInterrupt(1, false);
    encoder.intCnt++;
}
