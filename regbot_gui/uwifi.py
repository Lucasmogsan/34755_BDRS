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

 

class URobotInfo(object):
  wheelbase = 0.0
  gear = 0.0
  pulsePerRev = 0
  wheelLRadius = 0.0
  wheelRRadius = 0.0
  version = ""
  robotID = 0
  robotHWtype = 2 # type 1 is old with no wifi and line sensor, 2 if with satellite PCB for wifi and power, 3 is with power mgmt on board
  balanceOffset = 0.0
  batteryUse = True
  reverseMotor = False
  reverseEncoderLeft = False
  reverseEncoderRight = False
  batteryIdleVolt = 9.9
  name = "empty"
  gotRobotName = False
  wifiIP = "0.0.0.0"
  wifiMAC= "00:00:00:00:00:00"
  #wifiport = 24001

class UInfo(object):
  dataRead = True
  dataWifi = True
  dataClient = True
  robotID = 0
  gyroOK = False
  lock = threading.RLock()
  robots = [URobotInfo]
  thisRobot = robots[0]
  inEdit = False
  wifiUse = False
  wifiSSID = "aaa"
  wifiPW = ""
  wifiGotIP = False
  wifiPortOpen = False
  wifiInEdit = False
  wifiSetupState = -1;
  wifiSleep = False;
  wifiPort = 24001
  clientRxCnt = [0,0,0,0,0]
  clientTxCnt = [0,0,0,0,0]
  wifiGood = 0
  #wifiLost = 0
  wifiLoss = 0.0
  lastDataRequestTime = time.time()
  lastDataRequest = 0
  lastAliveTime = 0.0
  talkToBridge = False
  talkToBridgeOld = False;
  lastTab = ""
  regbotSampleTime = 0.001
  dataReadTiming = False;
  hasFocus = False
  #  
  def __init__(self, parent):
    self.main = parent.main
    self.ui = parent.ui

  def gotAliveMsg(self):
    self.lastAliveTime = time.time()
  # find robot with this ID, and create it if it is not there
  def getRobot(self, id):
    rb = []
    for rb in self.robots:
      if rb.robotID == id:
        #print("found robot " + str(id))
        return rb
    rb = URobotInfo()
    rb.robotID = id
    self.robots.append(rb)
    #print("made robot with id " + str(id))
    return rb
  #
  def decode(self, gg):
    used = True
    self.lock.acquire()
    try:
      if gg[0] == "confw":
        self.robotID = int(gg[1],10)
        self.thisRobot = self.getRobot(self.robotID)
        self.thisRobot.wheelbase = float(gg[2])
        self.thisRobot.gear = float(gg[3])
        self.thisRobot.pulsePerRev = int(gg[4], 10)
        self.thisRobot.wheelLRadius = float(gg[5])
        self.thisRobot.wheelRRadius = float(gg[6])
        self.thisRobot.balanceOffset = float(gg[7])
        flags = int(gg[8])
        self.thisRobot.batteryUse = flags & 1
        self.thisRobot.reverseMotor = flags & 2
        self.thisRobot.reverseEncoderLeft = flags & 4
        self.thisRobot.reverseEncoderRight = flags & 8
        self.thisRobot.batteryIdleVolt = float(gg[9])
        try:
          self.thisRobot.robotHWtype = float(gg[10])
          self.thisRobot.name = gg[11]
        except:
          # missing robot hardware version
          self.thisRobot.robotHWtype = 2;
          self.thisRobot.name = gg[10]
        self.thisRobot.gotRobotName = True
        self.dataRead = True
      #
      elif gg[0] == "version":
        self.thisRobot.version = gg[1]
        self.gyroOK = int(gg[2],10)
        self.dataRead = True
      elif gg[0] == "vti": # static timing info (request using "v4")
        if len(gg) > 1:
          self.regbotSampleTime = float(gg[1])
          self.dataReadTiming = True
          pass
      # wifi info
      elif gg[0] == "wfi":
        self.wifiUse = bool(gg[1] == "1")
        self.wifiSetupState = int(gg[2])
        # debug
        #print("# got wfi use = " + str(self.wifiUse) + " (" + gg[1] + "), setup=" + gg[2] + ", sleep=" + gg[4])
        ## debug end
        self.wifiPortOpen = gg[2] == "99"
        self.wifiGotIP = gg[3] >= "3"
        self.wifiSleep = bool(gg[4]=="1")
        self.thisRobot.wifiIP = gg[5] + '.' + gg[6] + '.' + gg[7] + '.' + gg[8]
        # got also MAC
        self.thisRobot.wifiMAC = gg[9]
        self.wifiPort = int(gg[10], 0)
        self.wifiSSID = "no data"
        if (len(gg) > 11):
          # get number of characters in password
          n = int(gg[11], 10)
          #print("wifi pw length=" + str(n))
          if n < 32:
            self.wifiPW = ""
            for i in range(1,n):
              self.wifiPW += "*"
          else:
            print("wifi error in password length >=32")
          #print("wifi pw =" + self.wifiPW)
        if (len(gg) > 12):
          #print("wifi got SSID=" + gg[11] + " len(gg)=" + str(len(gg)))
          self.wifiSSID = ""
          for i in range(12,len(gg)):
            self.wifiSSID += gg[i] + " "
          #print("wifi SSID =" + self.wifiSSID)
        else:
          print("wifi no SSID")
        self.dataWifi = True
        #print("# got wfi")
      # client info
      elif gg[0] == "wfc":
        self.clientRxCnt[0] = int(gg[1], 10)
        self.clientTxCnt[0] = int(gg[2], 10)
        self.clientRxCnt[1] = int(gg[3], 10)
        self.clientTxCnt[1] = int(gg[4], 10)
        self.clientRxCnt[2] = int(gg[5], 10)
        self.clientTxCnt[2] = int(gg[6], 10)
        self.clientRxCnt[3] = int(gg[7], 10)
        self.clientTxCnt[3] = int(gg[8], 10)
        self.clientRxCnt[4] = int(gg[9], 10)
        self.clientTxCnt[4] = int(gg[10], 10)
        #self.wifiGood = int(gg[11], 10)
        #self.wifiLost = int(gg[12], 10)
        self.dataClient = True
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
    if (self.talkToBridge != self.talkToBridgeOld):
      self.talkToBridgeOld = self.talkToBridge
      if (self.talkToBridge):
        self.ui.connect_wifi.setText("Network (bridge)")
        # initiate bas subscriptions
        self.main.devWrite("# subscribe 6\n")
        self.main.devWrite("logdata subscribe 6\n")
        self.main.devWrite("hbt subscribe 3\n")
      else:
        self.ui.connect_wifi.setText("Network")
    if (self.dataRead):
      self.dataRead = False
      self.lock.acquire()
      if (not self.inEdit):
        # set new values
        self.ui.robot_gear.setProperty("value", self.thisRobot.gear)
        self.ui.robot_pulse_per_rev.setValue(self.thisRobot.pulsePerRev)
        self.ui.robot_wheel_radius_left.setValue(self.thisRobot.wheelLRadius)
        self.ui.robot_wheel_radius_right.setValue(self.thisRobot.wheelRRadius)
        self.ui.robot_hw_type.setValue(self.thisRobot.robotHWtype)
        self.ui.robot_balance_offset.setValue(self.thisRobot.balanceOffset)
        self.ui.save_id_on_robot.setEnabled(False)
        self.ui.robot_base.setValue(self.thisRobot.wheelbase)
        self.ui.robot_id.setValue(self.robotID)
        #self.ui.label_ID.setText(self.thisRobot.name + " (" + str(self.robotID) + ")")
        self.ui.robot_on_battery.setChecked(self.thisRobot.batteryUse)
        self.ui.robot_rev_motor.setChecked(self.thisRobot.reverseMotor)
        self.ui.robot_rev_encoder.setChecked(self.thisRobot.reverseEncoderLeft)
        self.ui.robot_rev_encoder_right.setChecked(self.thisRobot.reverseEncoderRight)
        self.ui.robot_battery_idle_volt.setValue(self.thisRobot.batteryIdleVolt)
      self.lock.release()
    if (self.dataReadTiming):
      # static timing
      self.dataReadTiming = False
      # set new values
      self.ui.regbotSampleTime.setProperty("value", self.regbotSampleTime)
    if (not self.inEdit):
      # and buttons
      self.ui.save_id_on_robot.setEnabled(False)
      self.ui.robot_cancel.setEnabled(False)
      self.ui.robot_edit.setEnabled(True)
    else:
      self.ui.save_id_on_robot.setEnabled(True)
      self.ui.robot_cancel.setEnabled(True)
      self.ui.robot_edit.setEnabled(False)
    # wifi alive
    if time.time() - self.lastAliveTime > 5:
      self.ui.wifi_sendAlive.setChecked(False)
    else:
      self.ui.wifi_sendAlive.setChecked(True)
    if (self.dataWifi):
      #print("# showing wfi")
      self.ui.wifi_got_ip.setChecked(self.wifiGotIP)
      self.ui.wifi_port_open.setChecked(self.wifiPortOpen)
      self.ui.wifi_ip.setText(self.thisRobot.wifiIP)
      self.ui.wifi_mac.setText(self.thisRobot.wifiMAC)
      if (not self.wifiInEdit):
        # update fields with data received from robot
        #print("wifi update to" + self.wifiSSID + " " + self.wifiPW)
        self.lock.acquire()
        self.ui.wifi_use.setChecked(self.wifiUse and not self.wifiSleep)
        #print("# wfi use setChecked = " + str(self.wifiUse))
        self.ui.wifi_port.setText(str(self.wifiPort))
        self.ui.wifi_ssid.setText(self.wifiSSID)
        self.ui.wifi_pw.setText(self.wifiPW)
        # set button properties
        self.ui.wifi_use.setEnabled(False)
        self.ui.wifi_port.setReadOnly(True)
        self.ui.wifi_ssid.setReadOnly(True)
        self.ui.wifi_pw.setReadOnly(True)
        self.ui.wifi_ip.setReadOnly(True)
        self.ui.wifi_apply.setEnabled(False)
        self.ui.wifi_got_ip.setEnabled(True)
        self.ui.wifi_port_open.setEnabled(True)
        self.lock.release()
      else:
        self.ui.wifi_use.setEnabled(True)
        self.ui.wifi_port.setReadOnly(False)
        self.ui.wifi_ssid.setReadOnly(False)
        self.ui.wifi_pw.setReadOnly(False)
        #print("wifi in edit")
      # data implemented / or potentially too old
      self.dataWifi = False
    # info about wifi clients
    if self.dataClient:
      self.lock.acquire()
      self.ui.wifi_client_1.setText(str(self.clientTxCnt[0]))
      self.ui.wifi_client_2.setText(str(self.clientTxCnt[1]))
      self.ui.wifi_client_3.setText(str(self.clientTxCnt[2]))
      self.ui.wifi_client_4.setText(str(self.clientTxCnt[3]))
      self.ui.wifi_client_5.setText(str(self.clientTxCnt[4]))
      self.ui.wifi_client_1_rx.setText(str(self.clientRxCnt[0]))
      self.ui.wifi_client_2_rx.setText(str(self.clientRxCnt[1]))
      self.ui.wifi_client_3_rx.setText(str(self.clientRxCnt[2]))
      self.ui.wifi_client_4_rx.setText(str(self.clientRxCnt[3]))
      self.ui.wifi_client_5_rx.setText(str(self.clientRxCnt[4]))
      self.dataClient = False
      self.lock.release()
    pass
    # reqest data periodically, as these data may change
    thisTab = self.ui.tabPages.indexOf(self.ui.tab_wifi) # robot hardware info
    #thisTab = self.ui.tabPages.indexOf(self.ui.tab_wifi) # on-board wifi
    if (self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() != thisTab:
      # just switched away from this tab
      self.hasFocus = False
      if self.main.isBridge():
        self.main.devWrite(":wfi subscribe 0\n")
        self.main.devWrite(":wfc subscribe 0\n")
      else:
        # talking to Teensy directly, so no subscribe here
        #self.main.devWrite("sub lfl 0\n") # log flags
        pass
      pass
    if (not self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() == thisTab:
      # just entering this tab
      self.hasFocus = True
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        self.main.devWrite(":wfi subscribe -1\n")
        self.main.devWrite(":wfc subscribe -1\n")
      else:
        # talking to Teensy directly, so subscribe here
        #self.main.devWrite("sub lfl 320\n") # log flags
        pass
      pass
    pass
  # 
  def dataChangedManually(self):
    # robot static parameters
    if (not self.main.timerUpdate):
      self.lock.acquire()
      self.inEdit = True
      self.lock.release()
  #def dataChangedManuallyWifi(self):
    #if (not self.robot.timerUpdate):
      #self.lock.acquire()
      #self.wifiInEdit = True
      #self.lock.release()
  #
  def cancelEdit(self):
    self.inEdit = False;
  # send data as is in edit fields
  def wifiSendData(self):
    s = ('wifi ' + str(int(self.ui.wifi_use.isChecked())) + ' ' + str(self.ui.wifi_port.text()) + 
              ' "' + str(self.ui.wifi_ssid.text()).strip() + '" "' + 
                     str(self.ui.wifi_pw.text()).strip() + '"\n')
    self.main.devWrite(s)
    self.wifiInEdit = False
    self.ui.wifi_edit.setEnabled(True)
    self.ui.wifi_cancel.setEnabled(False)
    self.ui.wifi_apply.setEnabled(False)
    pass
  # Cansel wifi edit
  def wifiCancel(self):
    #self.main.devWrite("v1\n")
    print("cancel wifi edit")
    self.wifiInEdit = False
    self.ui.wifi_port.setReadOnly(True)
    self.ui.wifi_ssid.setReadOnly(True)
    self.ui.wifi_pw.setReadOnly(True)
    self.ui.wifi_use.setEnabled(False)
    self.ui.wifi_edit.setEnabled(True)
    self.ui.wifi_cancel.setEnabled(False)
    self.ui.wifi_apply.setEnabled(False)
    pass
  # wifi fields edit
  def wifiEdit(self):
    self.lock.acquire()
    self.wifiInEdit = True
    self.ui.wifi_port.setReadOnly(False)
    self.ui.wifi_ssid.setReadOnly(False)
    self.ui.wifi_pw.setReadOnly(False)
    self.ui.wifi_use.setEnabled(True)
    self.ui.wifi_edit.setEnabled(False)
    self.ui.wifi_cancel.setEnabled(True)
    self.ui.wifi_apply.setEnabled(True)
    self.lock.release()
    print("wifi going in edit mode")
    pass
  def wifiSaveMacList(self):
    try:
      fn = "regbot_mac.txt"
      f = open(fn, "w")
      f.write('%% MAC list for robots in regbot.ini file\r\n')
      for rb in self.robots:
        if (rb.robotID == self.robotID):
          # current robot
          if (self.ui.wifi_and_save_IP.isChecked()):
            f.write(str(self.thisRobot.wifiMAC) + str(" 10.16.166." + str(self.thisRobot.robotID) + ' "(' + self.thisRobot.wifiIP + " " + self.thisRobot.name) + ')"\r\n')
          else:
            f.write(str(self.thisRobot.wifiMAC) + str(" 10.16.166." + str(self.thisRobot.robotID) + ' "(' + self.thisRobot.name) + ')"\r\n')
        elif rb.robotID > 0:
          if (self.ui.wifi_and_save_IP.isChecked()):
            f.write(str(rb.wifiMAC) +  " 10.16.166." + str(rb.robotID) + ' "(' + str(rb.wifiIP) + " " + str(rb.name) + ')"\r\n')
          else:
            f.write(str(rb.wifiMAC) +  " 10.16.166." + str(rb.robotID) + ' "(' + str(rb.name) + ')"\r\n')
      f.close()
    except:
      self.ui.statusbar.showMessage("Failed to open file " + fn + "!", 3000)
    pass

