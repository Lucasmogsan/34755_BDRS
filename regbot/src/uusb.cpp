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

#include <core_pins.h>
#include <usb_serial.h>
#include "main.h"
#include "uusb.h"
#include "ucommand.h"
#include "ulog.h"

UUSB usb;

void UUSB::setup()
{ // init USB connection (parameter is not used - always 12MB/s)
  Serial.begin ( 115200 ); // USB init serial
  send("# welcome - ready in a moment\r\n");
}

void UUSB::tick()
{ // check for messages
  handleIncoming();
}

bool UUSB::send(const char* str) // , bool blocking)
{
  bool sendOK;
  if (localEcho == 1 and justSendPrompt)
  {
    client_send_str("\n\r", 2);
    justSendPrompt = false;
  }
  uint32_t t = hb10us;
  int n = strlen(str);
  sendOK = client_send_str(str, n);
  //
  if (false and debugCnt < 6)
  { // debug 
    const int MSL = 100;
    char s[MSL];
    snprintf(s, MSL, "# UUSB::send %d/%d free=%d t=%ld/%lu\n", sendOK, n, usb_serial_write_buffer_free(), hb10us - t, t);
    client_send_str(s, strlen(s));
    debugCnt++;
  }
  return sendOK;
}

void UUSB::sendHelp()
{
  const int MRL = 320;
  char reply[MRL];
  snprintf(reply, MRL, "# ------ USB connection ------- \r\n");
  send(reply);
  snprintf(reply, MRL, "#   i=V          Interactive: V=0: GUI info (require activity), V=1: local echo, V=2:no timeout (i=%d)\r\n", localEcho);
  send(reply);
  snprintf(reply, MRL, "#   silent=1     Should USB be silent, if no communication (1=auto silent) silent=%d\r\n", silenceUSBauto);
  send(reply);
}

bool UUSB::sendInfoAsCommentWithTime(const char* info, const char * msg)
{
  const int MSL = 400;
  char s[MSL];
  bool isOK = false;
  snprintf(s, MSL, "# %.3f %s: %s\n", float(hb10us) * 0.00001, info, msg);
  isOK = send(s);
  return isOK;
}


//////////////////////////////////////////////////

bool UUSB::client_send_str(const char * str, int m) // , bool blocking) //, bool toUSB, bool toWifi)
{
  //int n = strlen(str);
  bool okSend = true;
  if (true)
  { // generate q-code first
    int sum = 0;
    const char * p1 = str;
    for (int i = 0; i < m; i++)
    {
      if (*p1 >= ' ')
        sum += *p1;
      p1++;
    } 
    const int MQL = 4;
    char q[MQL];
    snprintf(q, MQL, ";%02d", (sum % 99) + 1);
    int a = usb_serial_write(q, 3);
    if (a == 3)
    {
      a = usb_serial_write(str, m);
      okSend += a + 2;
    }
    else
      okSend = false;
  }
  return okSend;
}

////////////////////////////////////////////////////////////////

void UUSB::handleIncoming()
{
  int n = 0, m;
  // get number of available chars in USB buffer
  m = usb_serial_available();
  if (m > 20)
    // limit to no more than 20 chars in one 1ms cycle
    m = 20;
  // 
  if (m > 0)
  { // get characters
//     const int MSL = 100;
//     char s[MSL];
//     snprintf(s, MSL, "# UUSB::handleIncoming: got %d at %luus\r\n", m, hb10us*10);
//     usb.send(s);
    for (int i = 0; i < m; i++)
    { // get pending characters
      n = usb_serial_getchar();
      if (n < 0)
        break;
      if (n >= '\n' and n < 0x80)
      { // there is data from USB, so it is active
        usbTimeoutGotData = hbTimerCnt;
        // command arriving from USB
        //usb_send_str("#got a char from USB\r\n");
        receivedCharFromUSB(n) ;
        break;
      }
    }
//     snprintf(s, MSL, "# UUSB::handleIncoming: end %d at %luus\r\n", m, hb10us*10);
//     usb.send(s);
  }
}


/**
 * Got a new character from USB channel
 * Put it into buffer, and if a full line, then intrepid the result.
 * \param n is the new character */
void UUSB::receivedCharFromUSB(uint8_t n)
{ // got another character from usb host (command)
//   if (usbRxBufCnt == 0)
//     // got first char in new message
//     rxStartHb = hbTimerCnt;
  if (n >= ' ')
  {
    usbRxBuf[usbRxBufCnt] = n;
    if (usbRxBufCnt < RX_BUF_SIZE - 1)
      usbRxBufCnt++;
    else
    {
      usbRxBufOverflow = true;
      usbRxBufCnt = 0;
      usbRxBuf[usbRxBufCnt] = '\0';
    }
  }
  //
  if (localEcho) // and not silentUSB)
    // echo characters back to terminal
    usb_serial_putchar(n);
  if (n == '\n' or n=='\r')
  { // zero terminate
    if (usbRxBufOverflow)
    {
      usbRxBufOverflow = false;
      send("# USB rx-buffer overflow\n");
    }
    else
    {
      if (usbRxBufCnt > 0)
      {
//         if (logger.logUSB and logger.isLogging())
//         { // add string with newline to USB log
//           usbRxBuf[usbRxBufCnt] = '\n'; // to make log readable
//           logger.addUSBLogEntry(usbRxBuf, usbRxBufCnt + 1, rxStartHb, -1);
//         }
        usbRxBuf[usbRxBufCnt] = '\0';
        // check CRC
        bool crcOK = false;
        if (usbRxBuf[0] == ';')
        {
          const char * p1 = usbRxBuf;
          int crc = int(p1[1] - '0') * 10 + int(p1[2] - '0');
          int sum = 0;
          int sumCnt = 0;
          for (int i = 3; i < usbRxBufCnt; i++)
          {
            if (usbRxBuf[i] < ' ')
              break;
            sum += usbRxBuf[i];
            sumCnt++;
          }
          crcOK = (sum % 99) + 1 == crc;
          if (crcOK or localEcho)
          {
            command.parse_and_execute_command(&usbRxBuf[3]);
            debugCnt = 0;
          }
          else
          {
            const int MSL = 200;
            char s[MSL];
            snprintf(s, MSL, "# CRC failed (crc=%d, found to be %d, sum=%d, %d chars), for '%s'\n",
                     crc, (sum % 99) + 1, sum, sumCnt, usbRxBuf);
            send(s);
          }
        }
        else
        {
          const int MSL = 300;
          char s[MSL];
          snprintf(s, MSL, "# processing under protest - no CRC '%s'\n", usbRxBuf);
          send(s);
          command.parse_and_execute_command(usbRxBuf);
          debugCnt = 0;
        }
      }
      if (localEcho == 1)
      {
        send("\r\n>>");
        justSendPrompt = true;
      }
    }
    // flush remaining input
    usbRxBufCnt = 0;
  }
  else if (usbRxBufCnt >= RX_BUF_SIZE - 1)
  { // garbage in buffer, just discard
    usbRxBuf[usbRxBufCnt] = 0;
    const char * msg = "** Discarded (missing \\n)\n";
    send(msg);
    usbRxBufCnt = 0;
  }
}


