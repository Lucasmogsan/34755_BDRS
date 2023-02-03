/***************************************************************************
*   Copyright (C) 2016 by DTU (Christian Andersen)                        *
*   jca@elektro.dtu.dk                                                    *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Lesser General Public License as        *
*   published by the Free Software Foundation; either version 2 of the    *
*   License, or (at your option) any later version.                       *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU Lesser General Public License for more details.                   *
*                                                                         *
*   You should have received a copy of the GNU Lesser General Public      *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef UREGBOT_H
#define UREGBOT_H

#include <sys/time.h>
#include <mutex>

#include "urun.h"
#include "utime.h"
#include "ulogfile.h"
#include "tcpCase.h"
#include "usource.h"

class UHandler;


using namespace std;




/**
 * The robot class handles the 
 * port to the REGBOT part of the robot,
 * REGBOT handles the most real time issues
 * and this class is the interface to that. */
class UTeensy : public URun, public USource
{ // REGBOT interface
public:
  /// Is port opened successfully
  bool teensyConnectionOpen;
  // mission state from hbt 
  int missionState = 0;
  
  
private:
  // serial port handle
  int usbport;
  // serial port (USB)
  int usbdeviceNum = 0;
  // simulator hostname
  const char * simHost;
  // simulator port
  int simPort = 0;
  // mutex to ensure commands to regbot is not mixed
//   mutex txLock;
//   mutex logMtx;
  mutex eventUpdate;
  // receive buffer
  static const int MAX_RX_CNT = 1000;
  char rx[MAX_RX_CNT];
  // number of characters in rx buffer
  int rxCnt;
  //
  UTime lastTxTime;
  UTime justConnectedTime;
  // socket to simulator
  tcpCase socket;
  /**
   * communication count */
  int gotCnt = 0;
  int sendCnt = 0;
  /** interface just opened */
  bool justConnected = false;
  
public:
  /** constructor */
//     UTeensy(/*UBridge * bridge*/);
  /** destructor */
    ~UTeensy();
  /**
   * Set device */
  void setup(int usbDevNum, int simport, char * simhost, const char * id);
  
  /**
   * send device details to client */
  virtual void sendDeviceDetails(USource * toClient);
  /**
   * send a string to the serial port 
   * But wait no longer than timeout - the unsend part is skipped 
   * \param message is c_string to send,
   * \param timeoutMs is number of ms to wait at maximum */
  void sendString(const char * message, int timeoutMs);
  /**
   * runs the receive thread 
   * This run() function is called in a thread after a start() call.
   * This function will not return until the thread is stopped. */
  void run();
  /**
   * Init data types to and from robot */
  void initMessageTypes();
  /**
   * open log with communication with robot */
  void openCommLog(const char * path);
  /** close logfile */    
  void closeCommLog();
  /**
   * decode commands potentially for this device */
  bool decode(const char * key, const char * params, USource * client);
  /**
   * is data source active (is device open) */
  virtual bool isActive()
  {
    return (usbport >= 0 or socket.connected) and gotActivityRecently and not justConnected and shouldBeActive;
  }
  
  void activate(bool toActive);
  void activate()
  {
    activate(shouldBeActive);
  };
  
private:
  /**
   * Open the connection.
   * \returns true if successful */
  bool openToTeensy();
  
  const char * getTeensyDeviceName(int devNum);
  /**
   * generate new host rename script */
  void saveRobotName(const char * newName);
  /**
   * Logvile */
  ULogFile * botComTx;
  ULogFile * botComRx;
  //   mutex logMtx;
  int connectErrCnt = 0;
  ///
  bool gotActivityRecently = true;
  UTime lastRxTime;
  static const int MAX_USB_DEVS = 5;
  static int usbDevIsOpen[MAX_USB_DEVS];
  static const int MAX_DEV_NAME_LENGTH = 20;
  static mutex usbDevOpenList;
  char usbDevName[MAX_DEV_NAME_LENGTH];
  bool shouldBeActive = true;
  bool initialized = false;
};

extern UTeensy teensy1;
extern UTeensy teensy2;

#endif
