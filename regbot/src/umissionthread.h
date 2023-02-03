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

#ifndef REGBOT_MISSION_THREAD_H
#define REGBOT_MISSION_THREAD_H

#include <string.h>
#include <stdlib.h>
//#include "WProgram.h"
#include "usubss.h"
#include "umissionline.h"

//class UMissionLine;
class UMission;


class UMissionThread
{
  friend class UMissionLine;
public:
  int8_t misLineNum = 0;     // current mission line number
  //int misLineNumLast = 0;     // last line number to detect change
  int misStartTime;           // start time for this line
  bool turnEndedAtEndAngle;  // true if turn finished at desired angle
  float linStartDist;         // start distance for this line
  float linStartAngle;        // start heading (ref) for this line
  // thread number
  int16_t threadNr;
  uint8_t lineCnt;
  uint8_t lineStartIndex;
  int16_t moreLine;
  bool threadActive;   // can be activated and deactivated by events
  uint32_t activateFlag;   // eventflag set that will activate the thread - will call "start mission" when event occur
  uint32_t deactivateFlag; // eventflag set, that will stop processing of thread
  uint8_t continueReason;
  bool theEnd;
protected:  
  UMissionLine * currentLine;
  uint16_t gotoLabel; // set to label number, otherwise 0.
  float turnAngSum;            // turned angle for this line
  float turnSumLast;          // last angle for summing
public:
  void clear(uint8_t idx);
  /**
   * Test finished and advance to next line 
   * \param label is a label number, if supposed to go to a label line.
   * 
   * \returns true if line is valid (i.e. not finished with mission) */
  bool advanceLine(int16_t toLabel);
  /**
   * Test if current line is finished, and termiate any line specific actions (i.e. turn).
   * \retuns true if line is finished. */
  bool testFinished();
  /**
   * Implement new line, this advance to next line and implement 
   * any new line specific items.
   * \returns true if no more lines area available */
  bool moveToNextLine();
  /**
   * reset thread to start a new mission
   * \param setActivate if activateFlag is 0
   * resets line number and implements first line in thread */
  void startMission(bool setActivate);
  /**
   * set thread to inactive
   *  */
  void stopMission();
  /**
   * initialize new line
   * \returns true if no errors */
  void implementNewLine();
  /**
   * Reset visit counter on all lines */
  void resetVisits();
  /**
   * Get lines in this thread as readable syntax to client. 
   * \param more is false at start of command and false if to continue
   * \returns false when all is send, */
  bool getLines(bool more);
  /** 
   * Get lines as tokens (for debug) for this thread */
  void getToken();
  /**
   * serach for use of IR sensor and line sensor 
   * \param lineSensor return value true if line sensor is ever used in thread
   * \param isSensor returns true if IR sensor is ever used in thread 
   * \param chirpLog returns true if a chirp is started in this thread */
  void getPowerUse(bool * lineSensor, bool *  irSensor, bool * chirpLog);
  /**
   * Get the thread number as a token line */
  int toTokenString(char * bf, int bfCnt);
  /**
   * decode of any thread conditions, activate or deactivate event 
   * \param lineToAdd is the thread line that may include events before and after the ':',
   * \param newThreadNumber is not used 
   * \return true if no syntax errors were found */
//   bool addThreadOptions(const char * lineToAdd, int16_t * newThreadNumber);
  /**
   * Add a line to this thread, if the line holds a thread keyword, 
   * that do not match the thread number and is not the first line in the thread,
   * then do not add, but return false and the thread number in the newThreadNumber.
   * \param lineToAdd is the line in normal syntax.
   * \returns true, if the line is added. */
  bool addLine(const char * lineToAdd);
  /**
   * Decode this line of tokens.
   * \param line is the line, it is ended with a new-line, but not a zero.
   * \param tn should be set to the thread-number, if line line is a thread-number token.
   * \returns true if line is added, returns false if thread-number do not match current thread. */
  bool decodeToken(char * line, uint16_t * tn);
  /**
   * modify specific line
   * NB! this modifies the line even if a mission is in progress.
   * \param line is line number in this thread to modify - first line has number 1
   * \returns true is line exist (and is modified), else false. */
  bool modLine(int16_t line, const char * p2);
  
};



#endif
