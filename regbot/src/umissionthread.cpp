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
// #include "motor_controller.h"
#include "uservo.h"

#include "ulog.h"
#include "uusb.h"
#include "umotor.h"
#include "ustate.h"
#include "uencoder.h"
#include "uirdist.h"

// UMission userMission;
// 
// UMissionLine miLines[miLinesCntMax];
// int miLinesCnt = 0;
// 
// char missionErrStr[missionErrStrMaxCnt];

////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

void UMissionThread::clear(uint8_t idx)
{ // give thread a number and clear the rest
  threadNr = idx;
  lineCnt = 0;
  lineStartIndex = 0;
  misLineNum = -1;
  misStartTime = 0;
  linStartAngle = 0;
  turnAngSum = 0;
  linStartDist = 0;
  turnSumLast = 0;
  theEnd = false;
  activateFlag = 0;
  deactivateFlag = 0;
  threadActive = true;
  continueReason = 0;
}



bool UMissionThread::advanceLine (int16_t toLabel)
{
  if (misLineNum < lineCnt)
    // default is goto next line
    misLineNum++;
  if (toLabel > 0)
  { // there is a label number, find line
    // if label is not found in this thread, then
    // continue to next line (default action)
    for (int i = 0; i < lineCnt; i++)
      if (miLines[i + lineStartIndex].label == toLabel)
      {
        misLineNum = i;
        break;
      }
  }
  // debug
//   const int MSL = 50;
//   char s[MSL];
//   snprintf(s, MSL, "# advance line to %d of %d (label=%d)\n", misLineNum, lineCnt, toLabel);
//   usb.send(s);
  // debug end
  if (misLineNum < lineCnt)
  { // new line is available
    currentLine = &miLines[misLineNum + lineStartIndex];
    return true;
  }
  else
    return false;
}

/////////////////////////////////////////////////////////////////

bool UMissionThread::testFinished() 
{
  bool LineEnded = false;
  //usb.send("# Thread::testFinished -> start\n");
  // we are not finished 
  // are thread active or deactivated
  if (threadActive and ((deactivateFlag & userMission.eventFlags) > 0))
  {
    // debug
//     const int MSL=80;
//     char s[MSL];
//     snprintf(s, MSL, "# thread %d de-activated\n", threadNr);
//     usb.send(s);
    // debug end
    theEnd = true;
  }
  else if (not threadActive and ((activateFlag & userMission.eventFlags) > 0))
  {
    // debug
    const int MSL=80;
    char s[MSL];
    snprintf(s, MSL, "# thread %d activated\n", threadNr);
    usb.send(s);
    // debug end
    //     usb.send("# UMissionThread::testFinished : thread activated\n");
    // restart lines in this thread
    // - and implement first line (and clears "theEnd" (flag for this thread))
    threadActive = true;
    startMission(false);
    // if no lines, then the end is reached already
    LineEnded = theEnd;
  }
  else if (theEnd)
  { // line (and thread) has ended (or inactive)
    // debug
//     const int MSL=80;
//     char s[MSL];
//     snprintf(s, MSL, "# UMissionThread::testFinished : inactive thread %d ended\n", threadNr);
//     usb.send(s);
    // debug end
    LineEnded = true;
  }
  else if (misLineNum < lineCnt)
  {
    // debug
//     const int MSL=100;
//     char s[MSL];
//     snprintf(s, MSL, "# UMissionThread::testFinished : thread %d has %d lines, now %d, thread active=%d\n", threadNr, lineCnt, misLineNum, threadActive);
//     usb.send(s);
    // debug end
    if (threadActive)
    {
      //uint16_t labelNum = 0; // 0 is a flag for no goto
      //UMissionLine * line = &miLines[misLineNum + lineStartIndex];
      gotoLabel = 0;
      char cr = ' ';
      if (currentLine->finished(this, &gotoLabel, &turnEndedAtEndAngle, &cr, continueReason))
      { // stop any special drive scheme set by this line
  //       line->postProcess(misLastAng);
  //       theEnd = not advanceLine(labelNum);
  //       if (not theEnd)
  //       {
  //         implementNewLine();
  //       }
        if (not currentLine->gotoUse)
          // save new reason to continue
          continueReason = cr;
        //usb.send("#Line ended!");;
        LineEnded = true;
      }
  //     if (gotoLabel > 0)
  //     {
  //       const int MSL = 50;
  //       char s[MSL];
  //       snprintf(s, MSL, "# in thread goto=%d\n", gotoLabel);
  //       usb.send(s);
  //     }
    }
  }
  else
    LineEnded = true;
  return LineEnded;
}


//////////////////////////////////////////////////////////////

bool UMissionThread::moveToNextLine()
{
  if (not theEnd)
  { // finish last line
    currentLine->postProcess(linStartAngle, turnEndedAtEndAngle);
    // advance line (maybe a goto)
    theEnd = not advanceLine(gotoLabel);
    if (not theEnd)
    { // implement all parameter settings
      implementNewLine();
    }
    else
      threadActive = false;
  }
  return theEnd;
}

//////////////////////////////////////////////////////////////

void UMissionThread::resetVisits()
{
  for (int i = 0; i < lineCnt; i++)
    miLines[i + lineStartIndex].visits = 0;
}

//////////////////////////////////////////////////////////////

void UMissionThread::startMission(bool setActivate)
{
  resetVisits();
  misLineNum = 0;
  
  theEnd = false;
  if (setActivate)
    threadActive = activateFlag == 0;
  if (misLineNum < lineCnt and threadActive)
    implementNewLine();
//   else
//     theEnd = true;
//   const int MSL = 100;
//   char s[MSL];
//   snprintf(s, MSL, "# thread=%d initiated is active=%d\n", threadNr, threadActive);
//   usb.send(s);
}

void UMissionThread::stopMission()
{
  theEnd = true;
}
  
//////////////////////////////////////////////////////////////

void UMissionThread::implementNewLine()
{
//   // debug
//   const int MSL = 120;
//   char s[MSL];
//   // debug end
 // initialize new mission line
  currentLine = &miLines[misLineNum + lineStartIndex];
  currentLine->visits++;
  // if old line controlled angle, then
  // use end angle as new reference
  if (control.mission_wall_turn or control.regul_line_use)
  { // Wall follow or line follow
    // just use current heading as new reference
    control.mission_turn_ref = encoder.pose[2];
//     regTurnM[1] =encoder.pose[2];
  }
  if (misLineNum < miLinesCnt)
  { // prepare next line
    turnAngSum = 0; 
    turnSumLast = control.mission_turn_ref;
    linStartAngle = control.mission_turn_ref;
    turnEndedAtEndAngle = false;
    linStartDist = encoder.distance;
    misStartTime = hbTimerCnt;
    // all other settings
    currentLine->implementLine();
  }
  // every time a thread implements a new line 
  // this data is set - to allow user state reporting
  control.misLine = currentLine;
  control.misThread = threadNr;
  control.missionLineNum = misLineNum;
//   // debug
//   snprintf(s, MSL, "# implemented line %d in thread %d\r\n", misLineNum, threadNr);
//   usb.send(s);
//   // debug end
}

////////////////////////////////////////////////////////

bool UMissionThread::addLine(const char * lineToAdd)
{
  int idx = lineCnt + lineStartIndex;
  bool isOK = idx < miLinesCntMax;
  const int MSL = 150;
  char s[MSL];
  if (isOK)
  { 
    isOK = miLines[idx].decodeLine(lineToAdd, NULL);
    // debug
//     snprintf(s, MSL, "# UMissionThread::addLine ok:%d th:%d line:%d: %s\n", isOK, threadNr, lineCnt, lineToAdd);
//     usb.send(s);
    // debug end
    if (not isOK)
    { // report error
      snprintf(s, MSL, "# UMissionThread::addLine syntax error thread %d line %d: %s\n", threadNr, lineCnt, missionErrStr);
      usb.send(s);
    }
    if (isOK)
    { // OK to add line
      isOK = miLines[idx].valid;
      if (isOK)
      { // test if empty
        int n = miLines[idx].toString(s, MSL - 3, false);
        if (n > 2)
        { // not an empty line (only added if not empty)
          lineCnt++;
          if (idx >= miLinesCnt)
            miLinesCnt++;
        }
      }
      else
      {
        snprintf(s, MSL, "\r\n# UMissionThread::addLine add line %d failed: %s\r\n", idx, missionErrStr);
        usb.send(s);
      }
    }
  }
  return isOK;
}


bool UMissionThread::modLine(int16_t line, const char * p2)
{
  bool isOK = line <= lineCnt  and line > 0;
  if (isOK)
  { // decode the line twice first for syntax check, then for real
    UMissionLine tmp;
    int16_t thnr = threadNr;
    isOK = tmp.decodeLine(p2, &thnr);
    if (thnr != threadNr)
    { // there must not be a line with new thread number
      isOK = false;
      usb.send("# not legal to modify thread number\n");
    }
    // debug
//     const int MSL = 50;
//     char s[MSL];
//     snprintf(s, MSL, "# line %d found isOK=%d\n", line, isOK);
//     usb.send(s);
    // debug end
    
    if (isOK)
    { // OK, modify the line
      int idx = line + lineStartIndex - 1;
      isOK = miLines[idx].decodeLine(p2, &thnr);
    }
  }
  else
    usb.send("# modify failed - no such line\n");
  return isOK;
}

/////////////////////////////////////

bool UMissionThread::getLines(bool more)
{ // get lines in thread as string
  const int MRL = 1300;
  char reply[MRL];
  bool result = true;
  if (moreLine == -1)
  {
//     usb.send("# UMissionThread::getLines: first line in thread\n");
    snprintf(reply, MRL - 3, "mline thread=%d", threadNr);
    int n = strlen(reply);
    char * p1 = &reply[n];
    if (activateFlag or deactivateFlag)
    { // add event part(s)
      const char sep = ',';
      if (activateFlag)
      {
        for (int i = 0; i < 32; i++)
        { // test all possible events
          if (activateFlag & (1 << i))
          { // event i is set, add to line
            *p1++=sep; 
            n++;
            snprintf(p1, MRL - n - 3, "event=%d", i);
            n += strlen(p1);
            p1 = &reply[n];
          }
        }
      }
      if (deactivateFlag)
      {
        *p1++ = ':';
        n++;
        for (int i = 0; i < 32; i++)
        { // test all possible events
          if (deactivateFlag & (1 << i))
          { // event i is set, add to line
//             usb.send("# found deactivate flag\n");
            if (p1[-1] != ':')
            { // first stop event should not be preceded by a separator
              *p1++=sep; 
              n++;
//               usb.send("# found deactivate flag and :\n");
            }
//             else
//               usb.send("# found deactivate flag no :\n");
            snprintf(p1, MRL - n - 3, "event=%d", i);
            n += strlen(p1);
            p1 = &reply[n];
          }
        }
      }
    }
    reply[n++] = '\r';
    reply[n++] = '\n';
    reply[n] = '\0';
    if (usb.send(reply))
    {
      result = lineCnt > 0;
      moreLine = 0;
//       usb.send("#thread start\n");
    }
  }
  else
  {
    int n = miLines[moreLine + lineStartIndex].toString(reply, MRL - 3);
    if (n > 2)
    { // a thread line may have no other attributes
      if (usb.send(reply))
      {
        moreLine++;
        result = moreLine < lineCnt;
      }
    }
    else
    {
      moreLine++;
      result = moreLine < lineCnt;
    }
//     usb.send("#thread line\n");
  }
  return result;
}

/////////////////////////////////////////////////////


/**
 * serach for use of IR sensor and line sensor */
void UMissionThread::getPowerUse ( bool* lineSensor, bool* irSensor, bool * chirpLog )
{
  for (int i = 0; i < lineCnt; i++)
  {
    UMissionLine * ml = &miLines[i + lineStartIndex];
    if (ml->edgeLUse or ml->edgeRUse or ml->lineValidUse /*or ml->lineValidUse*/ or ml->edgeWhiteUse)
      *lineSensor = true;
    if (ml->irDist1Use or ml->irDist2Use or ml->irSensorUse or ml->irDistRefUse)
      *irSensor = true;
    if (ml->chirp > 0)
      *chirpLog = true;
  }
}



/////////////////////////////////////////////////////

void UMissionThread::getToken()
{ // get token lines to console for debug
  const int MRL = 30;
  char reply[MRL];
  const int MSL = 100;
  char s[MSL];
  // first the thread number
  toTokenString(reply, MRL);
  snprintf(s, MSL, "#thread %d (%d) %s", threadNr, strlen(reply), reply);
  usb.send(s);
  // then the lines in the thread
  for (int i = 0; i < lineCnt; i++)
  {
    //usb_write("# line\n");
    miLines[i + lineStartIndex].toTokenString(reply, MRL);
    //usb.send(reply);
    snprintf(s, MSL, "#line %d (%d) %s", i, strlen(reply), reply);
    usb.send(s);
    //         m.decodeToken(&reply[1]);
    //         m.toString(&reply[1], MRL-1);
    //         usb_write(reply);
  }
  usb.send("<done tokens>\n");
}

///////////////////////////////////////////

int UMissionThread::toTokenString(char * bf, int bfCnt)
{
  int n = snprintf(bf, bfCnt, "%c%d", UMissionLine::MP_THREAD, threadNr);
  if (activateFlag or deactivateFlag)
  {
    char * p1 = &bf[n];
    if (activateFlag)
    {
      snprintf(p1, bfCnt - n - 2, "%c%lu", UMissionLine::MP_EVENT, activateFlag);
      n += strlen(p1);
      p1 = &bf[n];
    }
    if (deactivateFlag)
    { // add both ':' and stop-event(s)
      snprintf(p1, bfCnt - n - 2, ":%c%lu", UMissionLine::MC_EVENT, deactivateFlag);
      n += strlen(p1);
    }
  }
  // terminate with a new-line
  bf[n++] = '\n';
  bf[n] = '\0';
  return n;
}

///////////////////////////////////////////

bool UMissionThread::decodeToken(char * line, uint16_t * tn)
{ // decode token (from EE-flash)
  bool isOK = true;
  if (line[0] == UMissionLine::MP_THREAD)
  { // get thread number
    // tokens thread number is on a separate line
    char * p1 = &line[1];
    *tn = strtol(p1, &p1, 10);
    isOK = lineCnt == 0;
    if (isOK)
    {
      threadNr = *tn;
      if (*p1 > ' ')
      { // there is more
        UMissionLine tmp;
        tmp.decodeToken(p1);
        // set start - stop flags directly - is 0 if unused
        activateFlag = tmp.eventSet;
        deactivateFlag = tmp.eventMask;
      }
    }
  }
  else
  { // not a thread number
    int idx = lineStartIndex + lineCnt;
    isOK = miLines[idx].decodeToken(line);
    if (isOK)
    {
      lineCnt++;
      if (idx >= miLinesCnt)
        miLinesCnt++;
    }
  }
  return isOK;
}

