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
#import pyqtgraph as pg
from datetime import datetime

 

class URobot(object):
  wheelbase = 0.0
  gear = 0.0
  pulsePerRev = 0
  wheelLRadius = 0.0
  wheelRRadius = 0.0
  version = ""
  robotID = 0
  robotHWtype = -1 # used versions 3..8
  balanceOffset = 0.0
  batteryUse = True
  reverseMotor = False
  batteryIdleVolt = 9.9
  name = "empty"
  gotRobotName = False
  wifiIP = "0.0.0.0"             # network IP of raspberry pi - if available
  wifiMAC= "00:00:00:00:00:00"   # MAC of raspberry pi wifi port
  dataDate = "never"
  #wifiport = 24001
  def pp(self, ppp):
    # debug
    print("# ppp " + ppp)
  def saveToIni(self, config):
    subj = 'robot' + str(self.robotID)
    config.add_section(subj)
    config.set(subj, 'id', str(self.robotID))
    config.set(subj, 'name', str(self.name))
    config.set(subj, 'robotHWtype', str(self.robotHWtype))
    config.set(subj, 'version', str(self.version))
    config.set(subj, 'gear', str(self.gear))
    config.set(subj, 'wheelBase', str(self.wheelbase))
    config.set(subj, 'pulsePerRev', str(self.pulsePerRev))
    config.set(subj, 'wheelLRadius', str(self.wheelLRadius))
    config.set(subj, 'wheelRRadius', str(self.wheelRRadius))
    config.set(subj, 'balanceOffset', str(self.balanceOffset))
    config.set(subj, 'wifiIP', self.wifiIP)
    config.set(subj, 'wifiMAC', self.wifiMAC)
    config.set(subj, 'datadate', self.dataDate)
    config.set(subj, 'reverse', str(self.reverseMotor))
    pass
  def loadFromIni(self, config):
    subj = 'robot' + str(self.robotID)
    self.name = config.get(subj, "name")
    self.robotHWtype = int(config.get(subj, "robotHWtype"))
    self.version = config.get(subj, "version")
    self.gear = float(config.get(subj, "gear"))
    self.wheelbase = float(config.get(subj, "wheelBase"))
    self.pulsePerRev = int(config.get(subj, "pulsePerRev"))
    self.wheelLRadius = float(config.get(subj, "wheelLRadius"))
    self.wheelRRadius = float(config.get(subj, "wheelRRadius"))
    self.balanceOffset = float(config.get(subj, "balanceOffset"))
    self.wifiIP = config.get(subj, "wifiIP")
    self.wifiMAC = config.get(subj, "wifiMAC")
    self.dataDate = config.get(subj, "datadate")
    self.reverseMotor = int(config.get(subj, "reverse"))
    pass

class URobotInfo(object):
  dataRead = True
  dataWifi = True
  dataClient = True
  robotID = 0
  gyroOK = False
  lock = threading.RLock()
  robots = [URobot]
  thisRobot = robots[0]
  inEdit = False
  lastDataRequestTime = time.time()
  lastDataRequest = 0
  lastAliveTime = 0.0
  talkToBridge = False
  talkToBridgeOld = False;
  lastTab = ""
  regbotSampleTime = 0.001
  dataReadTiming = False;
  hasFocus = False
  gotConfData = False
  gotBoardData = False
  gotHwType = False
  justConnectedAt = 0;
  #  
  def __init__(self, parent):
    self.main = parent.main
    self.ui = parent.ui
    
  def setup(self):
    self.ui.robot_edit.clicked.connect(self.clickedEdit)
    self.ui.robot_cancel.clicked.connect(self.cancelEdit)
    self.ui.robot_apply.clicked.connect(self.clickedApply)
    self.ui.robot_pose_reset.clicked.connect(self.resetPose)
    pass

  def gotAliveMsg(self):
    self.lastAliveTime = time.time()
  # find robot with this ID, and create it if it is not there
  def getRobot(self, id):
    rb = []
    for rb in self.robots:
      if rb.robotID == id:
        #print("found robot " + str(id))
        return rb
    rb = URobot()
    rb.robotID = id
    self.robots.append(rb)
    #if (id > 0):
      #print("# created robot object with id " + str(id) + ", now " + str(len(self.robots)) + " known robots")
    return rb
  #
  def decode(self, gg):
    used = True
    self.lock.acquire()
    try:
      #snprintf(s, MSL, "conf %.4f %.4f %.3f %u %4f %4f\r\n", odoWheelRadius[0], odoWheelRadius[1], 
      #gear, pulsPerRev, odoWheelBase, SAMPLETIME
      if gg[0] == "conf":
        self.thisRobot = self.getRobot(self.main.deviceID)
        self.thisRobot.wheelLRadius = float(gg[1])
        self.thisRobot.wheelRRadius = float(gg[2])
        self.thisRobot.gear = float(gg[3])
        self.thisRobot.pulsePerRev = int(gg[4], 10)
        self.thisRobot.wheelbase = float(gg[5])
        self.thisRobot.sampleTime = float(gg[6])
        self.thisRobot.reverseMotor = int(gg[7])
        self.thisRobot.dataDate = str(datetime.now())
        self.gotConfData = True
      elif gg[0] == "board":
        self.thisRobot = self.getRobot(self.main.deviceID)
        rx = float(gg[1])
        ry = float(gg[2])
        rz = float(gg[3])
        self.thisRobot.balanceOffset = ry
        self.gotBoardData = True;
      elif gg[0] == "version":
        self.thisRobot = self.getRobot(self.main.deviceID)
        self.thisRobot.version = gg[1] + " (" + gg[3] + " " + gg[4] + ")"
      elif gg[0] == "ip":
        if len(gg) > 2:
          self.thisRobot.wifiIP = gg[1] + " " + gg[2]
        else:
          self.thisRobot.wifiIP = gg[1]
      elif gg[0] == "mac":
        self.thisRobot.wifiMAC = ""
        for pp in range(1, len(gg)):
          self.thisRobot.wifiMAC += gg[pp] + " "
      #
      else:
        used = False
    except:
      print("URobot: data read error - skipped a " + gg[0])
      pass
    self.lock.release()
    return used
  #
  #
  def timerUpdate(self, timerCnt, justConnected):
    if justConnected:
      self.justConnectedAt = timerCnt
    if timerCnt == self.justConnectedAt + 15:
      if self.main.isBridge():
        self.main.devWrite("host:mac get\n") 
        self.main.devWrite("host:ip get\n") 
      else:
        # get some base data
        self.main.devWrite("confi\n")
    if self.ui.robot_id.value() != self.main.deviceID:
      if not self.inEdit:
        self.ui.robot_id.setValue(self.main.deviceID)
    if (self.gotConfData):
      if (not self.inEdit):
        self.gotConfData = False
        self.lock.acquire()
        # set new values
        self.ui.robot_gear.setProperty("value", self.thisRobot.gear)
        self.ui.robot_pulse_per_rev.setValue(self.thisRobot.pulsePerRev)
        self.ui.robot_wheel_radius_left.setValue(self.thisRobot.wheelLRadius)
        self.ui.robot_wheel_radius_right.setValue(self.thisRobot.wheelRRadius)
        self.ui.robot_hw_type.setValue(self.thisRobot.robotHWtype)
        self.ui.robot_balance_offset.setValue(self.thisRobot.balanceOffset)
        self.ui.robot_base.setValue(self.thisRobot.wheelbase)
        #self.ui.robot_id.setValue(self.robotID)
        ##self.ui.label_ID.setText(self.thisRobot.name + " (" + str(self.robotID) + ")")
        #self.ui.robot_on_battery.setChecked(self.thisRobot.batteryUse)
        self.ui.robot_rev_motor.setChecked(self.thisRobot.reverseMotor)
        #self.ui.robot_battery_idle_volt.setValue(self.thisRobot.batteryIdleVolt)
        self.lock.release()
    if self.gotBoardData:
      if not self.inEdit:
        self.ui.robot_balance_offset.setValue(self.thisRobot.balanceOffset)
        self.gotBoardData = False;
    if (self.dataReadTiming):
      # static timing
      self.dataReadTiming = False
      # set new values
      self.ui.regbotSampleTime.setProperty("value", self.regbotSampleTime)
    if self.gotHwType:
      if not self.inEdit:
        self.thisRobot = self.getRobot(self.main.deviceID)
        self.ui.robot_hw_type.setValue(self.thisRobot.robotHWtype)
        self.gotHwType = False;
    if (not self.inEdit):
      # and buttons
      self.ui.robot_apply.setEnabled(False)
      self.ui.robot_cancel.setEnabled(False)
      self.ui.robot_edit.setEnabled(True)
    else:
      self.ui.robot_apply.setEnabled(True)
      self.ui.robot_cancel.setEnabled(True)
      self.ui.robot_edit.setEnabled(False)
    # wifi alive
    if time.time() - self.lastAliveTime > 5:
      self.ui.wifi_sendAlive.setChecked(False)
    else:
      self.ui.wifi_sendAlive.setChecked(True)
    # reqest data periodically, as these data may change
    thisTab = self.ui.tabPages.indexOf(self.ui.tab_robot) # robot hardware info
    if (self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() != thisTab:
      # just switched away from this tab
      self.hasFocus = False
      # if we are talking to a bridge - then stop subscriptions here
      if self.main.isBridge():
        self.main.devWrite("regbot:board subscribe 0\n")
        self.main.devWrite("regbot:conf subscribe 0\n")
        self.main.devWrite("host:netip subscribe 0\n")
      else:
        # talking to Teensy directly, so stop subscriptions here
        self.main.devWrite("sub board 0\n") 
        self.main.devWrite("sub conf 0\n") 
        pass
      pass
    if (not self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() == thisTab:
      # just entering this tab
      self.hasFocus = True
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        self.main.devWrite("regbot:board subscribe -1\n")
        self.main.devWrite("regbot:conf subscribe -1\n")
        self.main.devWrite("host:netip subscribe -1\n")
      else:
        # talking to Teensy directly, so subscribe here
        #print("# robot_info requests board and conf")
        self.main.devWrite("sub board 320\n")
        self.main.devWrite("sub conf 321\n")
        pass
      pass
    pass
  # 
  #def dataChangedManually(self):
    ## robot static parameters
    #if (not self.main.timerUpdate):
      #self.lock.acquire()
      #self.inEdit = True
      #self.lock.release()
    
  def setHwType(self, id, hwType):
    self.thisRobot = self.getRobot(id)
    self.thisRobot.robotHWtype = hwType
    self.gotHwType = True
    
  def setName(self, id, name):
    self.thisRobot = self.getRobot(id)
    self.thisRobot.name = name
    self.gotHwType = True

  def clickedApply(self):
    self.inEdit = False;
    changeID = self.main.deviceID != self.ui.robot_id.value()
    if changeID:
      self.main.devWrite("setidx {:d}\n".format(int(self.ui.robot_id.value())), True)
    else:
      self.thisRobot = self.getRobot(self.main.deviceID)
      if self.thisRobot.robotHWtype != self.ui.robot_hw_type.value():
        self.main.devWrite("sethw {:d}\n".format(int(self.ui.robot_hw_type.value())), True)
        pass
      if self.thisRobot.reverseMotor != self.ui.robot_rev_motor.isChecked():
        self.main.devWrite("motr {:d}\n".format(self.ui.robot_rev_motor.isChecked()), True)
      self.main.devWrite("confw {:f} {:f} {:f} {:d} {:f}\n".format(
        self.ui.robot_wheel_radius_left.value(), 
        self.ui.robot_wheel_radius_right.value(),
        self.ui.robot_gear.value(),
        int(self.ui.robot_pulse_per_rev.value()),
        self.ui.robot_base.value()), True)
      if abs(self.thisRobot.balanceOffset - self.ui.robot_balance_offset.value()) > 0.0001:
        self.main.devWrite("board 0 {:f} 0\n".format(self.ui.robot_balance_offset.value()), True)
    pass
  
  def clickedEdit(self):
    self.inEdit = True;
  def cancelEdit(self):
    self.inEdit = False;
  # send data as is in edit fields
  def resetPose(self):
    self.main.devWrite("enc0\n", True)
    self.main.pose.poseReset()

  def saveToIniFile(self, config):
    # save in increasing order
    sr = sorted(self.robots, key=lambda rr: rr.robotID)
    rb = []
    for rb in sr:
      if (rb.robotID > 0):
        print("# saving for robot " + str(rb.robotID))
        rb.saveToIni(config)
        #self.pp(rb, config)
    pass
  
  #def pp(self, rp, config):
      #subj = 'robot' + str(rp.robotID)
      #config.add_section(subj)
      #config.set(subj, 'id', str(rp.robotID))
      #config.set(subj, 'name', str(rp.name))
      #config.set(subj, 'robotHWtype', str(rp.robotHWtype))
      #config.set(subj, 'version', str(rp.version))
      #config.set(subj, 'gear', str(rp.gear))
      #config.set(subj, 'wheelBase', str(rp.wheelbase))
      #config.set(subj, 'pulsePerRev', str(rp.pulsePerRev))
      #config.set(subj, 'wheelLRadius', str(rp.wheelLRadius))
      #config.set(subj, 'wheelRRadius', str(rp.wheelRRadius))
      #config.set(subj, 'balanceOffset', str(rp.balanceOffset))
      #config.set(subj, 'wifiIP', str(rp.wifiIP))
      #config.set(subj, 'wifiMAC', str(rp.wifiMAC))
      #config.set(subj, 'dataDate', dataDate)
  
  def loadFromIniFile(self, config):
    rb = []
    # reload all saved robot configurations
    for r in range(1,200):
      subj = 'robot' + str(r)
      try:
        config.get(subj, "name")
        print("# loading for robot " + str(r))
        rb = self.getRobot(r)
        rb.loadFromIni(config)
      except:
        # no such robot
        pass
    pass
    
