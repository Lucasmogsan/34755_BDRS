/*
 * Handling messages to and from clients
 * and to and from regbot
 * 
 ***************************************************************************
 *   Copyright (C) 2017 by DTU (Christian Andersen)                        *
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

#ifndef UHANDLER_H
#define UHANDLER_H

#include <string>
#include <mutex>
#include "udata.h"
#include "userverport.h"

// do not need full class specification at this point
class UTeensy;
class UJoy;
class USource;
class UBridge;
// class UOled;

class UHandler
{
public:
  /**
   * Constructor */
//   UHandler(UData * broker/*, UBridge * brg*/);
  /**
   * destructor */
//   ~UHandler();
  
  void setup();
  /**
   * add all valid message types */
  void addDataItems();
  /**
   * Handle incoming command
   * \param source is the source of the command or message
   * \param msg is a textline with the command or message
   * \param noCRC CRC is not needed for internal commands */
  void handleCommand(USource * source, const char* msg, bool noCRC);
  /**
   * Stop processing */
  void stop()
  {
    stopHandle = true;
  }
  
public:
   UData * items;
//   UBridge * bridge;
protected:
  /**
   * update database only */
  void regularDataUpdate(USource * source, const char* msg);
  
private:
  /**
   * for message handler */
  bool stopHandle = false;
};

extern UHandler handler;

#endif
