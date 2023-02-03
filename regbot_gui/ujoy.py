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

class UJoy(object):
  dataRead = False # for all servos
  lock = threading.RLock()
  lastDataRequestTime = time.time()
  lastDataSetTime = time.time()
  inEdit = False
  lastDataRequest = 0
  inTimerUpdate = True
  gotFirstData = False;
  joyPresent = False
  manualControl = False
  axisCount = 8
  axis = [0,0,0,0,0,0,0,0]
  buttonCount = 11
  buttons = [0,0,0,0,0,0,0,0,0,0,0]
  hasFocus = False
  #
  def __init__(self, parent):
    self.parent = parent
    self.main = parent.main
    self.ui = parent.ui
    
  def setup(self):
    pass
    
  def decode(self, gg):
    dataUsed = True
    self.lock.acquire()
    try:
      if gg[0] == 'joy':
        # joy 1 0 8 11 0 -2 -32767 0 -2 -32767 0 0 0 0 0 0 0 0 0 0 0 0 0'
        # 1:  1 = running
        # 2:  0 = not in manual control
        # 3:  8 = number of axis
        # 4: 11 = number of buttons
        # 5,6:  = axis 1,2
        # 7,8:  = axis 3,4
        # 9,10:  = axis 5,6
        # 11,12:  = axis 7,8
        # 13..24: = button 1..11
        if len(gg) > 23:
          self.joyPresent = int(gg[1],0)
          self.manualControl = int(gg[2],0)
          self.axisCount = int(gg[3], 0)
          self.buttonCount = int(gg[4],0)
          for a in range(self.axisCount):
            self.axis[a] = int(gg[a + 5],0)
          for b in range(self.buttonCount):
            self.buttons[b] = int(gg[b + 13],0)
          self.dataRead = True;
          self.gotFirstData = True
        else:
          print("Failed joy message too short {} values need 24!".format(str(len(gg))))
      else:
        dataUsed = False
    except:
      print("Joy: data read error - skipped a " + gg[0])
      pass
    self.lock.release()
    return dataUsed


  def timerUpdate(self, timerCnt, justConnected):
    self.lock.acquire()
    self.inTimerUpdate = True
    if self.dataRead:
      self.ui.joy_axis_1.setValue(self.axis[0])
      self.ui.joy_axis_1_num.setText("1: " + str(self.axis[0]))
      self.ui.joy_axis_2.setValue(self.axis[1])
      self.ui.joy_axis_2_num.setText("2: " + str(self.axis[1]))
      self.ui.joy_axis_3.setValue(self.axis[2])
      self.ui.joy_axis_3_num.setText("3: " + str(self.axis[2]))
      self.ui.joy_axis_4.setValue(self.axis[3])
      self.ui.joy_axis_4_num.setText("4: " + str(self.axis[3]))
      self.ui.joy_axis_5.setValue(self.axis[4])
      self.ui.joy_axis_5_num.setText("5: " + str(self.axis[4]))
      self.ui.joy_axis_6.setValue(self.axis[5])
      self.ui.joy_axis_6_num.setText("6: " + str(self.axis[5]))
      self.ui.joy_axis_7.setValue(self.axis[6])
      self.ui.joy_axis_7_num.setText("7: " + str(self.axis[6]))
      self.ui.joy_axis_8.setValue(self.axis[7])
      self.ui.joy_axis_8_num.setText("8: " + str(self.axis[7]))
      self.ui.joy_button_1.setChecked(self.buttons[0])
      self.ui.joy_button_2.setChecked(self.buttons[1])
      self.ui.joy_button_3.setChecked(self.buttons[2])
      self.ui.joy_button_4.setChecked(self.buttons[3])
      self.ui.joy_button_5.setChecked(self.buttons[4])
      self.ui.joy_button_6.setChecked(self.buttons[5])
      self.ui.joy_button_7.setChecked(self.buttons[6])
      self.ui.joy_button_8.setChecked(self.buttons[7])
      self.ui.joy_button_9.setChecked(self.buttons[8])
      self.ui.joy_button_10.setChecked(self.buttons[9])
      self.ui.joy_button_11.setChecked(self.buttons[10])
      if self.manualControl:
        self.ui.joy_manual.setText("Manual")
      else:
        self.ui.joy_manual.setText("Auto (mission)")
      if self.joyPresent:
        self.ui.joy_available.setText("Available")
      else:
        self.ui.joy_available.setText("Not available")
      self.dataRead = False
        #        
    #
    thisTab = self.ui.tabPages.indexOf(self.ui.tab_joy)
    if (self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() != thisTab:
      # just switched away from this tab
      self.hasFocus = False
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        self.main.devWrite(":joy subscribe 0\n")
      else:
        # talking to Teensy directly, so subscribe here
        self.main.devWrite("sub joy 0\n") # stop subs
        pass
      pass
    if (not self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() == thisTab:
      # just entering this tab
      self.hasFocus = True
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        self.main.devWrite(":joy subscribe -1\n")
      else:
        # talking to Teensy directly, so subscribe here
        self.main.devWrite("sub joy 150\n") # log entry count
      pass
    pass
    self.lock.release()
    #
    

  def cancelEdit(self):
    self.inEdit = False;

  def edit(self):
    self.inEdit = True;

  def apply(self):
    #self.applyServo1()
    #self.lastDataRequestTime = time.time()
    self.inEdit = False;

  
  
