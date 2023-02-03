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
import numpy as np
#import pyqtgraph as pg
import time


class ULineSensor(object):
  # line sensor
  lineValue = [0,0,0,0,0,0,0,0]
  lineValN = [0,0,0,0,0,0,0,0]
  lineMaxWhite = [0,0,0,0,0,0,0,0]
  lineMaxBlack = [0,0,0,0,0,0,0,0]
  lineWhite = False
  lineUse = True
  edgeLeft = 0.0
  edgeRight = 0.0
  edgeLeftValid = False
  edgeRightValid = False
  followLeft = True
  wideSensor = False;
  swapLeftRight = False;
  # crossing
  crossingDetectLimit = 4;
  crossingWhite = False
  crossingBlack = False
  crossingWhiteCnt = 0
  #crossingBlackCnt = 0
  power_high = False
  power_auto = True
  # management
  dataReadLiv = False
  dataReadLivn = False
  dataReadLip = False
  dataReadbw = True
  #
  inUpdate = False
  inEdit = False
  nextDataRequest = 1
  lastDataRequestTime = time.time()
  #
  hasFocus = False
  thisTab = -1
  inTimerUpdate = False

  #
  about_box = None
  # resource lock
  lock = threading.RLock()

  def __init__(self, parent):
    self.parent = parent
    self.main = parent.main
    self.ui = parent.ui
    
  def setup(self):
    # Edge/line sensor
    self.ui.line_disp_max_value.valueChanged.connect(self.max_value_changed)
    self.ui.ls_use_sensor.clicked.connect(self.setWhiteLine)
    self.ui.ls_calibrate_white.clicked.connect(self.calibrateWhite)
    self.ui.ls_calibrate_black.clicked.connect(self.calibrateBlack)
    self.ui.ls_apply.clicked.connect(self.butApply) 
    self.ui.ls_cancel.clicked.connect(self.butCancel)
    self.ui.ls_edit.clicked.connect(self.butEdit) 
    pass

  def decode(self, gg):
    used = True
    self.lock.acquire()
    try:
      if gg[0] == "liv":
        # actual data
        self.lineValue[0] = int(gg[1],0)
        self.lineValue[1] = int(gg[2],0)
        self.lineValue[2] = int(gg[3],0)
        self.lineValue[3] = int(gg[4],0)
        self.lineValue[4] = int(gg[5],0)
        self.lineValue[5] = int(gg[6],0)
        self.lineValue[6] = int(gg[7],0)
        self.lineValue[7] = int(gg[8],0)
        pass
        self.dataReadLiv = True
      elif gg[0] == "livn":
        # actual data
        self.lineValN[0] = int(gg[1],0)
        self.lineValN[1] = int(gg[2],0)
        self.lineValN[2] = int(gg[3],0)
        self.lineValN[3] = int(gg[4],0)
        self.lineValN[4] = int(gg[5],0)
        self.lineValN[5] = int(gg[6],0)
        self.lineValN[6] = int(gg[7],0)
        self.lineValN[7] = int(gg[8],0)
        pass
        self.dataReadLivN = True
      elif gg[0] == "liw":
        # white calibration
        self.lineMaxWhite[0] = int(gg[1],0)
        self.lineMaxWhite[1] = int(gg[2],0)
        self.lineMaxWhite[2] = int(gg[3],0)
        self.lineMaxWhite[3] = int(gg[4],0)
        self.lineMaxWhite[4] = int(gg[5],0)
        self.lineMaxWhite[5] = int(gg[6],0)
        self.lineMaxWhite[6] = int(gg[7],0)
        self.lineMaxWhite[7] = int(gg[8],0)
        self.dataReadbw = True
      elif gg[0] == "lib":
        # black calibration
        self.lineMaxBlack[0] = int(gg[1],0)
        self.lineMaxBlack[1] = int(gg[2],0)
        self.lineMaxBlack[2] = int(gg[3],0)
        self.lineMaxBlack[3] = int(gg[4],0)
        self.lineMaxBlack[4] = int(gg[5],0)
        self.lineMaxBlack[5] = int(gg[6],0)
        self.lineMaxBlack[6] = int(gg[7],0)
        self.lineMaxBlack[7] = int(gg[8],0)
        self.dataReadbw = True
      elif gg[0] == "lip":
        # derived data
        self.lineUse = int(gg[1],0)
        self.lineWhite = int(gg[2],0)
        self.edgeLeft = float(gg[3])
        self.edgeLeftValid = int(gg[4],0)
        self.edgeRight = float(gg[5])
        self.edgeRightValid = int(gg[6],0)
        self.followLeft = int(gg[7],0)
        self.crossingWhite = int(gg[8],0)
        #self.crossingBlack = int(gg[9],0) # not used anymore
        self.crossingWhiteCnt = int(gg[10],0)
        #self.crossingBlackCnt = int(gg[11],0) # not used anymore
        self.power_high = int(gg[12],0)
        self.power_auto = int(gg[13],0)
        if (len(gg) > 14):
          self.crossingDetectLimit = float(gg[14])
        if (len(gg) > 15):
          self.wideSensor = int(gg[15])
        if (len(gg) > 16):
          self.swapLeftRight = int(gg[16])
        self.dataReadLip = True
      else:
        used = False
    except:
      print("URegLine: data read error - skipped a " + gg[0] + " length=" + str(len(gg)))
      pass
    self.lock.release()
    return used
  #

  def timerUpdate(self, timerCnt, justConnected):
    self.lock.acquire()
    if self.dataReadbw:
      # calibration settings
      self.dataReadbw = False
      self.ui.ls_max_white_1.setText(str(self.lineMaxWhite[0]))
      self.ui.ls_max_white_2.setText(str(self.lineMaxWhite[1]))
      self.ui.ls_max_white_3.setText(str(self.lineMaxWhite[2]))
      self.ui.ls_max_white_4.setText(str(self.lineMaxWhite[3]))
      self.ui.ls_max_white_5.setText(str(self.lineMaxWhite[4]))
      self.ui.ls_max_white_6.setText(str(self.lineMaxWhite[5]))
      self.ui.ls_max_white_7.setText(str(self.lineMaxWhite[6]))
      self.ui.ls_max_white_8.setText(str(self.lineMaxWhite[7]))
      self.ui.ls_max_black_1.setText(str(self.lineMaxBlack[0]))
      self.ui.ls_max_black_2.setText(str(self.lineMaxBlack[1]))
      self.ui.ls_max_black_3.setText(str(self.lineMaxBlack[2]))
      self.ui.ls_max_black_4.setText(str(self.lineMaxBlack[3]))
      self.ui.ls_max_black_5.setText(str(self.lineMaxBlack[4]))
      self.ui.ls_max_black_6.setText(str(self.lineMaxBlack[5]))
      self.ui.ls_max_black_7.setText(str(self.lineMaxBlack[6]))
      self.ui.ls_max_black_8.setText(str(self.lineMaxBlack[7]))
    if (self.dataReadLip):
      self.ui.ls_left_side.setValue(self.edgeLeft)
      self.ui.ls_right_side.setValue(self.edgeRight)
      # Show editable settings
      if (self.dataReadLip and not self.inEdit):
        self.dataReadLip = False
        self.inUpdate = True
        self.ui.ls_use_sensor.setChecked(self.lineUse)
        self.ui.ls_line_white.setChecked(self.lineWhite)
        #self.ui.ls_left_side_valid.setChecked(self.edgeLeftValid)
        #self.ui.ls_right_side_valid.setChecked(self.edgeRightValid)
        ev = 100 - (self.edgeLeft + 2.5) * 20.0
        if (ev < 0):
          ev = 0
        elif ev >100:
          ev = 100
        elif np.isnan(ev):
          ev = 0
        if self.edgeLeftValid:
          self.ui.ls_left_bar.setValue(int(ev))
        else:
          self.ui.ls_left_bar.setValue(-3)
        ev = (self.edgeRight + 2.5) * 20.0
        if (ev < 0):
          ev = 0
        elif ev >100:
          ev = 100
        elif np.isnan(ev):
          ev = 0
        if self.edgeRightValid:
          self.ui.ls_right_bar.setValue(int(ev))
        else:
          self.ui.ls_right_bar.setValue(-3)
        #self.ui.ls_left_bar.setEnabled(self.edgeLeftValid)
        #self.ui.ls_right_bar.setEnabled(self.edgeRightValid)
        #self.ui.ls_follow_left.setChecked(self.followLeft)
        self.ui.ls_crossing_cnt.setText(str(self.crossingWhiteCnt))
        self.ui.ls_line_valid_cnt.setText(str(self.edgeLeftValid))
        #self.ui.ls_crossing_black.setValue(self.crossingBlackCnt)
        #self.ui.frame_ls_crossing_white.setEnabled(self.crossingWhite)
        #self.ui.frame_ls_crossing_black.setEnabled(self.crossingBlack)
        self.ui.ls_power_high.setChecked(self.power_high)
        self.ui.ls_power_auto.setChecked(self.power_auto)
        self.ui.ls_crossing_detect.setValue(self.crossingDetectLimit)
        self.ui.ls_wideSensor.setChecked(self.wideSensor)
        self.ui.ls_swap_left_right.setChecked(self.swapLeftRight)
    # Show line sensor bar values
    mv = self.ui.line_disp_max_value.value() - 1
    if (self.dataReadLiv):
      self.dataReadLiv = False
      self.inUpdate = True
      if self.ui.ls_show_normalized.isChecked():
        val = self.lineValN
      else:
        val = self.lineValue
      if (val[0] > mv):
        self.ui.line_bar_1.setValue(mv)
      elif val[0] < 0:
        self.ui.line_bar_1.setValue(0)
      else:
        self.ui.line_bar_1.setValue(val[0])
      self.ui.ls_actual_1.setText(str(val[0]))
      if (val[1] > mv):
        self.ui.line_bar_2.setValue(mv)
      elif val[1] < 0:
        self.ui.line_bar_2.setValue(0)
      else:
        self.ui.line_bar_2.setValue(val[1])
      self.ui.ls_actual_2.setText(str(val[1]))
      if (val[2] > mv):
        self.ui.line_bar_3.setValue(mv)
      elif val[2] < 0:
        self.ui.line_bar_3.setValue(0)
      else:
        self.ui.line_bar_3.setValue(val[2])
      self.ui.ls_actual_3.setText(str(val[2]))
      if (val[3] > mv):
        self.ui.line_bar_4.setValue(mv)
      elif val[3] < 0:
        self.ui.line_bar_4.setValue(0)
      else:
        self.ui.line_bar_4.setValue(val[3])
      self.ui.ls_actual_4.setText(str(val[3]))
      if (val[4] > mv):
        self.ui.line_bar_5.setValue(mv)
      elif val[4] < 0:
        self.ui.line_bar_5.setValue(0)
      else:
        self.ui.line_bar_5.setValue(val[4])
      self.ui.ls_actual_5.setText(str(val[4]))
      if (val[5] > mv):
        self.ui.line_bar_6.setValue(mv)
      elif val[5] < 0:
        self.ui.line_bar_6.setValue(0)
      else:
        self.ui.line_bar_6.setValue(val[5])
      self.ui.ls_actual_6.setText(str(val[5]))
      if (val[6] > mv):
        self.ui.line_bar_7.setValue(mv)
      elif val[6] < 0:
        self.ui.line_bar_7.setValue(0)
      else:
        self.ui.line_bar_7.setValue(val[6])
      self.ui.ls_actual_7.setText(str(val[6]))
      if (val[7] > mv):
        self.ui.line_bar_8.setValue(mv)
      elif val[7] < 0:
        self.ui.line_bar_8.setValue(0)
      else:
        self.ui.line_bar_8.setValue(val[7])
      self.ui.ls_actual_8.setText(str(val[7]))
    self.inUpdate = False
    # connected to robot
    #self.ui.ls_line_white.setEnabled(self.ui.frame_batt_time.isEnabled())
    self.lock.release()
    #
    thisTab = self.ui.tabPages.indexOf(self.ui.tab_edge)
    if (self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() != thisTab:
      # just switched away from this tab
      self.hasFocus = False
      if self.main.isBridge():
        #print("# Edge talking to bridge")
        # subscribe to bridge data
        self.main.devWrite(":liw subscribe 0\n") # calibrate white
        self.main.devWrite(":lib subscribe 0\n") # calibrate black
        self.main.devWrite(":liv subscribe 0\n") # sensor values
        self.main.devWrite(":livn subscribe 0\n") # sensor values normalized
        self.main.devWrite(":lip subscribe 0\n") # flags and result
        self.main.devWrite("regbot sub liw 0\n") # calibrate white
        self.main.devWrite("regbot sub lib 0\n") # calibrate black
        self.main.devWrite("regbot sub liv 0\n") # sensor values
        self.main.devWrite("regbot sub livn 0\n") # sensor values normalized
        self.main.devWrite("regbot sub lip 0\n") # flags and result
      else:
        #unsubscribe from regbot directly
        self.main.devWrite("sub liw 0\n") # calibrate white
        self.main.devWrite("sub lib 0\n") # calibrate black
        self.main.devWrite("sub liv 0\n") # sensor values
        self.main.devWrite("sub livn 0\n") # sensor values normalized
        self.main.devWrite("sub lip 0\n") # flags and result
    if (not self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() == thisTab:
      # just entering this tab
      self.hasFocus = True
      if self.main.isBridge():
        self.configChanged()
        # subscribe to bridge data
        self.main.devWrite("regbot sub liw 300\n") # calibrate white
        self.main.devWrite("regbot sub lib 301\n") # calibrate black
        self.main.devWrite("regbot sub liv 100\n") # sensor values
        self.main.devWrite("regbot sub lip 104\n") # flags and result
        self.main.devWrite(":liw subscribe -1\n") # calibrate white
        self.main.devWrite(":lib subscribe -1\n") # calibrate black
        self.main.devWrite(":liv subscribe -1\n") # sensor values
        self.main.devWrite(":livn subscribe -1\n") # sensor values normalized
        self.main.devWrite(":lip subscribe -1\n") # flags and result
        self.max_value_changed()
        print("# ULineSensor:: requested data from bridge")
      else:
        self.main.devWrite("sub liw 300\n") # calibrate white
        self.main.devWrite("sub lib 301\n") # calibrate black
        self.main.devWrite("sub liv 100\n") # sensor values
        self.main.devWrite("sub livn 103\n") # sensor values normalized
        self.main.devWrite("sub lip 104\n") # flags and result
        self.max_value_changed()
        print("# ULineSensor:: requested data from Teensy")

  #
  def helpbox(self):
    # QMessageBox.about (QWidget parent, QString caption, QString text)
    if (self.about_box == None):
      self.about_box = QtGui.QMessageBox(self.main.parent)
      self.about_box.setText('''<p><span style=" font-size:20pt;">
                Line sensor</span></p>
                <p>
                Values are difference between illuminated and not illuminated in A/D units.
                </p>
                <hr />
                <p>
                Left and right edge is in meters relative to center of robot.
                </p>
                ''');
      #about_box.setIconPixmap(QtGui.QPixmap("dtulogo_125x182.png"))
      self.about_box.setWindowTitle("Line sensor help")
      self.about_box.setWindowModality(QtCore.Qt.NonModal)
    self.about_box.show()

  def max_value_changed(self):
    v = self.ui.line_disp_max_value.value()
    self.ui.line_bar_1.setMaximum(v)
    self.ui.line_bar_2.setMaximum(v)
    self.ui.line_bar_3.setMaximum(v)
    self.ui.line_bar_4.setMaximum(v)
    self.ui.line_bar_5.setMaximum(v)
    self.ui.line_bar_6.setMaximum(v)
    self.ui.line_bar_7.setMaximum(v)
    self.ui.line_bar_8.setMaximum(v)

  def configChanged(self):
    # load the right value into step from-to widget
    self.inUpdate = True
    self.ui.ls_right_side.setEnabled(self.ui.ls_use_sensor.isChecked())
    self.ui.ls_left_side.setEnabled(self.ui.ls_use_sensor.isChecked())
    self.max_value_changed()
    self.inUpdate = False
    #pass

  def sensorOnClicked(self):
    self.ui.ls_use_sensor.setChecked(self.ui.ls_sensor_on.isChecked())
    self.setWhiteLine()

  def setWhiteLine(self):
    # send white line assumption to robot
    self.main.devWrite("lip %d %d %d %d %g %d %d\n" % (
            self.ui.ls_use_sensor.isChecked(),
            self.ui.ls_line_white.isChecked(),
            self.ui.ls_power_high.isChecked(),
            self.ui.ls_power_auto.isChecked(),
            self.ui.ls_crossing_detect.value(),
            self.ui.ls_wideSensor.isChecked(),
            self.ui.ls_swap_left_right.isChecked()
            ), True)

  def calibrateWhite(self):
    # send white line assumption to robot
    self.main.devWrite("licw\n", True)

  def calibrateBlack(self):
    # send white line assumption to robot
    self.main.devWrite("licb\n", True)

  def butEdit(self):
    if not self.inTimerUpdate:
      self.lock.acquire()
      self.inEdit = True
      self.lock.release()
  # when any of the parameters are changed - allow apply button to be pressed

  def butCancel(self):
    # cancel button pressed
    self.inEdit = False
  #

  def butApply(self):
    self.setWhiteLine()
    self.inEdit = False
    

