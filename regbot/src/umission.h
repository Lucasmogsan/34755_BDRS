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

#ifndef REGBOT_MISSION_H
#define REGBOT_MISSION_H

#include <string.h>
// #include <stdlib.h>
//#include "WProgram.h"
#include "usubss.h"
#include "umissionline.h"
#include "umissionthread.h"


class UMission : public USubss
{
protected:
  // mission line pool
  //UMissionLine miLines[miLinesCntMax];
  // number of used lines.
  //uint8_t mLinesCnt;
  // number of threads
  static const int threadsMax = 5;
  // thread start list
  UMissionThread threads[threadsMax];
  // number of threads used
  uint8_t threadsCnt;
  // state when listing lines
  uint8_t moreThread;
  /// used when sending mission lines in blocks
  bool moreMissionLines = false;
  
public:
  // event flags
  uint32_t eventFlags;
//   uint32_t eventFlagsSaved1;
  uint32_t eventFlagsSavedLog;
  uint32_t eventFlagsSavedState = 0;
  // flag for last send value - but gives problems, if client missed last message
//   uint32_t eventFlagsSavedStateSend = 0;
//   int missionStateSend = 0;
  // should mission be stopped
  bool missionStop = false;
  // mission time in seconds
  float missionTime = 0;

protected:
  virtual void sendData(int item) override;
  /**
   * Send mission status */
  void sendStatus();
  /*
   * Sent flags set since last message */
  void sendEvent();
  /**
   * send short mission status (for GUI) */
  void sendMis();
  /**
   * Send number of mission lines used and max (and threads) */
  void sendMisMax();
  
  

public:
  
  void setup();
  
  bool decode(const char * buf);
  
  void tick();
  void sendHelp();
  /** zero all mission lines. */
  void clear();
  /**
   * Test all threads for a finished mission
   * \returns true if mission is finished */
  bool testFinished();
  /**
   *(re)start current mission */
  bool startMission();
  /**
   * stop current mission */
  void stopMission();
  /**
   * save all mission lines to EE-prom */
  void eePromSaveMission();
  /**
   * Load mission from ee-prom */
  void eePromLoadMission();
  /**
   * Add a mission line to default thread 
   * This is not legal if a mision is in progress 
   * \param lineToAdd is a string in clear text added to the active (latest) thread.
   * \returns true if no syntax error is found. */
  bool addLine(const char * lineToAdd);
  /**
   * modify specific line
   * NB! this modifies the line even if a mission is in progress.
   * \param thread is thread number to modify (0 to 32767)
   * \param line is line number in this thread
   * \returns true is line exist (and is modified), else false. */
  bool modLine(int16_t thread, int16_t line, const char * p2);
  /**
   * Send all mission lines to console (USB or serial)
   * \param more set to false if restart send and true to send next line 
   * \returns false when all is send. */
  bool getLines(bool more);
  /**
   * Get the packed token string to client - for debug */
  void getToken();
  /**
   * serach for use of IR sensor and line sensor 
   * \param lineSensor return value true if line sensor is ever used in mission
   * \param isSensor returns true if IR sensor is ever used in mission 
   * \param chirpLog returns true if a chirp is started during mission */
  void getPowerUse(bool * lineSensor, bool *  irSensor, bool * chirpLog);
  
  /**
   * Get number of threads */
  inline int getThreadsCnt()
  { return threadsCnt; };
  /**
   * Get number of lines */
  int getLinesCnt();
  /**
   * Get thread index number.
   * \param threadNumber is the thread number to look for,
   * \returns the index of the thread, or -1 if not found. */
  int16_t getThreadIndex(int16_t thread);
  /**
   * Set an event flag 
   * \param event number in range 0..31 */
  void setEvent(int number);
  /**
   * Decode event number and activate this event.
   * \param string from client, everything after keyword 'event'
   * assumed to be '=12\ n' or similar */
  void decodeEvent(const char * eventNumber);
  
  
};

extern UMission userMission;


#endif
