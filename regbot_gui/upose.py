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
#import math




class UPose(object):
  motorCurrent = [0.0, 0.0]
  motorVolt = [0.0, 0.0]
  motorEncoder = [0, 0]
  wheelVelocity = [0.0, 0.0]
  pose = [0.0, 0.0, 0.0, 0.0] # in m,m,radians
  #tilt = 0.0 # in radians
  distance = 0.0
  battery = 0.0
  lock = threading.RLock()
  dataReadMca = False
  dataReadMcv = False
  dataReadEnc = False
  dataReadVel = False
  #dataReadWpo = False
  dataReadPse = False
  dataReadBat = False
  lastDataRequestTime = time.time()
  nextDataRequest = 0
  lastTab = ""
  cpu_load = 1000
    #
  pwp = 0 # handle for plot window
  pwt = 0 # handle for plot window tilt
  cg = 0  # position plot
  cgt = 0 # tilt plot
  idxMax = 3000;
  data = np.zeros((2, idxMax))
  newPos = False
  #datat = np.zeros(100)    # tilt data
  a1 = 0 # pose arrow
  idx = 0 # index (count of) pose items
  #idxt = 0 # index to tilt data
  minx = -1.0
  miny = -1.0
  maxx = 2.0
  maxy = 2.0
  hasFocus = False
  #
  def __init__(self, parent, ui):
    self.main = parent.main
    self.ui = ui

  def setup(self):
    self.initGraph()
    pass

  def setFont(self, toFont):
    item= self.pwg.getPlotItem()
    item.getAxis("bottom").setTickFont(toFont)
    item.getAxis("left").setTickFont(toFont)


  def decode(self, gg):
    used = True
    self.lock.acquire()
    if True: #try:
      if gg[0] == "mca":
        if len(gg) > 2:
          self.motorCurrent[0] = float(gg[1])
          self.motorCurrent[1] = float(gg[2])
          self.dataReadMca = True
      elif gg[0] == "mot":
        self.motorVolt[0] = float(gg[1])
        self.motorVolt[1] = float(gg[2])
        self.dataReadMcv = True
      elif gg[0] == "enc":
        e = int(gg[1],10)
        if (e > 0x7fffffff):
          self.motorEncoder[0] = e - 0x100000000 
        else:
          self.motorEncoder[0] = e
        e = int(gg[2],10)
        if (e > 0x7fffffff):
          self.motorEncoder[1] = e - 0x100000000 
        else:
          self.motorEncoder[1] = e
        self.dataReadEnc = True
      elif gg[0] == "vel":
        if len(gg) > 3:
          self.wheelVelocity[0] = float(gg[2])
          self.wheelVelocity[1] = float(gg[3])
          self.dataReadVel = True
      #elif gg[0] == "wpo":
        #self.wheelPos[0] = float(gg[1])
        #self.wheelPos[1] = float(gg[2])
        #self.dataReadWpo = True
      elif gg[0] == "pose":
        if len(gg) > 5:
          self.poseTime = float(gg[1])
          self.pose[0] = float(gg[2])
          self.pose[1] = float(gg[3])
          self.pose[2] = float(gg[4])
          self.pose[3] = float(gg[5])
        # need for new pose history entry (for plotting)
        if abs(self.pose[0] - self.data[0,self.idx]) > 0.001 or abs(self.pose[1] - self.data[1,self.idx]) > 0.001:
          self.idx += 1
          if (self.idx >= self.idxMax):
            # remove old data to get space in array
            #print("drive overflow idx==" + str(self.idx) + ", max= " + str(self.idxMax))
            self.idx=self.idxMax // 2 # Integer division
            #print("drive at idx=" + str(self.idx) + ", data length= " + str(self.data.shape))
            self.data = np.append(self.data[:,self.idx:], np.zeros((2,self.idxMax - self.idx)), axis=1) 
            #self.data = self.data[:,self.idx:]
          self.data[0, self.idx] = self.pose[0]
          self.data[1, self.idx] = self.pose[1]
          self.newPos = True
        if (self.pose[0] > self.maxx):
          self.maxx = self.pose[0]
        elif (self.pose[0] < self.minx):
          self.minx = self.pose[0]  
        if (self.pose[1] > self.maxy):
          self.maxy = self.pose[1]
        elif (self.pose[1] < self.miny):
          self.miny = self.pose[1]  
        self.dataReadPse = True
      #elif gg[0] == "bat":
        #self.battery = float(gg[1])
        #self.dataReadBat = True
      else:
        used = False
    #except:
      #print("UDrive: data read error - skipped a " + gg[0])
      #pass
    self.lock.release()
    return used
  
  
  def timerUpdate(self, justConnected):
    if (self.dataReadBat):
      self.dataReadBat = False
      self.ui.val_batt.setValue(self.battery)
      self.ui.cpu_load.setText(str(self.cpu_load))
    if self.dataReadEnc:
      self.dataReadEnc = False
      self.ui.robot_enc_left.setValue(self.motorEncoder[0])
      self.ui.robot_enc_right.setValue(self.motorEncoder[1])
    if self.dataReadMca:
      self.dataReadMca = False
      self.ui.robot_current_left.setValue(self.motorCurrent[0])
      self.ui.robot_current_right.setValue(self.motorCurrent[1])
    if self.dataReadMcv:
      self.dataReadMcv = False
      #self.ui.robot_volt_left.setValue(self.motorVolt[0])
      #self.ui.robot_volt_right.setValue(self.motorVolt[1])
    if self.dataReadVel:
      self.dataReadVel = False
      self.ui.robot_wheel_vel_left.setValue(self.wheelVelocity[0])
      self.ui.robot_wheel_vel_right.setValue(self.wheelVelocity[1])
    if self.dataReadPse:
      self.dataReadPse = False
      self.ui.robot_pose_x.setValue(self.pose[0])
      self.ui.robot_pose_y.setValue(self.pose[1])
      self.ui.robot_pose_h.setValue(self.pose[2])
      self.ui.robot_pose_h_2.setValue(self.pose[2]*180.0/np.pi);
      self.ui.robot_distance.setValue(self.distance)
      tilt = self.pose[3]
      self.ui.robot_tilt.setValue(tilt)
      self.ui.robot_tilt_2.setValue(tilt*180.0/np.pi);
      self.ui.val_imu_tilt.setValue(tilt)
      self.ui.val_imu_tilt_2.setValue(tilt*180.0/np.pi)
    # show new robot pose
    if (self.newPos):
      # send data slize up to index to be displayed
      self.cg.setData(x=self.data[0,:self.idx + 1], y=self.data[1,:self.idx + 1])
      #print("drive " + str(self.idx) + " pos " + str(self.data[0,self.idx]) + ", " + str(self.data[1,self.idx]))
      self.newPos = False
      # self.cg.setLimits(self.minx, self.maxx, self.miny, self.maxy)
      self.pwg.removeItem(self.a1)
      self.a1 = pg.ArrowItem(angle=180 - self.pose[2]*180/np.pi)
      self.a1.setPos(self.pose[0],self.pose[1])
      self.pwg.addItem(self.a1)
    #self.a1.setData(self.pose[2])
    # show new tilt angle
    #self.cgt.setData(self.datat)
    # request new data
    thisTab = self.ui.tabPages.indexOf(self.ui.tab_robot)
    if (self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() != thisTab:
      # just switched away from this tab
      self.hasFocus = False
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        self.main.devWrite(":pose subscribe 0\n")
        self.main.devWrite(":enc subscribe 0\n")
        self.main.devWrite(":vel subscribe 0\n")
        self.main.devWrite(":mca subscribe 0\n")
        self.main.devWrite("regbot sub enc 0\n") # encoder position
        self.main.devWrite("regbot sub vel 0\n") # velocity
        self.main.devWrite("regbot sub mca 0\n") # motor current
      else:
        # talking to Teensy directly, so subscribe here
        self.main.devWrite("sub pose 0\n") # pose and tilt
        self.main.devWrite("sub enc 0\n") # encoder position
        self.main.devWrite("sub vel 0\n") # velocity
        self.main.devWrite("sub mca 0\n") # motor current
        pass
      pass
    if (not self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() == thisTab:
      # just entering this tab
      self.hasFocus = True
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        self.main.devWrite(":pose subscribe 30\n")
        self.main.devWrite(":enc subscribe 30\n")
        self.main.devWrite(":vel subscribe 60\n")
        self.main.devWrite(":mca subscribe 60\n")
        self.main.devWrite("regbot sub enc 61\n") # encoder position
        self.main.devWrite("regbot sub vel 55\n") # velocity
        self.main.devWrite("regbot sub mca 57\n") # motor current
      else:
        # talking to Teensy directly, so subscribe here
        #print("# UPose is requesting pose, enc, vel and mca")
        self.main.devWrite("sub pose 35\n") # pose and tilt
        self.main.devWrite("sub enc 61\n") # encoder position
        self.main.devWrite("sub vel 55\n") # velocity
        self.main.devWrite("sub mca 57\n") # motor current
      pass
    pass
  #
  #
  def poseReset(self):
    self.idx = 0
    self.data = np.zeros((2, self.idxMax))
    self.newPos = True
    #self.timerUpdate()
    print("drive pose reset")
  #
  #
  def initGraph(self):
    "initialize graph plot robot pose"
    # pose
    self.pwg = pg.PlotWidget(name='robot-pose',title='robot position')  ## giving the plots names allows us to link their axes together
    self.pwg.setLabel('bottom','x position','m')
    self.pwg.setLabel('left','y position','m')
    self.pwg.setWindowTitle('Pose')
    self.ui.robot_pose_layout.addWidget(self.pwg)
    self.cg = self.pwg.plot(pen='r',name='position m')
    #self.cg.setAspectLocked()
    vb = self.cg.getViewBox()
    #self.pwg.setLimits(minXRange=3.0, minYRange=3.0)
    #self.pwg.setLimits(-1.0, 2.0, -1.0, 2.0)
    self.pwg.setAspectLocked()
    self.cg.setData(x=self.data[0], y=self.data[1])
    # pose arrow
    self.a1 = pg.ArrowItem(angle=60)
    self.a1.setPos(0,0)
    self.pwg.addItem(self.a1)
    # tilt
    #self.pwt = pg.PlotWidget(name='IMU-plot tilt',title='Tilt angle')  ## giving the plots names allows us to link their axes together
    #self.pwt.setWindowTitle('IMU tilt')
    ##self.pwt.setLabel('bottom','sample')
    #self.pwt.setLabel('left','',' deg')
    #self.ui.robot_graph_layout_tilt.addWidget(self.pwt)
    #self.cgt = self.pwt.plot(pen='r',name='degres')
    #self.cgt.setData(self.datat)
  #
