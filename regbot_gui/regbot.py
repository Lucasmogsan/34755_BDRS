#!/usr/bin/env python3
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
import time
try:
  import configparser as cp
except:
  import ConfigParser as cp
from PyQt5 import QtWidgets, QtCore, QtGui
from mainwindow import Ui_regbot
from umain import UMain
from udebug import UDebug
from ulog import ULog
from upose import UPose
from uimu import UImu
from ucontrol import UControlUnit

from uusb import UUsb 
from unet import UNet
from uirsensor import UIRDistance
from umission import UMission, UMissionLine
from urobot_info import URobotInfo
from ulinesensor import ULineSensor
from uirsensor import UIRDistance
from ujoy import UJoy
from uservo import UServo
from ulist import UList
from ubridge import UBridge
from copy import deepcopy

from upaintsupport import *


# this is the revision number shown in the about-box
CLIENT_REV = "$Id: regbot.py 1521 2023-01-28 07:16:57Z jcan $"

class MainWindow(QtWidgets.QMainWindow):
  tickCnt = 0
  
  def clientVersion(self):
    gg = CLIENT_REV.split()
    return gg[2]
  def clientVersionDate(self):
    gg = CLIENT_REV.split()
    return gg[3] + " " + gg[4]

  def __init__(self):
        super(MainWindow, self).__init__()
        #
        # create modules
        self.ui = Ui_regbot()
        self.ui.setupUi(self)
        self.debug = UDebug(self, self.ui)
        self.main = UMain(self, self.ui)
        self.log = ULog(self)
        self.pose = UPose(self, self.ui)
        self.imu = UImu(self, self.ui)
        self.ctrlVelocity = UControlUnit("cvel", self, "Wheel Velocity (left and right)")
        self.ctrlTurn = UControlUnit("ctrn", self, "Heading")
        self.ctrlWallVel = UControlUnit("cwve", self, "IR forward distance")
        self.ctrlWallTurn = UControlUnit("cwth", self, "Wall distance")
        self.ctrlPos = UControlUnit("cpos", self, "Position (drive distance)")
        self.ctrlEdge = UControlUnit("cedg", self, "Line edge")
        self.ctrlBalance = UControlUnit("cbal", self, "Balance")
        self.ctrlBalVel = UControlUnit("cbav", self, "Balance velocity")
        self.ctrlBalPos = UControlUnit("cbap", self, "Balance position")
        #
        self.robotInfo = URobotInfo(self);
        self.usb = UUsb(self)
        self.ir = UIRDistance(self)
        self.servo = UServo(self)
        self.joy = UJoy(self)
        self.net = UNet(self)
        self.mission = UMission(self)
        self.edge = ULineSensor(self)
        self.list = UList(self)
        self.bridge = UBridge(self)
        # initialize now all modules are loaded
        print("Starting ...")
        self.debug.setup()
        self.main.setup()
        self.log.setup()
        self.pose.setup()
        self.imu.setup()
        #self.control.setup()
        self.usb.setup()
        self.net.setup()
        self.ir.setup()
        self.mission.setup()
        self.servo.setup()
        self.joy.setup()
        self.robotInfo.setup()
        self.edge.setup()
        self.list.setup()
        self.bridge.setup()
        # connect buttons and other actions
        self.main.setStatusBar("Starting ...")
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self.timerUpdate)
        # timer event every 50 ms
        self.timer.start(50)
        # menu buttons
        self.ui.actionQuit.triggered.connect(self.stop)
        self.ui.actionAbout.triggered.connect(self.menuAbout)
        # view menu
        self.ui.actionDebug.changed.connect(self.menuViewDebug)
        self.ui.actionLog.changed.connect(self.menuViewLogging)
        self.ui.actionMission.changed.connect(self.menuViewMission)
        self.ui.actionIMU.changed.connect(self.menuViewImu)
        self.ui.actionDistance.changed.connect(self.menuViewIr)
        self.ui.actionControl.changed.connect(self.menuViewControl)
        self.ui.actionWifi.changed.connect(self.menuViewWifi)
        self.ui.actionRobot.changed.connect(self.menuViewRobot)
        self.ui.actionEdge.changed.connect(self.menuViewEdge)
        self.ui.actionServo.changed.connect(self.menuViewServo)
        self.ui.actionJoy.changed.connect(self.menuViewJoy)
        self.ui.actionList.changed.connect(self.menuViewList)
        self.ui.actionBridge.changed.connect(self.menuViewBridge)
        #
        self.ui.actionHide_most.triggered.connect(self.menuViewHideMost)
        self.ui.actionShow_all.triggered.connect(self.menuViewShowAll)
        # ini
        self.ui.actionSave_to_ini_file.triggered.connect(self.menuSaveToIniFile)
        self.ui.actionLoad_from_ini_file.triggered.connect(self.loadFromIniFile)
        #
        self.zoomPlus = QtWidgets.QShortcut(QtGui.QKeySequence('Ctrl++'), self)
        self.zoomPlus.activated.connect(self.zoomIncrease)
        self.zoomMinus = QtWidgets.QShortcut(QtGui.QKeySequence('Ctrl+-'), self)
        self.zoomMinus.activated.connect(self.zoomDecrease)
        self.myfont = self.font()
        self.fontSize = self.myfont.pointSize()
        # # load from ini file

  def zoomIncrease(self):
    self.fontSize = self.fontSize + 1;
    self.setFontSizeAll()
    
  def setFontSizeAll(self):
    self.myfont.setPointSize(self.fontSize)
    QtWidgets.QApplication.instance().setFont(self.myfont, "QLabel")
    QtWidgets.QApplication.instance().setFont(self.myfont, "QCheckBox")
    QtWidgets.QApplication.instance().setFont(self.myfont, "QPushButton")
    QtWidgets.QApplication.instance().setFont(self.myfont, "QWidget")
    # graph font 
    self.imu.setFont(self.myfont)
    self.pose.setFont(self.myfont)

  def zoomDecrease(self):
    if self.fontSize > 8:
      self.fontSize = self.fontSize - 1;
      self.setFontSizeAll()
    pass

  def menuAbout(self):
    # QMessageBox.about (QWidget parent, QString caption, QString text)
    # debug
    about_box = QtWidgets.QMessageBox(my_mainWindow)
    about_box.setText('''<p><span style=" font-size:20pt;">
               <a href="http://www.dtu.dk">DTU</a>
               <a href="http://www.electro.dtu.dk"> Elektro</a>
               <a href="http://rsewiki.elektro.dtu.dk/index.php"> Regbot, Robobot and Fejemis<a></span></p>
               <p><span style=" font-size:10pt;">This is the GUI for calibration of a number of robots<br> 
               Use link above for more details.</span></p>
               <p><span style=" font-size:10pt;">GUI version ''' + self.clientVersion() + ''' (''' + self.clientVersionDate() + ''')</span></p>
               <p><span style=" font-size:10pt;">Teensy version: ''' + self.robotInfo.thisRobot.version + '''</span></p>
               <p><span style=" font-size:10pt;">(contact: jcan@dtu.dk)</span></p>''');
    about_box.setIconPixmap(QtGui.QPixmap("dtulogo_125x182.png"))
    about_box.setWindowTitle("Fejemis about")
    about_box.exec_()


  def closeEvent(self, event):
    print("regbot.py::closeEvent: Stopping (close [X] or ctrl-C)\n")
    self.main.terminate();
    event.accept() # let the window close

  def stop(self):
    print("regbot.py::stop: Stopping (file->quit)\n")
    self.main.terminate();
    QtWidgets.QApplication.quit();

        
  def timerUpdate(self):
    # main will call the rest
    if self.tickCnt == 1:
      # ensure tabs are in correct order
      #print("# show   all " + str(self.tickCnt))
      self.menuViewShowAll()
    elif self.tickCnt == 3:
      # ensure tabs are in correct order
      #print("# hiding most " + str(self.tickCnt))
      self.menuViewHideMost()
    elif self.tickCnt == 5:
      #print("# load from ini " + str(self.tickCnt))
      # load from file - if exist
      self.loadFromIniFile();
    self.tickCnt += 1
    #
    # let main handle the rest
    self.main.timerUpdate()
        
  def getIndex(self, tab):
    i = 0
    if tab <= 0:
      return i
    if (self.ui.actionDebug.isChecked()): 
      i = i + 1
    if (tab <= 1):
      return i
    if (self.ui.actionLog.isChecked()):
      i = i + 1
    if (tab <= 2):
      return i
    if (self.ui.actionMission.isChecked()):
      i = i + 1
    if (tab <= 3):
      return i
    if (self.ui.actionRobot.isChecked()):
      i = i + 1
    if (tab <= 4):
      return i
    if (self.ui.actionIMU.isChecked()):
      i = i + 1
    if (tab <= 5):
      return i
    if (self.ui.actionEdge.isChecked()):
      i = i + 1
    if (tab <= 6):
      return i
    if (self.ui.actionDistance.isChecked()):
      i = i + 1
    if (tab <= 7):
      return i
    if (self.ui.actionControl.isChecked()):
      i = i + 1
    if (tab <= 8):
      return i
    if (self.ui.actionServo.isChecked()):
      i = i + 1
    if (tab <= 9):
      return i
    if (self.ui.actionJoy.isChecked()):
      i = i + 1
    if (tab <= 10):
      return i
    if (self.ui.actionWifi.isChecked()):
      i = i + 1
    if (tab <= 11):
      return i
    if (self.ui.actionList.isChecked()):
      i = i + 1
    if (tab <= 12):
      return i
    if (self.ui.actionBridge.isChecked()):
      i = i + 1
    if (tab <= 13):
      return i
    return i

  def menuShowTab(self, action, order, tab, tabName):
    idx = self.getIndex(order)
    #print("# menuShow show tab " + tab.objectName() + " " + tabName + " order:" + str(order) + ", idx=" + str(idx))
    if (action.isChecked()):
      self.ui.tabPages.insertTab(idx, tab, tabName)
      self.ui.tabPages.setCurrentIndex(idx)
    else:
      #print("# menuShow hide tab " + tab.objectName() + " " + tabName)
      t = self.ui.tabPages.indexOf(tab)
      if t >= 0:
        self.ui.tabPages.removeTab(t)
    pass

  def menuViewDebug(self):
    self.menuShowTab(self.ui.actionDebug, 0, self.ui.tab_debug, "Debug")
  def menuViewLogging(self):
    self.menuShowTab(self.ui.actionLog, 1, self.ui.tab_log, "Logging")
  def menuViewMission(self):
    self.menuShowTab(self.ui.actionMission, 2, self.ui.tab_mission, "Mission")
  def menuViewRobot(self):
    self.menuShowTab(self.ui.actionRobot, 3, self.ui.tab_robot, "Robot")
  def menuViewImu(self):
    self.menuShowTab(self.ui.actionIMU, 4, self.ui.tab_imu, "Imu")
  def menuViewEdge(self):
    self.menuShowTab(self.ui.actionEdge, 5, self.ui.tab_edge, "Edge")
  def menuViewIr(self):
    self.menuShowTab(self.ui.actionDistance, 6, self.ui.tab_ir, "IR")
  def menuViewControl(self):
    self.menuShowTab(self.ui.actionControl, 7, self.ui.tab_control, "Control")
  def menuViewServo(self):
    self.menuShowTab(self.ui.actionServo, 8, self.ui.tab_servo, "Servo")
  def menuViewJoy(self):
    self.menuShowTab(self.ui.actionJoy, 9, self.ui.tab_joy, "Gamepad")
  def menuViewWifi(self):
    self.menuShowTab(self.ui.actionWifi, 10, self.ui.tab_wifi, "Wifi")
  def menuViewList(self):
    self.menuShowTab(self.ui.actionList, 11, self.ui.tab_list, "List")
  def menuViewBridge(self):
    self.menuShowTab(self.ui.actionBridge, 12, self.ui.tab_bridge, "Bridge")

  def menuViewShowAll(self):
    print("# show all")
    self.ui.actionDebug.setChecked(True)
    self.ui.actionWifi.setChecked(True)
    self.ui.actionLog.setChecked(True)
    self.ui.actionMission.setChecked(True)
    self.ui.actionRobot.setChecked(True)
    self.ui.actionIMU.setChecked(True)
    self.ui.actionEdge.setChecked(True)
    self.ui.actionDistance.setChecked(True)
    self.ui.actionControl.setChecked(True)
    self.ui.actionServo.setChecked(True)
    self.ui.actionJoy.setChecked(True)
    self.ui.actionList.setChecked(True)
    self.ui.actionBridge.setChecked(True)

  def menuViewHideMost(self):
    print("# hiding most tabs")
    self.ui.actionDebug.setChecked(True)
    self.ui.actionWifi.setChecked(False)
    self.ui.actionLog.setChecked(True)
    self.ui.actionRobot.setChecked(False)
    self.ui.actionMission.setChecked(True)
    self.ui.actionIMU.setChecked(False)
    self.ui.actionEdge.setChecked(False)
    self.ui.actionDistance.setChecked(False)
    self.ui.actionControl.setChecked(True)
    self.ui.actionServo.setChecked(False)
    self.ui.actionJoy.setChecked(False)
    self.ui.actionList.setChecked(False)
    self.ui.actionBridge.setChecked(False)

  def menuViewHideAll(self):
    print("# hiding all tabs")
    self.ui.actionDebug.setChecked(False)
    self.ui.actionWifi.setChecked(False)
    self.ui.actionLog.setChecked(False)
    self.ui.actionRobot.setChecked(False)
    self.ui.actionMission.setChecked(False)
    self.ui.actionIMU.setChecked(False)
    self.ui.actionEdge.setChecked(False)
    self.ui.actionDistance.setChecked(False)
    self.ui.actionControl.setChecked(False)
    self.ui.actionServo.setChecked(False)
    self.ui.actionJoy.setChecked(False)
    self.ui.actionList.setChecked(False)
    self.ui.actionBridge.setChecked(False)

  def menuSaveToIniFile(self):
    iniFilename = 'regbot.ini'
    print("# saving to ini-file " + iniFilename)
    # must request data first
    if self.main.isConnected() or True:
      #print("# trying to save to ini-file")
      config = cp.ConfigParser()
      config.add_section('main')
      config.set('main', 'version', self.clientVersion())
      config.set('main', 'version date', self.clientVersionDate())
      config.set('main', 'usbDeviceName', str(self.ui.usb_device_name.text()))
      config.set('main', 'wifiDeviceName', str(self.ui.wifi_host_name.text()))
      config.set('main', 'connect', str(self.ui.connect_usb.isChecked()))
      config.set('main', 'connectWifi', str(self.ui.connect_wifi.isChecked()))
      #
      self.debug.saveToIniFile(config)
      # menu settings
      config.add_section('menu')
      config.set('menu', 'debug', str(self.ui.actionDebug.isChecked()))
      config.set('menu', 'log', str(self.ui.actionLog.isChecked()))
      config.set('menu', 'robot', str(self.ui.actionRobot.isChecked()))
      config.set('menu', 'mission', str(self.ui.actionMission.isChecked()))
      config.set('menu', 'imu', str(self.ui.actionIMU.isChecked()))
      config.set('menu', 'edge', str(self.ui.actionEdge.isChecked()))
      config.set('menu', 'ir', str(self.ui.actionDistance.isChecked()))
      config.set('menu', 'control', str(self.ui.actionControl.isChecked()))
      config.set('menu', 'Servo', str(self.ui.actionServo.isChecked()))
      config.set('menu', 'joy', str(self.ui.actionJoy.isChecked()))
      config.set('menu', 'wifi', str(self.ui.actionWifi.isChecked()))
      config.set('menu', 'list', str(self.ui.actionList.isChecked()))
      config.set('menu', 'bridge', str(self.ui.actionBridge.isChecked()))
      #
      config.set('menu', 'tab', str(self.ui.tabPages.currentIndex()))
      #
      # and then all control parts - should be based on regbot name
      self.ctrlVelocity.saveToIniFile(config)
      self.ctrlTurn.saveToIniFile(config)
      self.ctrlWallVel.saveToIniFile(config)
      self.ctrlWallTurn.saveToIniFile(config)
      self.ctrlPos.saveToIniFile(config)
      self.ctrlEdge.saveToIniFile(config)
      self.ctrlBalance.saveToIniFile(config)
      self.ctrlBalVel.saveToIniFile(config)
      self.ctrlBalPos.saveToIniFile(config)
      #config.set('menu', 'manual', str(self.ui.actionManual.isChecked()))
      self.robotInfo.saveToIniFile(config)
      self.bridge.saveToIniFile(config)
      # save old version - if any
      if os.path.exists(iniFilename):
        newname = 'regbot_' + time.strftime("%Y%2m%2d_%02H%2M%2S") + '.ini'
        self.main.setStatusBar("# Renamed " + iniFilename + " to " + newname)
        os.rename(iniFilename, newname)
      #
      try:
        with open(iniFilename, 'w') as configFile:
          config.write(configFile)
          self.main.message("# Saved configuration to " + iniFilename);
      except:
        self.main.message("# failed to save configuration to " + iniFilename);
    else:
      print("# will not save configuration with no contact to drone")
      self.main.message("# will not save configuration with no contact: (" + iniFilename + ")");
  
  def loadFromIniFile(self):
    config = cp.ConfigParser()
    print("#Trying to load from " + self.main.iniFilename)
    config.read(self.main.iniFilename)
    self.debug.loadFromIniFile(config)
    self.robotInfo.loadFromIniFile(config)
    self.bridge.loadFromIniFile(config)
    #self.mag.loadFromIniFile(config)
    #self.esc.loadFromIniFile(config)
    tab = 0
    try:
      #if True:
      self.ui.actionDebug.setChecked(config.getboolean('menu', 'debug'))
      self.ui.actionLog.setChecked(config.getboolean('menu', 'log'))
      self.ui.actionRobot.setChecked(config.getboolean('menu', 'robot'))
      self.ui.actionMission.setChecked(config.getboolean('menu', 'mission'))
      self.ui.actionIMU.setChecked(config.getboolean('menu', 'imu'))
      self.ui.actionEdge.setChecked(config.getboolean('menu', 'edge'))
      self.ui.actionDistance.setChecked(config.getboolean('menu', 'ir'))
      self.ui.actionControl.setChecked(config.getboolean('menu', 'control'))
      self.ui.actionServo.setChecked(config.getboolean('menu', 'servo'))
      self.ui.actionJoy.setChecked(config.getboolean('menu', 'joy'))
      self.ui.actionWifi.setChecked(config.getboolean('menu', 'wifi'))
      self.ui.actionList.setChecked(config.getboolean('menu', 'list'))
      self.ui.actionBridge.setChecked(config.getboolean('menu', 'bridge'))
      # set also the active tab
      tab = config.getint('menu', 'tab')
      # connect
      self.usb.close()
      self.net.close()
      self.ui.usb_device_name.setText(config.get('main','usbDeviceName'))
      self.ui.wifi_host_name.setText(config.get('main','wifiDeviceName'))
      self.ui.connect_usb.setChecked(config.getboolean('main', 'connect'))
      self.ui.connect_wifi.setChecked(config.getboolean('main', 'connectWifi'))
      self.usb.close()
      self.net.close()
      print("# loaded settings, usb = " + 
                             str(self.ui.connect_usb.isChecked()) + ", net = " +
                             str(self.ui.connect_wifi.isChecked()))
    except:
      self.main.message("# missing part of .ini-file")
      print("# missing part of .ini-file")
    #self.configurationFileLoad(config)
    self.ui.tabPages.setCurrentIndex(tab)
    pass

#
# Main start of APP
#

app = QtWidgets.QApplication(sys.argv)

my_mainWindow = MainWindow()
my_mainWindow.setWindowIcon(QtGui.QIcon("dtulogoicon_123x123.png"))
my_mainWindow.setWindowTitle("Teensy based robot calibration interface")
my_mainWindow.show()

sys.exit(app.exec_())
