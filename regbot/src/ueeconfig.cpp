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
#include "ueeconfig.h"
#include "avr_functions.h"
// #include "robot.h"
// #include "mpu9150.h"
#include "umission.h"
#include "ucontrol.h"
#include "ulog.h"
//#include "dist_sensor.h"
// #include "robot.h"
#include "umission.h"
#include "wifi8266.h"
#include "uservo.h"
#include "umotor.h"
#include "uusb.h"

#include "ucommand.h"
#include "ustate.h"
#include "uencoder.h"
#include "ulinesensor.h"
#include "uirdist.h"
#include "uimu2.h"
#include "udisplay.h"


#ifdef REGBOT_HW4
const int EEPROM_SIZE = 4048;
#else
#ifdef REGBOT_HW41
const int EEPROM_SIZE = 4284;
#else
const int EEPROM_SIZE = 2024;
#endif
#endif


/**
 * Global configuration */
EEConfig eeConfig;


/** initialize */
EEConfig::EEConfig()
{
  sbufCnt = 0;
  stringConfig = false;
  config = NULL;
}

bool EEConfig::decode(const char* buf)
{
  bool used = true;
  if (strncmp(buf, "eer", 3) == 0)
  {  // load from flash to ccurrent configuration
    eePromLoadStatus(false);
  }
  else if (strncmp(buf, "eew", 3) == 0)
  {  // load from flash to ccurrent configuration
    eePromSaveStatus(false);
  }
  else if (strncmp(buf, "eeusb", 5) == 0)
  {  // load from flash to ccurrent configuration
    const int MSL = 2048;
    uint8_t s[MSL];
    // set buffer where to save the the configuration (binary)
    setStringBuffer(s, false);
    // save the config to this buffer
    eePromSaveStatus(true);
    // convert binary configuration to string and send to USB
    stringConfigToUSB(nullptr, 0);
    // clear the buffer (to avoid invalid pointer)
    clearStringBuffer();
  }
  else
    used = false;
  return used;
}

void EEConfig::sendHelp()
{
  const int MRL = 250;
  char reply[MRL];
  usb.send("# EE (configuration flash) --------\r\n");
  snprintf(reply, MRL, "# \teew \tSave configuration to EE-Prom (flash)\r\n");
  usb.send(reply);
  snprintf(reply, MRL, "# \teer \tRead configuration from EE-Prom\r\n");
  usb.send(reply);
  snprintf(reply, MRL, "# \teeusb \tGet current configuration to USB\r\n");
  usb.send(reply);
}



void EEConfig::stringConfigToUSB(const uint8_t * configBuffer, int configBufferLength)
{
  int length = configBufferLength;
  const uint8_t * cfg = configBuffer;
  if (cfg == NULL)
  {
    cfg = config;
    length = configAddrMax;
  }
  if (cfg == NULL)
  {
    usb.send("# error: configuration not generated as string\n");
  }
  else
  {
    const int MSL = 110;
    char s[MSL];
    char * p1 = s;
    int n = 0;
    int line = 0;
    int i = 0;
    while (i < length)
    {
      snprintf(s, MSL, "#cfg%02d", line++);
      n += strlen(p1);
      p1 = &s[n];
      for (int j = 0; j < 32; j++)
      {
        snprintf(p1, MSL-n, " %02x", cfg[i]);
        n += strlen(p1);
        p1 = &s[n];
        i++;
        if (i >= length)
          break;
      }
      if (i < length)
      { // not finished, so add a linefeed escape character
        // to make it easier to copy-paste into code
        *p1++ = '\\';
      }
      *p1++ = '\n';
      *p1++ = '\0';
      usb.send(s);
      if (n > MSL - 4)
        usb.send("# stringConfigToUSB error\n");
      p1 = s;
      n = 0;
    }
  }
}
  
void EEConfig::push32(uint32_t value)
{
  //   const int MSL = 100;
  //   char s[MSL];
  //   snprintf(s, MSL, "# ee saved: at %lu, value %lu\r\n", eePushAdr, value);
  //   usb.send(s);
  //
  if (stringConfig)
  {
    if (config != NULL)
      memcpy(&config[configAddr], &value, 4);
  }
  else
  {
    eeprom_busy_wait();
    eeprom_write_dword((uint32_t*)configAddr, value);
  }
  configAddr += 4;
  if (configAddr > configAddrMax)
    configAddrMax = configAddr;
}

////////////////////////////////////////////////

void EEConfig::pushByte(uint8_t value)
{ // save one byte
  if (stringConfig)
  {
    if (config != NULL)
      config[configAddr] = value;
  }
  else
  {
    eeprom_busy_wait();
    eeprom_write_byte((uint8_t*)configAddr, value);
  }
  configAddr++;
  if (configAddr > configAddrMax)
    configAddrMax = configAddr;
}

////////////////////////////////////////////////

void EEConfig::pushWord(uint16_t value)
{ // save one byte
  if (stringConfig)
  {
    if (config != NULL)
      memcpy(&config[configAddr], &value, 4);
  }
  else
  {
    eeprom_busy_wait();
    eeprom_write_word((uint16_t*)configAddr, value);
  }
  configAddr += 2;
  if (configAddr > configAddrMax)
    configAddrMax = configAddr;
}

//////////////////////////////////////////////

uint32_t EEConfig::read32()
{
  uint32_t b;
  if (stringConfig)
  {
    if (config != NULL)
      b = *(uint32_t *)&config[configAddr];
    else
      b = 0;
  }
  else
  {
    b = eeprom_read_dword((uint32_t*)configAddr);
  }
  configAddr += 4;
  return b;
}

/////////////////////////////////////////////////

uint8_t EEConfig::readByte()
{
  uint8_t b;
  if (stringConfig)
  {
    if (config != NULL)
      b = config[configAddr];
    else
      b = 0;
  }
  else
  {
    b = eeprom_read_byte((uint8_t*)configAddr);
  }
//   { // debug
//     const int MSL = 100;
//     char s[MSL];
//     snprintf(s, MSL, "# read byte %d (%c) at address %d (%x), max=%d\n", b, b, configAddr, configAddr, configAddrMax);
//     usb.send(s);
//   }
  configAddr++;
  return b;
}

/////////////////////////////////////////////////

uint16_t EEConfig::readWord()
{
  uint16_t b;
  if (stringConfig)
  {
    if (config != NULL)
      b = *(uint16_t *)&config[configAddr];
    else
      b = 0;
  }
  else
  {
    b = eeprom_read_word((uint16_t*)configAddr);
  }
  configAddr += 2;
  return b;
}
  
///////////////////////////////////////////////////

void EEConfig::eePromSaveStatus(bool toUSB)
{ // reserve first 4 bytes for dword count
  const int MSL = 100;
  char s[MSL];
  // debug
  // debug end
  stringConfig = toUSB;
  // save space for used bytes in configuration
  configAddr = 4;
  configAddrMax = 4;
  // save revision number
  push32(command.getRevisionNumber());
  // main values
  state.eePromSave();
  // save gyro zero offset
  imu2.eePromSave();
  // logger values
  logger.eePromSaveStatusLog();
  // save controller status
  // values to controller
  control.eePromSaveCtrl();
  // save mission - if space
  userMission.eePromSaveMission();
  // save line sensor calibration
  ls.eePromSaveLinesensor();
  // and IR distance sensor
  irdist.eePromSave();
  // save ifi configuration
//   wifi.eePromSaveWifi();
  // save servo configuration
  servo.eePromSave();
  // encoder calibration values
  encoder.eePromSave();
  display.eePromSave();
  motor.eePromSave();
  
//   eePromSaveEncoderCalibrateInfo();
  // then save length
  uint32_t cnt = configAddr;
  configAddr = 0;
  if (not state.robotIDvalid())
  {
    // ignore ee-prom at next reboot
    push32(0);
    snprintf(s, MSL, "# EE-prom D set to default values at next reboot\r\n");
  }
  else
  {
    push32(cnt);
    if (toUSB)
      snprintf(s, MSL, "# Send %lu config bytes (of %d) to USB\r\n", cnt, EEPROM_SIZE);
    else
      snprintf(s, MSL, "# Saved %lu bytes (of %d) to EE-prom D\r\n", cnt, EEPROM_SIZE);
  }
  configAddr = cnt;
  // tell user
  usb.send(s);
}

//////////////////////////////////////////////////

void EEConfig::eePromLoadStatus(bool from2Kbuffer)
{ 
  const int MSL = 100;
  char s[MSL]; 
  //eePushAdr = 0;
  stringConfig = from2Kbuffer;  
  configAddr = 0;
  uint32_t cnt = read32();
  uint32_t rev = read32();
  snprintf(s, MSL, "# Reading configuration - in flash cnt=%lu, rev=%lu, this is rev=%d\r\n", cnt, rev, command.getRevisionNumber());
  usb.send(s);
  if (cnt == 0 or cnt >= uint32_t(maxEESize) or rev == 0)
  {
    snprintf(s, MSL, "# No saved configuration - save a configuration first (config size=%lu, rev=%lu)\r\n", cnt, rev);
    usb.send(s);
    return;
  }
  if (rev != command.getRevisionNumber())
  {
    snprintf(s, MSL, "# configuration from old SW version now:%g != ee:%g - continues\r\n", command.getRevisionNumber()/100.0, rev/100.0);
    usb.send(s);
  }
  state.eePromLoad();
  
  if (state.robotIDvalid())
  { // gyro zero value
    imu2.eePromLoad();
    // values to logger
    logger.eePromLoadStatusLog();
    // values to controller
    control.eePromLoadCtrl();
    // load saved mission
    userMission.eePromLoadMission();
    ls.eePromLoadLinesensor();
    // load data from IR sensor
    irdist.eePromLoad();
    // load servo settings (mostly steering parameters)
    servo.eePromLoad();
    encoder.eePromLoad();
    display.eePromLoad();
    motor.eePromLoad();
    // motor pins depend on HW-version, so re-setup (reboot is better)
    motor.setup();
    // note changes in ee-prom size
    if (cnt != (uint32_t)configAddr)
    {
      snprintf(s, MSL, "# configuration size has changed! %lu != %d bytes\r\n", cnt, configAddr);
      usb.send(s);
    }
  }
  else
  {
    usb.send("# skipped major part of ee-load, as ID == 0\n");
  }
  // pin position may have changed, so reinit
  motor.motorSetEnable(0,0);
}
  
  /////////////////////////////////////////////////
  
  
int EEConfig::getHardConfigString(uint8_t* buffer, int configIdx)
{
  int n = 0;
  int line = 0;
  const int MSL = 60;
  char s[MSL];
  if (buffer == NULL or configIdx < 0 or configIdx >= hardConfigCnt)
  {
    usb.send("# error in get config from string, no buffer or index out of range\n");
  }
  else
  { // buffer string assumed to be 2kbyte
    const char * p1 = hardConfig[configIdx];
    int nl = strlen(p1);
    while (p1 != NULL and n < 2048 and (p1 - hardConfig[configIdx]) < nl)
    {
      if (*p1 != '#')
      {
        snprintf(s, MSL, "# error in hard config found '%c' at n=%d\n", *p1, n); 
        usb.send(s);
        p1++;
      }
      else
      { // skip "cfg" - assumed to be OK
        p1 += 4;
        line = strtol(p1, (char**)&p1, 10);
        if (line > 64)
        { // more than 2kB configuration string
          snprintf(s, MSL, "# too many hard lines: %d (n=%d)\n", line, n);
          usb.send(s);
          break;
        }
        // read data
        p1++; // skip separator
        for (int i = 0; i < 32; i++)
        {
          buffer[n] = strtol(p1, (char**)&p1, 16);
          n++;
          if (*p1 == '\0' or (p1 - hardConfig[configIdx]) > nl)
            break;
        }
        while (*p1 == ' ')
          p1++;
        if (n >= 2048)
          break;
      }
    }
    snprintf(s, MSL, "# loaded %d byte sized values from config string %d\n", n, configIdx); 
    usb.send(s);
  }
  return n;
}
  
//////////////////////////////////////////////////////

bool EEConfig::hardConfigLoad(int hardConfigIdx, bool andToUsb)
{
  bool isOK = false;
  // uses 2K buffer event if the processor has more
  // i.e. hard coded missions should work on all platforms.
  uint8_t buffer2k[2048];
  // set stringConfig flag and set 2k buffer pointer
  setStringBuffer(buffer2k, false);
  // convert hard coded string configuration to 2k buffer
  // returns number of used values in the 2k buffer
  int n = getHardConfigString(buffer2k, hardConfigIdx);
  if (n > 100)
  { // string config is now in buffer2k, ready to be used
    // and set flag to use this rather than the real 2k flash
//     usb.send("# EEConfig::hardConfigLoad: now loading as if in flash\n");
    eePromLoadStatus(true);
//     usb.send("# EEConfig::hardConfigLoad: finished loading\n");
    // debug
    if (andToUsb)
      stringConfigToUSB(buffer2k, n);
    // debug end
    isOK = true;
  }
  else
  {
    usb.send("# EEConfig::hardConfigLoad: config string too short\n");
//     userMission.stopMission();
  }
  // clear stringConfig flag - and reset 2k buffer pointer
  clearStringBuffer();
  return isOK;
}


bool EEConfig::pushBlock(const char * data, int dataCnt)
{
  if (getAddr() + dataCnt < 2048 - 2)
  {
    busy_wait();
    write_block(data, dataCnt);
    return true;
  }
  else
    return false;
}

bool EEConfig::readBlock(char * data, int dataCnt)
{
  if (getAddr() + dataCnt < 2048 - 2)
  {
    busy_wait();
    for (int n = 0; n < dataCnt; n++)
    {
      data[n] = readByte();
    }
    return true;
  }
  else
    return false;
}
