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
import numpy as np
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

class UNet(object):
  threadRunning = False
  thread = []
  stopThread = False
  timeLastCnt = 0;
  sendOKTime = time.time()
  parent = []
  ui = []
  main = []
  debug = []

  wificlient = socket.socket()
  sendLock = threading.Lock()
  wifiPort = 24001
  hopo = ["none"]
  txCnt = 0
  txMiss = 0;
  rxCnt = 0
  connected = False
  wifiTimeLastCnt = 0;
  talkingToBridge = False

  # color
  dtugreen = QtGui.QColor(0, 136, 53, 127)
  dtured   = QtGui.QColor(153, 0, 0, 127)
  dtugrey  = QtGui.QColor(239, 240, 241, 255)
  dtuyellow = QtGui.QColor(0xf6, 0xd0, 0x4d, 64)
  
  # methods / functions
  def __init__(self, parent):
    self.parent = parent
    self.ui = parent.ui

  def setup(self):
    # connect frame
    self.main = self.parent.main
    self.debug = self.parent.debug
    self.ui.connect_wifi.clicked.connect(self.connectClicked_wifi)
    self.close();
    self.thread = threading.Thread(target=self.run, name="WIFI_reader")
    self.thread.start()
    
  def isOpen(self):
    return self.connected
  

  def open(self):
    if not self.connected:
      self.hopo = self.ui.wifi_host_name.text().split(':')
      try:
        self.wifiPort = int(str(self.hopo[1]), 0)
      except:
        self.wifiPort = 24001
      print("# Network opening to: hopo=" + str(self.hopo[0]) + " port " + str(self.wifiPort))
      try:
        for res in socket.getaddrinfo(str(self.hopo[0]), self.wifiPort, socket.AF_UNSPEC, socket.SOCK_STREAM):
          #print("# socket res " + str(res))
          af = res[0]
          socktype = res[1] 
          proto = res[2]
          canonname = res[3]
          sa = res[4] # both IP and port number
          try:
              self.wificlient = socket.socket(af, socktype, proto)
              self.wificlient.settimeout(0.5)
              #print("# wifi socket created")
          except OSError as msg:
              self.wificlient = None
              print("# Network connection timeout - retry")
              continue
          try:
              self.wificlient.connect(sa)
              self.connected = True
              self.timeLastCnt = 0
          except OSError as msg:
              self.wificlient.close()
              self.wificlient = None
              print("# Network connect failed")
              continue
          except:
            print("# Network other except")
          break
        pass
      except:
        print("# network address not found")
        self.debug.mainStatus += "Net not found: " + str(self.hopo[0]) + "\n"
        self.debug.mainStatusSet = True
        self.main.message("Net not found: " + str(self.hopo[0]))
    if self.connected:
      print("Network is open")
      self.sendOKTime = time.time()
      self.debug.mainStatus += "Networking on " + str(self.hopo[0]) + ":" + str(self.wifiPort) + "\n"
      self.debug.mainStatusSet = True
      #self.wifiWaiting4reply = False
      setFrameColor(self.ui.frame_wifi_connect, self.dtugreen)
      pass
    pass
  
  
  def close(self):
    if (self.connected):
      print("Network stopping")
      self.connected = False
      #self.info.thisRobot.gotRobotName = False
      # finished sending and receiving (may send HUP?)
      try:
        self.wificlient.shutdown(2);
      except:
        print("socket shutdown error - no connections open - ignoring");
      self.wificlient.close()
      self.ui.statusbar.showMessage("Network client - disconnected", 2000)
      self.debug.mainStatus += "Network " + self.hopo[0] + " disconnected\n"
      self.debug.mainStatusSet = True
      #self.wifiWaiting4reply = False
      #self.talkToBridge = False
      #self.clearLastTab()
    if self.ui.connect_wifi.isChecked():
      setFrameColor(self.ui.frame_wifi_connect, self.dtured)
    else:
      setFrameColor(self.ui.frame_wifi_connect, self.dtugrey)
    pass



  def connectClicked_wifi(self):
    if self.ui.connect_wifi.isChecked():
      self.open()
    else:
      self.close()


  def run(self):
    count = 0
    m = 0
    n = 0
    c = 0
    b = '\0'
    q = -1
    ok = True
    sum = 0
    self.threadRunning = True
    print("# Net thread running")
    got = ""
    self.wificlient.settimeout(0.3)
    while (not self.stopThread):
      if self.connected:
        n = 0
        if (b == '\n'):
          got = ""
          b = []
        try: 
          while (b != '\n' and self.connected):
            c = self.wificlient.recv(1)
            if (len(c) > 0):
              b = c.decode('utf-8')
              if (b >= ' ' or b == '\n' or b == '\t'):
                # filter all control characters but newline and tab
                got = got + b
          n = len(got)
        except:
          m = m + 1
          time.sleep(0.01)
          #if m > 15:
            #print("# Read from wifi failed " + str(m) + " times")
            #self.close()
        if (n > 0):
          #print("# got (" + str(m) + ", len=" + str(n) + ")=" + got)
          self.rxCnt += 1
          # we are talking to a bridge, so source name is expected
          # look for source name
          # print("# got " + got)
          # look for CRC
          source = ""
          if got[0] == ';':
            sum = 0
            try:
              q = int(got[1:3])
              if q > 0:
                for i in range(3,len(got)):
                  if got[i] >= ' ':
                    sum = sum + ord(got[i])
              ok = (sum % 99) + 1 == q
              if ok:
                got = got[3:]
            except:
              ok = False # character 1 and 2 is not numeric
          # look for explicit data source
          if ok:
            spaceIndex = got.find(" ", 0, 32)
            srcidx = got.find(":", 0, 32)
            if srcidx >= 0 and srcidx < spaceIndex:
              # there is a source
              source = got[:srcidx]
              got = got[srcidx+1:]
              # print("# found source " + source + " at index=" + str(srcidx) + ", the rest is '" + got + "'")
          # decode command
          if not ok:
            print("Teensy data failed q-test (q=" + str(q) + " sum%99+1=" + str((sum % 99) + 1) +"): " + got)
          else:
            if got[0] == "#" and not self.talkingToBridge:
              b = got.find("bridge",0,100)
              if b: # is talking to a bridge, i.e. no CRC needed.
                self.talkingToBridge = True
            # handle command
            self.main.decodeCommand(got, n, "n", source)
          m = 0;
      else:
        time.sleep(0.1)
    print("# Net read thread ended")
    self.threadRunning = False
    pass

  def stop(self):
    self.close()
    if self.threadRunning:
      self.stopThread = True
      self.thread.join(2)
      print("# Net thread joined")


    ### send string to wifi socket
  def wifiWrite(self, s):
    self.sendLock.acquire()
    n = len(s)
    d = -1
    #print("# sending " + s)
    if (n > 0):
      #m = 0;
      if True: # not self.talkingToBridge:
        sum = 0;
        for i in range(0,n):
          if s[i] >= ' ':
            sum += ord(s[i])
        # print("# sending ';" + str((sum % 99) + 1) + s + "'\n")
        ss = ";{:02d}".format((sum % 99) + 1) + s
      else:
        ss = s
      #print("# trying to send " + ss)
      try:
        d = self.wificlient.send(ss.encode())
        self.txCnt += 1
        self.txMiss = 0
      except:
        self.txMiss += 1
        if self.txMiss < 5:
          self.main.message("Net send fail " + str(self.txMiss))
        pass
    self.sendLock.release()
    if (d <= 0):
      dt = time.time() - self.sendOKTime
      #print("# send failed, lastOK " + str(dt) + "s, for " + s)
      if dt > 10:
        self.close()
    else:
     self.sendOKTime = time.time()
      # raise Exception("Write error wifi")
    return self.isOpen()


  def timerUpdate(self, timerCnt, justConnected):
    if not self.isOpen():
      if self.ui.connect_wifi.isChecked():
        self.wifiTimeLastCnt += 1
        setFrameColor(self.ui.frame_wifi_connect, self.dtured)
        if self.wifiTimeLastCnt > 30:
          self.wifiTimeLastCnt = 0
          self.open()
          #print("# Trying to connect to wifi" + str(self.wifiTimeLastCnt))
        pass
      pass
      
