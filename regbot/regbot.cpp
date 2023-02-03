/***************************************************************************
*   Copyright (C) 2019-2022 by DTU                             *
*   jca@elektro.dtu.dk                                                    *
*
*   Main function for small regulation control object (regbot)
*   build on Teensy 3.1 or higher,
*   intended for 31300/1 Linear control
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

// #define REV_ID "$Id: regbot.cpp 1484 2023-01-08 16:22:11Z jcan $"


#define MISSING_SYSCALL_NAMES

#include <malloc.h>
#include <ADC.h>
#include "pins.h"
#include "IntervalTimer.h"
#include "ulog.h"
#include "src/main.h"
#include "umission.h"
#include "ulinesensor.h"
#include "ueeconfig.h"
#include "wifi8266.h"
#include "uservo.h"
#include "usound.h"

#include "uusb.h"
#include "ustate.h"
#include "uencoder.h"
#include "umotor.h"
#include "uad.h"
#include "ucurrent.h"
#include "uirdist.h"
#include "uimu2.h"
#include "udisplay.h"
#include "uusbhost.h"


#ifdef REGBOT_HW4
#pragma message "REGBOT v4 COMPILE"
#elif defined(REGBOT_HW41)
#pragma message "REGBOT with Teensy4.1 COMPILE"
#else
#pragma message "REGBOT v3 and older COMPILE"
#endif

// int16_t robotId = 0;
// uint8_t robotHWversion = 6;

// main heartbeat timer to service source data and control loop interval
IntervalTimer hbTimer;
/// has positive reply been received frrom IMU
// bool imuAvailable = false;
// battery low filter
// uint16_t batVoltInt = 0;
// heart beat timer
volatile uint32_t hbTimerCnt = 0; /// heart beat timer count (control_period - typically 1ms)
volatile uint32_t hb10us = 0;     /// heart beat timer count (10 us)
volatile uint32_t tsec; /// time that will not overrun
volatile uint32_t tusec; /// time that will stay within [0...999999]
// flag for start of new control period
volatile bool startNewCycle = false;
// uint32_t startCycleCPU;
// //
// uint32_t controlUsedTime[3] = { 0, 0, 0 }; // third item is acceleration limit is reached (active)
int pushHBlast = 0;
// bool batteryHalt = false;
float steerWheelAngle = 0;
// Heart beat interrupt service routine
void hbIsr ( void );
///


// ////////////////////////////////////////

void setup()   // INITIALIZATION
{
  state.setStatusLed(HIGH);
  sound.setup();
  sound.play(2); // jingle 2 (HW 7-8 only)
  delay (1500); // ms (for the welcome tune)
  usb.setup();
  ad.setup();
  current.setup();
  command.setup();
  state.setup();
  encoder.setup();
  ls.setup();
  irdist.setup();
  userMission.setup();
  control.setup();
  imu2.setup();
  usbhost.setup();
  // start 10us timer (heartbeat timer)
  hbTimer.begin ( hbIsr, ( unsigned int ) 10 ); // heartbeat timer, value in usec
  // data logger init
  logger.setup();
  logger.setLogFlagDefault();
  logger.initLogStructure ( 100000 / state.CONTROL_PERIOD_10us );
  // read configuration from EE-prom (if ever saved)
  // this overwrites the just set configuration for e.g. logger
  // if a configuration is saved
  if ( true )
    eeConfig.eePromLoadStatus (false);
  // configuration changes setup
  servo.setup();  // set PWM for available servo pins
  motor.setup();  // set motor pins
  // display on green-board (regbot 5.0 (HW 7-8)) requires reboot after setting HW version
  display.setup();  
}


int debugPos = 0;
uint32_t debugTime = 0;
int debugSaved = 0;
/**
* Main loop
* primarily for initialization,
* non-real time services and
* synchronisation with heartbeat.*/
void loop ( void )
{
  control.resetControl();
  bool cycleStarted = false;
  state.setStatusLed(LOW);
  int debugSaved2 = 0;
  int debugCnt = 0;
//   sound.play(2);
  // then just:
  // - listen for incoming commands
  //   and at regular intervals (1ms)
  // - read sensors,
  // - run control
  // - implement on actuators
  // - do data logging
  while ( true ) 
  { // main loop
    debugPos = 2;
    usb.tick(); // service commands from USB
    debugPos = 3;
    debugTime = hb10us;
    // startNewCycle is set by 10us timer interrupt every 1 ms
    if (startNewCycle ) // start of new control cycle
    { // error detect
      if (debugSaved > 0)
      {
        const int MSL = 100;
        char s[MSL];
        snprintf(s, MSL, "# timing late, detected at position  %d\n", debugSaved);
        usb.send(s);
        debugSaved2 = debugSaved;
        debugSaved = 0;
        debugCnt++;
      }
      if (control.debugMissionEnd and debugCnt > 0)
      { // ending the mission
        const int MSL = 100;
        char s[MSL];
        snprintf(s, MSL, "# timing late, detected at position %d (count=%d)\n", debugSaved2, debugCnt);
        debugSaved2 = 0;
        usb.send(s);
        control.debugMissionEnd = false;
        debugCnt = 0;
      }
//       debugPos = 10;
      startNewCycle = false;
      cycleStarted = true;
      // AD converter should start as soon as possible, to also get a reading at half time
      // values are not assumed to change faster than this
      ad.tick();
      debugPos = 11;
      state.timing(1);
      debugPos = 12;
      encoder.tick();
      debugPos = 13;
      current.tick();
      debugPos = 14;
      imu2.tick();
      debugPos = 15;
      // record read sensor time
      state.timing(2);
      debugPos = 16;
      // calculate sensor-related values
      ls.tick();
      debugPos = 17;
      irdist.tick();
      debugPos = 18;
      // advance mission
      userMission.tick();
      debugPos = 19;
      // do control
      control.tick();
      debugPos = 20;
      // Implement on actuators
      servo.tick();
      debugPos = 21;
      motor.tick();
      debugPos = 22;
      state.tick();
      debugPos = 23;
      command.tick();
      // record read sensor time + control time
      debugPos = 24;
      state.timing(3);
      debugPos = 25;
      // non-critical functions
      logger.tick();
      debugPos = 26;
      sound.tick();
      debugPos = 27;
      display.tick();
      debugPos = 28;
      usbhost.tick();
      debugPos = 29;
    }
    // loop end time
    if (cycleStarted)
    { // control tick do not service subscriptions, so now is the time
      debugPos = 30;
      control.subscribeTick();
      debugPos = 31;
      state.timing(4);
      debugPos = 32;
      state.saveCycleTime();
      debugPos = 33;
      cycleStarted = false;
    }
  }
//   return 0;
}

/**
* Heartbeat interrupt routine
* schedules data collection and control loop timing.
* */
void hbIsr ( void ) // called every 10 microsecond
{ // as basis for all timing
  hb10us++;
  tusec += 10;
  if (tusec > 1000000)
  {
    tsec++;
    tusec = 0;
  }
  if (hb10us - debugTime > 150)
  {
    debugSaved = debugPos;
  }
  if ( hb10us % state.CONTROL_PERIOD_10us == 0 ) // main control period start
  {
    userMission.missionTime += 1e-5 * state.CONTROL_PERIOD_10us;
    hbTimerCnt++;
    startNewCycle = true;
    state.timing(0);
  }
  if ( int(hb10us % state.CONTROL_PERIOD_10us) == state.CONTROL_PERIOD_10us/2 ) // start half-time ad conversion
  {
    ad.tickHalfTime();
  }
}

/////////////////////////////////////////////////////////////////

