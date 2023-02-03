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
import pyqtgraph as pg
import time
from PyQt5 import QtWidgets, QtCore, QtGui


class UImu(object):
  "Class to handle IMU data"
  gyro = [0.0, 0.0, 0.0]
  gyroOffset = [0, 0, 0]
  gyroOffsetOK = False;
  acc = [0.0, 0.0, 0.0]
  # IMU orientation relative to robot coordinates (radians)
  board = [0.0,0.0,0.0]
  imupose = [0.0, 0.0, 0.0]
  imudataReadAcw = False
  imudataReadGyo = False
  imudataReadGyw = False
  imuposeRead = False;
  lock = threading.RLock()
  # plot if IMU data - test
  pwg = 0 # handle for plot window
  pwa = 0 # handle for plot window acc
  pwt = 0 # handle for plot window tilt
  cg = 0
  cga = 0
  cgt = 0
  idxg = 0
  idxa = 0
  idxt = 0
  data = np.zeros((3,100)) # gyro
  dataa = np.zeros((3,100)) # accelerometer
  datat = np.zeros(100) # tilt
  lastDataRequestTime = time.time()
  lastDataRequest = 0
  waitingForData = False
  hasFocus = False
  thisTab = -1
  # edit bord orientation
  inEdit = False
  imuBoardNew = False


  #datat = np.zeros(100)
  def __init__(self, parent, ui):
    self.parent = parent
    self.ui = ui
    self.main = parent.main
    
  def setup(self):
    #p = self.ui.tab_imu.palette() # IMU
    #p.setColor(self.ui.tab_imu.backgroundRole(), QtCore.Qt.lightGray)
    #self.ui.tab_imu.setPalette(p)
    #self.ui.tab_imu.setAutoFillBackground(True)
    #
    self.initGraph()
    self.ui.imu_gyro_calibrate.clicked.connect(self.doGyroOffset)
    self.ui.pushButton_board_apply.clicked.connect(self.butApply) 
    self.ui.pushButton_board_cancel.clicked.connect(self.butCancel)
    self.ui.pushButton_board_edit.clicked.connect(self.butEdit) 
    #self.ui.board_rot_x.returnPressed.connect(self.butApply)
    #self.ui.board_rot_y.returnPressed.connect(self.butApply)
    #self.ui.board_rot_z.returnPressed.connect(self.butApply)


    pass
  #
  def initGraph(self):
    "initialize graph plot, IMU data"
    self.pwg = pg.PlotWidget(name='IMU-plot gyro',title='Gyro')  ## giving the plots names allows us to link their axes together
    #self.pwg.setWindowTitle('IMU gyro')
    self.pwg.setLabel('left','rotation velocity','deg/s')
    self.pwg.addLegend()    
    self.ui.robot_graph_layout_gyro.addWidget(self.pwg)
    self.cg = [self.pwg.plot(pen='r',name='x'), self.pwg.plot(pen='b',name='y'), self.pwg.plot(pen='g',name='z')]
    self.cg[0].setData(self.data[0])
    self.cg[1].setData(self.data[1])
    self.cg[2].setData(self.data[2])
    # acc
    self.pwa = pg.PlotWidget(name='IMU-plot acc',title='Accelerometer')  ## giving the plots names allows us to link their axes together
    #self.pwa.setWindowTitle('IMU accelerometer')
    self.pwa.setLabel('left','acceleration','m/s^2')
    self.pwa.addLegend()    
    self.ui.robot_graph_layout_acc.addWidget(self.pwa)
    self.cga = [self.pwa.plot(pen='r',name='x'), self.pwa.plot(pen='b',name='y'), self.pwa.plot(pen='g',name='z')]
    self.cga[0].setData(self.dataa[0])
    self.cga[1].setData(self.dataa[1])
    self.cga[2].setData(self.dataa[2])
    # tilt
    self.pwt = pg.PlotWidget(name='IMU-plot tilt',title='Tilt')  ## giving the plots names allows us to link their axes together
    self.pwt.setWindowTitle('IMU tilt')
    #self.pwt.addLegend()    
    self.pwt.setLabel('left','tilt','deg')
    self.ui.robot_graph_layout_tilt.addWidget(self.pwt)
    self.cgt = self.pwt.plot(pen='r',name='x')
    self.cgt.setData(self.datat)
    #
    #widg = pq.PlotWidget(background=(0, 0, 0, 255), x=[0, 1, 2, 3], y=[0, 1, 2, 3],font= font,font_size= 30) 
  
  def setFont(self, toFont):
    item= self.pwg.getPlotItem()
    item.getAxis("bottom").setTickFont(toFont)
    item.getAxis("left").setTickFont(toFont)
    item= self.pwa.getPlotItem()
    item.getAxis("bottom").setTickFont(toFont)
    item.getAxis("left").setTickFont(toFont)
    item= self.pwt.getPlotItem()
    item.getAxis("bottom").setTickFont(toFont)
    item.getAxis("left").setTickFont(toFont)
  #
  def decode(self, gg):
    used = True
    self.lock.acquire()
    if True: #try:
      if gg[0] == "gyro":
        self.gyro[0] = float(gg[1])
        self.gyro[1] = float(gg[2])
        self.gyro[2] = float(gg[3])
        self.data[0,self.idxg] = self.gyro[0]
        self.data[1,self.idxg] = self.gyro[1]
        self.data[2,self.idxg] = self.gyro[2]
        if self.idxg < 100 - 1:
          self.idxg += 1
        else:
          self.idxg = 0
        self.imudataReadGyw = True
        self.waitingForData = False
      elif gg[0] == "acc": 
        self.acc[0] = float(gg[1])
        self.acc[1] = float(gg[2])
        self.acc[2] = float(gg[3])
        self.dataa[0,self.idxa] = self.acc[0]
        self.dataa[1,self.idxa] = self.acc[1]
        self.dataa[2,self.idxa] = self.acc[2]
        if self.idxa < 100 - 1:
          self.idxa += 1
        else:
          self.idxa = 0
        self.imudataReadAcw = True
        self.waitingForData = False
      elif gg[0] == "gyroo":
        self.gyroOffset[0] = float(gg[1])
        self.gyroOffset[1] = float(gg[2])
        self.gyroOffset[2] = float(gg[3])
        #self.gyroOffsetOK = int(gg[4], 10)
        self.imudataReadGyo = True
        self.waitingForData = False
      elif gg[0] == "imupose":
        self.imupose[0] = float(gg[1])
        self.imupose[1] = float(gg[2])
        self.imupose[2] = float(gg[3])
        self.datat[self.idxt] = self.imupose[1] * 180/np.pi # change to degrees
        if self.idxt < 100 - 1:
          self.idxt += 1
        else:
          self.idxt = 0
        self.imuposeRead = True;
      elif gg[0] == "board":
        self.board[0] = float(gg[1])
        self.board[1] = float(gg[2])
        self.board[2] = float(gg[3])
        self.imuBoardNew = True;
      else:
        used = False
    #except:
      #print("UImu: data read error - skipped a " + gg[0])
      #pass
    self.lock.release()
    return used
  
  
  def timerUpdate(self, timerCnt, justConnected):
    if self.imudataReadAcw:
      self.imudataReadAcw = False
      self.ui.val_acc.setValue(self.acc[0])
      self.ui.val_acc_2.setValue(self.acc[1])
      self.ui.val_acc_3.setValue(self.acc[2])
      self.cga[0].setData(self.dataa[0])
      self.cga[1].setData(self.dataa[1])
      self.cga[2].setData(self.dataa[2])
    if self.imudataReadGyw:
      self.imudataReadGyw = False
      self.ui.val_gyro.setValue(self.gyro[0])
      self.ui.val_gyro_2.setValue(self.gyro[1])
      self.ui.val_gyro_3.setValue(self.gyro[2])
      self.cg[0].setData(self.data[0])
      self.cg[1].setData(self.data[1])
      self.cg[2].setData(self.data[2])
    if self.imudataReadGyo:
      self.imudataReadGyo = False
      self.ui.val_gyro_offset_x.setValue(self.gyroOffset[0])
      self.ui.val_gyro_offset_y.setValue(self.gyroOffset[1])
      self.ui.val_gyro_offset_z.setValue(self.gyroOffset[2])
      #self.ui.imu_gyro_offset_done.setVisible(False)
      #self.ui.imu_gyro_offset_done.setChecked(self.gyroOffsetOK)
    if self.imuposeRead:
      self.imuposeRead = False
      self.ui.val_imu_tilt.setValue(self.imupose[1])
      self.ui.val_imu_tilt_2.setValue(self.imupose[1] * 180 / np.pi)
      self.cgt.setData(self.datat)
    if self.imuBoardNew:
      self.imuBoardNew = False
      if not self.inEdit:
        self.ui.board_rot_x.setValue(self.board[0] * 180.0 / np.pi)
        self.ui.board_rot_y.setValue(self.board[1] * 180.0 / np.pi)
        self.ui.board_rot_z.setValue(self.board[2] * 180.0 / np.pi)
    #
    self.ui.pushButton_board_edit.setEnabled(not self.inEdit)
    self.ui.pushButton_board_cancel.setEnabled(self.inEdit)
    self.ui.pushButton_board_apply.setEnabled(self.inEdit)

    #
    thisTab = self.ui.tabPages.indexOf(self.ui.tab_imu)
    if (self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() != thisTab:
      # just switched away from this tab
      self.hasFocus = False
      # if we are talking to a bridge - then just un-subscribe
      if self.main.isBridge():
        # NB! not the right messages
        self.main.devWrite("regbot:gyro subscribe 0\n") # gyro
        self.main.devWrite("regbot:gyroo subscribe 0\n") # gyro offset
        self.main.devWrite("regbot:acc subscribe 0\n") # accelerometer
        self.main.devWrite("regbot:imupose subscribe 0\n") # imu based pose
        self.main.devWrite("regbot:board subscribe 0\n") # imu board pose
        self.main.devWrite("regbot sub imupose 0\n") # pose
        self.main.devWrite("regbot sub gyroo 0\n") # gyro offset
        self.main.devWrite("regbot sub board 0\n") # imu board pose
      else:
        # talking to Teensy directly, so un-subscribe here
        self.main.devWrite("sub acc 0\n") # acc subscribe
        self.main.devWrite("sub gyro 0\n") # gyro subscribe
        self.main.devWrite("sub gyroo 0\n") # gyro offset
        self.main.devWrite("sub imupose 0\n") # imu pose
        self.main.devWrite("sub board 0\n") # imu pose
        pass
      pass
    if (not self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() == thisTab:
      # just entering this tab
      self.hasFocus = True
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        self.main.devWrite("regbot:imupose subscribe -1\n")
        self.main.devWrite("regbot:acc subscribe -1\n")
        self.main.devWrite("regbot:gyro subscribe -1\n")
        self.main.devWrite("regbot:gyroo subscribe -1\n")
        self.main.devWrite("regbot:board subscribe -1\n") # imu board
        self.main.devWrite("regbot sub imupose 40\n") # pose
        self.main.devWrite("regbot sub gyroo 1000\n") # gyro offset
        self.main.devWrite("regbot sub board 1101\n") # imu board pose
      else:
        # talking to Teensy directly, so subscribe here
        self.main.devWrite("sub gyro 40\n") # gyro subscribe
        self.main.devWrite("sub acc 40\n") # accelerometer
        self.main.devWrite("sub imupose 40\n") # pose
        self.main.devWrite("sub gyroo 1000\n") # gyro offset
        self.main.devWrite("sub board 1101\n") # imu board pose
      pass
    pass


    
  def doGyroOffset(self):
    #self.ui.imu_gyro_offset_done.setChecked(False)
    self.main.devWrite("gyroc\n", True)

  def butApply(self):
    self.main.devWrite("board %g %g %g\n" % (
            self.ui.board_rot_x.value() * np.pi / 180.0,
            self.ui.board_rot_y.value() * np.pi / 180.0,
            self.ui.board_rot_z.value() * np.pi / 180.0
            ), True)
    self.inEdit = False
    pass
  
  def butCancel(self):
    self.inEdit = False
    pass
  
  def butEdit(self):
    self.lock.acquire()
    self.inEdit = True
    self.lock.release()
    pass
