 /***************************************************************************
 *   Copyright (C) 2022 by DTU (Christian Andersen)                        *
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
 
#ifndef USUBSCRIBE_H
#define USUBSCRIBE_H

#include <string>
#include "utime.h"

class USource;

class USubscribe 
{
public:
  /** client for a subscription
   * or source for a onUpdate action */
  USource * cli;
  /// last time this item was send to this client
  UTime itemSend;
  /**
   * command to send when this item is updated */
  std::string onUpdate;
  /**
   * desired priority (update interval) in seconds */
  float interval = 0;
  /**
   * all means all updates regardless of interval */
  bool all = false;
  
public:
  void sendStatus(USource * client);
  void printStatus();
  /**
   * check subscribers */
  bool tick(const char ** todo);
};

 
#endif
