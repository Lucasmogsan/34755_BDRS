/***************************************************************************
 *   Copyright (C) 2019-2022 by DTU                             *
 *   jca@elektro.dtu.dk                                                    *
 * 
 * 
 * The MIT License (MIT)  https://mit-license.org/
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software 
 * is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies 
 * or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE. */

#define REV_MINOR 3

#define REV_ID "$Id: ucommand.cpp 1520 2023-01-28 07:15:36Z jcan $" 

#include <malloc.h>
#include <ADC.h>
#include "IntervalTimer.h"
#include "main.h"
#include "ueeconfig.h"
#include "ucommand.h"
#include "ucontrol.h"
#include "uusb.h"
#include "ustate.h"

#include "uencoder.h"
#include "umotor.h"
#include "uad.h"
#include "ucurrent.h"
#include "ulinesensor.h"
#include "uirdist.h"
#include "ulog.h"
#include "uservo.h"
#include "uimu2.h"
#include "usound.h"
#include "udisplay.h"
#include "uusbhost.h"

UCommand command;


void UCommand::setup()
{
  addPublistItem("ver", "get version 'version SVN_rev.x date time'");
}
/**
 * Get SVN revision number */
uint16_t UCommand::getRevisionNumber()
{
  const char * p1 = strstr(REV_ID, ".cpp");
  return strtol(&p1[4], NULL, 10) * 10 + REV_MINOR;
}

/**
 * Get SVN revision number */
const char * UCommand::getCompileDate()
{
  const char * p1 = strstr(REV_ID, ".cpp");
  p1+=4;
  strtol(p1, (char**)&p1, 10);
  p1++;
  strncpy(compileDate, p1, 20);
    compileDate[20] = '\0';
    return compileDate;
}

////////////////////////////////////////////

void UCommand::sendStatusVersion()
{
  const int MRL = 100;
  char reply[MRL];
  snprintf(reply, MRL, "version %.1f %d %s\r\n", (float)getRevisionNumber() / 10.0, state.robotHWversion, getCompileDate());
  usb.send(reply);
}


// parse a user command and execute it, or print an error message
//
void UCommand::parse_and_execute_command(char * buf)
{ // command may be preceded by 'robot' or 'teensy' or robot type
  if (strncmp(buf, "robot ", 6) == 0)
  {
    buf += 6; // move pointer 6 characters forward
    usb.send("# discarding the 'robot' part\n");
    while (*buf == ' ')
      buf++;
  }
  else if (strncmp(buf, "teensy ", 7) == 0)
  {
    buf += 7; // move pointer 7 characters forward
    while (*buf == ' ')
      buf++;
    usb.send("# discarding the 'teensy' part\n");
  }
  else if (strncmp(buf, "regbot ", 7) == 0)
  {
    buf += 7; // move pointer 7 characters forward
    while (*buf == ' ')
      buf++;
    usb.send("# discarding the 'regbot' part\n");
  }
  // check if commands are handled by someone (most urgent first)
  if (state.decode(buf)) {} // handled: arm, stop, bypass
  else if (logger.decode(buf)) {}
  else if (logger.logToUSB)
    // while sending log to client, no other commands are valid
    return;
  else if (subscribeDecode(buf)) {}
  else if (control.decode(buf)) {} // a control issue - decoded there, handles ref, limit, mix, rc
  else if (encoder.decode(buf)) {}
  else if (motor.decode(buf)) {}
  else if (eeConfig.decode(buf)) {}
  else if (ad.decode(buf)) {}
  else if (current.decode(buf)) {}
  else if (ls.decode(buf)) {}
  else if (irdist.decode(buf)) {}
  else if (userMission.decode(buf)) {}
  else if (servo.decode(buf)) {}
  else if (imu2.decode(buf)) {}
  else if (sound.decode(buf)) {}
  else if (display.decode(buf)) {}
  else if (usbhost.decode(buf)) {}
  // commands handled here, e.g. help
  else if (strncmp(buf, "help", 4) == 0)
  { // send on-line help options
    const int MRL = 320;
    char reply[MRL];
    snprintf(reply, MRL, "# Commands available for %s %s ------- \r\n", state.deviceName, state.getRobotName());
    usb.send(reply);
    snprintf(reply, MRL, "# \thelp \tThis help text.\r\n");
    usb.send(reply);
    subscribeSendHelp();
    //
    // and from other units
    state.sendHelp();
    encoder.sendHelp();
    eeConfig.sendHelp();
    motor.sendHelp();
    servo.sendHelp();
    ad.sendHelp();
    current.sendHelp();
    ls.sendHelp();
    irdist.sendHelp();
    userMission.sendHelp();
    logger.sendHelp();
    control.sendHelp();
    imu2.sendHelp();
    sound.sendHelp();
    display.sendHelp();
    usbhost.sendHelp();
    //
  }
  else if (strncmp(buf, "leave", 5) == 0)
  { // host are leaving - stop subscriptions
    state.stopSubscriptions();
    encoder.stopSubscriptions();
//     eeConfig.stopSubscriptions();
    motor.stopSubscriptions();
    ad.stopSubscriptions();
    current.stopSubscriptions();
    ls.stopSubscriptions();
    irdist.stopSubscriptions();
    userMission.stopSubscriptions();
    logger.stopSubscriptions();
    imu2.stopSubscriptions();
    control.stopSubscriptions();
    stopSubscriptions();
    imu2.stopSubscriptions();
    sound.stopSubscriptions();
    display.stopSubscriptions();
  }
  else
  {
    usb.sendInfoAsCommentWithTime("Unhandled message", buf);
  }
}

/////////////////////////////////////////////////

void UCommand::tick()
{
  // subscription service
  subscribeTick();
}

void UCommand::sendData(int item)
{ // called from subscribeTick()
  if (item == 0)
    sendStatusVersion();
}


