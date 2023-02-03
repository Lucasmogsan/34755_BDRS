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

class UServo(object):
  servoUse = [False, False, False, False, False]
  servoVal = [0,0,0,0,0]
  servoVel = [0,0,0,0,0]
  steerScale = 90  # angle change from 1ms to 2 ms
  set_manually = False
  dataRead = False # for all servos
  lock = threading.RLock()
  lastDataRequestTime = time.time()
  lastDataSetTime = time.time()
  servoEnableSend = [0, -1, -1, -1, -1, -1]
  # used when "use" is checked
  servoUseByGUI = [False, False, False, False, False]
  servoChanged =  [0,0,0,0,0]
  inEdit = False
  lastDataRequest = 0
  inFastUpdate = False
  lastTab = ""
  inTimerUpdate = True
  gotFirstData = False
  hasFocus = False
  thisTab = -1
  #
  def __init__(self, parent):
    self.main = parent.main
    self.ui = parent.ui

  def setup(self):
    # variable inFastUpdate needs to be set to False after 
    # slider movement 
    self.ui.horizontalSlider_servo1.valueChanged.connect(self.servo1bar)
    self.ui.horizontalSlider_servo2.valueChanged.connect(self.servo2bar)
    self.ui.horizontalSlider_servo3.valueChanged.connect(self.servo3bar)
    self.ui.horizontalSlider_servo4.valueChanged.connect(self.servo4bar)
    self.ui.horizontalSlider_servo5.valueChanged.connect(self.servo5bar)
    #
    self.ui.checkBox_servo1_use.toggled.connect(self.servo1use)
    self.ui.checkBox_servo2_use.toggled.connect(self.servo2use)
    self.ui.checkBox_servo3_use.toggled.connect(self.servo3use)
    self.ui.checkBox_servo4_use.toggled.connect(self.servo4use)
    self.ui.checkBox_servo5_use.toggled.connect(self.servo5use)
    pass

  def timerUpdate(self, timerCnt, justConnected):
    self.lock.acquire()
    self.inTimerUpdate = True
    if self.dataRead:
      if not self.servoUseByGUI[0]:
        self.ui.checkBox_servo1.setChecked(self.servoUse[0])
        self.ui.servo_value_1.setValue(self.servoVal[0])
        self.ui.horizontalSlider_servo1.setValue(self.servoVal[0])
        self.ui.servo1_vel.setValue(self.servoVel[0])
      if not self.servoUseByGUI[1]:
        self.ui.checkBox_servo2.setChecked(self.servoUse[1])
        self.ui.servo_value_2.setValue(self.servoVal[1])
        self.ui.horizontalSlider_servo2.setValue(self.servoVal[1])
        self.ui.servo2_vel.setValue(self.servoVel[1])
      if not self.servoUseByGUI[2]:
        self.ui.checkBox_servo3.setChecked(self.servoUse[2])
        self.ui.servo_value_3.setValue(self.servoVal[2])
        self.ui.horizontalSlider_servo3.setValue(self.servoVal[2])
        self.ui.servo3_vel.setValue(self.servoVel[2])
      if not self.servoUseByGUI[3]:
        self.ui.checkBox_servo4.setChecked(self.servoUse[3])
        self.ui.servo_value_4.setValue(self.servoVal[3])
        self.ui.horizontalSlider_servo4.setValue(self.servoVal[3])
      if not self.servoUseByGUI[4]:
        self.ui.checkBox_servo5.setChecked(self.servoUse[4])
        self.ui.servo_value_5.setValue(self.servoVal[4])
        self.ui.horizontalSlider_servo5.setValue(self.servoVal[4])
      self.dataRead = False
    self.lock.release()
    # send servo change - but not too fast 
    thisTab = self.ui.tabPages.indexOf(self.ui.tab_servo)
    if self.ui.tabPages.currentIndex() == thisTab:
      if True: # time.time() - self.lastDataSetTime > 0.1:
        use = [self.ui.checkBox_servo1_use.isChecked(),
              self.ui.checkBox_servo2_use.isChecked(),
              self.ui.checkBox_servo3_use.isChecked(),
              self.ui.checkBox_servo4_use.isChecked(),
              self.ui.checkBox_servo5_use.isChecked()]
        ena = [self.ui.checkBox_servo1.isChecked(),
              self.ui.checkBox_servo2.isChecked(),
              self.ui.checkBox_servo3.isChecked(),
              self.ui.checkBox_servo4.isChecked(),
              self.ui.checkBox_servo5.isChecked()]
        val = [self.ui.servo_value_1.value(),
              self.ui.servo_value_2.value(),
              self.ui.servo_value_3.value(),
              self.ui.servo_value_4.value(),
              self.ui.servo_value_5.value()]
        vel = [self.ui.servo1_vel.value(),
              self.ui.servo2_vel.value(),
              self.ui.servo3_vel.value(),
              self.ui.servo4_vel.value(),
              self.ui.servo5_vel.value()]
        #print("servo update " + str(ena) + " val " + str(val))
        for i in range(5):
          if use[i]:
            self.servoUseByGUI[i] = True;
            if self.servoEnableSend[i] != ena[i]:
              if not ena[i]:
                s = "servo {} 10000 0\n".format(i+1)
                self.main.devWrite(s)
              self.servoEnableSend[i] = ena[i]
            if ena[i] and self.servoChanged[i]:
              s = "servo {} {} {}\n".format(i+1, int(val[i]), int(vel[i]))
              self.main.devWrite(s)
              self.servoChanged[i] = False
        # save check time
        self.lastDataSetTime = time.time()
    #
    if (self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() != thisTab:
      # just switched away from this tab
      self.hasFocus = False
      # if we are talking to a bridge - then just un-subscribe
      if self.main.isBridge():
        # NB! not the right messages
        self.main.devWrite(":svo subscribe 0\n") # gyro
      else:
        # talking to Teensy directly, so un-subscribe here
        self.main.devWrite("sub svo 0\n") # acc subscribe
        pass
      pass
    if (not self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() == thisTab:
      # just entering this tab
      self.hasFocus = True
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        self.main.devWrite(":svo subscribe -1\n")
      else:
        # talking to Teensy directly, so subscribe here
        self.main.devWrite("sub svo 50\n") # gyro subscribe
      pass
    pass
    if self.gotFirstData:
      # keep flag, until we got data from robot,
      # else we may send default data to robot
      self.inTimerUpdate = False;

  
  def setSingleServo(self, idx, enable, value):
    self.servoVal[idx] = value
    self.servoChanged[idx] = True
    self.inFastUpdate = False
  #
  def decode(self, gg):
    dataUsed = True
    self.lock.acquire()
    try:
      if gg[0] == "svo":
        if len(gg) > 12:
          # new version with velocity
          self.servoUse[0] = int(gg[1],0)
          self.servoVal[0] = int(gg[2],0)
          self.servoVel[0] = int(gg[3],0)
          #
          self.servoUse[1] = int(gg[4],0)
          self.servoVal[1] = int(gg[5],0)
          self.servoVel[1] = int(gg[6],0)
          #
          self.servoUse[2] = int(gg[7],0)
          self.servoVal[2] = int(gg[8],0)
          self.servoVel[2] = int(gg[9],0)
          #
          self.servoUse[3] = int(gg[10],0)
          self.servoVal[3] = int(gg[11],0)
          self.servoVel[3] = int(gg[12],0)
          #
          self.servoUse[4] = int(gg[13],0)
          self.servoVal[4] = int(gg[14],0)
          self.servoVel[4] = int(gg[15],0)
          self.dataRead = True
        elif len(gg) > 5:
          print("Failed svo message too short {} values!".format(str(len(gg))))
      else:
        dataUsed = False
        #self.dataRead = True
    except:
      print("UServo: data read error - skipped a " + gg[0])
      pass
    self.lock.release()
    return dataUsed
  def logSave(self):
    try:
      f = open(self.ui.log_filename.text(), "w")
      if (self.ui.log_save_header.isChecked()):
        f.write('%% logfile from robot ' + self.ui.robot_id_main.text() + '\n')
        f.write(self.logList)
      if (self.ui.log_save_config.isChecked()):
        f.write('%% Configuration as seen in client (assumed updated from robot)\n')
        fil = open('.regbotConfigTemp.ini', 'r');
        for line in fil:
          f.write('% ' + line)
        fil.close()
        f.write('%% data log\n')
      f.write(self.logData)
      f.close()
    except:
      self.ui.statusbar.showMessage("Failed to open file " + self.ui.log_filename.text() + "!", 3000)
  def logClear(self):
    self.logData = ""
    self.logList = ""
    self.logDataRead = True

  def servo1bar(self):
    if not self.inFastUpdate:
      self.inFastUpdate = True
      self.ui.servo_value_1.setValue(self.ui.horizontalSlider_servo1.value())
      self.setSingleServo(0, self.ui.checkBox_servo1.isChecked(), self.ui.horizontalSlider_servo1.value())
    pass      

  def servo2bar(self):
    #print(" - servo 2 slider fast=" + str(self.inFastUpdate) + ", timer:" + str(self.inTimerUpdate))
    if not self.inFastUpdate:
      self.inFastUpdate = True
      self.ui.servo_value_2.setValue(self.ui.horizontalSlider_servo2.value())
      self.setSingleServo(1, self.ui.checkBox_servo2.isChecked(), self.ui.horizontalSlider_servo2.value())
    pass      

  def servo3bar(self):
    #print(" - servo 3a slider")
    if not self.inFastUpdate:
      self.inFastUpdate = True
      self.ui.servo_value_3.setValue(self.ui.horizontalSlider_servo3.value())
      self.setSingleServo(2, self.ui.checkBox_servo3.isChecked(), self.ui.horizontalSlider_servo3.value())
      #print(" - servo 3 slider")
    pass      

  def servo4bar(self):
    if not self.inFastUpdate:
      self.inFastUpdate = True
      self.ui.servo_value_4.setValue(self.ui.horizontalSlider_servo4.value())
      self.setSingleServo(3, self.ui.checkBox_servo4.isChecked(), self.ui.horizontalSlider_servo4.value())
    pass      

  def servo5bar(self):
    if not self.inFastUpdate:
      self.inFastUpdate = True
      self.ui.servo_value_5.setValue(self.ui.horizontalSlider_servo5.value())
      self.setSingleServo(4, self.ui.checkBox_servo5.isChecked(), self.ui.horizontalSlider_servo5.value())
    pass      

  def servo1num(self):
    if not self.inFastUpdate:
      self.inFastUpdate = True
      self.ui.horizontalSlider_servo1.setValue(self.ui.servo_value_1.value())
      self.setSingleServo(0, self.ui.checkBox_servo1.isChecked(), self.ui.horizontalSlider_servo1.value())
    pass      

  def servo2num(self):
    #print(" - servo 2 num: fast=" + str(self.inFastUpdate) + ", timer:" + str(self.inTimerUpdate))
    if not self.inFastUpdate:
      self.inFastUpdate = True
      self.ui.horizontalSlider_servo2.setValue(self.ui.servo_value_2.value())
      self.setSingleServo(1, self.ui.checkBox_servo2.isChecked(), self.ui.horizontalSlider_servo2.value())
    pass      
      
  def servo3num(self):
    if not self.inFastUpdate:
      self.inFastUpdate = True
      self.ui.horizontalSlider_servo3.setValue(self.ui.servo_value_3.value())
      self.setSingleServo(2, self.ui.checkBox_servo3.isChecked(), self.ui.horizontalSlider_servo3.value())
    pass

  def servo4num(self):
    if not self.inFastUpdate:
      self.inFastUpdate = True
      self.ui.horizontalSlider_servo4.setValue(self.ui.servo_value_4.value())
      self.setSingleServo(3, self.ui.checkBox_servo4.isChecked(), self.ui.horizontalSlider_servo4.value())
    pass      

  def servo5num(self):
    if not self.inFastUpdate:
      self.inFastUpdate = True
      self.ui.horizontalSlider_servo5.setValue(self.ui.servo_value_5.value())
      self.setSingleServo(4, self.ui.checkBox_servo5.isChecked(), self.ui.horizontalSlider_servo5.value())
    pass      
  
  def servo1use(self):
    self.servoUseByGUI[0] = self.ui.checkBox_servo1_use.isChecked()
    self.ui.checkBox_servo1.setEnabled(self.servoUseByGUI[0])
    self.ui.servo_value_1.setEnabled(self.servoUseByGUI[0])
    self.ui.horizontalSlider_servo1.setEnabled(self.servoUseByGUI[0])
    self.ui.servo1_vel.setEnabled(self.servoUseByGUI[0])
    pass      

  def servo2use(self):
    self.servoUseByGUI[1] = self.ui.checkBox_servo2_use.isChecked()
    self.ui.checkBox_servo2.setEnabled(self.servoUseByGUI[1])
    self.ui.servo_value_2.setEnabled(self.servoUseByGUI[1])
    self.ui.horizontalSlider_servo2.setEnabled(self.servoUseByGUI[1])
    self.ui.servo2_vel.setEnabled(self.servoUseByGUI[1])
    pass      

  def servo3use(self):
    self.servoUseByGUI[2] = self.ui.checkBox_servo3_use.isChecked()
    self.ui.checkBox_servo3.setEnabled(self.servoUseByGUI[2])
    self.ui.servo_value_3.setEnabled(self.servoUseByGUI[2])
    self.ui.horizontalSlider_servo3.setEnabled(self.servoUseByGUI[2])
    self.ui.servo3_vel.setEnabled(self.servoUseByGUI[2])
    pass      

  def servo4use(self):
    self.servoUseByGUI[3] = self.ui.checkBox_servo4_use.isChecked()
    self.ui.checkBox_servo4.setEnabled(self.servoUseByGUI[3])
    self.ui.servo_value_4.setEnabled(self.servoUseByGUI[3])
    self.ui.horizontalSlider_servo4.setEnabled(self.servoUseByGUI[3])
    self.ui.servo4_vel.setEnabled(self.servoUseByGUI[3])
    pass      

  def servo5use(self):
    self.servoUseByGUI[4] = self.ui.checkBox_servo5_use.isChecked()
    self.ui.checkBox_servo5.setEnabled(self.servoUseByGUI[4])
    self.ui.servo_value_5.setEnabled(self.servoUseByGUI[4])
    self.ui.horizontalSlider_servo5.setEnabled(self.servoUseByGUI[4])
    self.ui.servo5_vel.setEnabled(self.servoUseByGUI[4])
    pass      

