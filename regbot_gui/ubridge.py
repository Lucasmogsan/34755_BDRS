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

class UBridge(object):
  list = "nothing to show"
  #
  bridgeListNew = False
  sourceListNew = False
  #
  lock = threading.RLock()
  inEdit = False
  inTimerUpdate = True
  hasFocus = False
  thisTab = -1
  #
  def __init__(self, parent):
    self.main = parent.main
    self.ui = parent.ui

  def setup(self):
    self.ui.pushButton_bridge_items.clicked.connect(self.getItems)
    self.ui.pushButton_bridge_sources.clicked.connect(self.getSources)
    self.ui.pushButton_bridge_clear.clicked.connect(self.listClear)
    self.ui.pushButton_bridge_send.clicked.connect(self.sendCmd)
    self.ui.lineEdit_bridge_cmd.returnPressed.connect(self.sendCmd)
    pass

  def timerUpdate(self, timerCnt, justConnected):
    self.lock.acquire()
    self.inTimerUpdate = True
    if self.bridgeListNew:
      self.bridgeListNew = False
      self.ui.plainTextEdit_bridge_list.setPlainText(self.list)
    if self.sourceListNew:
      self.sourceListNew = False
      self.ui.plainTextEdit_bridge_list.setPlainText(self.list)
    self.lock.release()
    #
    #if self.listChanged:
      #print("# bridge list changed to " + self.list)
      #self.ui.plainTextEdit_bridge_list.setPlainText(self.list)
      #self.listChanged = False
    # get data
    thisTab = self.ui.tabPages.indexOf(self.ui.tab_servo)
    if (self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() != thisTab:
      # just switched away from this tab
      self.hasFocus = False
      # if we are talking to a bridge - then just un-subscribe
      if self.main.isBridge():
        # noting to stop
        pass
      pass
    if (not self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() == thisTab:
      # just entering this tab
      self.hasFocus = True
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        # nothing to subscribe for this tab
        pass
      pass
    pass
    if justConnected:
      self.ui.checkBox_bridge.setChecked(self.main.isBridge())
  #
  #
  def decode(self, gg):
    dataUsed = True
    self.lock.acquire()
    try:
      if gg[0] == "source":
        self.list += gg[0]
        useTab = True
        for i in range(1, len(gg)):
          if useTab:
            self.list += " \t" + gg[i]
          else:
            self.list += " " + gg[i]
          if useTab and gg[i][0] == "'":
            useTab = False
        self.list += "\n"
        self.sourceListNew = True
        #
      elif gg[0] == "item":
        useTab = True
        srcKey = gg[4].split(':')
        # is this data requested
        use = self.ui.checkBox_bridge_all.isChecked()
        if not use:
          use = srcKey[0] == "regbot" and self.ui.checkBox_bridge_regbot.isChecked()
        if not use:
          use = srcKey[0] == "host" and self.ui.checkBox_bridge_host.isChecked()
        if not use:
          use = srcKey[0] == "ros" and self.ui.checkBox_bridge_ros.isChecked()
        if not use:
          use = gg[4][:6] == "socket" and self.ui.checkBox_bridge_socket.isChecked()
        if use:
          self.list += "item"
          for i in range(1, len(gg)):
            if useTab and i > 1 and i != 4:
              self.list += " \t" + gg[i]
            else:
              self.list += " " + gg[i]
            if i >= 5:
              useTab = False
          self.list += "\n"
          self.bridgeListNew = True
        pass
      elif gg[0] == "bridge":
        # message from bridge, when just connected
        # not used 
        pass
      else:
        dataUsed = False
        #self.dataRead = True
    except:
      print("UBridge: data read error - skipped a " + gg[0])
      pass
    self.lock.release()
    return dataUsed

  def getItems(self):
    self.list = "%% List of data items in bridge (ignoring items generated by socket connection)\n"
    self.list += "%% format: \n"
    self.list += "%%      'item, number, updates, logfile open, source:key, parameters, 'description'\n"
    self.main.devWrite("bridge items\n");
    self.listClear()
    pass
  
  def getSources(self):
    self.list = "%% List of data sources in bridge\n"
    self.list += "%% format: \n"
    self.list += "%%      'source, index, device name:dev, active, parameters'\n"
    self.main.devWrite("bridge sources\n");
    pass
  
  def sendCmd(self):
    self.main.devWrite(self.ui.lineEdit_bridge_cmd.text())
    pass
  
  def listClear(self):
    self.list = ""
    self.bridgeListNew = True

  def saveToIniFile(self, config):
    # settings
    config.add_section('bridge')
    config.set('bridge', 'cmd1', str(self.ui.lineEdit_bridge_cmd.text()))

  def loadFromIniFile(self, config):
    try:
      self.ui.lineEdit_bridge_cmd.setText(config.get('bridge', 'cmd1'))
    except:
      self.main.message("# failed to load bridge section from ini-file")
    pass

