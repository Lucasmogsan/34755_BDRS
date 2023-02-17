#!/usr/bin/python
# -*- coding: utf-8 -*-

#/***************************************************************************
 #*   Copyright (C) 2014-2022 by DTU
 #*   jca@elektro.dtu.dk            
 #* 
 #* 
 #* The MIT License (MIT)  https://mit-license.org/
 #* 
 #* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 #* and associated documentation files (the “Software”), to deal in the Software without restriction, 
 #* including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 #* sublicense, and/or sell copies of the Software, and to permit persons to whom the Software 
 #* is furnished to do so, subject to the following conditions:
 #* 
 #* The above copyright notice and this permission notice shall be included in all copies 
 #* or substantial portions of the Software.
 #* 
 #* THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 #* INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 #* PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
 #* FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 #* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 #* THE SOFTWARE. */

import threading
import time

class UList(object):
  list = "(empty)"
  listChanged = True
  #
  dataRead = False # for all servos
  inEdit = False
  inTimerUpdate = True
  hasFocus = False
  thisTab = -1
  #
  def __init__(self, parent):
    self.main = parent.main
    self.ui = parent.ui

  def setup(self):
    self.ui.pushButton_gui_robots.clicked.connect(self.listRobots)
    self.ui.pushButton_gui_MAC.clicked.connect(self.listMac)
    self.ui.pushButton_gui_clear.clicked.connect(self.listClear)
    pass

  def timerUpdate(self, timerCnt, justConnected):
    if self.listChanged:
      print("# setting list to " + self.list)
      self.ui.plainTextEdit_gui_list.setPlainText(self.list)
      self.listChanged = False
  #
  def decode(self, gg):
    # no associated communication
    dataUsed = False
    return dataUsed
  
  def listClear(self):
    self.list = ""
    self.listChanged = True

  def listRobots(self):
    print("# list robots " + str(len(self.main.robotInfo.robots)))
    sr = sorted(self.main.robotInfo.robots, key=lambda rr: rr.robotID)
    # columns described
    self.list = ("# Robot list (for known robots)\n" +
                 "# 1 \tRobot ID\n" + 
                 "# 2 \tRobot name\n" + 
                 "# 3 \tHardware type\n" + 
                 "# 4 \tGear ratio\n" + 
                 "# 5 \tEncoder pulses per revolution\n" +
                 "# 6-7 \tWheel radius (m)\n" +
                 "# 8 \tDate of data source\n")
    #wheelbase = 0.0
    #gear = 0.0
    #pulsePerRev = 0
    #wheelLRadius = 0.0
    #wheelRRadius = 0.0
    #version = ""
    #robotID = 0
    #robotHWtype = -1 # used versions 3..8
    #balanceOffset = 0.0
    #batteryUse = True
    #reverseMotor = False
    #reverseEncoderLeft = False
    #reverseEncoderRight = False
    #batteryIdleVolt = 9.9
    for lr in sr:
      if lr.robotID > 0:
        self.list += (str(lr.robotID) + " \t" + lr.name + " \t" + str(lr.robotHWtype) + " " +
                     str(lr.gear) + " \t" + str(lr.pulsePerRev) + " \t" +
                     str(lr.wheelLRadius) + " \t" + str(lr.wheelRRadius) + " \t" + 
                     lr.dataDate + " \n")
    self.listChanged = True
    pass
  
  def listMac(self):
    sr = sorted(self.main.robotInfo.robots, key=lambda rr: rr.robotID)
    print("# list MACs " + str(len(self.main.robotInfo.robots)))
    self.list = ("# Robot MACs (for known robots sorted by robot ID)\n" +
                 "# 1 \tRobot ID\n" +
                 "# 2 \tBridge host name\n" +
                 "# 3 \tBridge IP\n" +
                 "# 4 \tBridge MAC\n"
                 )
    #name = "empty"
    #gotRobotName = False
    #wifiIP = "0.0.0.0"             # network IP of raspberry pi - if available
    #wifiMAC= "00:00:00:00:00:00"   # MAC of raspberry pi wifi port
    #dataDate = "never"

    for lr in self.main.robotInfo.robots:
      if lr.robotID > 0:
        self.list += (str(lr.robotID) + "\t" +
                      lr.name + " \t" + 
                      lr.wifiIP + " \t" +
                      lr.wifiMAC + "\n")
    self.listChanged = True
    pass


