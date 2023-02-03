/***************************************************************************
 *   Copyright (C) 2014-2022 by DTU
 *   jca@elektro.dtu.dk            
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

#ifndef UUSBHOST_H
#define UUSBHOST_H

#ifdef REGBOT_HW41
// these are probably defined somewhere that I couldn't find
#define HEX 16
#define DEC 10
//
#include <core_pins.h>
#include <stdlib.h>
#include <Stream.h>
#include <string.h>
#include <USBHost_t36.h>

#endif
#include "usubss.h"



class UUsbHost : public USubss
{
public:
  /**
   * setup */
  void setup();
  /**
   * decode data command */
  bool decode(const char * buf);
  /**
   * send help on messages */
  void sendHelp();
  /**
   * Do sensor processing - at tick time */
  void tick();  
  
  void eePromSave();

  void eePromLoad();
  

protected:
  /**
   * send data to subscriber or requester over USB 
   * @param item is the item number corresponding to the added subscription during setup. */
  void sendData(int item);
  
  void sendDeviceData();
  void sendAxis();
  void sendButtons();
  void sendJoy();
  int tickCnt;
  
#ifdef REGBOT_HW41
  USBHost myusb;
  USBHub hub1{myusb};
  USBHIDParser hid1{myusb};
  
  static const int COUNT_JOYSTICKS = 4;
  JoystickController joysticks[COUNT_JOYSTICKS] = {
    JoystickController(myusb), JoystickController(myusb),
    JoystickController(myusb), JoystickController(myusb)
  };
  int user_axis[64];

  USBDriver *drivers[6] = {&hub1, &joysticks[0], &joysticks[1], &joysticks[2], &joysticks[3], &hid1};
  #define CNT_DEVICES (sizeof(drivers)/sizeof(drivers[0]))
  const char * driver_names[CNT_DEVICES] = {"Hub1", "joystick[0D]", "joystick[1D]", "joystick[2D]", "joystick[3D]",  "HID1"};
  bool driver_active[CNT_DEVICES] = {false, false, false, false};

  // Lets also look at HID Input devices
  USBHIDInput *hiddrivers[4] = {&joysticks[0], &joysticks[1], &joysticks[2], &joysticks[3]};
  #define CNT_HIDDEVICES (sizeof(hiddrivers)/sizeof(hiddrivers[0]))
  const char * hid_driver_names[CNT_DEVICES] = {"joystick[0H]", "joystick[1H]", "joystick[2H]", "joystick[3H]"};
  bool hid_driver_active[CNT_DEVICES] = {false};
  bool show_changed_only = false;

  uint8_t joystick_left_trigger_value[COUNT_JOYSTICKS] = {0};
  uint8_t joystick_right_trigger_value[COUNT_JOYSTICKS] = {0};
  uint64_t joystick_full_notify_mask = (uint64_t) - 1;

  
protected:
  void PrintDeviceListChanges();

#endif
  //
  uint32_t buttons = 0;
  uint32_t buttons_prev = 0;
  uint32_t buttons_last = 0;
  int psAxis[64] = {0};
  int psAxisPrev[64] = {0};
  bool active = false;
  bool available = -1; // index of available device
  bool initialized = false;
  
  char number_of_axes = 8, number_of_buttons = 11;
  bool manOverride = false;
  bool buttonChanged = false;
  bool axisChanged = false;
  
  
  friend class ULogger;
  
};

extern UUsbHost usbhost;

#endif
