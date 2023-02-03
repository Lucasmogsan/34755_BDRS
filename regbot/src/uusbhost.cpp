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

#include "main.h"
#include "uusbhost.h"
#include "ueeconfig.h"
#include "pins.h"
#include "ustate.h"


/// Sharp IR distance sensor interface object.
UUsbHost usbhost;


void UUsbHost::setup()
{
  active = false;
#ifdef REGBOT_HW41
  active = true;
  myusb.begin();
  addPublistItem("joyaxis", "Get current joystick axis values");
  addPublistItem("joybut",  "Get current joystick button values");
  addPublistItem("joy",     "Get joystick message");
  initialized = true;
#endif
}


void UUsbHost::tick()
{
  tickCnt++;
#ifdef REGBOT_HW41
  if (active and tickCnt % 10 == 0)
  {
    if (not initialized)
    { // not initialized yet
      setup();
      return;
    }
    myusb.Task();
    //PrintDeviceListChanges();

    int isOK = -1;
    for (int joystick_index = 0; joystick_index < COUNT_JOYSTICKS; joystick_index++) {
      if (joysticks[joystick_index].available()) 
      {
        isOK = joystick_index;
//         uint64_t axis_mask = joysticks[joystick_index].axisMask();
//         uint64_t axis_changed_mask = joysticks[joystick_index].axisChangedMask();
        buttons = joysticks[joystick_index].getButtons();
        /*
        Serial.printf("Joystick(%d): buttons = %x\n", joystick_index, buttons);
        //Serial.printf(" AMasks: %x %x:%x", axis_mask, (uint32_t)(user_axis_mask >> 32), (uint32_t)(user_axis_mask & 0xffffffff));
        //Serial.printf(" M: %lx %lx", axis_mask, joysticks[joystick_index].axisChangedMask());
        if (show_changed_only) {
          for (uint8_t i = 0; axis_changed_mask != 0; i++, axis_changed_mask >>= 1) {
            if (axis_changed_mask & 1) {
              Serial.printf(" %d:%d", i, joysticks[joystick_index].getAxis(i));
            }
          }

        } else {
          for (uint8_t i = 0; axis_mask != 0; i++, axis_mask >>= 1) {
            if (axis_mask & 1) {
              Serial.printf(" %d:%d", i, joysticks[joystick_index].getAxis(i));
            }
          }
        }
        */
        uint8_t ltv;
        uint8_t rtv;

        for (uint8_t i = 0; i<64; i++) {
            psAxis[i] = joysticks[joystick_index].getAxis(i);
        }
        
        switch (joysticks[joystick_index].joystickType()) {
          default:
            break;
          case JoystickController::PS4:
//             printAngles();
            ltv = joysticks[joystick_index].getAxis(3);
            rtv = joysticks[joystick_index].getAxis(4);
            if ((ltv != joystick_left_trigger_value[joystick_index]) || (rtv != joystick_right_trigger_value[joystick_index])) {
              joystick_left_trigger_value[joystick_index] = ltv;
              joystick_right_trigger_value[joystick_index] = rtv;
              joysticks[joystick_index].setRumble(ltv, rtv);
            }
            break;

          case JoystickController::PS3:
            ltv = joysticks[joystick_index].getAxis(18);
            rtv = joysticks[joystick_index].getAxis(19);
            if ((ltv != joystick_left_trigger_value[joystick_index]) || (rtv != joystick_right_trigger_value[joystick_index])) {
              joystick_left_trigger_value[joystick_index] = ltv;
              joystick_right_trigger_value[joystick_index] = rtv;
              joysticks[joystick_index].setRumble(ltv, rtv,50);
            }
            break;

          case JoystickController::XBOXONE:
          case JoystickController::XBOX360:
            ltv = joysticks[joystick_index].getAxis(3);
            rtv = joysticks[joystick_index].getAxis(4);
            if ((ltv != joystick_left_trigger_value[joystick_index]) || (rtv != joystick_right_trigger_value[joystick_index])) {
              joystick_left_trigger_value[joystick_index] = ltv;
              joystick_right_trigger_value[joystick_index] = rtv;
              joysticks[joystick_index].setRumble(ltv, rtv,50);
//               Serial.printf(" Set Rumble %d %d", ltv, rtv);
            }
            break;
        }
        if (buttons != buttons_prev) {
          if (joysticks[joystick_index].joystickType() == JoystickController::PS3) {
            //joysticks[joystick_index].setLEDs((buttons >> 12) & 0xf); //  try to get to TRI/CIR/X/SQuare
            uint8_t leds = 0;
            if (buttons & 0x8000) leds = 1;   //Srq
            if (buttons & 0x2000) leds = 2;   //Cir
            if (buttons & 0x1000) leds = 4;   //Tri
            if (buttons & 0x4000) leds = 8;   //X  //Tri
            joysticks[joystick_index].setLEDs(leds);
          } else {
            uint8_t lr = (buttons & 1) ? 0xff : 0;
            uint8_t lg = (buttons & 2) ? 0xff : 0;
            uint8_t lb = (buttons & 4) ? 0xff : 0;
            joysticks[joystick_index].setLEDs(lr, lg, lb);
          }
          buttons_prev = buttons;
        }

//         Serial.println();
        joysticks[joystick_index].joystickDataClear();
      }
    }
    if (isOK)
    {
      if (buttons & (1 << 8) and not manOverride)
      {
        manOverride = true;
      }
      if (buttons & (1 << 9) and manOverride)
      {
        manOverride = false;
        control.setRemoteControl(0, 0, 0, 0);
      }
      int bal = 0;
      bool hiSpeed = 0;
      if (buttons & (1 << 4)) // LB
        bal = 1;
      if (buttons & (1 << 5)) // RB
        hiSpeed = 1;
      if (manOverride)
      {
        float lv, hv;
        if (hiSpeed)
        {
          lv = (psAxis[5] - 127) * -0.02;
          hv = (psAxis[2] - 127) * -0.015;
        }
        else
        {
          lv =  (psAxis[5] - 127) * -0.005;
          hv = (psAxis[2] - 127) * -0.005;
        }
        control.setRemoteControl(2, lv, hv, bal);
        // debug
        if (false)
        {
          const int MSL = 100;
          char s[MSL];
          snprintf(s, MSL, "# joy::tick rc 2 %g %g %d\n", lv, hv, bal);
          usb.send(s);
        }
        // debug end
      }
      if (buttons != buttons_last)
      {
        buttonChanged = true;
      }
      buttons_last = buttons;
      for (int i = 0; i < 8; i++)
      {
        if (psAxis[i] != psAxisPrev[i])
          axisChanged = true;
        psAxisPrev[i] = psAxis[i];
      }
    }
    available = isOK;
  }
#endif
  subscribeTick();
}


bool UUsbHost::decode(const char* buf)
{
  bool used = true;
#ifdef REGBOT_HW41
  if (strncmp(buf, "usbhost ", 8) == 0)
  {
    const char * p1 = &buf[8];
    int v = strtol(p1, (char **)p1, 10);
    active = v;
  }
  else if (strncmp(buf, "joyn", 4) == 0)
  {
    for (int joystick_index = 0; joystick_index < COUNT_JOYSTICKS; joystick_index++)
      joysticks[joystick_index].axisChangeNotifyMask(joystick_full_notify_mask);
    
    PrintDeviceListChanges();
//       for (int joystick_index = 0; joystick_index < COUNT_JOYSTICKS; joystick_index++)
//         joysticks[joystick_index].axisChangeNotifyMask(0x3ff);
  }
  else if (strncmp(buf, "joyc ", 4) == 0)
  {
    const char * p1 = &buf[5];
    int v = strtol(p1, nullptr, 10);
    show_changed_only = v == 1;
  }
  else if (subscribeDecode(buf)) {}
  else
#endif
    used = false;
  return used;
}

void UUsbHost::sendData(int item)
{
  if (item == 0)
    sendAxis();
  if (item == 1)
    sendButtons();
  if (item == 2 and (axisChanged or buttonChanged))
  { // send only if changed
    buttonChanged = false;
    axisChanged = false;
    sendJoy();
  }
}


void UUsbHost::sendHelp()
{
#ifdef REGBOT_HW41
  usb.send("# USB host -------\r\n");
  usb.send("# \tusbhost A \tSet USB host as active or not A=1 is active\r\n");
  usb.send("# \tjoyn A \tNotify on updates\r\n");
  usb.send("# \tjoyc A \tChanges only\r\n");
  subscribeSendHelp();
#endif
}

void UUsbHost::sendAxis()
{
  const int MSL = 300;
  char s[MSL];
  snprintf(s, MSL, "joyaxis %d", available);
  int n = strlen(s);
  char * p1 = &s[n];
  for (int i = 0; i < number_of_axes; i++)
  {
    snprintf(p1, MSL - n, " %d",  psAxis[i]);
    n += strlen(p1);
    p1 = &s[n];
  }
  snprintf(p1, MSL - n, "\r\n");
  usb.send(s);
}

void UUsbHost::sendButtons()
{
  const int MSL = 300;
  char s[MSL];
  snprintf(s, MSL, "joybut %d %lx\n", available, buttons);
  usb.send(s);
}

void UUsbHost::sendJoy()
{
  const int MSL = 300;
  char s[MSL];
  snprintf(s, MSL, "joy %d %d %d %d  %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d %d\r\n", 
          available, manOverride, number_of_axes, number_of_buttons,
          (psAxis[0] - 127) << 7, 
          -(psAxis[1] - 127) << 7, 
          (psAxis[3] - 127) << 7, 
          (psAxis[2] - 127) << 7, 
          -(psAxis[5] - 127) << 7, 
          (psAxis[4] - 127) << 7, 
          (psAxis[6] - 127) << 7, 
          (psAxis[7] - 127) << 7,
          (buttons & (1 << 2)) > 0,
          (buttons & (1 << 1)) > 0,
          (buttons & (1 << 3)) > 0,
          (buttons & (1 << 0)) > 0,
          (buttons & (1 << 4)) > 0,
          (buttons & (1 << 5)) > 0,
          (buttons & (1 << 6)) > 0,
          (buttons & (1 << 9)) > 0,
          (buttons & (1 << 8)) > 0,
          (buttons & (1 << 10)) > 0,
          (buttons & (1 << 7)) > 0
  );
  usb.send(s);
}


/////////////////////////////////////

void UUsbHost::eePromSave()
{
//   eeConfig.pushByte(played);
}

/////////////////////////////////////

void UUsbHost::eePromLoad()
{
//   int f = eeConfig.readByte();
//   if (f >= 0 and f < 11)
//     play(f);
}

#ifdef REGBOT_HW41
//=============================================================================
// Show when devices are added or removed
//=============================================================================
void UUsbHost::PrintDeviceListChanges() 
{
  const int MSL = 100;
  char s[MSL];
  if (false)
  {
    for (uint8_t i = 0; i < CNT_DEVICES; i++) 
    {
      if (true or *drivers[i] != driver_active[i]) 
      {
        if (not *drivers[i]) 
        {
  //         snprintf(s, MSL, "# *** Device %d %s - disconnected ***\n", i, driver_names[i]);
  //         usb.send(s);
  //         driver_active[i] = false;
        } 
        else 
        {
          snprintf(s, MSL, "# *** Device %d type=%d, %s %x:%x - connected ***\n", i, joysticks[i].joystickType(), driver_names[i], drivers[i]->idVendor(), drivers[i]->idProduct());
          usb.send(s);
  //         driver_active[i] = true;

          const uint8_t *psz = drivers[i]->manufacturer();
          if (psz && *psz)
          {
            snprintf(s, MSL, "#  manufacturer: %s\n", psz);
            usb.send(s);
          }
          psz = drivers[i]->product();
          if (psz && *psz)
          {
            snprintf(s, MSL, "#  product: %s\n", psz);
            usb.send(s);
          }
          psz = drivers[i]->serialNumber();
          if (psz && *psz) 
          {
            snprintf(s, MSL, "#  Serial: %s\n", psz);
            usb.send(s);
          }
        }
      }
    }
  }

  bool anyAvailable = false;
  for (uint8_t i = 0; i < CNT_HIDDEVICES; i++) 
  {
    if (true or *hiddrivers[i] != hid_driver_active[i]) 
    {
      if (not *hiddrivers[i]) 
      {
//         snprintf(s, MSL, "# *** HID Device %d %s - disconnected ***\n", i, hid_driver_names[i]);
//         usb.send(s);
//         hid_driver_active[i] = false;
      } 
      else 
      {
        anyAvailable = true;
        snprintf(s, MSL, "# *** HID Device %d type=%d, %s %x:%x - connected ***\n", i, joysticks[i].joystickType(), hid_driver_names[i], hiddrivers[i]->idVendor(), hiddrivers[i]->idProduct());
        usb.send(s);
//         hid_driver_active[i] = true;

        const uint8_t *psz = hiddrivers[i]->manufacturer();
        if (psz && *psz) 
        {
          snprintf(s, MSL, "#  manufacturer: %s\n", psz);
          usb.send(s);
        }
        psz = hiddrivers[i]->product();
        if (psz && *psz) 
        {
          snprintf(s, MSL, "#  product: %s\n", psz);
          usb.send(s);
        }
        psz = hiddrivers[i]->serialNumber();
        if (psz && *psz) 
        {
          snprintf(s, MSL, "#  Serial: %s\n", psz);
          usb.send(s);
        }
      }
    }
  }
  if (not anyAvailable)
    usb.send("# No HID devices available\r\n");
}
#endif

