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


from pyqtgraph.Qt import QtGui #, QtCore
try:
  import configparser as cp
except:
  import ConfigParser  as cp

from dialog_control import Ui_dialog_control_edit
from upaintsupport import *
from time import sleep



# ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////

class UControlPZ(object):
  use = False
  tauNum = 0.0
  tauDen = 0.0
  def setTauNumTauDen(self, doUse, tNum, tDen):
    self.use = doUse
    self.tauNum = tNum
    self.tauDen = tDen
  def setLead(self, doUse, tau, alpha):
    self.use = doUse
    self.tauNum = tau
    self.tauDen = tau * alpha
  def setPole(self, doUse, tauPole):
    self.use = doUse
    self.tauNum = 0
    self.tauDen = tauPole
  def asString(self):
    return " {:d} {:g} {:g}".format(self.use, self.tauNum, self.tauDen)
  def fromString(self, gg1, gg2, gg3):
    #print("# control PZ")
    self.use = int(gg1, 0)
    self.tauNum = float(gg2)
    self.tauDen = float(gg3)
    #print("# control PZ ud")
  pass

# ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////

class UControlI(object):
  use = False
  tau = 0.0
  limit = 0.0
  andZero = True
  def setIntegrator(self, doUse, taui, ilimit, zero):
    self.use = doUse
    self.tau = taui
    self.limit = ilimit
    # if andZero is false, then the input is not added to output of integrator
    self.andZero = zero
  #def setIntegratorKi(self, doUse, ki, ilimit):
    #self.use = doUse
    #if ki > 0.0:
      #self.tau = 1/ki
    #self.limit = ilimit
  def asString(self):
    return " {:d} {:g} {:g} {:d}".format(self.use, self.tau, self.limit, self.andZero)
  def fromString(self, gg1, gg2, gg3, gg4):
    #print("# control I " + gg1 + " " + gg2 + " " + gg3 + " " + gg4)
    self.use = int(gg1, 0)
    self.tau = float(gg2)
    self.limit = float(gg3)
    self.andZero = int(gg4, 0)

# ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////

# class that combines QT-design setup and a GUI-dialog class
class FormControllerEdit(QtWidgets.QDialog, Ui_dialog_control_edit):
  def __init__(self, parent):
    QtWidgets.QWidget.__init__(self, parent)
    self.setupUi(self)
    self.checkBox_integrator_use.clicked.connect(self.check_integrator)
    self.checkBox_lead_fwd.setChecked(True)
    self.checkBox_lead_fwd.clicked.connect(self.check_lead_fwd)
    self.checkBox_lead_back.clicked.connect(self.check_lead_back)
    self.checkBox_pre_use.clicked.connect(self.check_pre_use)
    self.checkBox_post_integrator.clicked.connect(self.check_post_integrator_use)
    self.checkBox_ff_use.clicked.connect(self.check_ff_use)
    self.checkBox_ff_pole_zero.clicked.connect(self.check_ff_pole_zero)
    self.checkBox_out_limit.clicked.connect(self.check_out_limit)
    #self.buttonBox.button(QtWidgetsQDialogButtonBox.Retry).clicked.connect(self.button_retry_clicked)
  def check_integrator(self):
    checked = self.checkBox_integrator_use.isChecked()
    self.edit_tau_i.setVisible(checked)
    self.label_tau_i.setVisible(checked)
    self.label_integrator_limit.setVisible(checked)
    self.edit_integrator_limit.setVisible(checked)
  def check_lead_fwd(self):
    self.label_tau_zero_fwd.setVisible(self.checkBox_lead_fwd.isChecked())
    self.label_tau_pole_fwd.setVisible(self.checkBox_lead_fwd.isChecked())
    self.edit_tau_pole_fwd.setVisible(self.checkBox_lead_fwd.isChecked())
    self.edit_tau_zero_fwd.setVisible(self.checkBox_lead_fwd.isChecked())
  def check_lead_back(self):
    self.label_tau_zero_back.setVisible(self.checkBox_lead_back.isChecked())
    self.label_tau_pole_back.setVisible(self.checkBox_lead_back.isChecked())
    self.edit_tau_pole_back.setVisible(self.checkBox_lead_back.isChecked())
    self.edit_tau_zero_back.setVisible(self.checkBox_lead_back.isChecked())
  def check_pre_use(self):
    checked = self.checkBox_pre_use.isChecked()
    self.label_pre_tau_pole.setVisible(checked)
    self.label_pre_tau_zero.setVisible(checked)
    self.edit_pre_tau_zero.setVisible(checked)
    self.edit_pre_tau_pole.setVisible(checked) 
    #self.frame_pre_integrator.setVisible(checked)
  def check_post_integrator_use(self):
    checked = self.checkBox_post_integrator.isChecked()
    self.checkBox_post_integrator_zero.setVisible(checked)
    self.label_post_tau_i.setVisible(checked)
    self.label_post_limit.setVisible(checked)
    self.edit_post_tau_i.setVisible(checked)
    self.edit_post_integrator_limit.setVisible(checked)
  def check_ff_use(self):
    checked = self.checkBox_ff_use.isChecked()
    self.label_ff_kp.setVisible(checked)
    self.edit_ff_kp.setVisible(checked)
    self.frame_ff_pole_zero.setVisible(checked)
  def check_ff_pole_zero(self):
    checked = self.checkBox_ff_pole_zero.isChecked()
    self.label_ff_tau_zero.setVisible(checked)
    self.label_ff_tau_pole.setVisible(checked)
    self.edit_ff_tau_pole.setVisible(checked)
    self.edit_ff_tau_zero.setVisible(checked)
  def check_out_limit(self):
    checked = self.checkBox_out_limit.isChecked()
    self.label_out_limit.setVisible(checked)
    self.edit_output_limit.setVisible(checked)

# ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////
# ///////////////////////////////////////////////////

class UControlUnit(object):
  ctrlID = ""
  use = False
  Kp = 0.0
  ffUse = False
  ffKp = 1.0
  outLimit = 10.0
  outLimitUse = True
  needUpdate = False
  gotDataFromRobot = False
  #
  #lock = threading.RLock()
  #editDlg = None
  pass
  def __init__(self, id, parent, name):
    self.ctrlID = id
    self.parent = parent
    self.name = name
    self.main = parent.main
    self.ui = parent.ui
    self.setup()
  
  def setup(self):
    print("## setup " + self.name)
    self.editDlg = FormControllerEdit(self.parent)
    self.integrator = UControlI()
    self.leadFwd = UControlPZ()
    self.leadBack = UControlPZ()
    self.preFilt = UControlPZ()
    self.postFiltI = UControlI()
    self.ffFilt = UControlPZ()
    self.editDlg.pushButton_ctrlRetry.clicked.connect(self.button_retry_clicked)
    self.editDlg.pushButton_ctrlIniLoad.clicked.connect(self.button_ini_load_clicked)
    self.editDlg.buttonBox.button(QtWidgets.QDialogButtonBox.Apply).clicked.connect(self.configuration_to_robot)
    self.editDlg.buttonBox.button(QtWidgets.QDialogButtonBox.Cancel).clicked.connect(self.buttonCancelClicked)
    self.editDlg.buttonBox.button(QtWidgets.QDialogButtonBox.Ok).clicked.connect(self.buttonOkClicked)
    #
    # create list from regbot.py
    #self.ctrlVelocity = UControlUnit("cvel", self, "Wheel Velocity (left and right)")
    #self.ctrlTurn = UControlUnit("ctrn", self, "Heading")
    #self.ctrlWallVel = UControlUnit("cwve", self, "IR forward distance")
    #self.ctrlWallTurn = UControlUnit("cwth", self, "Wall distance")
    #self.ctrlPos = UControlUnit("cpos", self, "Position (drive distance)")
    #self.ctrlEdge = UControlUnit("cedg", self, "Line edge")
    #self.ctrlBalance = UControlUnit("cbal", self, "Balance")
    #self.ctrlBalVel = UControlUnit("cbav", self, "Balance velocity")
    #self.ctrlBalPos = UControlUnit("cbap", self, "Balance position")
    #
    if self.ctrlID == "cvel":
      self.ui.pushButton_ctrl_vel.clicked.connect(self.editControlValues)
    elif self.ctrlID == "ctrn":
      self.ui.pushButton_ctrlTurn.clicked.connect(self.editControlValues)
    elif self.ctrlID == "cwve":
      self.ui.pushButton_ctrlWallVel.clicked.connect(self.editControlValues)
    elif self.ctrlID == "cwth":
      self.ui.pushButton_ctrlWallTurn.clicked.connect(self.editControlValues)
    elif self.ctrlID == "cpos":
      self.ui.pushButton_ctrlPos.clicked.connect(self.editControlValues)
    elif self.ctrlID == "cedg":
      self.ui.pushButton_ctrlEdge.clicked.connect(self.editControlValues)
    elif self.ctrlID == "cbal":
      self.ui.pushButton_ctrlBal.clicked.connect(self.editControlValues)
    elif self.ctrlID == "cbav":
      self.ui.pushButton_ctrlBalVel.clicked.connect(self.editControlValues)
    elif self.ctrlID == "cbap":
      self.ui.pushButton_ctrlBalPos.clicked.connect(self.editControlValues)
    # paint lines and arrows on tab
    self.ctrl_paint_space = ControlPaintSpace(self.ui.frame_ctrl, self.main)
    self.ctrl_paint_space.setGeometry(QtCore.QRect(0, 0, 1600, 1600))
    self.ctrl_paint_space.setFocusPolicy(QtCore.Qt.NoFocus)
    self.ctrl_paint_space.lower()


  
  def decode(self, gg):
    dataUsed = True
    if (gg[0] == self.ctrlID):
      self.fromString(gg)
    else:
      dataUsed = False
    return dataUsed
  
  def buttonCancelClicked(self):
    # update dialog fields from memory
    self.needUpdate = True
    pass
  
  def buttonOkClicked(self):
    self.setFromDialog()
    self.configuration_to_robot()
    pass

  def button_retry_clicked(self):
    self.main.devWrite(self.ctrlID + "i\n")
    print("Send: " + self.ctrlID + "i\n")
    #self.fillDialog()
    #print("#filled dialog")
    pass

  def button_ini_load_clicked(self):
    config = cp.SafeConfigParser()
    config.read(self.main.iniFilename)
    self.loadFromIniFile(config)
    pass

  def asString(self):
    return (self.ctrlID + " {:d} {:g}".format(self.use, self.Kp) + self.integrator.asString() + self.leadFwd.asString() +
        self.leadBack.asString() + self.preFilt.asString() + self.postFiltI.asString() +
        " {:d} {:g}".format(self.ffUse, self.ffKp) + self.ffFilt.asString() + 
        " {:d} {:g}".format(self.outLimitUse, self.outLimit))
  
  
  def fromString(self, gg):
    isOK = gg[0] == self.ctrlID and len(gg) > 23
    if isOK:
      #self.lock.acquire()
      try:
        self.use = gg[1] != '0'
        self.Kp = float(gg[2])
        self.integrator.fromString(gg[3], gg[4], gg[5], gg[6])
        self.leadFwd.fromString(gg[7], gg[8], gg[9])
        self.leadBack.fromString(gg[10], gg[11], gg[12])
        self.preFilt.fromString(gg[13], gg[14], gg[15])
        self.postFiltI.fromString(gg[16], gg[17], gg[18], gg[19])
        self.ffUse = gg[20] != '0'
        self.ffKp = float(gg[21])
        self.ffFilt.fromString(gg[22], gg[23], gg[24])
        self.outLimitUse = gg[25] != '0'
        self.outLimit = float(gg[26])
        #print("+ ff use in " + str(self.ffUse) + " <- '" + gg[20] + "'")
        self.gotDataFromRobot = True
      except:
        print("# control " + self.ctrlID + " failed data read from regbot, len={:d} expected 26".format(len(gg)))
      #print("fill dialog:" + self.name)
      #self.fillDialog()
      print("Got fresh data from robot ...")
      #self.lock.release()
      self.needUpdate = True
    return isOK

  def timerUpdate(self, timerCnt, justConnected):
    #print("-- timer update")
    if self.needUpdate:
      #print("---- timer update 1")
      self.fillDialog()
      self.needUpdate = False
    if justConnected:
      self.gotDataFromRobot = False
  
  def fillDialog(self):
    try:
      #print("fill in dialog")
      self.editDlg.label_ID_lead.setText("Controller parameters (" + self.ctrlID + ")")
      self.editDlg.label_ID.setText(self.name )
      self.editDlg.ctrl_main_use.setChecked(self.use)
      self.editDlg.edit_kp.setValue(self.Kp)
      # i-term
      self.editDlg.checkBox_integrator_use.setChecked(self.integrator.use)
      self.editDlg.edit_tau_i.setValue(self.integrator.tau)
      self.editDlg.edit_integrator_limit.setValue(self.integrator.limit)
      self.editDlg.check_integrator()
      # lead forward pole-zero term
      self.editDlg.checkBox_lead_fwd.setChecked(self.leadFwd.use)
      self.editDlg.edit_tau_zero_fwd.setValue(self.leadFwd.tauNum)
      self.editDlg.edit_tau_pole_fwd.setValue(self.leadFwd.tauDen)
      self.editDlg.check_lead_fwd()
      # lead backward pole-zero term
      self.editDlg.checkBox_lead_back.setChecked(self.leadBack.use)
      self.editDlg.edit_tau_zero_back.setValue(self.leadBack.tauNum)
      self.editDlg.edit_tau_pole_back.setValue(self.leadBack.tauDen)
      self.editDlg.check_lead_back()
      # prefilter pole-zero term
      self.editDlg.checkBox_pre_use.setChecked(self.preFilt.use)
      self.editDlg.edit_pre_tau_zero.setValue(self.preFilt.tauNum)
      self.editDlg.edit_pre_tau_pole.setValue(self.preFilt.tauDen)
      self.editDlg.check_pre_use()
      # prefilter i-term
      self.editDlg.checkBox_post_integrator.setChecked(self.postFiltI.use)
      self.editDlg.edit_post_tau_i.setValue(self.postFiltI.tau)
      self.editDlg.edit_post_integrator_limit.setValue(self.postFiltI.limit)
      self.editDlg.checkBox_post_integrator_zero.setChecked(self.postFiltI.andZero)
      self.editDlg.check_post_integrator_use()
      # feed forward
      #print("+ ff use " + str(self.ffUse) + " -> " + str(self.editDlg.checkBox_ff_use.isChecked()))
      self.editDlg.checkBox_ff_use.setChecked(self.ffUse)
      self.editDlg.edit_ff_kp.setValue(self.ffKp)
      self.editDlg.check_ff_use()
      self.editDlg.checkBox_ff_pole_zero.setChecked(self.ffFilt.use)
      self.editDlg.edit_ff_tau_zero.setValue(self.ffFilt.tauNum)
      self.editDlg.edit_ff_tau_pole.setValue(self.ffFilt.tauDen)
      self.editDlg.check_ff_pole_zero()
      # output limit
      self.editDlg.checkBox_out_limit.setChecked(self.outLimitUse)
      self.editDlg.edit_output_limit.setValue(self.outLimit)
      self.editDlg.check_out_limit()
    except:
      print("failed to set values in dialog")
      pass
    pass

  
  def configuration_to_robot(self):
    self.setFromDialog()
    self.main.devWrite(self.asString() + "\n", True)
    print("send data")
    pass

  def setFromDialog(self):
    self.use = self.editDlg.ctrl_main_use.isChecked()
    self.Kp = self.editDlg.edit_kp.value()
    # i-term
    self.integrator.use = self.editDlg.checkBox_integrator_use.isChecked()
    self.integrator.tau = self.editDlg.edit_tau_i.value()
    self.integrator.limit = self.editDlg.edit_integrator_limit.value()
    # lead in forward branch
    self.leadFwd.use = self.editDlg.checkBox_lead_fwd.isChecked()
    self.leadFwd.tauNum = self.editDlg.edit_tau_zero_fwd.value()
    self.leadFwd.tauDen = self.editDlg.edit_tau_pole_fwd.value()
    # lead in feedback branch
    self.leadBack.use = self.editDlg.checkBox_lead_back.isChecked()
    self.leadBack.tauNum = self.editDlg.edit_tau_zero_back.value()
    self.leadBack.tauDen = self.editDlg.edit_tau_pole_back.value()
    # prefilter
    self.preFilt.use = self.editDlg.checkBox_pre_use.isChecked()
    self.preFilt.tauNum = self.editDlg.edit_pre_tau_zero.value()
    self.preFilt.tauDen = self.editDlg.edit_pre_tau_pole.value()
    # prefilter - integrator
    self.postFiltI.use = self.editDlg.checkBox_post_integrator.isChecked()
    self.postFiltI.tau = self.editDlg.edit_post_tau_i.value()
    self.postFiltI.limit = self.editDlg.edit_post_integrator_limit.value()
    self.postFiltI.andZero = self.editDlg.checkBox_post_integrator_zero.isChecked()
    # feed forward konstant gain
    self.ffUse = self.editDlg.checkBox_ff_use.isChecked()
    self.ffKp = self.editDlg.edit_ff_kp.value()
    # feed forward
    self.ffFilt.use = self.editDlg.checkBox_ff_pole_zero.isChecked()
    self.ffFilt.tauNum = self.editDlg.edit_ff_tau_zero.value()
    self.ffFilt.tauDen = self.editDlg.edit_ff_tau_pole.value()
    # output limit
    self.outLimitUse = self.editDlg.checkBox_out_limit.isChecked()
    self.outLimit = self.editDlg.edit_output_limit.value()

  def requestDataFromRobot(self):
    if self.main.isBridge():
      # also ask to see reply
      self.main.devWrite("regbot:" + self.ctrlID + " subscribe -1\n")
    self.main.devWrite(self.ctrlID + "i\n", True)

  def editControlValues(self):
    self.requestDataFromRobot()
    # fill dialog with current values
    #self.fillDialog()
    #
    #print("# before show")
    self.needUpdate = True
    self.editDlg.show()
    #print("# after show")
    #result = self.editDlg.exec_()
    #if result == QtWidgetsQDialog.Accepted:
      ## move result to data variables,
      #self.lock.acquire()
      #self.setFromDialog()
      ## send all data to robot (if connected)
      #self.configuration_to_robot()
      #self.lock.release()
    #else: 
      #self.edited = False
    #self.editDlg = None
    pass
  #
  # save configuration to regbot.ini 
  def saveToIniFile(self, config):
    if not self.gotDataFromRobot and self.main.isConnected():
      self.requestDataFromRobot()
      sleep(0.2)
    config.add_section(self.ctrlID)
    config.set(self.ctrlID, 'name', self.name)
    config.set(self.ctrlID, 'use', str(self.use))
    config.set(self.ctrlID, 'kp', str(self.Kp))
    config.set(self.ctrlID, 'i_Use', str(self.integrator.use))
    config.set(self.ctrlID, 'i_tau', str(self.integrator.tau))
    config.set(self.ctrlID, 'i_limit', str(self.integrator.limit))
    config.set(self.ctrlID, 'lead_fwd_use', str(self.leadFwd.use))
    config.set(self.ctrlID, 'lead_fwd_tau_zero', str(self.leadFwd.tauNum))
    config.set(self.ctrlID, 'lead_fwd_tau_pole', str(self.leadFwd.tauDen))
    config.set(self.ctrlID, 'lead_back_use', str(self.leadBack.use))
    config.set(self.ctrlID, 'lead_back_tau_zero', str(self.leadBack.tauNum))
    config.set(self.ctrlID, 'lead_back_tau_pole', str(self.leadBack.tauDen))
    config.set(self.ctrlID, 'pre_filt_use', str(self.preFilt.use))
    config.set(self.ctrlID, 'pre_filt_tau_zero', str(self.preFilt.tauNum))
    config.set(self.ctrlID, 'pre_filt_tau_pole', str(self.preFilt.tauDen))
    config.set(self.ctrlID, 'post_filt_i_use', str(self.postFiltI.use))
    config.set(self.ctrlID, 'post_filt_i_tau', str(self.postFiltI.tau))
    config.set(self.ctrlID, 'post_filt_i_limit', str(self.postFiltI.limit))
    config.set(self.ctrlID, 'post_filt_and_zero', str(self.postFiltI.andZero))
    config.set(self.ctrlID, 'ff_use', str(self.ffUse))
    config.set(self.ctrlID, 'ff_kp', str(self.ffKp))
    config.set(self.ctrlID, 'ff_filt_use', str(self.ffFilt.use))
    config.set(self.ctrlID, 'ff_filt_tau_zero', str(self.ffFilt.tauNum))
    config.set(self.ctrlID, 'ff_filt_tau_pole', str(self.ffFilt.tauDen))
    config.set(self.ctrlID, 'out_limit_use', str(self.outLimitUse))
    config.set(self.ctrlID, 'out_limit', str(self.outLimit))
  #
  # load configuration for regot.ini
  def loadFromIniFile(self, config):
    try:
      self.use = config.getboolean(self.ctrlID, 'use')
      self.Kp = config.getfloat(self.ctrlID, 'kp')
      self.integrator.use = config.getboolean(self.ctrlID, 'i_use')
      self.integrator.tau = config.getfloat(self.ctrlID, 'i_tau')
      self.integrator.limit = config.getfloat(self.ctrlID, 'i_limit')
      self.leadFwd.use = config.getboolean(self.ctrlID, 'lead_fwd_use')
      self.leadFwd.tauNum = config.getfloat(self.ctrlID, 'lead_fwd_tau_zero')
      self.leadFwd.tauDen = config.getfloat(self.ctrlID, 'lead_fwd_tau_pole')
      self.leadBack.use = config.getboolean(self.ctrlID, 'lead_back_use')
      self.leadBack.tauNum = config.getfloat(self.ctrlID, 'lead_back_tau_zero')
      self.leadBack.tauDen = config.getfloat(self.ctrlID, 'lead_back_tau_pole')
      self.preFilt.use = config.getboolean(self.ctrlID, 'pre_filt_use')
      self.preFilt.tauNum = config.getfloat(self.ctrlID, 'pre_filt_tau_zero')
      self.preFilt.tauDen = config.getfloat(self.ctrlID, 'pre_filt_tau_pole')
      self.postFiltI.use = config.getboolean(self.ctrlID, 'post_filt_i_use')
      self.postFiltI.tau = config.getfloat(self.ctrlID, 'post_filt_i_tau')
      self.postFiltI.limit = config.getfloat(self.ctrlID, 'post_filt_i_limit')
      self.postFiltI.andZero = config.getboolean(self.ctrlID, 'post_filt_and_zero')
      self.ffUse = config.getboolean(self.ctrlID, 'ff_use')
      self.ffKp = config.getfloat(self.ctrlID, 'ff_kp')
      self.ffFilt.use = config.getboolean(self.ctrlID, 'ff_filt_use')
      self.ffFilt.tauNum = config.getfloat(self.ctrlID, 'ff_filt_tau_zero')
      self.ffFilt.tauDen = config.getfloat(self.ctrlID, 'ff_filt_tau_pole')
      self.outLimitUse = config.getboolean(self.ctrlID, 'out_limit_use')
      self.outLimit = config.getfloat(self.ctrlID, 'out_limit')
      print("# loaded control data from ini file")
      self.needUpdate = True
      # loading from ini is just as good as getting it from robot
      self.gotDataFromRobot = True
    except:
      print("Missed config file info for " + self.ctrlID + " or one of its elements - continues")

