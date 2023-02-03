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

#ifndef REGBOT_EESTRING_H
#define REGBOT_EESTRING_H

#include <string.h>
#include <stdlib.h>
#include "main.h"

#include "ucommand.h"
#include "uusb.h"




class EEConfig
{
public:
  // constructor
  EEConfig();
  /**
   * Send command help */
  void sendHelp();
  /**
   * decode commands */
  bool decode(const char * buf);
  /** send configuration to USB
   * \param configBuffer set to NULL to use the just saved configuration or to another configuration to fetch.
   * Sends the configuration as byte hex code  */
  void stringConfigToUSB(const uint8_t * configBuffer, int configBufferLength);
  /**
   * is stringbuffer in use, i.e. loaded from a hard-coaded configuration (non-robot specific) */
  bool isStringConfig()
  {
    return stringConfig;
  }
  /** set in use flag and clear buffer */
  inline void setStringBuffer(uint8_t * string2kBuffer,  bool initializeEEprom)
  {
    config = string2kBuffer;
    configAddr = 0;
    configAddrMax = 0;
    if (initializeEEprom)
      eeprom_initialize();
  }
  inline void clearStringBuffer()
  {
    config = NULL;
  }
  /* get 2k config buffer pointer */
//   uint8_t * get2KConfigBuffer()
//   {
//     return config;
//   }
  /**
   * Load configuration from either a config string or from eeProm (flash) 
   * dependent on the "stringConfig" flag
   * \param from2Kbuffer if true, then load from string buffer (must be loaded first), if false, then read for eeprom (flask).
   * */
  void eePromLoadStatus(bool from2Kbuffer);
  /**
   * Save configuration to eeProm (flash) or sent configuration to USB in hex format.
   * \param toUSB set to true to read to USB, or false to save to eeProm */
  void eePromSaveStatus(bool toUSB);
  
public:
  /** save a 32 bit value */
  void push32(uint32_t value);
  /** save a byte */
  void pushByte(uint8_t value);
  /** save a word in configuration stack */
  void pushWord(uint16_t value);
  /** get a 32 bit integer from configuration stack */
  uint32_t read32();
  /** get a byte from configuration stack */
  uint8_t readByte();
  /** get a 16 bit integer from configuration stack */
  uint16_t readWord();
  /**
   * Add a block of data to ee-Prom area 
   * \param data is the byte data block,
   * \param dataCnt is the number of bytes to write 
   * \returns true if space to write all. */
  bool pushBlock(const char * data, int dataCnt);
  /**
   * Read a number of bytes to a string 
   * \param data is a pointer to a byte array with space for at least dataCnt bytes.
   * \param dataCnt is number of bytes to read
   * \returns true if data is added to data array and false if 
   * requested number of bytes is not available */
  bool readBlock(char * data, int dataCnt);
  
  /** save a 32 bit float to configuration stack */
  inline void pushFloat(float value)
  {
    union {float f; uint32_t u32;} u;
    u.f = value;
    push32(u.u32);
  }
  // read 32 bit as float from configuration stack
  inline float readFloat()
  {
    union {float f; uint32_t u32;} u;
    u.u32 = read32();
    return u.f;  
  }
  /** write a word to a specific place in configuration stack
   * typically a size that is not known before pushing all the data */
  inline void write_word(int adr, uint16_t v)
  {
    if (not stringConfig)
      eeprom_write_word((uint16_t*)adr, v);
    else if (config != NULL)
    {
      memcpy(&config[adr], &v, 2);
    }
    else
      usb.send("# failed to save word\n");
    if (adr > configAddr - 2)
      configAddr = adr + 2;
  }
  /**
   * a busy wit if the flash write system is busy */
  inline void busy_wait()
  {
    if (not stringConfig)
    {
      eeprom_busy_wait();
    }
  }
  /** push a block of data to the configuration stack */
  inline void write_block(const char * data, int n)
  {
    if (not stringConfig)
    {
      eeprom_write_block(data, (void*)configAddr, n);
    }
    else
    {
      memcpy(&config[configAddr], data, n);
    }
    configAddr += n;
  }
  /** set the adress for the next push or read operation on the configuration stack */
  void setAddr(int newAddr)
  {
    configAddr = newAddr;
  }
  /** skip some bytes from the configuration stack
   * \param bytes is the number of bytes to skib. */
  void skipAddr(int bytes)
  {
    configAddr+=bytes;
    // debug
//     const int MSL = 100;
//     char s[MSL];
//     snprintf(s, MSL, "# skipped %d bytes\n", bytes);
//     usb.send(s);
    // debug end
  }
  /** get the address of the next push or read operation on the configuration stack */
  int getAddr()
  {
    return configAddr;
  }
  /**
   * Implement one of the hard-coded configurations 
   * \param hardConfigIdx is index to the hardConfig array, as defined in eeconfig.h and set in the constructor.
   * \param andToUsb is a debug flag, that also will return the just loaded configuration to the USB
   * */
  bool hardConfigLoad(int hardConfigIdx, bool andToUsb);
  
protected:
  /**
   * Get hard coded configuration string and load it into buffer in binary form
   * like for the real flash configuration memory. */
   int getHardConfigString(uint8_t * buffer, int configIdx);
  
#ifdef TEENSY35
  const int maxEESize = 4096;
#else
  const int maxEESize = 2048;
#endif
  // public:
//   /**
//    * use hard-coded string values flag - should be false for configurations related to a specific robot
//    * if false, then the flash-version is maintained while this flag is false. */
//   bool eeFromStringuse;
  
private:
  /** full configuration buffer, as real eeProm 
   * is either NULL or points to a 2048 byte array */
  uint8_t * config;
  /** max number of bytes in string buffer */
//   static const int sbufMaxCnt = 64;
//   /** string buffer for data reply to client */
//   uint8_t sbuf[sbufMaxCnt];
  /** number of bytes written to string buffer */
  int sbufCnt;
  /** is string buffer in use - else flash is in use */
  bool stringConfig;
  /** current read/write adress in config array */
  int configAddr;
  /** highest number used in configuration in config array */
  int configAddrMax;
  /** hard coded configuration 
   * for configuration as of 25 dec 2018, NB! must be changed if configuration layout changes
   */
  const char * hardBalance6s = 
  "(Ru) : #cfg00 e0 01 00 00 be 39 00 00 64 00 38 00 03 67 62 6f 74 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00\
  (Ru) : #cfg01 00 00 00 00 00 00 00 00 00 00 00 5f 00 01 80 1b 64 40 00 e8 67 bf 00 0e 11 bf 00 00 00 00 00 00\
  (Ru) : #cfg02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 00 00 00 00 00 00 00 00\
  (Ru) : #cfg03 00 00 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00\
  (Ru) : #cfg04 80 3f 00 00 00 00 94 30 37 40 00 00 00 00 63 4e 00 00 0a 00 00 00 00 4b 03 00 00 60 41 8f c2 75\
  (Ru) : #cfg05 3d 00 00 a0 40 00 00 00 00 0a d7 a3 3b 00 00 90 40 00 00 00 41 4b 03 00 00 60 41 8f c2 75 3d 00\
  (Ru) : #cfg06 00 a0 40 00 00 00 00 0a d7 a3 3b 00 00 90 40 00 00 00 41 1b 03 00 00 80 3f 00 00 80 3e cd cc cc\
  (Ru) : #cfg07 3d 9a 99 19 3e 0a d7 a3 3c 00 00 00 00 0a d7 23 3c 00 00 00 3f 01 02 00 00 c0 3f 01 03 66 66 46\
  (Ru) : #cfg08 40 cd cc cc 3d 01 02 00 00 80 3f 0d 02 cd cc cc 3e 00 00 00 3f 00 00 c0 3f 00 00 00 00 0a d7 a3\
  (Ru) : #cfg09 3b 2b 03 b8 1e 25 c0 60 e5 50 3d 00 00 00 40 e3 a5 1b 3d a6 9b 44 3b cd cc cc 3d f6 3f 1c 46 00\
  (Ru) : #cfg10 00 c0 3f 0b 03 1f 85 6b 3e 66 66 86 3f 9a 99 99 3e 00 00 00 00 cd cc cc 3d 9a 99 99 3e 07 01 cd\
  (Ru) : #cfg11 cc cc 3e ec 51 b8 3f 00 00 80 3f 8f c2 95 3f c3 f5 a8 3e 00 00 80 3f 20 00 6f 31 0a 64 31 30 3a\
  (Ru) : #cfg12 42 3d 30 2e 30 35 0a 66 31 3a 42 3d 36 0a 66 30 3a 42 3d 30 2e 35 0a 02 28 45 00 7b 04 3e 00 2c\
  (Ru) : #cfg13 05 48 00 66 05 5f 00 6d 05 58 00 66 05 49 00 3e 05 51 00 31 05 4b 00 d3 03 00 6c 1a 6c 1a d4 30\
  (Ru) : #cfg14 d4 30 00 00 00 00 00 00 00 00 00 00 00 8f c2 f5 3c 8f c2 f5 3c 48 e1 1a 41 30 00 52 b8 1e 3e 01";
  const char * hardSquareNoBalance = 
  "#cfg00 24 02 00 00 be 39 00 00 64 00 38 00 03 67 62 6f 74 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00\
  #cfg01 00 00 00 00 00 00 00 00 00 00 00 5f 00 01 80 1b 64 40 00 e8 67 bf 00 0e 11 bf 00 00 00 00 00 00\
  #cfg02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 00 00 00 00 00 00 00 00\
  #cfg03 00 00 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00\
  #cfg04 80 3f 00 00 00 00 94 30 37 40 00 00 00 00 6b 4a 00 00 14 00 00 00 00 4b 03 00 00 60 41 8f c2 75\
  #cfg05 3d 00 00 a0 40 00 00 00 00 0a d7 a3 3b 00 00 90 40 00 00 00 41 4b 03 00 00 60 41 8f c2 75 3d 00\
  #cfg06 00 a0 40 00 00 00 00 0a d7 a3 3b 00 00 90 40 00 00 00 41 1b 03 00 00 80 3f 00 00 80 3e cd cc cc\
  #cfg07 3d 9a 99 19 3e 0a d7 a3 3c 00 00 00 00 0a d7 23 3c 00 00 00 3f 01 02 00 00 c0 3f 01 03 66 66 46\
  #cfg08 40 cd cc cc 3d 01 02 00 00 80 3f 0d 02 cd cc cc 3e 00 00 00 3f 00 00 c0 3f 00 00 00 00 0a d7 a3\
  #cfg09 3b 2b 03 b8 1e 25 c0 60 e5 50 3d 00 00 00 40 e3 a5 1b 3d a6 9b 44 3b cd cc cc 3d f6 3f 1c 46 00\
  #cfg10 00 c0 3f 0b 03 1f 85 6b 3e 66 66 86 3f 9a 99 99 3e 00 00 00 00 cd cc cc 3d 9a 99 99 3e 07 01 cd\
  #cfg11 cc cc 3e ec 51 b8 3f 00 00 80 3f 8f c2 95 3f c3 f5 a8 3e 00 00 80 3f 64 00 6f 31 0a 64 32 30 3a\
  #cfg12 42 3d 30 2e 30 35 0a 61 30 2e 33 62 32 3a 41 3d 30 2e 32 35 0a 61 30 3a 42 3d 30 2e 31 0a 61 30\
  #cfg13 2e 32 63 30 3a 43 3d 39 30 0a 61 30 2e 33 63 30 2e 32 35 3a 43 3d 33 36 30 0a 61 30 2e 32 63 30\
  #cfg14 3a 43 3d 39 30 0a 61 30 2e 33 3a 41 3d 30 2e 32 35 0a 61 30 3a 42 3d 30 2e 32 0a 02 28 45 00 7b\
  #cfg15 04 3e 00 2c 05 48 00 66 05 5f 00 6d 05 58 00 66 05 49 00 3e 05 51 00 31 05 4b 00 d3 03 00 6c 1a\
  #cfg16 6c 1a d4 30 d4 30 00 00 00 00 00 00 00 00 00 00 00 8f c2 f5 3c 8f c2 f5 3c 48 e1 1a 41 30 00 52\
  #cfg17 b8 1e 3e 01";
  const char * hardBalance1mOutAndBack = 
  "#cfg00 0b 02 00 00 be 39 00 00 64 00 38 00 03 67 62 6f 74 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00\
  #cfg01 00 00 00 00 00 00 00 00 00 00 00 5f 00 01 80 1b 64 40 00 e8 67 bf 00 0e 11 bf 00 00 00 00 00 00\
  #cfg02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 00 00 00 00 00 00 00 00\
  #cfg03 00 00 00 00 80 3f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3f 00 00 80 3f 00 00 80 3f 00 00\
  #cfg04 80 3f 00 00 00 00 94 30 37 40 00 00 00 00 6b 4a 00 00 19 00 00 00 00 4b 03 00 00 60 41 8f c2 75\
  #cfg05 3d 00 00 a0 40 00 00 00 00 0a d7 a3 3b 00 00 90 40 00 00 00 41 4b 03 00 00 60 41 8f c2 75 3d 00\
  #cfg06 00 a0 40 00 00 00 00 0a d7 a3 3b 00 00 90 40 00 00 00 41 1b 03 00 00 80 3f 00 00 80 3e cd cc cc\
  #cfg07 3d 9a 99 19 3e 0a d7 a3 3c 00 00 00 00 0a d7 23 3c 00 00 00 3f 01 02 00 00 c0 3f 01 03 66 66 46\
  #cfg08 40 cd cc cc 3d 01 02 00 00 80 3f 0d 02 cd cc cc 3e 00 00 00 3f 00 00 c0 3f 00 00 00 00 0a d7 a3\
  #cfg09 3b 2b 03 b8 1e 25 c0 60 e5 50 3d 00 00 00 40 e3 a5 1b 3d a6 9b 44 3b cd cc cc 3d f6 3f 1c 46 00\
  #cfg10 00 c0 3f 0b 03 1f 85 6b 3e 66 66 86 3f 9a 99 99 3e 00 00 00 00 cd cc cc 3d 9a 99 99 3e 07 01 cd\
  #cfg11 cc cc 3e ec 51 b8 3f 00 00 80 3f 8f c2 95 3f c3 f5 a8 3e 00 00 80 3f 4b 00 6f 31 0a 64 32 35 3a\
  #cfg12 42 3d 30 2e 31 0a 66 31 3a 42 3d 31 0a 61 30 2e 36 66 31 6b 31 3a 41 3d 31 42 3d 31 30 0a 61 30\
  #cfg13 66 31 3a 42 3d 31 0a 61 30 2e 36 66 31 6b 2d 31 3a 41 3d 31 42 3d 31 30 0a 61 30 66 31 3a 42 3d\
  #cfg14 33 0a 02 28 45 00 7b 04 3e 00 2c 05 48 00 66 05 5f 00 6d 05 58 00 66 05 49 00 3e 05 51 00 31 05\
  #cfg15 4b 00 d3 03 00 6c 1a 6c 1a d4 30 d4 30 00 00 00 00 00 00 00 00 00 00 00 8f c2 f5 3c 8f c2 f5 3c\
  #cfg16 48 e1 1a 41 30 00 52 b8 1e 3e 01";
  
  const char * hardConfigFollowWall = 
  "#cfg00:0d 02 00 00 d7 24 00 00 1a 00 05 52 b8 1e 3e 48 e1 1a 41 30 00 6a 4d f3 3c 6a 4d f3 3c e1 7a 14\
  #cfg01:3e 01 8e 13 cb ff ff ff fe ff ff ff 1c 00 00 00 03 45 00 05 28 00 00 00 00 41 03 00 00 70 41 00\
  #cfg02:00 90 40 00 00 10 41 41 03 00 00 70 41 00 00 90 40 00 00 10 41 13 03 00 00 80 3f 00 00 80 3e cd\
  #cfg03:cc cc 3d 00 00 00 00 0a d7 23 3c 00 00 00 3f 01 02 00 00 c0 3f 01 03 66 66 46 40 cd cc cc 3d 00\
  #cfg04:02 05 02 0a d7 a3 3d cd cc cc 3e 0a d7 23 3d 29 02 00 00 00 c0 ae 47 61 3d 00 00 80 3f 29 5c 8f\
  #cfg05:3d 00 00 a0 40 1f 03 cd cc 4c 3d 33 33 b3 3e cd cc 4c 3e 66 66 06 40 3d 0a d7 3e 6f 12 83 3a 0a\
  #cfg06:d7 23 3c 0a d7 23 3c 9a 99 19 3e cd cc cc 3e 0d 01 66 66 e6 3f 00 00 80 3f 00 00 00 40 00 00 00\
  #cfg07:3f cd cc 4c 3d 00 00 c0 3f 21 00 6f 32 0a 61 30 66 31 3a 42 3d 32 0a 61 30 2e 33 35 6c 31 6d 30\
  #cfg08:2e 31 38 3a 42 3d 31 32 30 0a 0e 37 01 00 00 71 3d 0a 3e 00 00 b4 42 03 0a 3e 00 00 b4 42 65 34\
  #cfg09:09 61 6c 66 61 72 6f 6d 65 6f c1 5d 02 34 21 b7 20 b8 0b b8 0b 01 06 64 65 76 69 63 65 00 c1 5d\
  #cfg10:01 00 00 71 3d 0a 3e 00 00 b4 42 05 30 d6 03 d8 03 ef 03 ef 03 d8 03 d2 03 e8 03 44 04 e1 03 df\
  #cfg11:03 ea 03 25 04 d8 03 ae 03 e5 03 0c 04 de 03 c9 03 ef 03 0a 04 e3 03 d6 03 ed 03 00 04 dd 03 e1\
  #cfg12:03 e8 03 c2 03 d9 03 41 04 e9 03 99 03 d4 03 d5 03 f0 03 2b 04 d5 03 c5 03 e9 03 1e 04 d8 03 ec\
  #cfg13:03 e6 03 d4 03 cf 03 01 04 e6 03 03 04 ea 03 e3 03 ed 03 fc 03 e6 03 7b 03 ec 03 d1 03 ec 03 0b\
  #cfg14:04 ee 03 16 04 ea 03 ee 03 eb 03 9a 03 e9 03 2c 04 eb 03 ae 03 e9 03 20 04 f1 03 d4 03 ec 03 07\
  #cfg15:04 e9 03 e4 03 e9 03 14 04 ef 03 d5 03 e9 03 d8 03 ef 03 d4 03 e7 03 f7 03 ec 03 d0 03 e8 03 16\
  #cfg16:04 ea 03 ad 03 e8 03 f6 03 ec 03 df 03";
public:
  static const int hardConfigCnt = 3;
  const char * hardConfig[hardConfigCnt] = {
    hardBalance6s,
    hardSquareNoBalance, 
    hardBalance1mOutAndBack } ;
};

/**
 * Instans af ee og string config */
extern EEConfig eeConfig;


#endif
