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

#include <stdint.h>
#include "usubss.h"

#ifndef DATA_LOGGER_H2
#define DATA_LOGGER_H2
// Initialization
//void loggerInit(int interval, int samples);
#ifdef REGBOT_HW4
// Teensy 3.5
#define LOG_BUFFER_MAX 70000
// #include <SD.h>
//#include <SPI.h>
#elif  REGBOT_HW41
// Teensy 4.1
#define LOG_BUFFER_MAX 250000

#else
// Teensy 3.2
#define LOG_BUFFER_MAX 25000

#endif

class ULog : public USubss
{
public:
  
  void setup();
  bool decode(const char * buf);
  void tick();
  void sendHelp();
  
protected:
  void sendData(int item) override;
  void sendLogFlagsControl();
  void sendLogFlagsOther();
  void sendLogInfo();
  void setLogFlagsOther(const char * buf);
  void setLogFlagsControl(const char * buf);
  

public:  
  
  // data logging buffer
  typedef enum
  {
    LOG_TIME = 0,
    LOG_MISSION,
    LOG_ACC,
    LOG_GYRO,
    LOG_MAG,
    LOG_MOTV_REF,
    LOG_MOTV,
    LOG_MOTA, 
    LOG_ENC,
    LOG_WHEELVEL,
    LOG_TURNRATE,
    LOG_POSE,
    LOG_LINE,
    LOG_DIST,
    LOG_BATT,
    LOG_CTRLTIME,
    LOG_CTRL_VELL,
    LOG_CTRL_VELR,
    LOG_CTRL_TURN,
    LOG_CTRL_POS,
    LOG_CTRL_EDGE,
    LOG_CTRL_WALL,
    LOG_CTRL_FWD_DIST,
    LOG_CTRL_BAL,
    LOG_CTRL_BAL_VEL,
    LOG_CTRL_BAL_POS,
    LOG_EXTRA,
    LOG_CHIRP,
    LOG_MAX_CNT
  } logItem;
  // size code    f=float, d=double (64 bit), i=int8_t, I=uint8_t, j=int16_t, J=uint16_t, k=int32_t, K=uint32
  #define LOG_INT8   'i'
  #define LOG_UINT8  'I'
  #define LOG_INT16  'j'
  #define LOG_UINT16 'J'
  #define LOG_INT32  'k'
  #define LOG_UINT32 'K'
  #define LOG_FLOAT  'f'
  #define LOG_DOUBLE 'd'
  // number if datapoints in a control log item
  #define CTRL_LOG_SIZE 10

  int logRowCnt;
  int logRowsCntMax;
  bool logRowFlags[LOG_MAX_CNT];
  // 1,2.. = send status or log at this sample interval count
  int logInterval; 
//   bool logAllow;
  bool logToUSB;

  //extern char rxbuffer[];
  int rxbufferCnt;
  int rxCharCnt;

  uint32_t timeAtMissionStart;
  bool toLog;
  bool logFull;
#ifdef REGBOT_HW41
  // Teensy 41 should use malloc to access reserved memory
  int8_t * logBuffer = nullptr; //[LOG_BUFFER_MAX];
#else
//   int8_t * logBuffer = nullptr;
  int8_t logBuffer[LOG_BUFFER_MAX];
#endif
  static const int dataloggerExtraSize = 20;
  float dataloggerExtra[dataloggerExtraSize];


  /**
  * Start logging with current log flags.
  * \param logInterval 0: unchanged, else set to this number of milliseconds
  * \param restart force restart of logging, even if running already
  * */
  void startLogging(int loginterval, bool restart);
  /**
  * Stop (actually just pause) logging. */
  void stopLogging(void);
  /**
  * Returns true if logger is logging and not full */
  inline bool loggerLogging() 
  {
    return toLog and not logFull;
  }
  /**
  * Save requested data to log buffer */
  void stateToLog();
  /**
  * Mission init is called before and sets the log default values
  * battery voltage. */
  void setLogFlagDefault();

  /**
  * init log structure after changing any of the log flags */
  void initLogStructure(int timeFactor);
  /**
  * add data for this item
  * \param item is the data logger object (ACC, GYRO, pose ...)
  * \param data is a pointer to the data to log
  * \param dataCnt is the number of bytes to log */
  void addToLog(logItem item, void * data, int dataCnt);
  /**
  * start new log row
  * \returns true if space for one more row */
  bool addNewRowToLog();

  /**
  * should this item be logged
  * \param item
  * \returns true if item is to be logged */
  inline bool logThisItem(logItem item)
  {
    return logRowFlags[item];
  }

  /**
  * Set size and type of data logger item
  * \param item is logger item
  * \param count is number of values to log for this item
  * \param type id data type to be logged - implicit gives the byte size of each value
  */
  void setLogSize(logItem item, int count, char type);
  /**
  * Transfer log to USB connection */
  int logWriteBufferTo(int row);
  /**
  * read communication from data logger to a buffer
  * \returns true if a \\n or a \\r is detected
  * data is available in rxBuffer and there is rxBufferCnt characters */
//   bool loggerRead();

  void eePromSaveStatusLog();
  void eePromLoadStatusLog();

//   void sendStatusLogging();
  
private:
  
  int col = 0;
  int logRowSize = 1;
//   int logTimeFactor = 1000;
  uint16_t logRowPos[LOG_MAX_CNT];
  uint8_t logRowItemSize[LOG_MAX_CNT*2];
  bool rowSendOK;
  int32_t ltc, lastTimerCnt = 0; /// heartbeat timer loop count
  int m; 
  int row = -1; // send log row
  uint32_t loggerRowWait = 0;
  
  void writeTime(int8_t * data, int row, char * p1, int maxLength);
  void writeMission(int8_t * data, int row, char * p1, int maxLength);
  void writeAcc(int8_t * data, int row, char * p1, int maxLength);
  void writeGyro(int8_t * data, int row, char * p1, int maxLength);
  void writeMag(int8_t * data, int row, char * p1, int maxLength);
  void writeCurrent(int8_t * data, int row, char * p1, int maxLength);
  void writeVel(int8_t * data, int row, char * p1, int maxLength);
  void writeTurnrate(int8_t * data, int row, char * p1, int maxLength);
  void writeEnc(int8_t * data, int row, char * p1, int maxLength);
  void writeMotPWM(int8_t * data, int row, char * p1, int maxLength);
  void writeMotVRef(int8_t * data, int row, char * p1, int maxLength);
  void writeMotVolt(int8_t * data, int row, char * p1, int maxLength);
  void writeBaro(int8_t * data, int row, char * p1, int maxLength);
  void writePose(int8_t * data, int row, char * p1, int maxLength);
  void writeBatt(int8_t * data, int row, char * p1, int maxLength);
  void writeCtrlTime(int8_t * data, int row, char * p1, int maxLength);
  void writeCtrlVal(int8_t * data, int row, char * p1, int maxLength, int item);
  void writeLinesensorExtra(int8_t * data, int row, char * p1, int maxLength);
  void writeExtra(int8_t * data, int row, char * p1, int maxLength);
  void writeChirp(int8_t * data, int row, char * p1, int maxLength);
  void writeLineSensor(int8_t * data, int row, char * p1, int maxLength);
  void writeDistSensor(int8_t * data, int row, char * p1, int maxLength);
  
};

extern ULog logger;

#endif // DATA_LOGGER_H
