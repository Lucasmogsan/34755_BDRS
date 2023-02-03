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
from PyQt5 import QtCore, QtGui
import os

class ULog(object):
  #log_allow = True
  log_lms = 0 # mission
  log_lac = 0 # acc
  log_lgy = 0 # gyro
  log_mag = 0 # magnetometer
  log_lvr = 0 # motor velocity reference
  log_lmv = 0 # motor voltage
  log_lma = 0 # motor current
  log_lme = 0 # encoder
  log_lmr = 0 # wheel velocity
  log_ltr = 0 # turn rate
  log_lpo = 0 # pose
  log_line = 0 # line sensor
  log_dist = 0 # distance (IR)
  log_lbt = 0 # battery
  log_lex = 0 # log extra
  log_hirp = 0 # chirp log
  log_ctrl_vel = 0;
  log_ctrl_turn = 0;
  log_ctrl_pos = 0;
  log_ctrl_edge = 0;
  log_ctrl_wall = 0;
  log_ctrl_fwd_dist = 0;
  log_ctrl_bal = 0;
  log_ctrl_bal_vel = 0;
  log_ctrl_bal_pos = 0;
  #log_lbo = 0 # barometer not used
  #log_lbc = 0 # log of balance control details
  #log_lct = 0 # control time in us
  log_lin = 0 # interval
  sampleTime = 0.001 # REGBOT sample time - normally 1ms
  log_lcn = [0,0] # log buffer rows and max rows
  log_set_manually = False
  logFlagRead = False
  logFlagReadS = False
  logFlagReadC = False
  logDataRead = False
  lock = threading.RLock()
  logList = ""
  logData = ""
  lastDataRequestTime = time.time()
  lastTab = ""
  hasFocus = False
  #thisTab = -1
  inEdit = False;
  saveAsSelected = False
  logfileExtraName = "_"

  #
  def __init__(self, parent):
    self.parent = parent
    self.ui = parent.ui
    self.main = parent.main

  def setup(self):
    self.ui.pushButton_log_edit.clicked.connect(self.edit)
    self.ui.pushButton_log_apply.clicked.connect(self.apply)
    self.ui.pushButton_log_apply.setEnabled(False)
    self.ui.log_get.clicked.connect(self.logGet)
    self.ui.log_clear.clicked.connect(self.logClear)
    self.ui.log_save.clicked.connect(self.logSave)
    self.ui.log_get_filename.clicked.connect(self.logSaveAs)
    pass

  def edit(self):
    #print("# in edit")
    self.inEdit = True
    self.ui.pushButton_log_edit.setEnabled(False)
    self.ui.pushButton_log_apply.setEnabled(True)
    pass
  
  def apply(self):
    #print("# apply pressed")
    # lfl mis acc gyro mag motref motv mota enc vel turnr pose line dist batt ex chirp
    self.main.devWrite("lfls {:d} {:d} {:d} 0 {:d} {:d} {:d} {:d} {:d} {:d} {:d} {:d} {:d} {:d} {:d} 0\n".format(
                       self.ui.log_lms.isChecked(),
                       self.ui.log_lac.isChecked(),
                       self.ui.log_lgy.isChecked(),
                       self.ui.log_lvr.isChecked(),
                       self.ui.log_lmv.isChecked(),
                       self.ui.log_lma.isChecked(),
                       self.ui.log_lme.isChecked(),
                       self.ui.log_lmr.isChecked(),
                       self.ui.log_turn_rate.isChecked(),
                       self.ui.log_lpo.isChecked(),
                       self.ui.log_line.isChecked(),
                       self.ui.log_distance.isChecked(),
                       self.ui.log_lbt.isChecked(),
                       self.ui.log_lex.isChecked()
                           ), True)
    # lfc vel turn pos edge wall dist bal balvel balpos
    self.main.devWrite("lfcs {:d} {:d} {:d} {:d} {:d} {:d} {:d} {:d} {:d}\n".format(
                       self.ui.log_ctrl_vel.isChecked(),
                       self.ui.log_ctrl_turn.isChecked(),
                       self.ui.log_ctrl_pos.isChecked(),
                       self.ui.log_ctrl_edge.isChecked(),
                       self.ui.log_ctrl_wall.isChecked(),
                       self.ui.log_ctrl_fwd_dist.isChecked(),
                       self.ui.log_ctrl_bal.isChecked(),
                       self.ui.log_ctrl_bal_vel.isChecked(),
                       self.ui.log_ctrl_bal_pos.isChecked()
                                    ), True)
    self.main.devWrite("lsts {:d}\n".format(self.ui.log_interval.value()), True)
    self.inEdit = False
    self.ui.pushButton_log_edit.setEnabled(True)
    self.ui.pushButton_log_apply.setEnabled(False)
    pass
    

  def timerUpdate(self, timerCnt, justConnected):
    if self.logFlagReadS:
      if not self.inEdit:
        self.ui.log_interval.setValue(self.log_lin)
        self.ui.log_buf_cnt.setText("used " + str(self.log_lcn[0]) + "/" + str(self.log_lcn[1]))
        self.ui.log_buffer_time.setText("Log time " + str(self.log_lin * self.log_lcn[1] * self.sampleTime) + " sec")
      self.logFlagReadS = False
    if self.logFlagRead:
      #if not self.log_set_manually:
      if not self.inEdit:
        self.lock.acquire()
        self.ui.log_lms.setChecked(self.log_lms)
        self.ui.log_lac.setChecked(self.log_lac)
        self.ui.log_lgy.setChecked(self.log_lgy)
        self.ui.log_lma.setChecked(self.log_lma)
        self.ui.log_lvr.setChecked(self.log_lvr)
        self.ui.log_lmv.setChecked(self.log_lmv)
        self.ui.log_lmr.setChecked(self.log_lmr)
        self.ui.log_lme.setChecked(self.log_lme)
        self.ui.log_turn_rate.setChecked(self.log_ltr)
        self.ui.log_lpo.setChecked(self.log_lpo)
        self.ui.log_line.setChecked(self.log_line)
        self.ui.log_distance.setChecked(self.log_dist)
        self.ui.log_lbt.setChecked(self.log_lbt)
        #self.ui.log_lbc.setChecked(self.log_lbc)
        #self.ui.log_lct.setChecked(self.log_lct)
        self.ui.log_lex.setChecked(self.log_lex)
        self.lock.release()
      self.logFlagRead = False
    if self.logFlagReadC:
      #
      if not self.inEdit:
        self.ui.log_ctrl_vel.setChecked(self.log_ctrl_vel)
        self.ui.log_ctrl_turn.setChecked(self.log_ctrl_turn)
        self.ui.log_ctrl_pos.setChecked(self.log_ctrl_pos)
        self.ui.log_ctrl_edge.setChecked(self.log_ctrl_edge)
        self.ui.log_ctrl_wall.setChecked(self.log_ctrl_wall)
        self.ui.log_ctrl_fwd_dist.setChecked(self.log_ctrl_fwd_dist)
        self.ui.log_ctrl_bal.setChecked(self.log_ctrl_bal)
        self.ui.log_ctrl_bal_vel.setChecked(self.log_ctrl_bal_vel)
        self.ui.log_ctrl_bal_pos.setChecked(self.log_ctrl_bal_pos)
      #
      #self.ui.log_allow.setChecked(self.log_allow)
      #self.ui.log_lbo.setChecked(self.log_lbo)
      self.logFlagReadC = False
    if (self.logDataRead):
      self.lock.acquire()
      self.ui.log_view.setText(self.logList + self.logData)
      self.logDataRead = False
      self.lock.release()
    # request update at times - if changed by another client
    thisTab = self.ui.tabPages.indexOf(self.ui.tab_log)
    if (self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() != thisTab:
      # just switched away from this tab
      self.hasFocus = False
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        self.main.devWrite(":lfl subscribe 0\n") # log flags
        self.main.devWrite(":lfc subscribe 0\n") # controller log flags
        self.main.devWrite(":lst subscribe 0\n") # log interval and entry count
        self.main.devWrite(":logdata subscribe 0\n") # log interval and entry count
        self.main.devWrite("sub lfl 0\n", True) # log flags
        self.main.devWrite("sub lfc 0\n", True) # control log flags
        self.main.devWrite("sub lst 0\n", True) # log status
      else:
        # talking to Teensy directly, so subscribe here
        self.main.devWrite("sub lfl 0\n") # log flags
        self.main.devWrite("sub lfc 0\n") # control log flags
        self.main.devWrite("sub lst 0\n") # log status
        pass
      pass
    if (not self.hasFocus or justConnected) and self.ui.tabPages.currentIndex() == thisTab:
      # just entering this tab
      self.hasFocus = True
      # if we are talking to a bridge - then just subscribe
      if self.main.isBridge():
        self.main.devWrite(":lfl subscribe -1\n") # log flags
        self.main.devWrite(":lcl subscribe -1\n") # controller log flags
        self.main.devWrite(":lst subscribe -1\n") # log interval and entry    
        self.main.devWrite(":logdata subscribe -1\n") # logger data lines
        self.main.devWrite("sub lfl 320\n", True) # log flags
        self.main.devWrite("sub lfc 310\n", True) # control log flags
        self.main.devWrite("sub lst 300\n", True) # log status
      else:
        # talking to Teensy directly, so subscribe here
        self.main.devWrite("sub lfl 320\n") # log flags
        self.main.devWrite("sub lfc 310\n") # control log flags
        self.main.devWrite("sub lst 300\n") # log status
      pass
    pass

  #
  def decode(self, gg, line):
    dataUsed = True
    self.lock.acquire()
    try:
      if (gg[0] == 'lfl'):
        try:
          self.log_lms = int(gg[1],0) # mission
          self.log_lac = int(gg[2],0) # acc
          self.log_lgy = int(gg[3],0) # gyro
          self.log_mag = int(gg[4],0) # magnetometer
          self.log_lvr = int(gg[5],0) # motor velocity reference
          self.log_lmv = int(gg[6],0) # motor voltage
          self.log_lma = int(gg[7],0) # motor current
          self.log_lme = int(gg[8],0) # encoder
          self.log_lmr = int(gg[9],0) # wheel velocity
          self.log_ltr = int(gg[10],0) # turn rate
          self.log_lpo = int(gg[11],0) # pose
          self.log_line = int(gg[12],0) # line sensor
          self.log_dist = int(gg[13],0) # distance (IR)
          self.log_lbt = int(gg[14],0) # battery
          self.log_lex = int(gg[15],0) # log extra
          self.log_hirp = int(gg[16],0) # chirp log
          self.logFlagRead = True
        except:
          print("Log decode - too few parameters")
        # make part of log filename to reflect log content
        lnx = "_"
        if self.log_lms:
          lnx += "m"
        if self.log_lac:
          lnx += "a"
        if self.log_lgy:
          lnx += "g"
        if self.log_mag:
          lnx += "m"
        if self.log_lvr:
          lnx += "r"
        if self.log_lmv:
          lnx += "v"
        if self.log_lma:
          lnx += "a"
        if self.log_lme:
          lnx += "e"
        if self.log_lmr:
          lnx += "w"
        if self.log_ltr:
          lnx += "t"
        if self.log_lpo:
          lnx += "p"
        if self.log_line:
          lnx += "l"
        if self.log_dist:
          lnx += "d"
        if self.log_lbt:
          lnx += "b"
        if self.log_lex:
          lnx += "x"
        if self.log_hirp:
          lnx += "c"
        if (lnx + "_") != self.logfileExtraName and not self.saveAsSelected:
          self.logfileExtraName = lnx + "_"
          #print("# log filename is " + self.logfileExtraName)
          self.setSaveFilename()
      # else one of the log-flags
      elif gg[0] == "lfc":
        try:
          self.log_ctrl_vel = int(gg[1],0)
          self.log_ctrl_turn = int(gg[2],0)
          self.log_ctrl_pos = int(gg[3],0)
          self.log_ctrl_edge = int(gg[4],0)
          self.log_ctrl_wall = int(gg[5],0)
          self.log_ctrl_fwd_dist = int(gg[6],0)
          self.log_ctrl_bal = int(gg[7],0)
          self.log_ctrl_bal_vel = int(gg[8],0)
          self.log_ctrl_bal_pos = int(gg[9],0)
          self.logFlagReadC = True
        except:
          print("Log decode - too few ctrl log parameters")
        # this should be the last, so time to update
        self.logFlagRead = True
      elif gg[0] == "lst":
        self.log_lin = int(gg[1],0) # log interval
        self.log_lcn[0] = int(gg[2], 0) # used log entries
        self.log_lcn[1] = int(gg[3], 0) # total log entries
        self.logFlagReadS = True
      elif gg[0] == "logdata":
        # from bridge
        self.logList += line[8:]
        self.logDataRead = True
      elif (gg[0][0] == '%'):
        # loglist direct from Teensy - explain line
        self.logList += line
        self.logDataRead = True
      elif ((gg[0][0] >= '0' and gg[0][0] <= '9') or gg[0][0] == '.'):
        # loglist direct from Teensy - data line
        self.logData += line
        self.logDataRead = True
      else:
        dataUsed = False
    except:
      print("ULog: data read error - skipped a " + gg[0] + " from " + line)
      pass
    self.lock.release()
    return dataUsed
  
  def logSaveAs(self):
    print("# save as ...")
    filename = QtGui.QFileDialog.getSaveFileName(self.parent,'logfile to use', os.getcwd(), 'log *.txt (*.txt)')
    if (filename and len(filename) > 0 and len(filename[0]) > 0):
      print("saving log to '" + filename[0] + "'")
      self.ui.log_filename.setText(filename[0])
      self.logSave()
      saveAsSelected = True
    pass
    #print("# logSaveAs filename '" + filename + "' length " + str(len(filename)))

  def setSaveFilename(self):
    # get current directory
    pwd = os.getcwd()
    npn = "{:03d}_".format(self.main.deviceID) + self.main.name
    # create subdirectory with robot name
    if not os.path.exists(os.path.join(pwd, npn)):
      os.mkdir(npn)
    nfn = "log" + self.logfileExtraName + "00.txt"
    #print("# log filename " + nfn)
    self.ui.log_filename.setText(os.path.join(pwd, npn, nfn))
    # mark as auto generated
    self.saveAsSelected = False
    pass
      
  def logSave(self):
    # do we have a log-name already
    fnl = len(self.ui.log_filename.text())
    if (fnl == 0):
      print("# no log filename -> auto name")
      self.saveAsSelected = False
      self.setSaveFilename()
    # save
    if True: #try:
      f = open(self.ui.log_filename.text(), "w")
      if (self.ui.log_save_header.isChecked()):
        f.write('%% logfile from REGBOT:\n')
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
    else: #except:
      self.ui.statusbar.showMessage("Failed to open file " + self.ui.log_filename.text() + "!", 3000)
    pass
      
  def logGet(self):
    self.main.devWrite("log\n", True)
    
  def logClear(self):
    self.logData = ""
    self.logList = ""
    self.logDataRead = True

