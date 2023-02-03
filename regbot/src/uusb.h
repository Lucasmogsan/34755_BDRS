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

#ifndef UUSB_H
#define UUSB_H


class UUSB
{
private:
  bool silenceUSBauto = true; // manuel inhibit of USB silent timeout
  uint32_t lastSend = 0;
  // local echo is used if we are talking to e.g. putty
  // and a command prompt would be nice.
  bool localEcho = false;
  bool justSendPrompt = false;
  /**
   * usb command buffer space */
  static const int RX_BUF_SIZE = 200;
  char usbRxBuf[RX_BUF_SIZE];
  int usbRxBufCnt = 0;
//   uint32_t rxStartHb = 0;
  bool usbRxBufOverflow = false;
  uint32_t usbTimeoutGotData = 0;
  
  int debugCnt = 0;
  
public:
  void setup();
  
  void tick();
  /** send message to USB host
   * \param str : string to send, should end with a '\n'
   * \param blocking : if true, then function will not return until send.
   *                   if false, message will be dropped, if no BW is available
   * return true if send. */
  bool send(const char* str); //, bool blocking = false);
  /** send to USB channel 
  * \param str is string to send
  * \param n is number of bytes to send
  * \param blocking if false, then send if space only, else don't return until send
  */
  inline bool send_block(const char * str, int n) //, bool blocking)
  {
    return client_send_str(str, n); //, blocking);
  }
  
  bool sendInfoAsCommentWithTime(const char* info, const char * msg);
  /**
   * send help and status */
  void sendHelp();
  
private:
  /// send a string - if possible - of this length to USB
  bool client_send_str(const char * str, int m); //, bool blocking);
  /// received a character from USB
  void receivedCharFromUSB(uint8_t n);
  /// USB incomming port handling
  void handleIncoming();
};
  
extern UUSB usb;
#endif
