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

#ifndef UOLED_H
#define UOLED_H

#include <string>
#include "urun.h"
#include "utime.h"

class ArduiPi_OLED;

class UOled : public URun
{
public:
  UOled();
  ~UOled();
 /**
  * Initialize display */
 bool initDisplay();
 /**
  * Display currently allocated IP adresses */
 void displayIPs();
 /**
  * Run  motoring */
 void run();
 /**
  * Display a line on the display 
  * \param lineNumber is nine number (0..5)
  * \param lineText is text to display
  * */
 void printLine(int lineNumber, const char * lineText);
 /**
  * Clear display */
 void clear();
 /*
  * Repaint display string list */
 void redisplay();
 /**
  * display battery voltage and name 
  * \param batteryVoltage display is updated only if battery voltage has changed */
 void displayVoltageAndName(float batteryVoltage, float time, float cpuTemp);
 

public:
  /// seems an oled display to be available (on a Raspberry PI with SPI interface)
  bool displayFound;
  bool displayIP;
  
private:
  ArduiPi_OLED * display = NULL;
//   std::string hostname;
  int lastIpCnt = 0;
  static const int MHL = 100;
  char hostname[MHL];
  static const int MAX_OLED_LINES = 8;
  static const int MAX_OLED_CHARS = 21;
  char oledLines[MAX_OLED_LINES][MAX_OLED_CHARS+1];
  static const int MIPS = 7;
  char * ips[MIPS] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL}; // list of IP strings
public:
  float oldBatteryVoltage = -1.0;
  float oldTemperature = -1;
  UTime oldTempTime;
  float oldRegbotTime = 0;
  float batteryTime = -100;
};

#endif
