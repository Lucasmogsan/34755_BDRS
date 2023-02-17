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
#import socket
try:
  import configparser
except:
  import ConfigParser  
from upaint import *
from PyQt5 import QtWidgets, QtCore, QtGui

class UUsb(object):
  # USB thread
  threadRunning = False
  stopThread = False
  thread = []
  dev = serial.Serial()
  sendLock = threading.Lock()
  dataTxCnt = 0
  #dataRxCnt = 0
  timeLastCnt = 95;
  notSendCnt = 0;
  parent = []
  ui = []
  main = []
  debug = []
  closingUSB = False
  # color
  dtugreen = QtGui.QColor(0, 136, 53, 127)
  dtured   = QtGui.QColor(153, 0, 0, 127)
  dtugrey  = QtGui.QColor(239, 240, 241, 255)
  dtuyellow = QtGui.QColor(0xf6, 0xd0, 0x4d, 64)
  dtublue   = QtGui.QColor(47, 62, 234, 127)
  
  # methods / functions
  def __init__(self, parent):
    self.parent = parent
    self.ui = parent.ui
    print("# USB __init__ called")

  def setup(self):
    # connect frame
    self.main = self.parent.main
    self.debug = self.parent.debug
    self.ui.connect_usb.clicked.connect(self.connectClicked)
    self.close();
    self.thread = threading.Thread(target=self.run, name="drone_gui_USB_reader")
    self.thread.start()
    
  def isOpen(self):
    if self.closingUSB:
      return False
    try:
      return self.dev.isOpen()
    except:
      return False
  
  # open USB to Teensy
  def usbopen(self, name):
    if (not self.isOpen()):
      self.dev.port = str(name)
      self.dev.timeout = 0.5
      print("# UUsb::usbopen:: Trying to open:" + str(name))
      try:
        self.dev.open()
        self.main.setStatusBar("USB - opened OK.")
        self.debug.mainStatus += "USB Connected OK\n"
        self.debug.mainStatusSet = True
        #self.failCnt = 0
        #print("usbopen:: open without error:" + str(name))
      except:
        print("# UUsb::usbopen:: open with error:" + str(name))
        self.close()
        self.main.setStatusBar("USB open failed :" + str(name))
        self.main.message("USB Failed to open " + str(name))
        self.debug.mainStatusSet = True
    if self.isOpen():
      #self.dev.flushInput()
      self.dev.flushOutput()
      self.main.message(str(name) + " is open")
      setFrameColor(self.ui.frame_usb_connect, self.dtugreen)
      # subscribe to heartbeat messages (state of flight controller)
      self.usbWrite("sub hbt 400\n")
    pass

  def close(self):
    # USB close
    #print("# closing USB\n")
    if self.isOpen():
      #print("stopping push S=0")
      self.usbWrite("leave\n")
      self.dev.close()
      self.debug.mainStatus += "Teensy is disconnected\n"
      self.debug.mainStatusSet = True
      self.main.message("USB closed")
    if self.ui.connect_usb.isChecked():
      setFrameColor(self.ui.frame_usb_connect, self.dtured)
    else:
      setFrameColor(self.ui.frame_usb_connect, self.dtugrey)
    pass

  def connectClicked(self):
    # USB connect
    if self.ui.connect_usb.isChecked():
      print("Trying to connect to " + self.ui.usb_device_name.text())
      self.usbopen(self.ui.usb_device_name.text())
    else:
      self.close()

  # always an active read on Teensy connection
  def run(self):
    count = 0
    m = 0
    n = 0
    self.threadRunning = True
    print("# USB thread running")
    got = ""
    gotraw = []
    while (not self.stopThread):
      if self.isOpen():
        n = 0
        try:
          # get characters until new-line (\n)
          gotraw = self.dev.readline()
          #if len(gotraw) > 0:
            #got = gotraw.decode('ascii')
          #if len(gotraw) > 0:
            #print("got " + str(len(gotraw)) + " chars:" + str(int(gotraw[0])) + ",'"+ str(gotraw) + "'")
          n = len(gotraw)
          m = 0
        except:
          m = m + 1
          time.sleep(0.01)
          print("# Read from USB failed " + str(m) + " times")
          if m > 5:
            self.closingUSB = True
        if n > 3:
          ok = True
          #for c in gotraw:
            #ok = ok and c < 127
          try:
            got = gotraw.decode('ascii')
          except:
            ok = False;
          if ok:
            #self.dataRxCnt += 1
            # look for CRC
            source = ""
            if got[0] == ';':
              sum = 0
              q = -1
              try:
                q = int(got[1:3])
                if q > 0:
                  for i in range(3,len(gotraw)):
                    if gotraw[i] >= ord(' '):
                      sum = sum + gotraw[i]
                ok = (sum % 99) +1 == q
              except:
                ok = False
              if ok:
                # test for source
                got = got[3:]
                srcidx = got.find(":",0,32)
                spaceidx = got.find(" ",0,32)
                if srcidx >= 0 and (spaceidx < 0 or spaceidx > srcidx):
                  # there is a source
                  source = got[:srcidx]
                  got = got[srcidx+1:]
                  # print("# found source " + source + " at index=" + str(srcidx) + ", the rest is '" + got + "'")
              if not ok:
                print("Teensy data failed crc-test (crc=" + str(q) + ",sum="+ str(sum) + ", %99+1=" + str((sum %99) + 1) + ") :" + got)
              else:
                self.main.decodeCommand(got, len(got), "u", source)
            else:
              print("Teensy msg do not start with ; discarded:" + got)
          else:
            print("# code has illegal chars " + str(gotraw))
        #time.sleep(0.01)
      else:
        time.sleep(0.1)
    print("# USB read thread ended")
    self.threadRunning = False

  def stop(self):
    self.close()
    if self.threadRunning:
      self.stopThread = True
      self.thread.join(2)
      print("# usb thread joined")

  ### send string to USB connection
  def usbWrite(self, s):
    self.sendLock.acquire()
    if self.isOpen():
      n = len(s)
      sum = 0
      sumCnt = 0
      for i in range(0,n):
        if s[i] >= ' ':
          sum += ord(s[i])
          sumCnt += 1
      #print("# to USB: CRC=" + str((sum % 99) + 1) + " for " + str(sumCnt) + "chars, sum=" + str(sum) + ", " + s + "'")
      ss = ";{:02d}".format((sum % 99) + 1) + s
      if (n > 0):
        try:
          n = self.dev.write(ss.encode())
          #print("# USB write returned " + str(n) + " bytes send")
          if (n == 0):
             raise Exception("Write error")
        except:
          # may not claa close here, as self.dev is running in another thread
          #self.dev.close()
          self.closingUSB = True;
          print("# UUsb::usbWrite(s) failed - closing connection")
          self.ui.statusbar.showMessage("Robot USB - connection broken")
    self.sendLock.release()
    return self.isOpen()

  def timerUpdate(self, timerCnt, justConnected):
    if self.closingUSB:
      self.dev.close()
      self.closingUSB = False
    #
    if not self.isOpen():
      if self.ui.connect_usb.isChecked():
        self.timeLastCnt += 1
        setFrameColor(self.ui.frame_usb_connect, self.dtured)
        if  self.timeLastCnt > 100:
          self.timeLastCnt = 0
          self.usbopen(self.ui.usb_device_name.text())
        pass
      pass
      
