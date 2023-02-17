#!/usr/bin/python
# -*- coding: utf-8 -*-

#/***************************************************************************
 #*   Copyright (C) 2014-2022 by DTU
 #*   jcan@dtu.dk            
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
import socket
import time
#from time import sleep
try:
  import configparser as cp
except:
  import ConfigParser as cp
  pass
import timeit
from PyQt5 import QtWidgets, QtCore

class UDebug(object):
  # class variables
  mainStatus  = "Drone GUI\n main debug status\n"
  mainStatusSet = True
  dataRxCntOld = 0
  oldTab = -1

  # methods / functions
  def __init__(self, parent, ui):
    self.parent = parent
    self.ui = ui

  def setup(self):
    # make a read thread to take care of infut from Teensy
    #self.stop = threading.Event()
    #self.doRead = threading.Thread(target=self.readThread, name="regbot_usb_reader")
    #self.doRead.start()
    ## the rest
    self.main = self.parent.main
    #self.usb = self.parent.usb
    #self.net = self.parent.net
    self.ui.pushButton_debug_send.clicked.connect(self.debugSend)
    self.ui.pushButton_debug_send_2.clicked.connect(self.debugSend2)
    self.ui.pushButton_debug_send_3.clicked.connect(self.debugSend3)
    self.ui.pushButton_debug_help.clicked.connect(self.debugHelp)
    self.ui.pushButton_debug_help_teensy.clicked.connect(self.debugHelpT)
    self.ui.pushButton_debug_clear.clicked.connect(self.clear)
    self.ui.pushButton_factory_reset.clicked.connect(self.setIdZero)
    self.ui.lineEdit_debug_command.returnPressed.connect(self.debugSend)
    self.ui.lineEdit_debug_command_2.returnPressed.connect(self.debugSend2)
    self.ui.lineEdit_debug_command_3.returnPressed.connect(self.debugSend3)
    pass


  def timerUpdate(self, timerCnt, justConnected):
    if self.mainStatusSet:
      if (len(self.mainStatus) > 1000000):
        self.mainStatus = "# status truncated\n"
      self.ui.textEdit_debug_text.setPlainText(str(self.mainStatus))
      self.mainStatusSet = False
      self.ui.textEdit_debug_text.verticalScrollBar().setValue(self.ui.textEdit_debug_text.verticalScrollBar().maximum())
    #if self.main.dataRxCnt != self.dataRxCntOld:
      #self.ui.label_debug_rx_cnt.setText("(rx) " + str(self.main.dataRxCnt) + " lines")
      #self.ui.label_debug_tx_cnt.setText("(tx) " + str(self.main.dataTxCnt) + " lines")
      #self.dataRxCntOld = self.main.dataRxCnt
    #if timerCnt % 20 == 0 or self.usb.dataRxCnt != self.dataRxCntOld:
      #self.ui.label_message_cnt.setText("tx: USB " + str(self.usb.dataTxCnt) + 
                                        #" net " + str(self.wifi.txCnt) + 
                                        #", rx: USB " + str(self.usb.dataRxCnt) + 
                                        #" net " + str(self.wifi.rxCnt) + 
                                        #" lines (loop " + str(timerCnt) + ")")
      #self.dataRxCntOld = self.usb.dataRxCnt
    if justConnected:
        self.mainStatus += "---- just connected -----\r\n"
        self.mainStatusSet = True;
        if self.main.isBridge():
          self.main.devWrite("*:conf get\n", False)
        else:
          self.main.devWrite("confi\n", True)
    if self.oldTab != self.ui.tabPages.currentIndex():
      #if self.ui.tabPages.currentIndex() == 0:
      # getting focus
      self.mainStatus += "----\r\n"
      self.mainStatusSet = True;
      self.oldTab = self.ui.tabPages.currentIndex()
      pass
    pass 
  
  def debugSend(self):
    s = str(self.ui.lineEdit_debug_command.text()) + "\n"
    #print(s)
    self.main.devWrite(s)
    pass
  
  def debugSend2(self):
    s = str(self.ui.lineEdit_debug_command_2.text()) + "\n"
    #print(s)
    self.main.devWrite(s)
    pass
  
  def debugSend3(self):
    s = str(self.ui.lineEdit_debug_command_3.text()) + "\n"
    #print(s)
    self.main.devWrite(s)
    pass

  def debugHelp(self):
    self.main.devWrite("help\n")
    pass

  def debugHelpT(self):
    self.main.devWrite("help\n", True)
    pass

  def clear(self):
    self.mainStatus = ""
    self.mainStatusSet = True

  def saveToIniFile(self, config):
    # settings
    config.add_section('debug')
    config.set('debug', 'show#', str(self.ui.checkBox_debug_show_all_hash.isChecked()))
    config.set('debug', 'showTx', str(self.ui.checkBox_debug_show_all_tx.isChecked()))
    config.set('debug', 'showRx', str(self.ui.checkBox_debug_show_all_rx.isChecked()))
    config.set('debug', 'showHbt', str(self.ui.checkBox_debug_show_hbt.isChecked()))
    config.set('debug', 'cmd1', str(self.ui.lineEdit_debug_command.text()))
    config.set('debug', 'cmd2', str(self.ui.lineEdit_debug_command_2.text()))
    config.set('debug', 'cmd3', str(self.ui.lineEdit_debug_command_3.text()))

  def loadFromIniFile(self, config):
    #config = cp.SafeConfigParser()
    #config.read("drone_ctrl.ini")
    try:
      self.ui.checkBox_debug_show_all_hash.setChecked(config.getboolean('debug', 'show#'))
      self.ui.checkBox_debug_show_all_tx.setChecked(config.getboolean('debug', 'showTx'))
      self.ui.checkBox_debug_show_all_rx.setChecked(config.getboolean('debug', 'showRx'))
      self.ui.checkBox_debug_show_hbt.setChecked(config.getboolean('debug', 'showHbt'))
      self.ui.lineEdit_debug_command.setText(config.get('debug', 'cmd1'))
      self.ui.lineEdit_debug_command_2.setText(config.get('debug', 'cmd2'))
      self.ui.lineEdit_debug_command_3.setText(config.get('debug', 'cmd3'))
    except:
      self.main.message("# failed to load debug from ini-file")
    pass
  
  def setIdZero(self):
    self.main.devWrite("setid 0", True)
    self.main.message("eew", True) # saves zero ID to flash, to undo, give an ID (setidx ID) before power cycle
    
