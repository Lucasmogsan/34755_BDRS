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
#import numpy as np
#import pyqtgraph as pg
import time
from PyQt5 import QtWidgets, QtCore, QtGui



class UIRDistance(object):
  # sensor control
  sensorOn = False
  sensorInstalled = False
  # measurements
  distS1 = 0
  distS2 = 0
  irRaw = [0, 0]
  irCal20cm = [3000, 3000]
  irCal80cm = [480, 480]
  # data management
  dataRead = False
  dataDistRead = False
  inUpdate = False
  nextCommand = 0
  inEdit2 = False # sensor buttons
  # help
  about_box = None
  # resource lock
  lock = threading.RLock()
  waitingForData = False
  lastDataRequestTime = time.time()
  hasFocus = False
  thisTab = -1


  def __init__(self, parent):
    #self.robot = robot
    self.ui = parent.ui
    self.main = parent.main

  def setup(self):
    #p = self.ui.tab_ir.palette() # IR distance
    #p.setColor(self.ui.tab_ir.backgroundRole(), QtCore.Qt.lightGray)
    #self.ui.tab_ir.setPalette(p)
    #self.ui.tab_ir.setAutoFillBackground(True)
    #
    self.ui.ir_apply.clicked.connect(self.paramApplyCal) 
    self.ui.ir_cancel.clicked.connect(self.paramCancelCal)
    self.ui.ir_edit.clicked.connect(self.dataEditCal)
    self.ui.ir_d1_20cm.valueChanged.connect(self.dataEditCal)
    self.ui.ir_d1_80cm.valueChanged.connect(self.dataEditCal)
    self.ui.ir_d2_20cm.valueChanged.connect(self.dataEditCal)
    self.ui.ir_d2_80cm.valueChanged.connect(self.dataEditCal)
    self.ui.checkBox_ir_use.clicked.connect(self.dataEditCal)
    pass

  def decode(self, gg):
    used = True
    self.lock.acquire()
    try:
      if gg[0] == "ir":
        self.distS1 = float(gg[1])
        self.distS2 = float(gg[2])
        self.irRaw[0] = int(gg[3],0)
        self.irRaw[1] = int(gg[4],0)
        self.irCal20cm[0] = int(gg[5],0)
        self.irCal80cm[0] = int(gg[6],0)
        self.irCal20cm[1] = int(gg[7],0)
        self.irCal80cm[1] = int(gg[8],0)
        self.sensorOn = int(gg[9],0)
        self.dataDistRead = True
        self.waitingForData = False
      else:
        used = False
    except:
      print("IR sensor: data read error - skipped a " + gg[0])
      pass
    self.lock.release()
    return used
  
  def timerUpdate(self, timerCnt, justConnected):
    self.lock.acquire()
    if (self.dataDistRead):
      self.dataDistRead = False
      self.inUpdate = True
      self.ui.ir_d1_meters.setText(str(self.distS1))
      self.ui.ir_d2_meters.setText(str(self.distS2))
      self.ui.ir_bar_1.setValue(self.irRaw[0])
      self.ui.ir_bar_2.setValue(self.irRaw[1])
      self.ui.ir_d1_raw.setValue(self.irRaw[0])
      self.ui.ir_d2_raw.setValue(self.irRaw[1])
      if not self.inEdit2:
        self.ui.ir_d1_20cm.setValue(self.irCal20cm[0])
        self.ui.ir_d2_20cm.setValue(self.irCal20cm[1])
        self.ui.ir_d1_80cm.setValue(self.irCal80cm[0])
        self.ui.ir_d2_80cm.setValue(self.irCal80cm[1])
        self.ui.checkBox_ir_use.setChecked(self.sensorOn)
      # enable buttons
      self.ui.ir_apply.setEnabled(self.inEdit2)
      self.ui.ir_edit.setEnabled(not self.inEdit2)
      self.ui.ir_cancel.setEnabled(self.inEdit2)
      #print("distance is " + str(self.distS1) + ", " + str(self.distS2))
      self.inUpdate = False
    # request new data
    thisTab = self.ui.tabPages.indexOf(self.ui.tab_ir)
    if (self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() != thisTab:
      # just switched away from this tab
      self.hasFocus = False
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        self.main.devWrite("regbot:ir subscribe 0\n")
      else:
        # talking to Teensy directly, so subscribe here
        self.main.devWrite("sub ir 0\n") # stop subs
        pass
      pass
    if (not self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() == thisTab:
      # just entering this tab
      self.hasFocus = True
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        self.main.devWrite("regbot:ir subscribe -1\n")
      else:
        # talking to Teensy directly, so subscribe here
        self.main.devWrite("sub ir 150\n") # log entry count
      pass
    pass
    self.lock.release()
  #
  # Button pressed or field changed
  def dataEditCal(self):
    self.lock.acquire()
    self.inEdit2 = True
    self.lock.release()
  #
  # when any of the parameters are changed - allow apply button to be pressed
  def paramCancelCal(self):
    # cancel button pressed
    self.inEdit2 = False
  #
  def paramApplyCal(self):
    self.main.devWrite("irc %g %g %g %g %d\n" % (
        self.ui.ir_d1_20cm.value(), 
        self.ui.ir_d1_80cm.value(), 
        self.ui.ir_d2_20cm.value(), 
        self.ui.ir_d2_80cm.value(),
        self.ui.checkBox_ir_use.isChecked(),
        ), True)
    self.inEdit2 = False

