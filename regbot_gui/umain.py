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

import sys 
import os
import threading
#import numpy as np
#import pyqtgraph as pg
import serial
import time
import timeit
import socket
try:
  import configparser
except:
  import ConfigParser  
from upaint import *
from PyQt5 import QtWidgets, QtCore, QtGui

class UMain(object):
  # class variables
  timerCnt = 0
  messageStr = ""
  messageSet = True
  version = "0.0"
  versionDate = 'noname'
  teensyTime = -0.1
  deviceID = -1
  revision = -1
  batteryVoltage = -1
  state = 0
  stateNew = False
  statusBarTime = time.time()
  decodeLock = threading.Lock()
  notSendCnt = 0
  wifiOn = False
  usbOn = False
  name = "unnamed"
  devType = "regbot"
  nameSet = False
  justConnected = False
  destID = "regbot"
  destIDold = "regbot"
  destChanged = False;
  iniFilename = "regbot.ini"
  load = 0
  hostTemp = 0
  hostTempNew = False
  
  # methods / functions
  def __init__(self, parent, ui):
    self.parent = parent
    self.ui = ui

  def setup(self):
    self.pose = self.parent.pose
    self.debug = self.parent.debug
    self.log = self.parent.log
    #self.ui.checkBox_connect.setChecked(True)
    self.ui.tabPages.setCurrentIndex(2)
    self.imu = self.parent.imu
    self.usb = self.parent.usb
    self.net = self.parent.net
    self.ir = self.parent.ir
    self.mission = self.parent.mission
    self.robotInfo = self.parent.robotInfo
    self.servo = self.parent.servo
    self.joy = self.parent.joy
    self.edge = self.parent.edge
    self.list = self.parent.list
    self.bridge = self.parent.bridge
    self.ctrlVelocity = self.parent.ctrlVelocity # UControlUnit("cvel", self, "Wheel Velocity (left and right)")
    self.ctrlTurn = self.parent.ctrlTurn # UControlUnit("ctrn", self, "Heading")
    self.ctrlWallVel = self.parent.ctrlWallVel # UControlUnit("cwve", self, "IR forward distance")
    self.ctrlWallTurn = self.parent.ctrlWallTurn #UControlUnit("cwth", self, "Wall distance")
    self.ctrlPos = self.parent.ctrlPos #UControlUnit("cpos", self, "Position (drive distance)")
    self.ctrlEdge = self.parent.ctrlEdge #UControlUnit("cedg", self, "Line edge")
    self.ctrlBalance = self.parent.ctrlBalance #UControlUnit("cbal", self, "Balance")
    self.ctrlBalVel = self.parent.ctrlBalVel #UControlUnit("cbav", self, "Balance velocity")
    self.ctrlBalPos = self.parent.ctrlBalPos #UControlUnit("cbap", self, "Balance position")
    #
    # button actions
    self.ui.main_start.clicked.connect(self.start)
    self.ui.main_stop.clicked.connect(self.emergency_stop)
    self.ui.pushButton_message_clear.clicked.connect(self.messageClear)
    self.ui.config_robot_save.clicked.connect(self.saveToFlash)
    #self.ui.radioButton_drive.clicked.connect(self.toTeensy)
    #self.ui.radioButton_front.clicked.connect(self.toTeensy)

  #def toTeensy(self):
    #self.destIDold = self.destID
    #if True: # or self.ui.radioButton_drive.isChecked():
      #self.destID = "drive"
    #else:
      #self.destID = "front"
    #self.destChanged = self.destID != self.destIDold;

    
  def message(self, s):
    self.messageStr += s + "\n"
    self.messageSet = True
    #self.ui.text_message.setText(s)

  def messageClear(self):
    self.messageStr = ""
    self.messageSet = True

  def terminate(self):
    # stop connection
    # print("# Sending emergency stop")
    # self.emergency_stop()
    self.usb.stop()
    self.net.stop()
    print("# all finished OK")
    

  def decodeCommand(self, got, n, w, source):
    self.decodeLock.acquire()
    pre = "(R" + w + ") " + source + ": "
    if n > 0:
      #print(got)
      if got[0] == '#' and self.ui.checkBox_debug_show_all_hash.isChecked():
        self.debug.mainStatus += pre + got
        self.debug.mainStatusSet = True
        #print("# hash: " + got);
        b = got.find("message",0,12)
        if b > 0:
          self.message(got[b+8:-1])
      pass
    if n > 3 and got[0] != '#':
      isOK = False
      if len(got) > 2:
        gg = got.split()
        if gg[0] != "hbt" and self.ui.checkBox_debug_show_all_rx.isChecked():
          self.debug.mainStatus += pre + got
          self.debug.mainStatusSet = True
        if True: #try:
          if gg[0] == "hbt":
            if self.ui.checkBox_debug_show_hbt.isChecked():
              self.debug.mainStatus += pre + got
              self.debug.mainStatusSet = True
            #/** format
            #* hbt 1 : time in seconds, updated every sample time
            #*     2 : device ID (probably 1)
            #*     3 : software revision number - from SVN
            #*     4 : Battery voltage (from AD converter)
            #*     5 : state 
            #*     6 : hw type
            #* */
            self.teensyTime = float(gg[1])
            try:
              #newID = 
              #if newID != self.deviceID and not self.log.saveAsSelected:
                ## detected new robot
                #self.log.setSaveFilename()
              self.deviceID = int(gg[2])
              self.revision = int(gg[3])
              self.batteryVoltage = float(gg[4])
              self.state = int(gg[5])
              self.robotInfo.setHwType(self.deviceID, int(gg[6]))
              self.load = float(gg[7])
            except:
              print("# hbt should have 7 values\nas 'hbt time ID rev battery state HW load': : " + got)
              self.message("# hbt should have 7 values\nas 'hbt time ID rev state HW load': " + got)
              self.messageSet = True
              pass
            self.stateNew = True
            pass
          elif gg[0] == "temp":
            self.hostTemp = float(gg[1])
            if self.ui.checkBox_debug_show_hbt.isChecked():
              self.debug.mainStatus += pre + got
              self.debug.mainStatusSet = True
            self.hostTempNew = True
          elif gg[0] == "dname":
            self.devType = gg[1]
            if (gg[2] != self.name):
              self.name = gg[2]
              self.log.setSaveFilename()
              self.robotInfo.setName(self.deviceID, self.name)
            self.nameSet = True
          elif self.log.decode(gg, got):
            pass
          elif self.pose.decode(gg):
            pass
          elif self.imu.decode(gg):
            pass
          elif self.robotInfo.decode(gg):
            pass
          elif self.servo.decode(gg):
            pass
          elif self.joy.decode(gg):
            pass
          elif self.edge.decode(gg):
            pass
          elif self.mission.decode(gg, got):
            pass
          elif self.ctrlBalance.decode(gg):
            pass
          elif self.ctrlBalPos.decode(gg):
            pass
          elif self.ctrlBalVel.decode(gg):
            pass
          elif self.ctrlEdge.decode(gg):
            pass
          elif self.ctrlPos.decode(gg):
            pass
          elif self.ctrlTurn.decode(gg):
            pass
          elif self.ctrlVelocity.decode(gg):
            pass
          elif self.ctrlWallTurn.decode(gg):
            pass
          elif self.ctrlWallVel.decode(gg):
            pass
          elif self.ir.decode(gg):
            pass
          elif gg[0] == "message":
            self.message(got[8:-1])
          elif self.bridge.decode(gg):
            pass
          else:
            print("# " + pre + "main.py::decodeCommand: noone decoded:" + got)
        else: #except:
          print("# " + pre + "decoded failed for (continues): " + got)
    self.decodeLock.release()
    pass
  
  def devWrite(self, s, addPreKey = False):
    isSend = False
    isWiFiSend = False
    if self.usb.isOpen() or self.net.isOpen():
      # pre-key is for bridge only
      if (self.usb.isOpen()):
        #print("# about to send to USB : " + s)
        isSend = self.usb.usbWrite(s)
        if (isSend):
          self.usb.dataTxCnt += 1 #len(s)
          if self.ui.checkBox_debug_show_all_tx.isChecked():
            self.debug.mainStatus += "(Tu) " + str(s)
            self.debug.mainStatusSet = True
      if self.net.isOpen():
        if addPreKey:
          pre = self.destID + " "
        else:
          pre = ""
        isWiFiSend = self.net.wifiWrite(pre + s)
        if (isWiFiSend):
          self.net.txCnt += 1 #len(s)
          if self.ui.checkBox_debug_show_all_tx.isChecked():
            self.debug.mainStatus += "(Tn) " + str(pre) + str(s)
            self.debug.mainStatusSet = True
        pass
        #print("# wifi send " + pre + s + " OK=" + str(isWiFiSend))
      pass
    if not (isSend or isWiFiSend):
      if self.notSendCnt < 5:
        self.debug.mainStatus += "not connected, could not send: " + str(s)
        self.debug.mainStatusSet = True
        self.notSendCnt += 1
    else:
      self.notSendCnt = 0
    pass

  def isConnected(self):
    return self.usb.isOpen() or self.net.isOpen()
  
  def isBridge(self):
    # should probably have better check
    return self.net.isOpen()

  def wifiSubscribe(self):
      if (self.destChanged):
        # unsubscribe to general messages from Teensy
        self.net.wifiWrite(self.destIDold + ":hbt subscribe 0\n")
        self.net.wifiWrite(self.destIDold + ":unhandled subscribe 0\n")
        self.net.wifiWrite(self.destIDold + ":# subscribe 0\n")
        self.net.wifiWrite(self.destIDold + ":dname subscribe 0\n")
        self.net.wifiWrite(self.destIDold + ":logdata subscribe 0\n")
        self.net.wifiWrite(self.destIDold + ":version subscribe 0\n")
        self.net.wifiWrite(self.destIDold + ":mis subscribe 0\n")
      # subscribe to general messages from Teensy
      self.net.wifiWrite(self.destID + ":hbt subscribe -1\n")
      self.net.wifiWrite(self.destID + ":unhandled subscribe -1\n")
      self.net.wifiWrite(self.destID + ":# subscribe -1\n")
      self.net.wifiWrite(self.destID + ":dname subscribe -1\n")
      self.net.wifiWrite(self.destID + ":logdata subscribe -1\n")
      self.net.wifiWrite(self.destID + ":version subscribe -1\n")
      self.net.wifiWrite(self.destID + ":mis subscribe -1\n")
      # bridge info
      self.net.wifiWrite("host:ip subscribe -1\n")
      self.net.wifiWrite("host:ip subscribe -1\n")
      self.net.wifiWrite("host:mac subscribe -1\n")
      self.net.wifiWrite("host:temp subscribe -1\n")

  def usbSubscribe(self):
    self.devWrite("sub hbt 400\n")
    self.devWrite("sub id 2000\n")
    self.devWrite("sub ver 2100\n")
    self.devWrite("sub mis 309\n")

  def timerUpdate(self):
    #print("uregbot::timerUpdate(self):\n")
    self.timerCnt += 1
    self.debug.timerUpdate(self.timerCnt, self.justConnected)
    self.log.timerUpdate(self.timerCnt, self.justConnected)
    self.imu.timerUpdate(self.timerCnt, self.justConnected)
    self.usb.timerUpdate(self.timerCnt, self.justConnected)
    self.net.timerUpdate(self.timerCnt, self.justConnected)
    self.ir.timerUpdate(self.timerCnt, self.justConnected)
    self.net.timerUpdate(self.timerCnt, self.justConnected)
    self.servo.timerUpdate(self.timerCnt, self.justConnected)
    self.pose.timerUpdate(self.justConnected)
    self.joy.timerUpdate(self.timerCnt, self.justConnected)
    self.edge.timerUpdate(self.timerCnt, self.justConnected)
    self.mission.timerUpdate(self.timerCnt, self.justConnected)
    self.robotInfo.timerUpdate(self.timerCnt, self.justConnected)
    self.ctrlBalance.timerUpdate(self.timerCnt, self.justConnected)
    self.ctrlBalPos.timerUpdate(self.timerCnt, self.justConnected)
    self.ctrlBalVel.timerUpdate(self.timerCnt, self.justConnected)
    self.ctrlEdge.timerUpdate(self.timerCnt, self.justConnected)
    self.ctrlPos.timerUpdate(self.timerCnt, self.justConnected)
    self.ctrlTurn.timerUpdate(self.timerCnt, self.justConnected)
    self.ctrlWallTurn.timerUpdate(self.timerCnt, self.justConnected)
    self.ctrlWallVel.timerUpdate(self.timerCnt, self.justConnected)
    self.ctrlVelocity.timerUpdate(self.timerCnt, self.justConnected)
    self.list.timerUpdate(self.timerCnt, self.justConnected)
    self.bridge.timerUpdate(self.timerCnt, self.justConnected)
    #
    self.justConnected = False
    # 
    if self.usb.isOpen() and not self.usbOn:
      self.usbOn = True
      self.justConnected = True
    elif not self.usb.isOpen() and self.usbOn:
      self.usbOn = False
      if not self.wifiOn:
        self.deviceID  = -1
        self.name = "noname"
    # 
    if self.net.isOpen() and not self.wifiOn:
      self.wifiOn = True
      self.justConnected = True
      #self.ui.radioButton_drive.setEnabled(True)
      #self.ui.radioButton_front.setEnabled(True)
    if not self.net.isOpen() and self.wifiOn:
      self.wifiOn = False
      self.ui.connect_wifi.setText('Network')
      self.hostTempNew = False
      if not self.usbOn:
        self.deviceID  = -1
        self.name = "noname"
    if self.justConnected:
      # clear old values
      self.batteryVoltage = 0
      self.name = "Empty"
      self.nameSet = True
      self.state = 0
      self.stateNew = True
      self.teensyTime = 0
      self.load = 0
      self.robotInfo.thisRobot = self.robotInfo.getRobot(self.deviceID)
    #
    #self.ui.radioButton_drive.setEnabled(False)
    #self.ui.radioButton_front.setEnabled(False)
    if self.messageSet:
      self.ui.Main_status_log.setPlainText(self.messageStr)
      self.ui.Main_status_log.verticalScrollBar().setValue(self.ui.Main_status_log.verticalScrollBar().maximum())
      self.messageSet = False
    if self.nameSet:
      self.ui.label_ID.setText('<html><head/><body><p>' + self.devType + 
                               ' <span style=" font-weight:600; font-size:{:d}pt">'.format(int(self.parent.fontSize * 1.5)) + 
                               self.name + '</span> (' + str(self.deviceID) + ')</p></body></html>')
      #self.ui.label_ID.setText(self.devType + " " + self.name + " (" + str(self.deviceID) + ")")
      self.nameSet = False
    pass
    if (self.destChanged or self.justConnected):
      if self.net.isOpen():
        self.wifiSubscribe()
      else:
        self.usbSubscribe()
      self.destChanged = False;
    # HTML text formatting: <html><head/><body><p>State:<span style=" font-size:14pt;"> init</span></p></body></html>
    if self.stateNew:
      self.stateNew = False
      if self.state < 0:
        sa = 'Emergency'
        setFrameColor(self.ui.frame_state, self.parent.usb.dtured)
      elif self.state > 2:
        sa = 'Running'
        setFrameColor(self.ui.frame_state, self.parent.usb.dtugreen)
      elif self.state == 1 or self.state == 2:
        sa = 'RC'
        setFrameColor(self.ui.frame_state, self.parent.usb.dtublue)
      else:
        sa = 'Stopped'
        setFrameColor(self.ui.frame_state, self.parent.usb.dtugrey)
      self.ui.main_remote_ctrl.setChecked(self.state == 1)
      # set label 
      self.ui.label_main_state.setText('<html><head/><body><p>State: <span style=" font-weight:600; font-size:{:d}pt">'.format(int(self.parent.fontSize * 1.5)) +
                                       sa + '</span></p></body></html>')
      # other status labels
      self.ui.label_main_bat.setText('Battery: ' + str(self.batteryVoltage) + ' V')
      self.ui.label_main_time.setText('devTime: {:.1f} sec'.format(self.teensyTime))
      self.ui.cpu_load.setText('{:.1f}'.format(self.load))
    if self.hostTempNew:
      self.hostTempNew = False
      self.ui.connect_wifi.setText('Network {:.1f} deg'.format(self.hostTemp))
    if time.time() - self.statusBarTime > 10:
      self.setStatusBar("")
      
  def start(self):
    self.debug.mainStatus += "# starting in 1s\n"
    self.debug.mainStatusSet = True
    self.devWrite("start\n", True)
    
  def emergency_stop(self):
    self.debug.mainStatus +="# Emergency stop\n"
    self.debug.mainStatusSet = True
    self.devWrite("stop\n", True)
    pass

  def saveToFlash(self):
      self.devWrite("eew\n", True)

  def setStatusBar(self, message):
    self.ui.statusbar.showMessage(message, 3000)
    self.statusBarTime = time.time()
