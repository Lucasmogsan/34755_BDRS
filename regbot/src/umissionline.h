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

#ifndef REGBOT_MISSION_LINE_H
#define REGBOT_MISSION_LINE_H

#include <string.h>
#include <stdlib.h>
#include <main.h>
//#include "WProgram.h"
#include "usubss.h"


class UMissionLine;
class UMissionThread;

const int miLinesCntMax = 50;
extern UMissionLine miLines[];
extern int miLinesCnt;

const int missionErrStrMaxCnt = 50;
extern char missionErrStr[missionErrStrMaxCnt];

// extern UMission userMission;


class UMissionLine
{
public:
  enum MPTYP {MP_VEL='a', 
    MP_ACC, 
    MP_TR, 
    MP_LOG, 
    MP_BAL='f', // skip e, as it can be seen as exponent 7e1 = 70
    MP_EDGE_L, 
    MP_EDGE_R, 
    MP_EDGE_WHITE, 
    MP_LABEL, 
    MP_DRIVE_DIST,  /// set destination distance (driven distance) using position controller 
    MP_IR_SENSOR, 
    MP_IR_DIST,  /// use distance sensor
    MP_GOTO,     /// goto destination
    MP_THREAD,   /// thread keyword to start new thread
    MP_EVENT,    /// trigger an event
    MP_HEADING,  /// set heading reference directly
    MP_SERVO,    /// servo number
    MP_SERVO_POS, /// position value for servo (-1000 .. 1000] 
    MP_SERVO_VEL,  /// velocity limit for servo (0=off, 1=slow, 10=fast)
    MP_CHIRP      /// add chirp to current control (velocity or heading) 0=off 1..200 is amplitude in cm/s or 0.01 to 2.00 radians amplitude for turn 
  };
  enum MPTYC {MC_DIST='A',                          // 0 - reason (last) number
    MC_TIME,                                        // 1
    MC_TURN,                                        // 2
    MC_COUNT,                                       // 3
    MC_XING = 'F',  // skip E  as 7E1 = 70         // 5
//     MC_XINGW,                                       // 6
    MC_LINE_VALID,                                // 6
    //MC_LINE_VALID,                                // 8 NB! new numbering
    MC_IR_DIST1, /// distance sensor 1 limit        // 7
    MC_IR_DIST2, /// distance sensor 2 limit        // 8
    MC_TILT, /// tilt angle                         // 9
    MC_EVENT, /// event test                        // 10
    MC_LOG,    /// test for space left in log       // 11
    MC_HEADING, /// test for heading absolute value // 12
    MC_VEL,      /// test for velocity              // 13
    MC_REASON    /// continue reason from last line // 14
  };
public:
  // drive parameters
  float vel;
  bool velUse;
  float acc;
  bool accUse;
  float log;
  bool logUse;
  bool bal;
  bool balUse;
  float tr;
  bool trUse;
  float edgeRef;
  bool edgeLUse;
  bool edgeRUse;
  bool edgeWhiteUse;
  bool edgeWhite;
  uint16_t label;
  uint16_t gotoDest;
  bool gotoUse;
  int8_t irSensor;
  int8_t irSensorUse; /// 0=no IR sensor use, 1: follow wall, 2 (fwd only) or 3 (both) keep distance 
  float irDistRef; // reference distance for ir sensor
  bool irDistRefUse;
  float drivePos;    // position controller should keep this position referencd (on this leg)
  bool drivePosUse;  // should position controller be used
  int8_t servoID;
  int16_t servoPosition;
  int8_t servoVel;
  bool headUse;
  float headValue;
  uint8_t chirp; // do a chirp - for Bode estimation
  /// continue conditions
  float dist;     // distance check distance
  char distUse;   // should distance check be used
  float velTest; // velocit check limit
  char velTestUse; // '<' | '=' | '>'
  float turn; // desired turn angle in degrees as a condition
  char turnUse; 
  float time;
  char timeUse;
  char xingUse;
  int8_t xingVal;
  char lineValidUse;
  char lineValidVal;  
  uint16_t count;
  char countUse;
  float irDist1;
  char irDist1Use;
  float irDist2;
  char irDist2Use;
  float tilt;
  char tiltUse;       /// either <,>,=, where '=' is within 1 degrees
  uint32_t eventSet;
  uint32_t eventMask;
  bool logFullUse;    /// test if log is full
  char headEndUse;    /// either <,>,=, where '=' is within 3 degrees
  float headEndValue; /// end angle in degrees
  char reasonUse; // either '=' or '!'
  char reasonValue;
public:
  bool valid; // no error found
  int visits; // number of times this line has beed executed
  
public:
  /** clear all valid flags */
  void clear();
  /** convert to string - for return message 
   * \param bf is a C char array 
   * \param bfCnt is the count of bytes in bf
   * \param frame adds linefeed if true
   * \returns buffer (bf) filled from class and the used number of bytes */
  int toString(char * bf, int bfCnt, bool frame = true);
  /** decode mission line 
   * \param buffer is expected to hold a mission line in a terminated C string
   * \param threadNumber is set to the new thread number - if this keyword is on the line.
   * \returns true if loaded with no errors */
  bool decodeLine(const char * buffer, int16_t * threadNumber);
  /** convert to token string - for save in ee-prom 
   * \param bf is a C char array (destination)
   * \param bfCnt is the count of bytes in bf
   * \returns buffer (bf) filled from class and the used number of bytes */
  int toTokenString(char* bf, int bfCnt);
  /** decode mission line from token string 
   * \param buffer is expected to hold a mission line as tokens in a terminated C string
   * \returns true if loaded with no errors */
  bool decodeToken(const char* buffer);
  /**
   * Is this line finished - i.e. ready to continue to next line
   * \param state is the state of this line so far.
   * \param labelNum is 0 if not going to a label, else the label number (labels are >= 1)
   * \param endedAtEndAngle will be set to true, if line finished at desired angle
   * \param continueReason will be set to contition value MC_* if line is finished
   * \param lastReason is the reason for the last (non-goto) line
   * \returns true if finished */
  bool finished(UMissionThread * state, 
                uint16_t * labelNum, 
                bool * endedAtEndAngle, 
                char * continueReason,
                char lastReason
               );
  /**
   * Set values from this line, called at start of the line */
  void implementLine();
  /**
   * Postprocess this line, i.e.
   * terminate any special drive mode (turn, follow wall etc. implemented by this
   * line and not supposed to be continued on next line.
   * \param lineStartAngle is the heading when this line were started (used for turn a specific number of degrees.
   * \param endAtAngle true turn end at desired angle, false if turn ended for other reasons
   * */
  void postProcess(float lineStartAngle, bool endAtAngle);
  
};



#endif
