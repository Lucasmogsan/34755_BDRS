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
#include "main.h"
#include "ueeconfig.h"
// #include "robot.h"
#include "ucontrol.h"
#include "ulinesensor.h"
#include "ulog.h"
// #include "motor_controller.h"
#include "uservo.h"

#include "uusb.h"
#include "umotor.h"
#include "ustate.h"
#include "uencoder.h"
#include "uirdist.h"

UMission userMission;

// UMissionLine miLines[miLinesCntMax];
// int miLinesCnt = 0;
// 
// char missionErrStr[missionErrStrMaxCnt];


void UMission::setup()
{
  clear(); // clear old mission lines
  addPublistItem("mget", "Get all mission lines");
  addPublistItem("mtok", "Get all mission lines as tokens (debug feature)");
  /*mt->threadNr, mt->threadActive, mt->misLineNum, mt->lineCnt, mt->theEnd, mt->activateFlag, mt->deactivateFlag);*/
  addPublistItem("mstat", "Get mission status for all threads (mstat thNum Active Lnum Lcnt atEnd AcOn DeAkOn)");
  addPublistItem("mevent", "Get event status ");
  addPublistItem("mis", "Get short mission status (mis active thread line ctrlActive)");
  addPublistItem("mmax", "Get used and maximum mission lines and threads");
}

bool UMission::decode(const char* buf)
{
  bool used = true;
  if (strncmp(buf, "mclear", 6) == 0)
    clear();
  else if (strncmp(buf, "madd ", 5) == 0)
  {
    addLine(&buf[5]);
    // debug
    //       snprintf(s, MSL, "# add %s\r\n", &buf[4]);
    //       usb.send(s);
    // debug end
  }
  else if (strncmp(buf, "event ", 6) == 0)
  {
    decodeEvent(&buf[6]);
  }
  else if (strncmp(buf, "mmod ", 5) == 0)
  { // modify line
    char * p2;
    int16_t thread = strtol(&buf[5], &p2, 10);
    bool isOK = p2 != &buf[5] and p2 != NULL;
    int16_t line   = strtol(p2, &p2, 10);
//     isOK &= p2 != &buf[5] and p2 != NULL;
    const int MSL = 100;
    char s[MSL];
    // debug
    //         const int MSL = 100;
    //         char s[MSL];
    //         snprintf(s, MSL, "# parse_and_execute_command: thread=%d, line=%d, isOK=%d\r\n", thread, line, isOK);
    //         usb.send(s);
    // debug end
    if (isOK and thread > 0 and line > 0)
    { // has got valid thread and line numbers
      isOK = modLine(thread, line, p2);
      if (not isOK)
      {
        snprintf(s, MSL, "# parse_and_execute_command: %s\r\n", missionErrStr);
        usb.send(s);
      }
    }
    else
    {
      snprintf(s, MSL, "# parse_and_execute_command: error thread=%d (< 1!), line=%d (<1!)\r\n", thread, line);
      usb.send(s);
    }
  }
  else if (subscribeDecode(buf)) {  }
  else
  {
    used = false;
//     usb.send("# UMission::decode not used\n");
  }
  return used;
}

void UMission::tick()
{
  // mission time control
  if (control.missionState == 0 ) // we are waiting to start, so time since start is zero
  {
//     timeAtMissionStart = hb10us;
    if (missionTime > 18000.0 )
      // restart time (after 5 hours) - max usable time is 32768, as added in 1ms bits
      missionTime = 0.0;
  }
  if (moreMissionLines)
    moreMissionLines = getLines(moreMissionLines);
  subscribeTick();
}

void UMission::sendHelp()
{
  const int MRL = 200;
  char reply[MRL];
  usb.send("# Mission -------\r\n");
  snprintf(reply, MRL, "# \tmadd user-mission-line \tAdd a user mission line (%d lines loaded in %d threads)\r\n", 
           getLinesCnt(), getThreadsCnt());
  usb.send(reply);
  snprintf(reply, MRL, "# \tmmod T L user-mission-line \tModify line L in thread T to new user-mission-line\r\n");
  usb.send(reply);
  snprintf(reply, MRL, "# \tmclear \tClear all user mission lines\r\n");
  usb.send(reply);
  snprintf(reply, MRL, "# \tevent X \tMake an event number X (from 0 to 31)\r\n");
  usb.send(reply);
  subscribeSendHelp();
}

void UMission::sendData(int item)
{
  if (item == 0)
  {
    moreMissionLines = getLines(false);
  }
  else if (item == 1)
  {
    getToken();
  }
  else if (item == 2)
  {
    sendStatus();
  }
  else if (item == 3)
  {
    sendEvent();
  }
  else if (item == 4)
    sendMis();
  else if (item == 5)
    sendMisMax();
}

void UMission::sendStatus()
{
  const int MSL = 200;
  char s[MSL];
  for (int i = 0; i < threadsMax; i++)
  {
    UMissionThread * mt = &threads[i];
    if (i < threadsCnt)
      snprintf(s, MSL, "mstat %d %d %d %d %d %d %lu %lu\n", i, mt->threadNr, mt->threadActive, mt->misLineNum, mt->lineCnt, mt->theEnd, mt->activateFlag, mt->deactivateFlag);
    else
      // no mission available for this thread
      snprintf(s, MSL, "mstat %d -1 0 0 0 1 0 0\n", i);
    usb.send(s);
  }
}

void UMission::sendEvent()
{
//   uint32_t ef = eventFlagsSavedState;
//   int ms = control.missionState;
//   if (eventFlagsSavedStateSend != ef or missionStateSend != ms)
//   { // resend only if changed
//     eventFlagsSavedStateSend = ef;
//     missionStateSend = ms;
    const int MSL = 200;
    char s[MSL];
    snprintf(s, MSL, "mevent %lu %d\n", eventFlagsSavedState, control.missionState);
  //   eventFlagsSavedState = 0;
    usb.send(s);
//   }
}

void UMission::sendMis()
{
  bool inMission = false;
  int activeLine = 0;
  int m;
  int thNum = 0;
  if (control.missionState > 0)
  {
    for (m = 0; m < threadsCnt; m++)
    {
      if (threads[m].threadActive and not threads[m].theEnd)
      {
        inMission = true;
        activeLine = threads[m].misLineNum;
        thNum = threads[m].threadNr;
        break;
      }
    }
  }
  const int MSL = 200;
  char s[MSL];
  snprintf(s, MSL, "mis %d %d %d %d\n", inMission, thNum, activeLine, control.missionState);
  //   eventFlagsSavedState = 0;
  usb.send(s);
  
}

void UMission::sendMisMax()
{
  const int MSL = 75;
  char s[MSL];
  snprintf(s, MSL, "mmax %d %d %d %d\n", miLinesCnt, miLinesCntMax, threadsCnt, threadsMax);
  usb.send(s);
}

bool UMission::testFinished()
{
  bool theEnd = true;
  bool lineFinished[threadsMax];
//  bool debugOut = false;
  for (int i = 0; i < threadsCnt; i++)
  {
    lineFinished[i] = threads[i].testFinished();
  }
  // save event for log (reset by stateToLog())
  eventFlagsSavedLog = eventFlagsSavedLog | eventFlags;
  eventFlagsSavedState = eventFlagsSavedState | eventFlags;
  // reset events before implementing next line
  bool event0 = eventFlags % 1;
  eventFlags = 0;
  //   eventFlagsSaved1 = eventFlags;
//  usb.send("# testFinished -> events cleared\n");
  // implement next line (if relevant)
  for (int i = 0; i < threadsCnt; i++)
  {
    if (lineFinished[i])
    {
      theEnd &= threads[i].moveToNextLine();
      // reset event flags at line change (for state in GUI only
    }
    else
      theEnd = false;
  }
  // event flag 0 has special meening : stop mission NOW!
  if (event0)
  {
    theEnd = true;
    usb.send("# message The end - event=0\n");
  }
  if (theEnd)
  {
    eventFlagsSavedLog |= 1;
    eventFlagsSavedState |= 1;
  }
  return theEnd;
}


bool UMission::startMission()
{
  bool isOK = threadsCnt > 0;
  if (isOK)
  {
    missionTime = 0.0;
    encoder.clearPose();
    control.balance_active = false;
    control.mission_vel_ref = 0;
    control.mission_turn_ref = 0;
    logger.toLog = false;
    ls.lineSensorOn = false;
    control.mission_wall_turn = false;
    control.mission_irSensor_use = false;
    control.regul_line_use = false;
    control.setRateLimit(100);
    motor.motorSetEnable(true, true);
    //
    for (int i = 0; i < threadsCnt; i++)
    {
      threads[i].startMission(true);
    }
    usb.send("# message ** Mission started **\n");
  }
  eventFlags = 0;
  eventFlagsSavedState = 0;
  eventFlagsSavedLog = 0;
  return isOK;
}

void UMission::stopMission()
{
  for (int i = 0; i < threadsCnt; i++)
  {
    threads[i].stopMission();
  }
}


///////////////////////////////////////////////////


void UMission::eePromSaveMission()
{
  const int MRL = 100;
  char reply[MRL];
//   const int MSL = 100;
//   char s[MSL];
  int n, i = 0, t = 0;
  uint32_t adr = eeConfig.getAddr();
  bool isOK = true;
  // reserve space for size
  eeConfig.setAddr(adr + 2);
  //eePushAdr += 2;
  for (t = 0; t < threadsCnt; t++)
  {
    UMissionThread * mt = &threads[t];
    n = mt->toTokenString(reply, MRL);
    isOK = eeConfig.pushBlock(reply, n);
//     // debug
//      snprintf(reply, MRL, "# saving thread %d (%d), %d line(s) ...\r\n", t, mt->threadNr, mt->lineCnt);
//      usb.send(reply);
//     // debug end
    for (i = 0; i < mt->lineCnt and isOK; i++)
    {
      n = miLines[mt->lineStartIndex + i].toTokenString(reply, MRL);
      isOK = eeConfig.pushBlock(reply, n);
//       // debug
//        snprintf(s, MSL, "# line %d OK=%d (%d bytes) %s\r\n", i, isOK, n, reply);
//        usb.send(s);
//       // debug end
    }
//     // debug
//     usb.send("# end\r\n");
//     // debug end
    if (not isOK) 
      break;
  }
  if (not isOK)
  { // not enough space
    snprintf(reply, MRL, "# failed to save mission thread %d line %d (of %d)\n", t, i, miLinesCnt);
    usb.send(reply);
  }
  // write size of (saved part of) misson in characters
  eeConfig.write_word(adr, eeConfig.getAddr() - adr);
}

////////////////////////////////////

void UMission::eePromLoadMission()
{
  const int MRL = 100;
  char reply[MRL];
//   const int MSL = 120;
//   char s[MSL];
//   const int MSL = 100;
//   char s[MSL];
  int n = 0, m;
  uint16_t t = -1;
  miLinesCnt = 0;
  bool isOK = true;
  UMissionThread * mt;
  // read number of bytes in mission
  clear();
  threadsCnt = 1;
  mt = threads; // get first thread
  m = eeConfig.readWord() - 1;
  while (m > 1 and n < MRL and isOK)
  {
    snprintf(missionErrStr, 10, "#none\n"); // no error
    // read one byte at a time
    reply[n] = eeConfig.readByte();
    m--;
    if (reply[n] <= ' ')
    { // newline marks end of line
      reply[n] = '\n';
      reply[n+1] = '\0';
      if (n > 1)
      { // this is a line
        isOK = mt->decodeToken(reply, &t);
        if (not isOK and (t != mt->threadNr))
        { // line belongs to next thread
          mt++;
          threadsCnt++;
          mt->lineStartIndex = miLinesCnt;
          isOK = mt->decodeToken(reply, &t);
        }
      }
      n = 0;
    }
    else
      n++;
  }
  if (not isOK)
    usb.send(missionErrStr);
}

/////////////////////////////////////////////////

bool UMission::addLine(const char * lineToAdd)
{
  int16_t tn;
  int16_t ti;
  bool isOK;
  if (threadsCnt == 0)
    threadsCnt = 1;
  UMissionThread * mt = &threads[threadsCnt - 1];
  
  // is it a thread line
  char * p1 = strstr(lineToAdd, "thread");
  if (p1 != NULL)
  { // yes, decode thread line
    UMissionLine tmp;
    isOK = tmp.decodeLine(lineToAdd, &tn);
//     if (not isOK and tn != mt->threadNr and threadsCnt < threadsMax - 1)
    if (isOK)
    { // new thread - find or create
      if (tn <= 0)
      { // threads should have a number >= 1
        tn = 1;
        usb.send("# UMission::addLine: thread number should be > 0\n");
      }
      else if (tn != mt->threadNr and mt->lineCnt == 0)
      { // thread has no lines yet, so renumber thread
        mt->threadNr = tn;
      }
      ti = getThreadIndex(tn);
      // returns -1 if not known, so add a new thread
      isOK = ti == -1;
      if (isOK)
      { // thread number not seen before
        if (threadsCnt <= threadsMax)
        { // space for more, so add thread
          threadsCnt++;
          // advance to next thread
          mt++;
          // assign the number
          mt->threadNr = tn;
        }
        else
          usb.send("# message too many threads, max is 5 threads! (number from 1 to 31000) - reused last thread\n");
        // set start line for this thread
        mt->lineStartIndex = miLinesCnt;
        //mt->threadNr = tn;
      }
      // add event part of thread (if any)
      mt->activateFlag = tmp.eventSet;
      // if it is an event activated thread, then start inactive
      mt->threadActive = mt->activateFlag == 0;
      // set also deactivate event flags
      mt->deactivateFlag = tmp.eventMask;
      //       isOK = mt->addThreadOptions(lineToAdd, &tn);
      // debug
//       snprintf(s, MSL, "# adding thread optinos tn=%d, activateEvents=0x%lx, stopEvents=0x%lx, isactive=%d\n", 
//                tn, mt->activateFlag, mt->deactivateFlag, mt->threadActive);
//       usb.send(s);
      // debug end
    }
    else
    {
      isOK = false;
      usb.send("# UMission::addLine: error in thread line\n");
    }
  }
  else
  {
    isOK = mt->addLine(lineToAdd);
  }
  return isOK;
}

////////////////////////////////////////////////

bool UMission::modLine(int16_t thread, int16_t line, const char * p2)
{
  int idx = getThreadIndex(thread);
  bool isOK = idx >= 0;
  if (isOK)
  {
    UMissionThread * mt = &threads[idx];
    isOK = mt->modLine(line, p2);
  }  
  return isOK;
}



////////////////////////////////////////////////

bool UMission::getLines(bool more)
{
  bool result = true;
//   const int MSL = 200;
//   char s[MSL];
  if (not more)
  { // start from first
    moreThread = 0;
    // and with thread number
    threads[moreThread].moreLine = -1;
  }
//   snprintf(s, MSL, "# UMission::getLines, threads=%d, moreThread=%d, more=%d\n", threadsCnt, moreThread, more);
//   usb.send(s);
  // Get one line from this thread
  if (not threads[moreThread].getLines(more))
  { // no more lines in thread, advance
    result = ++moreThread < threadsCnt;
//     usb.send("# UMission::getLines advance to next thread\n");
    if (result)
      // mark that first line is thread number
      threads[moreThread].moreLine = -1;
  }
  return result;
}

//////////////////////////////////////

void UMission::getToken()
{
  for (int t = 0; t < threadsCnt; t++)
    threads[t].getToken();
}

///////////////////////////////////////

void UMission::getPowerUse(bool * lineSensor, bool *  irSensor, bool * chirpLog)
{ // power should be on if log is enabled
//   * lineSensor = logRowFlags[LOG_LINE];
//   * irSensor = logRowFlags[LOG_DIST];
//   * chirpLog = logRowFlags[LOG_CHIRP];
//   const int MSL = 120;
//   char s[MSL];
//   snprintf(s, MSL, "# log power use 1, thread %d, Chirp=%d, LS=%d, ir=%d\n", 0, *chirpLog, *lineSensor, *irSensor);
//   usb.send(s);
  // look for interface use in current mission line for each thread
  for (int t = 0; t < threadsCnt; t++)
  {
    threads[t].getPowerUse(lineSensor, irSensor, chirpLog);
    if (*lineSensor and *irSensor and *chirpLog)
      break;
  }
}

//////////////////////////////////////
/**
 * Get number of threads */
int UMission::getLinesCnt()
{
  return miLinesCnt;
}

//////////////////////////////////////

void UMission::clear()
{
  miLinesCnt = 0;
  for (int i = 0; i < miLinesCntMax; i++)
    miLines[i].clear();
  threadsCnt = 0;
  for (int i = 0; i < threadsMax; i++)
    threads[i].clear(i + 1);
  eventFlags = 0;
}

//////////////////////////////////////////

int16_t UMission::getThreadIndex ( int16_t thread )
{
  int16_t idx = -1;
  for (int i = 0; i < threadsCnt; i++)
  {
    if (threads[i].threadNr == thread)
    {
      idx = i;
      break;
    }
  }
  return idx;
}

/////////////////////////////////////////

void UMission::decodeEvent(const char* eventNumber)
{
  const char * p1 = eventNumber;
  int e;
  while (*p1 == ' ' or *p1 == '=')
    p1++;
  e = strtol(p1, NULL, 10);
  if (e >= 0 and e < 32)
    setEvent(e);
  if (e==33 and control.missionState == 0)
  {
    missionStop = false;
    state.missionStart = true;
  }
}

void UMission::setEvent(int number)
{
  uint32_t f = 1 << number;
  eventFlags |= f;
//   if (number < 30)
  { // send message back to client (except 30 and 31 used to swap 
    // mission lines in Robobot" configuration (from raspberry pi)
    const int MSL = 20;
    char s[MSL];
    snprintf(s, MSL, "event %d\n", number);
    usb.send(s);
  }
}
