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
#include "usound.h"
#include "ueeconfig.h"
#include "pins.h"
#include "tunes.h"

/// Sharp IR distance sensor interface object.
USound sound;

void USound::setup()
{
  if (PIN_BUZZER > 0)
  {
    pinMode ( PIN_BUZZER, OUTPUT ); // line sensor LED half power (HW2) - or power to IR (with new power board) (output anyhow)
    tunes_init(PIN_BUZZER);
#ifdef REGBOT_HW41
    pinMode (PIN_MUTE, OUTPUT);
    digitalWriteFast(PIN_MUTE, unmute);
#endif
  }
  //
  addPublistItem("muted", "Is sound muted (on 4.1 only)");
  addPublistItem("sound", "Get current sound activity");
}


void USound::tick()
{
#ifdef REGBOT_HW41
  if (tickCnt < 2000)
  {
    tickCnt++;
    if (tickCnt >= 2000)
    {
      unmute = false;
      digitalWriteFast(PIN_MUTE, unmute);
      tickCnt = 5000;
    }
  }
#endif
  subscribeTick();
}


bool USound::decode(const char* buf)
{
  bool used = true;
//   if (subs[0]->decode(buf))
//     sendStatusDistIR();
  if (strncmp(buf, "play ", 5) == 0)
  {
#ifdef REGBOT_HW41
    const char * p1 = &buf[5];
    uint8_t v = strtol(p1, (char**)&p1, 10);
    play(v);
#endif
    usb.send("# playing a melody\n");
  }
  else if (strncmp(buf, "mute ", 5) == 0)
  {
    const char * p1 = &buf[5];
    int v = strtol(p1, (char**)&p1, 10);
    unmute = v == 0;
#ifdef REGBOT_HW41
    digitalWriteFast(PIN_MUTE, unmute);
#endif
  }
  else if (subscribeDecode(buf)) {}
  else
    used = false;
  return used;
}

void USound::sendData(int item)
{
  if (item == 0)
    sendMuteInfo();
  else if (item == 1)
    sendSoundInfo();
}


void USound::sendHelp()
{
  usb.send("# Sound (teensy 4.1 only) -------\r\n");
  usb.send("# \tNB! don't use play - uses shared timers, but 'mute' is OK\r\n");
  usb.send("# \tplay N \tPlay melody number N (0..10) to buzzer-pin\r\n");
  usb.send("# \tmute V \tMute is V=1 (unmute V=0)\r\n");
  subscribeSendHelp();
}


void USound::sendSoundInfo()
{
  sendTunesStatus();
}

/////////////////////////////////////

void USound::sendMuteInfo()
{
  if (unmute)
    usb.send("muted 0\r\n");
  else
    usb.send("muted 1\r\n");
}

/////////////////////////////////////

void USound::eePromSave()
{
  eeConfig.pushByte(played);
}

/////////////////////////////////////

void USound::eePromLoad()
{
  int f = eeConfig.readByte();
  if (f >= 0 and f < 11)
    play(f);
}

void USound::play(int song)
{
  sing(song);
  played = song;
}
