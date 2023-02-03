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

#include <string.h>
#include <stdio.h>
#include "math.h"
#include "umission.h"
#include <ctype.h>
//#include "serial_com.h"
#include "main.h"
// #include "eeconfig.h"
// #include "robot.h"
#include "ucontrol.h"
#include "ulinesensor.h"
//#include "dist_sensor.h"
#include "ulog.h"
// #include "motor_controller.h"
#include "uservo.h"

#include "uusb.h"
#include "umotor.h"
#include "ustate.h"
#include "uencoder.h"
#include "uirdist.h"
#include "uencoder.h"


UMissionLine miLines[miLinesCntMax];
int miLinesCnt = 0;

char missionErrStr[missionErrStrMaxCnt];



void UMissionLine::clear()
{
  accUse = false;
  logUse = false;
  trUse = false;
  velUse = false;
  edgeLUse = false;
  edgeRUse = false;
  edgeWhiteUse = false;
  balUse = false;
  gotoUse = false;
  drivePosUse = false;
  irSensorUse = false;
  irDistRefUse = false;
  label = 0;
  eventSet = 0;
  servoID = 0; // not valid (1..5 is valid)
  servoPosition = 0;
  headUse = false;
  //
  distUse = false;
  timeUse = false;
  turnUse = false;
  countUse = false;
  xingUse = false;
//  xingUse = false;
  lineValidUse = '\0';
//   lineValidUse = '\0';
  irDist1Use = '\0';
  irDist2Use = '\0';
  tiltUse = false;
  eventMask = 0;
  logFullUse = false;
  headEndUse = '\0';
  servoVel = 0;
  velTestUse = false;
  reasonUse = '\0'; 
  // 
  valid = false;
  visits = 0;
  chirp = 0;
}

///////////////////////////////////////////////////

bool UMissionLine::finished(UMissionThread * state, 
                            uint16_t * labelNum, 
                            bool * endedAtEndAngle, 
                            char * continueReason,
                            char lastReason
                           )
{
  bool finished;
  bool condition = turnUse or distUse or timeUse or
      xingUse or xingUse or lineValidUse or //lineValidUse or
      irDist1Use or irDist2Use or eventMask or tiltUse or logFullUse;
  // test for finished with this line
  //usb.send("# Line::testFinished -> start\n");
  if (condition or gotoUse)
  { // there is a continue condition
    //usb.send("# Line::testFinished -> condition or goto\n");
    finished = (userMission.eventFlags & eventMask) > 0;
    if (finished)
      *continueReason = MC_EVENT;
    if (not finished and turnUse)
    { // avoid angle folding
      float ta = encoder.pose[2] - state->turnSumLast;
      if (ta > M_PI)
        ta -= 2* M_PI;
      else if (ta < -M_PI)
        ta += 2 * M_PI; 
      state->turnAngSum += ta;
      state->turnSumLast =encoder.pose[2];
      //finished = misAngSum * 180/M_PI > fabs(misLine->turn);
    }
    if (not finished and distUse)
    { 
      if (distUse == '<')
        finished = fabsf(encoder.distance - state->linStartDist) < dist;
      else
        finished = fabsf(encoder.distance - state->linStartDist) >= dist;
      if (finished)
        *continueReason = MC_DIST;
    }
    if (not finished and velTestUse)
    { 
      if (velTestUse == '<')
        finished = (encoder.wheelVelocityEst[0] + encoder.wheelVelocityEst[1]) / 2  < velTest;
      else
        finished = (encoder.wheelVelocityEst[0] + encoder.wheelVelocityEst[1]) / 2 >= velTest;
      if (finished)
        *continueReason = MC_VEL;
    }
    if (not finished and timeUse)
    {
      finished = time < float(hbTimerCnt - state->misStartTime)/1000.0;
      if (finished)
        *continueReason = MC_TIME;
    }
    if (turnUse)
    {
      if (not finished)
      { // acc limit in turns
        float accAng = 0;
        float turnrad = fabsf(turn/180*M_PI);
        // acc limit in turns
        if (control.rateLimit < 100.0 and control.rateLimit > 0.1)
        { // use acceleration limit to reduce vel ref in final end of turn
          float velredmax = control.mission_vel_ref * encoder.odoWheelBase / (control.mission_turn_radius + encoder.odoWheelBase / 2.0);
          float accTime = velredmax / control.rateLimit;
          // stop always 5 deg before end angle
          accAng = 0.5 * control.rateLimit * accTime * accTime / encoder.odoWheelBase + 5*M_PI/180.0;
          // hand tuning added factor 
//           accAng *= 2.2;
          if (accAng > turnrad/2.0)
            accAng = turnrad/2.0;
        }
        // stop turning 5 deg before end angle, and let heading take the rest
        finished = fabsf(state->turnAngSum) > ((fabsf(turn) - 5.0)/180*M_PI - accAng);
        if (finished)
        {
          *endedAtEndAngle = true;
          *continueReason = MC_TURN;
          // debug
//           const int MSL = 100;
//           char s[MSL];
//           snprintf(s, MSL, "# turn finished angSum=%g, accAng=%g, rad2=%g, turn=%g\n", state->turnAngSum*180/M_PI, accAng*180/M_PI, turnrad/2.0*180/M_PI, turn);
//           usb.send(s);
        }
      }
      //             else
      //               mission_turn_ref =encoder.pose[2];
    }
    if (not finished and xingUse)
    {
      if (xingUse == '<')
        finished = ls.crossingLineCnt < xingVal;
      else if (xingUse == '>')
        finished = ls.crossingLineCnt > xingVal;
      else 
        finished = ls.crossingLineCnt == xingVal;
      if (finished)
        *continueReason = MC_XING;
    }
    if (not finished and lineValidUse)
    { // left valid test
      if (lineValidUse == '<')
        finished = ls.lsEdgeValidCnt < lineValidVal;
      else if (lineValidUse == '>')
        finished = ls.lsEdgeValidCnt > lineValidVal;
      else
        finished = ls.lsEdgeValidCnt == lineValidVal;
      if (finished)
        *continueReason = MC_LINE_VALID;
    }
    if (not finished and irDist1Use)
    {
      if (irDist1Use == '<')
        finished = irdist.irDistance[0] < irDist1;
      else
        finished = irdist.irDistance[0] >= irDist1;
      if (finished)
        *continueReason = MC_IR_DIST1;
    }
    if (not finished and irDist2Use)
    {
      if (irDist2Use == '<')
        finished = irdist.irDistance[1] < irDist2;
      else
        finished = irdist.irDistance[1] >= irDist2;
      if (finished)
        *continueReason = MC_IR_DIST2;
    }
    if (not finished and logFullUse)
    {
      finished = logger.logFull;
      if (finished)
        *continueReason = MC_LOG;
    }
    if (not finished and tiltUse)
    {
      if (tiltUse == '<')
        finished = encoder.pose[3] < tilt;
      else if (tiltUse == '>')
        finished = encoder.pose[3] >= tilt;
      else
        finished = fabs(tilt - encoder.pose[3]) < 1/180*M_PI;
      if (finished)
        *continueReason = MC_TILT;
    }
    if (not finished and headEndUse)
    {
      if (headEndUse == '<')
        finished = encoder.pose[2] < headEndValue;
      else if (headEndUse == '>')
        finished = encoder.pose[2] > headEndValue;
      else
      { // this heading reached
        float d = headEndValue -encoder.pose[2];
        if (d > M_PI)
          d -= 2*M_PI;
        if (d < -M_PI)
          d += 2*M_PI;
        finished = fabs(d) < (3 * M_PI / 180.0);
      }
      if (finished)
        *continueReason = MC_HEADING;
    }
    if (not finished and reasonUse)
    { // intended for use with '!' and goto, where goto is executed only if reason match,
      // in other lines a '=' is more likely to skip this line too
      if (reasonUse == '!')
        finished = reasonValue != lastReason;
      else if (reasonUse == '<')
        finished = reasonValue < lastReason;
      else if (reasonUse == '>')
        finished = reasonValue > lastReason;
      else
        finished = reasonValue == lastReason;
      if (finished)
        *continueReason = MC_REASON;
    }
    // with goto lines as the exception
    if (gotoUse)
    { // this is a goto-line
      if (true)
      { // count is incremented already
        if ((visits <= count or not countUse) and not finished)
        { // we need to jump, return the label number
          *labelNum = gotoDest;
        }
        else
        { // reset count
          visits = 0;
        }
      }
      finished = true;
    }
  }
  else
  { // no condition (or just count), so continue right away
    finished = true;
    //usb_write("# mission line - no condition\n");
  }
  return finished;
}

///////////////////////////////////////////////////////

void UMissionLine::postProcess(float lineStartAngle, bool endAtAngle)
{
  // turn off one-liner controls
  // edge follow, wall follow and turn
  if (turnUse or edgeLUse or edgeRUse or trUse or irSensorUse)
  {
    if (headEndUse and headEndUse == '=')
    {
      control.mission_turn_ref = headEndValue * M_PI / 180;
    }
    else if (turnUse and endAtAngle)
    { // ended at an angle condition, so we know
      // that new angle ref should be exactly this angle
      control.mission_turn_ref = lineStartAngle + turn * M_PI / 180.0;
      while (control.mission_turn_ref > M_PI)
        control.mission_turn_ref -= 2 * M_PI;
      while (control.mission_turn_ref < -M_PI)
        control.mission_turn_ref += 2 * M_PI;
    }
    else 
    { // not a specific turn angle, but turning allowed, so use 
      // current heading as new reference
      control.mission_turn_ref =encoder.pose[2];
    }
    if (edgeLUse or edgeRUse)
    {
      control.regul_line_use = false;
      ls.lsPostProcess();
    }
    else if (trUse)
      control.mission_turn_do = false;
    if (irSensorUse)
    { // ir-sensor drive control ended
      // both wall follow and velocity follow
      control.mission_irSensor_use = false;
      control.mission_wall_turn = false;
    }    
    // make sure old turn reference values are gone
    control.resetTurn();
  }
  // use of position controller is a one-liner also
  if (drivePosUse)
  {
    control.mission_pos_use = false;
    control.mission_vel_ref = 0;
  }
}



///////////////////////////////////////////////////////

void UMissionLine::implementLine()
{ // implement all line parameters
  // and note start state.
  //
  // tell control that control type may have changed
//   historyInvalid = false;
  // logging may change
  if (logUse and not logger.logFull)
  { // logging may have changed
    if (log > 0)
    { // (re)start logging
      logger.startLogging(log, false);
    }
    else
    { // pause logging, when set to log=0
      logger.stopLogging();
    }
  }
  if (chirp > 0 and not control.chirpRun)
  { // 
    control.chirpStart(chirp, headUse);
    // debug
    const int MSL = 50;
    char s[MSL];
    snprintf(s, MSL, "#chirp start chirp=%d, head=%d, interval=%d, frq=%g\n", chirp, headUse, logger.logInterval, control.chirpFrq);
    usb.send(s);
    // debug end
  }
  if (balUse)
    control.balance_active = bal;
  //
  if (irSensorUse)
  { // there is new 
    irdist.setIRpower(true);
    if (irSensor == 1 and not control.mission_wall_turn)
    { // sensor 1 is for wall follow
      control.mission_wall_turn = true; // turn is sensor 1 (side looking)
      // remove old historic values
      control.ctrlWallTurn->resetControl();
    }
    else if (control.mission_wall_turn)// sensor 2
    { // we are dooing - follow the leader - i.e. speed control
      control.mission_wall_turn = false; // is sensor 2 (fwd looking)
      control.ctrlWallVel->resetControl();
    }
    if (irDistRefUse)
    {
      control.mission_wall_ref = irDistRef;
      control.mission_wall_vel_ref = irDistRef;
    }
    // use ir sensor value as bolean 
    // irsensor=1: turn (follow wall), 
    // irsensor=2: distance based on sensor 2 only, 
    // irsensor=3: distance based on both irsensor 1 and 2 (minimum)
    control.mission_irSensor_use = irSensor;
  }
  //
  if (velUse)
  {  // change velocity
    control.mission_vel_ref = vel;
    //usb.send("# set mission_vel_ref\n");
  }
  if (drivePosUse)
  {
    control.mission_pos_ref = drivePos;
    control.misStartDist = encoder.distance;
    if (not control.mission_pos_use)
    {
      control.mission_pos_use = true;
      control.ctrlPos->resetControl();
    }
  }
  // edge/line sensor
  if (xingUse or lineValidUse or edgeLUse or edgeRUse)
  {  // turn on sensor for crossing detect
    ls.lineSensorOn = true;
    if (edgeLUse or edgeRUse)
    { // Remove potentially old history values - inclusive integrators
      // as edge, color or offset may have changed
      //
      // resetControl seems to make things worse, when driving at differet speed, but on same line
      // control.ctrlEdge->resetControl();
      //
      // activate control
      control.regul_line_use = true;
      // set edge to follow
      control.mission_line_LeftEdge = edgeLUse;
      // set offset to edge
      control.mission_line_ref = edgeRef;
      // line color
      if (edgeWhiteUse)
        ls.lsIsWhite = edgeWhite;
    }
  }
  // turn control
  if (trUse)
  { // turn radius in meters positive i left
    control.mission_turn_radius = tr;
    control.mission_turn_do = true;
    // set turn angle
    control.mission_tr_turn = turn;
    // reduce reference velocity
    // mission_vel_ref_preturn = mission_vel_ref;
    // reduce reference (mid-point) velocity and allow outher wheel to keep old velocity
    // especially when turn radius is very small -> 0
    // ¤¤¤              mission_vel_ref = mission_vel_ref_preturn * mission_turn_radius / 
    //                 (mission_turn_radius + odoWheelBase/2.0);
  }
  if (accUse)
    control.setRateLimit(acc);
  if (eventSet > 0)
  { // there is at least one user event, activate
    for (int i = 0; i < 32; i++)
    { // test all 32 possible user events
      if ((eventSet & (1 << i)) > 0)
        userMission.setEvent(i);
    }
  }
  if (servoID > 0)
  {
    servo.setServo(servoID, servoPosition, true, servoVel);
  }
  if (headUse)
  { // set reference heading
    control.mission_turn_ref = headValue * M_PI / 180.0;
    while (control.mission_turn_ref >= M_PI)
      control.mission_turn_ref -= 2 * M_PI;
    while (control.mission_turn_ref < -M_PI)
      control.mission_turn_ref += 2 * M_PI;
  }
}


///////////////////////////////////////////////////////

int UMissionLine::toString(char* bf, int bfCnt, bool frame)
{
  char * ms = bf;
  char * mc;
  int n = 0;
  const char sep = ',';
  if (not valid)
  {
    strncpy(ms, "\r# not a valid line", bfCnt - n);
    n += strlen(ms);
    ms = &bf[n];
  }
  else if (frame)
  {
    strcpy(ms, "mline ");
    n+=6;
    ms = &bf[n];
  }
  if (velUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n, "vel=%g", vel);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (accUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n, "acc=%g", acc);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (trUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "tr=%g", tr);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (edgeLUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "edgeL=%g", edgeRef);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (edgeRUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "edgeR=%g", edgeRef);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (edgeWhiteUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "white=%d", edgeWhite);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (logUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "log=%g", log);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (balUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "bal=%d", bal);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (irSensorUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "irsensor=%d", irSensor);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (irDistRefUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "irdist=%g", irDistRef);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (drivePosUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "topos=%g", drivePos);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (headUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "head=%g", headValue);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (chirp > 0)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "chirp=%g", chirp/100.0);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (servoID > 0)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "servo=%d", servoID);
    n += strlen(ms);
    ms = &bf[n];
    *ms++=sep; n++;
    snprintf(ms, bfCnt - n - 1, "pservo=%d", servoPosition);
    n += strlen(ms);
    ms = &bf[n];
    *ms++=sep; n++;
    snprintf(ms, bfCnt - n - 1, "vservo=%d", servoVel);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (label > 0)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "label=%d", label);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (gotoUse)
  {
    if (n > 7) {*ms++=sep; n++;}
    snprintf(ms, bfCnt - n - 1, "goto=%d", gotoDest);
    n += strlen(ms);
    ms = &bf[n];
  }
  if (eventSet)
  { // there  is at least one event set on this line
    // 
    for (int i = 0; i < 32; i++)
    { // test all possible events
      if (eventSet & (1 << i))
      { // event i is set, add to line
        if (n > 7) 
        { // add ','
          *ms++=sep; n++;
        }
        snprintf(ms, bfCnt - n - 1, "event=%d", i);
        n += strlen(ms);
        ms = &bf[n];
      }
    }
  }
  // now the condition part - if it exist
  mc = ms;
  *mc++ = ':';
  n++;
  if (distUse)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "dist%c%g", distUse, dist);
    n += strlen(mc);
    mc = &bf[n];
  }
  if (velTestUse)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "vel%c%g", velTestUse, velTest);
    n += strlen(mc);
    mc = &bf[n];
  }
  if (timeUse)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "time%c%g", timeUse, time);
    n += strlen(mc);
    mc = &bf[n];
  }
  if (turnUse)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "turn%c%g", turnUse, turn);
    n += strlen(mc);
    mc = &bf[n];
  }
  if (countUse)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "count%c%d", countUse, count);
    n += strlen(mc);
    mc = &bf[n];
  }
  if (xingUse)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "xl%c%d", xingUse, xingVal);
    n += strlen(mc);
    mc = &bf[n];
  }
  if (lineValidUse)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "lv%c%d", lineValidUse, lineValidVal);
    n += strlen(mc);
    mc = &bf[n];
  }
  if (irDist1Use)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "ir1%c%g", irDist1Use, irDist1);
    n += strlen(mc);
    mc = &bf[n];
  }
  if (irDist2Use)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "ir2%c%g", irDist2Use, irDist2);
    n += strlen(mc);
    mc = &bf[n];
  }
  if (tiltUse)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "tilt%c%g", tiltUse, tilt);
    n += strlen(mc);
    mc = &bf[n];
  }
  if (headEndUse)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "head%c%g", headEndUse, headEndValue);
    n += strlen(mc);
    mc = &bf[n];
  }
  if (eventMask)
  {
    for (int i = 0; i < 32; i++)
    {
      if (eventMask & (1 << i))
      {
        if (n > 7) {*mc++=sep; n++;}
        snprintf(mc, bfCnt - n - 1, "event=%d", i);
        n += strlen(mc);
        mc = &bf[n];
      }
    }
  }
  if (logFullUse)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "log=0");
    n += strlen(mc);
    mc = &bf[n];
  }
  if (reasonUse)
  {
    if (mc - ms > 1) {*mc++=sep; n++;}
    snprintf(mc, bfCnt - n - 1, "last%c%d", reasonUse, reasonValue - MC_DIST);
    n += strlen(mc);
    mc = &bf[n];
//     usb.send("#send a reason line\n");
  }
  if (frame)
  {
    strncpy(mc, "\r\n", bfCnt - n - 1);
    n += strlen(mc);
  }
  return n;
}



/////////////////////////////////////////////////

int UMissionLine::toTokenString(char* bf, int bfCnt)
{
  char * ms = bf;
  char * mc;
  int n = 0;
//   const char sep = ',';
  if (not valid)
  {
    ms[0] = '\0';
  }
  else
  {
    if (velUse)
    {
      snprintf(ms, bfCnt - n, "%c%g", MP_VEL, vel);
      n += strlen(ms);
      ms = &bf[n];
    }
    if (accUse)
    {
      snprintf(ms, bfCnt - n, "%c%g", MP_ACC, acc);
      n += strlen(ms);
      ms = &bf[n];
    }
    if (trUse)
    {
      snprintf(ms, bfCnt - n - 1, "%c%g", MP_TR, tr);
      n += strlen(ms);
      ms = &bf[n];
    }
    if (edgeWhiteUse)
    {
      snprintf(ms, bfCnt - n - 1, "%c%d", MP_EDGE_WHITE, edgeWhite);
      n += strlen(ms);
      ms = &bf[n];
    }
    if (edgeLUse)
    {
      snprintf(ms, bfCnt - n - 1, "%c%g", MP_EDGE_L, edgeRef);
      n += strlen(ms);
      ms = &bf[n];
    }
    if (edgeRUse)
    {
      snprintf(ms, bfCnt - n - 1, "%c%g", MP_EDGE_R, edgeRef);
      n += strlen(ms);
      ms = &bf[n];
    }
    if (logUse)
    {
      snprintf(ms, bfCnt - n - 1, "%c%g", MP_LOG, log);
      n += strlen(ms);
      ms = &bf[n];
    }
    if (balUse)
    {
      snprintf(ms, bfCnt - n - 1, "%c%d", MP_BAL, bal);
      n += strlen(ms);
      ms = &bf[n];
    }
    if (irSensorUse)
    {
      snprintf(ms, bfCnt - n - 1, "%c%d", MP_IR_SENSOR, irSensor);
      n += strlen(ms);
      ms = &bf[n];
    }
    if (irDistRefUse)
    {
      snprintf(ms, bfCnt - n - 1, "%c%g", MP_IR_DIST, irDistRef);
      n += strlen(ms);
      ms = &bf[n];
    }
    if (drivePosUse)
    {
      snprintf(ms, bfCnt - n - 1, "%c%g", MP_DRIVE_DIST, drivePos);
      n += strlen(ms);
      ms = &bf[n];
    }
    if (label > 0)
    {
      snprintf(ms, bfCnt - n - 1, "%c%d", MP_LABEL, label);
      n += strlen(ms);
      ms = &bf[n];
    }
    if (gotoUse)
    {
      snprintf(ms, bfCnt - n - 1, "%c%d", MP_GOTO, gotoDest);
      n += strlen(ms);
      ms = &bf[n]; 
    }
    if (eventSet)
    {
      snprintf(ms, bfCnt - n - 1, "%c%lu", MP_EVENT, eventSet);
      n += strlen(ms);
      ms = &bf[n];
    }      
    if (headUse)
    {
      snprintf(ms, bfCnt - n - 1, "%c%g", MP_HEADING, headValue);
      n += strlen(ms);
      ms = &bf[n];
    }      
    if (chirp > 0)
    {
      snprintf(ms, bfCnt - n - 1, "%c%d", MP_CHIRP, chirp);
      n += strlen(ms);
      ms = &bf[n];
    }      
    if (servoID > 0)
    {
      snprintf(ms, bfCnt - n - 1, "%c%d", MP_SERVO, servoID);
      n += strlen(ms);
      ms = &bf[n];
      snprintf(ms, bfCnt - n - 1, "%c%d", MP_SERVO_POS, servoPosition);
      n += strlen(ms);
      ms = &bf[n];
      snprintf(ms, bfCnt - n - 1, "%c%d", MP_SERVO_VEL, servoVel);
      n += strlen(ms);
      ms = &bf[n];
    }      
    // now the condition part - if it exist
    mc = ms;
    *mc++ = ':';
    n++;
    if (distUse)
    {
      snprintf(mc, bfCnt - n - 1, "%c%c%g", MC_DIST, distUse, dist);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (velTestUse)
    {
      snprintf(mc, bfCnt - n - 1, "%c%c%g", MC_VEL, velTestUse, velTest);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (timeUse)
    {
      snprintf(mc, bfCnt - n - 1, "%c%c%g", MC_TIME, timeUse, time);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (turnUse)
    {
      snprintf(mc, bfCnt - n - 1, "%c%c%g", MC_TURN, turnUse, turn);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (countUse)
    {
      snprintf(mc, bfCnt - n - 1, "%c%c%d", MC_COUNT, countUse, count);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (xingUse)
    {
      snprintf(mc, bfCnt - n - 1, "%c%c%d", MC_XING, xingUse, xingVal);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (lineValidUse)
    {
      snprintf(mc, bfCnt - n - 1, "%c%c%d", MC_LINE_VALID, lineValidUse, lineValidVal);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (irDist1Use)
    {
      snprintf(mc, bfCnt - n - 1, "%c%c%g", MC_IR_DIST1, irDist1Use, irDist1);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (irDist2Use)
    {
      snprintf(mc, bfCnt - n - 1, "%c%c%g", MC_IR_DIST2, irDist2Use, irDist2);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (tiltUse)
    {
      snprintf(mc, bfCnt - n - 1, "%c%c%g", MC_TILT, tiltUse, tilt);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (logFullUse)
    {
      snprintf(mc, bfCnt - n - 1, "%c0", MC_LOG);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (eventMask)
    {
      snprintf(mc, bfCnt - n - 1, "%c%lu", MC_EVENT, eventMask);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (headEndUse)
    {
      snprintf(mc, bfCnt - n - 1, "%c%c%g", MC_HEADING, headEndUse, headEndValue);
      n += strlen(mc);
      mc = &bf[n];
    }
    if (reasonUse)
    {
      snprintf(mc, bfCnt - n - 1, "%c%c%d", MC_REASON, reasonUse, reasonValue - MC_DIST);
      n += strlen(mc);
      mc = &bf[n];
//       usb.send("#Mission send a reason line\n");
    }
    strncpy(mc, "\n", bfCnt - n - 1);
    n += strlen(mc);
  }
  return n;
}




////////////////////////////////////////////////////////////////

bool UMissionLine::decodeLine(const char* buffer, int16_t * threadNumber)
{
  char * p1 = (char *)buffer;
  char * p2 = strchr(p1, ':');
  char * p3 = strchr(p1, '=');
  bool err = false;
//  int evCnt = 0; // debug
  // reset use flags
  clear();
  missionErrStr[0] = '\0';
  // strip white space
  while (*p1 <= ' ' and *p1 > '\0') p1++;
  // find all parameters until ':'
  while ((p1 < p2 or p2 == NULL) and p3 != NULL and not err)
  {
    if (strncmp (p1, "acc", 3) == 0)
    {
      accUse = true;
      acc = strtof(++p3, &p1);
    }
    else if (strncmp (p1, "vel", 3) == 0)
    {
      velUse = true;
      vel = strtof(++p3, &p1);
    }
    else if (strncmp(p1, "tr", 2) == 0)
    {
      trUse = true;
      tr = fabsf(strtof(++p3, &p1));
    }
    else if (strncmp(p1, "edgel", 5) == 0 or strncmp(p1, "liner", 5) == 0)
    {
      edgeLUse = true;
      edgeRef = strtof(++p3, &p1);
      if (edgeRef > 2.0)
        edgeRef = 2.0;
      if (edgeRef < -2.0)
        edgeRef = -2.0;
    }
    else if (strncmp(p1, "edger", 5) == 0 or strncmp(p1, "liner", 5) == 0)
    {
      edgeRUse = true;
      edgeRef = strtof(++p3, &p1);
      if (edgeRef > 2.0)
        edgeRef = 2.0;
      if (edgeRef < -2.0)
        edgeRef = -2.0;
    }
    else if (strncmp(p1, "white", 5) == 0)
    {
      edgeWhiteUse = true;
      edgeWhite = strtol(++p3, &p1, 10);
      if (edgeWhite)
        edgeWhite = true;
    }
    else if (strncmp (p1, "log", 3) == 0)
    {
      logUse = true;
      log = strtof(++p3, &p1);
    }
    else if (strncmp (p1, "bal", 3) == 0)
    {
      balUse = true;
      bal = strtof(++p3, &p1) > 0.5;
    }
    else if (strncmp (p1, "irsensor", 8) == 0)
    {
      irSensorUse = true;
      irSensor = strtol(++p3, &p1, 10);
      if (irSensor < 1)
        // default is follow wall
        irSensor = 1;
    }
    else if (strncmp (p1, "irdist", 6) == 0)
    {
      irDistRefUse = true;
      irDistRef = strtof(++p3, &p1);
    }
    else if (strncmp (p1, "topos", 5) == 0)
    {
      drivePosUse = true;
      drivePos = strtof(++p3, &p1);
    }
    else if (strncmp (p1, "label", 5) == 0)
    {
      label = strtol(++p3, &p1, 10);
    }
    else if (strncmp (p1, "goto", 4) == 0)
    {
      gotoUse = true;
      gotoDest = strtol(++p3, &p1, 0);
    }
    else if (strncmp (p1, "thread", 6) == 0)
    {
      if (threadNumber != NULL)
        *threadNumber = strtol(++p3, &p1, 0);
    }
    else if (strncmp (p1, "event", 5) == 0)
    {
      int s = strtol(++p3, &p1, 0);
      if (s >= 0 and s < 32)
        eventSet |= 1 << s;
    }
    else if (strncmp (p1, "head", 4) == 0)
    {
      headUse = true;
      headValue = strtof(++p3, &p1);
    }
    else if (strncmp (p1, "chirp", 5) == 0)
    { // convert to deci values (to save line space)
      chirp = int(strtof(++p3, &p1) * 100);
    }
    else if (strncmp (p1, "servo", 5) == 0)
    {
      int s = strtol(++p3, &p1, 0);
      if (s <= 5 and s > 0)
      {
        servoID = s;
      }
    }
    else if (strncmp (p1, "pservo", 6) == 0)
    {
      int s = strtol(++p3, &p1, 0);
      servoPosition = s;
    }
    else if (strncmp (p1, "vservo", 6) == 0)
    {
      int s = strtol(++p3, &p1, 0);
      servoVel = s;
    }
    else
    { // error, just skip
      snprintf(missionErrStr, missionErrStrMaxCnt, "failed parameter at %s", p1);
      p1 = ++p3;
      err = true;
    }
    // remove white space
    while ((*p1 <= ' ' or *p1 == ',') and *p1 > '\0') p1++;
    p3 = strchr(p1, '=');
    
  }
  // now the part after the ':', where p2 is pointing.
  // - if there is a ':'
  if (p2 != NULL)
  { // there might be a condition part
    p1 = p2 + 1;
    while (*p1 <= ' ' and *p1 > '\0') p1++;
    p3 = p1; // start of token
    // skip token until operator or reached end or next parameter (error)
    while (strchr("=<>!", *p3) == NULL and *p3 >= ' ' and *p3 != ',') p3++;
    if (strchr("=<>!", *p3) == NULL)
    { // not a valid operator
      snprintf(missionErrStr, missionErrStrMaxCnt, "invalid operator %s (pos %d)", p3, p3 - buffer);
      err = true;
    }
    while (*p1 > '\0' and p3 != NULL and not err)
    {
      if (strncmp (p1, "dist", 4) == 0)
      { // distance is always positive (even if reversing)
        distUse = *p3;
        dist = fabsf(strtof(++p3, &p1));
      }
      else if (strncmp (p1, "vel", 3) == 0)
      { // average velocity of robot center (including sign)
        velTestUse = *p3;
        velTest = strtof(++p3, &p1);
      }
      else if (strncmp (p1, "turn", 4) == 0)
      {
        turnUse = *p3;
        turn = strtof(++p3, &p1);
      }
      else if (strncmp (p1, "time", 4) == 0)
      {
        timeUse = *p3;
        time = strtof(++p3, &p1);
      }
      else if (strncmp (p1, "count", 4) == 0)
      {
        countUse = *p3;
        count = strtol(++p3, &p1, 10);
      }
      else if (strncmp (p1, "xl", 2) == 0)
      {
        xingUse = *p3;
        xingVal = strtol(++p3, &p1, 10);
      }
      else if (strncmp (p1, "lv", 2) == 0)
      {
        lineValidUse = *p3;
        lineValidVal = strtol(++p3, &p1, 10);
      }
      else if (strncmp (p1, "ir1", 3) == 0)
      {
        irDist1Use = *p3;
        irDist1 = strtof(++p3, &p1);
      }
      else if (strncmp (p1, "ir2", 3) == 0)
      {
        irDist2Use = *p3;
        irDist2 = strtof(++p3, &p1);
      }
      else if (strncmp (p1, "tilt", 4) == 0)
      {
        tiltUse = *p3;
        tilt = strtof(++p3, &p1);
      }
      else if (strncmp (p1, "log", 3) == 0)
      {
        logFullUse = true;
        strtol(++p3,&p1, 10);//skip zero (or any number)
        //p1 = p3 + 2; // skip zero
      }
      else if (strncmp (p1, "event", 5) == 0)
      {
        int s = strtol(++p3, &p1, 0);
        if (s < 32 and s >= 0)
          eventMask |= 1 << s;
//         evCnt++;
      }
      else if (strncmp (p1, "head", 4) == 0)
      {
        headEndUse = *p3;
        headEndValue = strtof(++p3, &p1);
      }
      else if (strncmp (p1, "last", 4) == 0)
      {
        if (*p3 == '!' or *p3 == '<' or *p3 == '>')
          reasonUse = *p3;
        else
          reasonUse = '=';
        reasonValue = strtol(++p3, &p1, 10) + MC_DIST;
//         usb.send("#found a last\n");
      }
      else
      { // error, just skip
        snprintf(missionErrStr, missionErrStrMaxCnt, "failed condition at %s (pos %d)", p1, p1 - buffer);
        p1 = ++p3;
        err = true;
      }
      // remove white space
      while ((*p1 <= ' ' or *p1 == ',') and *p1 > '\0') p1++;
      p3 = p1; // start of token
      // skip token until operator or reached end or next parameter (error)
      while (strchr("=<>!", *p3) == NULL and *p3 >= ' ' and *p3 != ',') p3++;
      if (strchr("=<>!", *p3) == NULL)
      { // not a valid operator
        snprintf(missionErrStr, missionErrStrMaxCnt, "invalid operator 2 %s (pos %d)", p3, p3 - buffer);
        err = true;
      }
    }
  }
  valid = not err;
  return valid;
}

bool UMissionLine::decodeToken(const char* buffer)
{
  char * p1 = (char *)buffer;
  char * p2 = strchr(p1, ':');
  bool err = false;
  uint8_t n = 255;
  // reset use flags
  clear();
  // find all parameters until ':'
  while ((p1 < p2 or p2==NULL) and *p1 >=' ')
  {
    if (*p1 == MP_ACC) 
    {
      accUse = true;
      acc = strtof(++p1, &p1);
    }
    else if (*p1 == MP_VEL)
    {
      velUse = true;
      vel = strtof(++p1, &p1);
    }
    else if (*p1 == MP_TR)
    {
      trUse = true;
      tr = fabsf(strtof(++p1, &p1));
    }
    else if (*p1 == MP_EDGE_L)
    {
      edgeLUse = true;
      edgeRef = strtof(++p1, &p1);
    }
    else if (*p1 == MP_EDGE_R)
    {
      edgeRUse = true;
      edgeRef = strtof(++p1, &p1);
    }
    else if (*p1 == MP_EDGE_WHITE)
    {
      edgeWhiteUse = true;
      edgeWhite = strtol(++p1, &p1, 10);
    }
    else if (*p1 == MP_LOG)
    {
      logUse = true;
      log = strtof(++p1, &p1);
    }
    else if (*p1 == MP_BAL)
    {
      balUse = true;
      bal = strtol(++p1, &p1, 10);
    }
    else if (*p1 == MP_IR_SENSOR)
    {
      irSensorUse = true;
      irSensor = strtol(++p1, &p1, 10);
    }
    else if (*p1 == MP_IR_DIST)
    {
      irDistRefUse = true;
      irDistRef = strtof(++p1, &p1);
    }
    else if (*p1 == MP_DRIVE_DIST)
    {
      drivePosUse = true;
      drivePos = strtof(++p1, &p1);
    }
    else if (*p1 == MP_LABEL)
    {
      label = strtol(++p1, &p1, 10);
    }
    else if (*p1 == MP_GOTO)
    {
      gotoUse = true;
      gotoDest = strtol(++p1, &p1, 10);
    }
    else if (*p1 == MP_EVENT)
    { // 0 2147483648 = 0x80000000
      eventSet = strtoll(++p1, &p1, 10);
    }
    else if (*p1 == MP_HEADING)
    {
      headUse = true;
      headValue = strtof(++p1, &p1);
    }
    else if (*p1 == MP_CHIRP)
    {
      chirp = strtol(++p1, &p1, 10);
    }
    else if (*p1 == MP_SERVO)
    {
      servoID = strtof(++p1, &p1);
    }
    else if (*p1 == MP_SERVO_POS)
    {
      servoPosition = strtof(++p1, &p1);
    }
    else if (*p1 == MP_SERVO_VEL)
    {
      servoVel = strtof(++p1, &p1);
    }
    else if (*p1 > ' ')
    { // error, just skip
      err = true;
      snprintf(missionErrStr, missionErrStrMaxCnt, "failed line P at %s\n", p1);
      break;
    }
    // remove seperator
//     if (*p1 == ',')
//       p1++;
  }
  if (p2 != NULL and not err)
  {
    p1 = p2 + 1;
    while (*p1 > ' ')
    {
      // debug
//       snprintf(s, MSL, "#pi1=%s\n", p1);
//       usb.send(s);
      // debug end
      if (*p1 == MC_DIST)
      { // distance is always positive (even if reversing)
        distUse = *(++p1);
        dist = fabsf(strtof(++p1, &p1));
      }
      else if (*p1 == MC_VEL)
      { // distance is always positive (even if reversing)
        velTestUse = *(++p1);
        velTest = strtof(++p1, &p1);
      }
      else if (*p1 == MC_TURN)
      {
        turnUse = *(++p1);
        turn = strtof(++p1, &p1);
      }
      else if (*p1 == MC_TIME)
      {
        timeUse = *(++p1);
        time = strtof(++p1, &p1);
      }
      else if (*p1 == MC_COUNT)
      {
        countUse = *(++p1);;
        count = strtol(++p1, &p1, 10);
      }
      else if (*p1 == MC_XING)
      {
        xingUse = *(++p1);
        xingVal = strtol(++p1, &p1, 10);
      }
//       else if (*p1 == MC_XINGW)
//       {
//         xingUse = *(++p1);
//         xingVal = strtol(++p1, &p1, 10);
//       }
      else if (*p1 == MC_LINE_VALID)
      {
        lineValidUse = *(++p1);
        lineValidVal = strtol(++p1, &p1, 10);
      }
      else if (*p1 == MC_LINE_VALID)
      {
        lineValidUse = *(++p1);
        lineValidVal = strtol(++p1, &p1, 10);
      }
      else if (*p1 == MC_IR_DIST1)
      {
        irDist1Use = *(++p1);
        irDist1 = strtof(++p1, &p1);
      }
      else if (*p1 == MC_IR_DIST2)
      {
        irDist2Use = *(++p1);
        irDist2 = strtof(++p1, &p1);
      }
      else if (*p1 == MC_TILT)
      {
        tiltUse = *(++p1);
        tilt = strtof(++p1, &p1);
      }
      else if (*p1 == MC_LOG)
      {
        logFullUse = true;
        p1++;
//         p1++;
        // ignore value
        while (isdigit(*p1))
          p1++;
      }
      else if (*p1 == MC_EVENT)
      {
        eventMask = strtoll(++p1, &p1, 10);
      }
      else if (*p1 == MC_HEADING)
      {
        headEndUse = *(++p1);
        headEndValue = strtof(++p1, &p1);
      }
      else if (*p1 == MC_REASON)
      {
        reasonUse = *(++p1);
        reasonValue = strtol(++p1, &p1, 10) + MC_DIST;
      }
      else
      { // error, just skip
        err = true;
        snprintf(missionErrStr, missionErrStrMaxCnt, "failed line C at %s (n=%d)\n", p1, n);
        break;
      }
//       if (*p1 > ' ' and *p1 == ',')
//         p1++;
    }
  }
  valid = not err;
  return valid;
}

