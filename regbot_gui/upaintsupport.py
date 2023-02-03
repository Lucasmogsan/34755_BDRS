#!/usr/bin/python
# -*- coding: utf-8 -*-

# "$Rev: 1469 $"

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


#import sys
#import os
#from PyQt4 import QtGui, QtCore, Qt
from PyQt5 import QtCore, QtGui, QtWidgets
#from pyqtgraph.Qt import QtGui #, QtCore




class PaintSupport(QtWidgets.QWidget):
  def __index__(self, widget):
      super(PaintSupport, self).__init__(self, widget)
  def paintPlus(self, paint, x, y):
      paint.drawLine(QtCore.QPoint(x - 3, y), QtCore.QPoint(x + 3, y))
      paint.drawLine(QtCore.QPoint(x, y - 3), QtCore.QPoint(x, y + 3))
  def paintMinus(self, paint, x, y):
      paint.drawLine(QtCore.QPoint(x - 3, y), QtCore.QPoint(x + 3, y))
  def paintDot(self, paint, x, y):
      paint.setBrush(QtCore.Qt.black)
      paint.drawEllipse(QtCore.QPoint(x, y), 3, 3)
      paint.setBrush(QtCore.Qt.NoBrush)
  def paintArrow(self, paint, x, y, dir, color = QtCore.Qt.black):
      dx1 = -5 # down
      dx2 = +5
      dy1 = -5
      dy2 = -5
      if (dir == 0): # right
        dx1 = -5
        dx2 = -5
        dy1 = -5
        dy2 = +5
      elif (dir == 1): # up
        dx1 = -5
        dx2 = +5
        dy1 = +5
        dy2 = +5
      elif (dir == 2): # left
        dx1 = +5
        dx2 = +5
        dy1 = -5
        dy2 = +5
      pts = [QtCore.QPoint(x,y), 
             QtCore.QPoint(x + dx1,y + dy1),
             QtCore.QPoint(x + dx2,y + dy2)]
      poly = QtGui.QPolygon(pts)
      paint.setBrush(color)
      paint.drawPolygon(poly)
      paint.setBrush(QtCore.Qt.NoBrush)
  pass

# //////////////////////////////////////////////////////////////7
# //////////////////////////////////////////////////////////////7
  
class UsbPaintSpace(PaintSupport):
  def __init__(self, frame, robot):
    self.robot = robot
    print("UsbPaintSpace call init")
    super(UsbPaintSpace, self).__init__(frame)
    self.ui = robot.ui
  def paintEvent(self, QPaintEvent):
    paint = QtGui.QPainter()
    paint.begin(self)
    paint.setPen(QtCore.Qt.darkGreen)
    if (self.robot.isConnected()):
      paint.setBrush(QtCore.Qt.green)
    else:
      paint.setBrush(QtCore.Qt.NoBrush)
    paint.drawRect(1,1, self.ui.frame_usb_connect.width(), self.ui.frame_usb_connect.height())
    paint.setBrush(QtCore.Qt.NoBrush)
    #print("usb frame painted")
    pass
      
class WifiPaintSpace(PaintSupport):
  def __init__(self, frame, robot):
    super(WifiPaintSpace, self).__init__(frame)
    self.robot = robot
    self.ui = robot.ui
  def paintEvent(self, QPaintEvent):
    paint = QtGui.QPainter()
    paint.begin(self)
    paint.setPen(QtCore.Qt.darkBlue)
    if self.robot.wifiConnected and not self.robot.isConnected():
      # green if wifi is connected and USB is not
      paint.setBrush(QtCore.Qt.green)
    else:
      paint.setBrush(QtCore.Qt.NoBrush)
    paint.drawRect(1,1, self.ui.frame_wifi_connect.width(), self.ui.frame_wifi_connect.height())
    paint.setBrush(QtCore.Qt.NoBrush)
    #print("wifi frame painted")
    pass


class ControlPaintSpace(PaintSupport):
  def __init__(self, frame, robot):  
    super(ControlPaintSpace, self).__init__(frame)
    self.robot = robot
    self.ui = robot.ui
  def paintEvent(self, QPaintEvent):
    paint = QtGui.QPainter()
    paint.begin(self)
    pen = QtGui.QPen(QtCore.Qt.darkBlue, 1, QtCore.Qt.SolidLine)
    paint.setPen(pen)
    col = QtCore.Qt.darkBlue
    #paint.setPen(QtCore.Qt.darkBlue)
    paint.setBrush(QtCore.Qt.NoBrush)
    #paint.drawRect(1,1, self.ui.frame_wifi_connect.width(), self.ui.frame_wifi_connect.height())
    #paint.setBrush(QtCore.Qt.NoBrush)
    #print("control paint")
    p1Mix = QtCore.QPoint(self.ui.frame_mix_select.x() + self.ui.frame_mix_select.width(), self.ui.frame_mix_select.y() + int(self.ui.frame_mix_select.height()/2));
    p1Vel = QtCore.QPoint(self.ui.frame_ctrl_vel.x() + int(self.ui.frame_ctrl_vel.width()/2), self.ui.frame_ctrl_vel.y());
    paint.drawLine(p1Mix, QtCore.QPoint(p1Vel.x(), p1Mix.y())) # mix right to above vel ctrl
    paint.drawLine(QtCore.QPoint(p1Vel.x(), p1Mix.y()), p1Vel) # down to vel Ctrl
    #paint.drawEllipse(QtCore.QPoint(p1.x(), p1.y()), 14, 14)
    self.paintArrow(paint, p1Vel.x(), p1Vel.y(), 3, col)
    p1Mot = QtCore.QPoint(p1Vel.x(), self.ui.frame_motors.y()); # motors frame
    paint.drawLine(QtCore.QPoint(p1Vel.x(), p1Vel.y() + self.ui.frame_ctrl_vel.height()), p1Mot) # down to vel Ctrl
    self.paintArrow(paint, p1Mot.x(), p1Mot.y(), 3, col)
    #
    p2tilt = QtCore.QPoint(self.ui.frame_ctrlBal.x() + self.ui.frame_ctrlBal.width(), self.ui.frame_ctrlBal.y() + int(self.ui.frame_ctrlBal.height()/2)); # balance ctrl
    p1MixX = self.ui.frame_mix_select.x()
    paint.drawLine(p2tilt, QtCore.QPoint(p1MixX, p2tilt.y())) # from tilt to mix
    self.paintArrow(paint, p1MixX, p2tilt.y(), 0, col)
    #
    p2WallDist = QtCore.QPoint(self.ui.frame_ctrlWallVel.x() + self.ui.frame_ctrlWallVel.width(), self.ui.frame_ctrlWallVel.y() + int(self.ui.frame_ctrlWallVel.height()/2)); # balance ctrl
    paint.drawLine(p2WallDist, QtCore.QPoint(p1MixX, p2WallDist.y())) # from tilt to mix
    self.paintArrow(paint, p1MixX, p2WallDist.y(), 0, col)
    #
    p2WallTurn = QtCore.QPoint(self.ui.frame_ctrlWallTurn.x() + self.ui.frame_ctrlWallTurn.width(), self.ui.frame_ctrlWallTurn.y() + int(self.ui.frame_ctrlWallTurn.height()/2)); # balance ctrl
    paint.drawLine(p2WallTurn, QtCore.QPoint(p1MixX, p2WallTurn.y())) # from tilt to mix
    self.paintArrow(paint, p1MixX, p2WallTurn.y(), 0, col)
    #
    p2Edge = QtCore.QPoint(self.ui.frame_ctrlEdge.x() + self.ui.frame_ctrlEdge.width(), self.ui.frame_ctrlEdge.y() + int(self.ui.frame_ctrlEdge.height()/2)); # edge ctrl
    paint.drawLine(p2Edge, QtCore.QPoint(p1MixX, p2Edge.y())) # from line edge to mix
    self.paintArrow(paint, p1MixX, p2Edge.y(), 0, col)
    #
    p2Pos = QtCore.QPoint(self.ui.frame_ctrlPos.x() + self.ui.frame_ctrlPos.width(), self.ui.frame_ctrlPos.y() + int(self.ui.frame_ctrlPos.height()/2)); # position ctrl
    paint.drawLine(p2Pos, QtCore.QPoint(p1MixX, p2Pos.y())) # from tilt to mix
    self.paintArrow(paint, p1MixX, p2Pos.y(), 0, col)
    #
    p2Turn = QtCore.QPoint(self.ui.frame_turn.x() + self.ui.frame_turn.width(), self.ui.frame_turn.y() + int(self.ui.frame_turn.height()/2)); # turn ctrl
    paint.drawLine(p2Turn, QtCore.QPoint(p1MixX, p2Turn.y())) # from turn to mix
    self.paintArrow(paint, p1MixX, p2Turn.y(), 0, col)
    #
    p2BalVel = QtCore.QPoint(self.ui.frame_ctrlBalVel.x() + self.ui.frame_ctrlBalVel.width(), self.ui.frame_ctrlBalVel.y() + int(self.ui.frame_ctrlBalVel.height()/2)); # balance velocity ctrl
    p2BalPos = QtCore.QPoint(self.ui.frame_ctrlBalPos.x() + self.ui.frame_ctrlBalPos.width(), self.ui.frame_ctrlBalPos.y() + int(self.ui.frame_ctrlBalPos.height()/2)); # balance position ctrl
    # from balance vel to balance
    paint.drawLine(p2BalVel, QtCore.QPoint(self.ui.frame_ctrlBal.x(), p2BalVel.y())) # from turn to mix
    self.paintArrow(paint, p1MixX, p2BalVel.y(), 0, col)
    # from balance pos to balance vel
    paint.drawLine(QtCore.QPoint(p2BalPos.x(), p2BalPos.y()), QtCore.QPoint(self.ui.frame_ctrlBalVel.x(), p2BalPos.y()))
    self.paintArrow(paint, self.ui.frame_ctrlBalVel.x(), p2BalVel.y(), 0, col)
    #
    # from mission to the right
    p1MisX = self.ui.frame_mission_ctrl.x() + self.ui.frame_mission_ctrl.width()
    paint.drawLine(QtCore.QPoint(p1MisX, p2BalVel.y()), QtCore.QPoint(self.ui.frame_ctrlBalPos.x(), p2BalVel.y()))
    self.paintArrow(paint, self.ui.frame_ctrlBalPos.x(), p2BalPos.y(), 0, col)
    self.paintArrow(paint, self.ui.frame_ctrlBal.x(), p2BalVel.y(), 0)
    paint.drawLine(QtCore.QPoint(p1MisX, p2WallDist.y()), QtCore.QPoint(self.ui.frame_ctrlWallVel.x(), p2WallDist.y()))
    self.paintArrow(paint, self.ui.frame_ctrlWallVel.x(), p2WallDist.y(), 0, col)
    paint.drawLine(QtCore.QPoint(p1MisX, p2WallTurn.y()), QtCore.QPoint(self.ui.frame_ctrlWallTurn.x(), p2WallTurn.y()))
    self.paintArrow(paint, self.ui.frame_ctrlWallTurn.x(), p2WallTurn.y(), 0, col)
    paint.drawLine(QtCore.QPoint(p1MisX, p2Edge.y()), QtCore.QPoint(self.ui.frame_ctrlEdge.x(), p2Edge.y()))
    self.paintArrow(paint, self.ui.frame_ctrlEdge.x(), p2Edge.y(), 0, col)
    paint.drawLine(QtCore.QPoint(p1MisX, p2Pos.y()), QtCore.QPoint(self.ui.frame_ctrlPos.x(), p2Pos.y()))
    self.paintArrow(paint, self.ui.frame_ctrlPos.x(), p2Pos.y(), 0, col)
    paint.drawLine(QtCore.QPoint(p1MisX, p2Turn.y()), QtCore.QPoint(self.ui.frame_turn.x(), p2Turn.y()))
    self.paintArrow(paint, self.ui.frame_turn.x(), p2Turn.y(), 0, col)
    p1MisMixY = int(p2BalVel.y() + p2WallDist.y()/2)
    paint.drawLine(QtCore.QPoint(p1MisX, p1MisMixY), QtCore.QPoint(p1MixX, p1MisMixY))
    self.paintArrow(paint, p1MixX, p1MisMixY, 0, col)
    #
    # up from sensors
    # - to mission
    col = QtCore.Qt.darkRed
    pen.setColor(col)
    paint.setPen(pen)
    p1SenY = self.ui.frame_sensors.y()
    p2MisX = self.ui.frame_mission_ctrl.x() + int(self.ui.frame_mission_ctrl.width()/2)
    p2MisY = self.ui.frame_mission_ctrl.y() + self.ui.frame_mission_ctrl.height()
    paint.drawLine(QtCore.QPoint(p2MisX, p1SenY), QtCore.QPoint(p2MisX, p2MisY))
    self.paintArrow(paint, p2MisX, p2MisY, 1, col)
    # - to position and velocity balance
    p1senY1 = self.ui.frame_sensors.y()
    p2PosX = self.ui.frame_ctrlPos.x() + int(self.ui.frame_ctrlPos.width()/2)
    p2PosY = self.ui.frame_ctrlPos.y() + self.ui.frame_ctrlPos.height()
    paint.drawLine(QtCore.QPoint(p2PosX, p1senY1), QtCore.QPoint(p2PosX, p2PosY))
    self.paintArrow(paint, p2PosX, p2PosY, 1, col)
    # from sensor to balance velocity
    p2BalVelY = self.ui.frame_ctrlBalVel.y() + self.ui.frame_ctrlBalVel.height()
    p2BalVelX = self.ui.frame_ctrlBalVel.x() + int(self.ui.frame_ctrlBalVel.width()/2)
    paint.drawLine(QtCore.QPoint(p2BalVelX, self.ui.frame_ctrlPos.y()), QtCore.QPoint(p2BalVelX, p2BalVelY))
    self.paintArrow(paint, p2BalVelX, p2BalVelY, 1, col)
    # from sensor to balance position
    p2BalPosX = self.ui.frame_ctrlBalPos.x() + int(self.ui.frame_ctrlBalPos.width()/2)
    paint.drawLine(QtCore.QPoint(p2BalPosX, p1SenY), QtCore.QPoint(p2BalPosX, p2BalVelY))
    self.paintArrow(paint, p2BalPosX, p2BalVelY, 1, col)
    # - to other controllers
    p2TurnX = self.ui.frame_turn.x() + int(self.ui.frame_turn.width()/2)
    p2TurnY = self.ui.frame_turn.y() + self.ui.frame_turn.height()
    paint.drawLine(QtCore.QPoint(p2TurnX, p1senY1), QtCore.QPoint(p2TurnX, p2TurnY))
    self.paintArrow(paint, p2TurnX, p2TurnY, 1, col)
    p2EdgeY = self.ui.frame_ctrlEdge.y() + self.ui.frame_ctrlEdge.height()
    paint.drawLine(QtCore.QPoint(p2TurnX, self.ui.frame_turn.y()), QtCore.QPoint(p2TurnX, p2EdgeY))
    self.paintArrow(paint, p2TurnX, p2EdgeY, 1, col)
    p2WallTurnY = self.ui.frame_ctrlWallTurn.y() + self.ui.frame_ctrlWallTurn.height()
    paint.drawLine(QtCore.QPoint(p2TurnX, self.ui.frame_ctrlEdge.y()), QtCore.QPoint(p2TurnX, p2WallTurnY))
    self.paintArrow(paint, p2TurnX, p2WallTurnY, 1, col)
    p2WallDistY = self.ui.frame_ctrlWallVel.y() + self.ui.frame_ctrlWallVel.height()
    paint.drawLine(QtCore.QPoint(p2TurnX, self.ui.frame_ctrlWallTurn.y()), QtCore.QPoint(p2TurnX, p2WallDistY))
    self.paintArrow(paint, p2TurnX, p2WallDistY, 1, col)
    p2balY = self.ui.frame_ctrlBal.y() + self.ui.frame_ctrlBal.height()
    paint.drawLine(QtCore.QPoint(p2TurnX, self.ui.frame_ctrlWallVel.y()), QtCore.QPoint(p2TurnX, p2balY))
    self.paintArrow(paint, p2TurnX, p2balY, 1, col)
    #
    pen.setStyle(QtCore.Qt.DashDotDotLine)
    paint.setPen(pen)
    p1MotorY = self.ui.frame_motors.y() + self.ui.frame_motors.height()
    p1SensorsY = self.ui.frame_sensors.y() + self.ui.frame_sensors.height()
    paint.drawLine(QtCore.QPoint(p1Vel.x(), p1MotorY), QtCore.QPoint(p1Vel.x(), p1SensorsY + 20))
    paint.drawLine(QtCore.QPoint(p1Vel.x(), p1SensorsY + 20), QtCore.QPoint(p2PosX, p1SensorsY + 20))
    paint.drawLine(QtCore.QPoint(p2PosX, p1SensorsY + 20), QtCore.QPoint(p2PosX, p1SensorsY))
    pen.setStyle(QtCore.Qt.SolidLine)
    paint.setPen(pen)
    self.paintArrow(paint, p2PosX, p1SensorsY, 1, col)
    pass




