/*
 * Oled 0.9" display interface
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

#ifndef UHOST_H
#define UHOST_H

#include <string>
#include "urun.h"
#include "utime.h"
#include "usource.h"


class UHostIp : public URun, public USource
{
public:
  void setup();
 /**
  * Display currently allocated IP adresses */
 bool findIPs();
 /**
  * Run  motoring */
 void run();
 /**
  * Is source connection open */
 bool isActive()
 {
   return true;
 }
 float measureCPUtemp();
 

public:
  /// seems an oled display to be available (on a Raspberry PI with SPI interface)
  bool displayIP;
  
private:
  
  bool check_wireless(const char* ifname, char* protocol);
  void updateIPlist();
  bool findWifiMACs();
  
  int lastIpCnt = 0;
  static const int MHL = 100;
  char hostname[MHL];
  static const int MIPS = 7;
  char ips[MIPS][MHL] = {'\0'}; // list of IP strings
  char macs[MIPS][MHL] = {'\0'}; // list of MAC strings
  int ipsCnt = 0;
  int macCnt = 0;
  static const int MHL2 = 150;
  char ip4list[MHL2];
  char maclist[MHL2];
};

extern UHostIp hostip;

#endif
