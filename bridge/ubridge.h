/*
 *
 ***************************************************************************
 *   Copyright (C) 2017-2022 by DTU (Christian Andersen)                        *
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

#ifndef UBRIDGE_H
#define UBRIDGE_H

#include "vector"
#include "urun.h"
#include "utime.h"
#include "usource.h"

class UDataItem;
class UTeensy;
// class UOled;
class UData;
class UServerPort;
class UHandler;

// shutdown and restart flags
extern bool quitBridge;
extern bool restartBridge;

class UBridge : public URun, public USource
{
public:
  float shutDownVoltage = 21.0; // when battery gets below - power off is send to drive
  /**
   * constructor */
//   UBridge(UBridge * bridge);
  void setup();
  /** destructor */
  ~UBridge();
  /** get responce number 
   * searches if this data item needs some special treatment,
   * such as send to oled display or send something to robot
   * \param key is data item key,
   * \param client is the message source client number (robot is -1, joystick=-2)
   * \param sequence is to return to data item, if this datatype
   *                 is a sequence (e.g. logfile or program list
   * \returns number of response process, or -1 if no special
   *          process is needed. */
//    USource * getDestination(const char * key, USource * client, bool * sequence);
  /**
   * Response function with pointer to data item 
   * \param respondseClass is allocated response function (allocated by the getResponseNumber() function
   * \param dataItem is pointer to updated data item 
   * \param client is the source number for the data update */
  void response(USource * responseClass, UDataItem * dataItem, USource * client);
  /**
   * bridge thread loop */
  void run();
  /**
   * time for last hbt */
  UTime lastHbt;
  /**
   * Add a source possibility */
  void addSource(USource * source);
  
  bool decodeSourceCmds(const char * key, const char * params, USource * client);
  
  void setPortServer(UServerPort * portServer)
  {
    serv = portServer;
  }
  USource * findSource(const char * id);
  /**
   * Bridge is always active */
  bool isActive()
  {
    return true;
  }
  /**
   * List data iotems and data sources with status
   * all send as comments 
   * \param client is the destination for the list */
  void list(USource * client);
  
  /**
   * more printout */
  bool verbose = false;
  /**
   * Set link to data */
//   void setData(UData * dataLink)
//   {
//     data = dataLink;
//   }
  
  const char * getLogPath();
  
  void listSources();
  
private:
  /**
   * information from socket server */
  void responceRobotID(UDataItem* dataItem, int client);
  /** measure CPU temperature
   * */
//   float measureCPUtemp();
  /**
   * is the message for this device */
  bool decode(const char * key, const char * params, USource * client);
  /**
   * den status */
  void sendDeviceDetails(USource* toClient);
  
private:
  // link to other parts of bridge
//   UData * data;
  UServerPort * serv = nullptr;
  //
  std::vector<USource *> sourceList;
  /*
   * CPU temperature in degrees C */
  float cputemp;
};

extern UBridge bridge;

#endif
