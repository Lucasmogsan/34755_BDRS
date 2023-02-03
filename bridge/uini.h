/*
 *
 ***************************************************************************
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

#ifndef UINI_H
#define UINI_H

#include "utime.h"
#include "usource.h"

class UHandler;
class UBridge;


class UIni : public USource
{
public:
  /**
   * initialize and set path for ini-file and logfiles
   * \param iniPath is the path where bridge.ini is found, and where logfiles are stored
   * */
void setup(const char * iniPath);
  /**
   * is console active, */
  bool isActive()
  {
    return iniExist;
  }
  /**
   * Load the ini-file with this name
   * \param name is the filename */
  void loadSystemIni()
  {
    loadIniFile(systemIniFile, nullptr);
  }
  /**
   * Reload the ini-file for this interface
   * \param is is the interface name */
  void reloadIni(const char * id)
  {
    loadIniFile(systemIniFile, id);
  }
protected:
  void printHelp();
  /**
   * Decode command
   * \param key is the first keyword of the command
   * \param params is the rest of the line
   * \returns true if used */
  bool decode(const char * key, const char * params, USource * client);
  /**
   * Load commands from an ini-file
   * \param iniName is the filename to open.
   * \param interface if NULL, then all lines are used, else interfaces with this name only
   * */
  void loadIniFile(const char * iniName, const char * interface);
private:
  bool iniExist = false;
  static const int SIFL = 300;
  char systemIniFile[SIFL];
};

extern UIni ini;

#endif
