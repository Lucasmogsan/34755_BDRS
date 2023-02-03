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

/** no longer supported 2022 */

#include "wifi8266.h"
#include "main.h"
// #include "command.h"
#include "ueeconfig.h"


#include "uusb.h"
#include "ustate.h"

// UWifi8266 wifi;
// 
// ////////////////////////////////////////////////////////////
// 
// void UWifi8266::sendStatusWiFiHuman()
// {
//   const int MSL = 120;
//   char s[MSL + 4];
//   int n = 0;
//   char * p1;
//   snprintf(s, MSL, "# WiFi (setup=%d) " , setup);
//   n = strlen(s);
//   p1 = &s[n];
//   switch (replyType)
//   {
//     case WFI_FINISHED: strncpy(p1, "FINISHED, ", MSL - n); break;
//     case WFI_NO_REPLY_YET: strncpy(p1, "NO REPLY YET, ", MSL - n); break;
//     case WFI_OK: strncpy(p1, "OK, ", MSL - n); break;
//     case WFI_ERR: strncpy(p1, "ERR, ", MSL - n); break;
//     case WFI_BUSY: strncpy(p1, "BUSY, ", MSL - n); break;
//     case WFI_DATA: strncpy(p1, "DATA, ", MSL - n); break;
//     case WFI_LINK_NOT_VALID: strncpy(p1, "NO LINK, ", MSL - n); break;
//     case WFI_MESSAGE: strncpy(p1, "MESSAGE, ", MSL - n); break;
//   }
//   n += strlen(p1);
//   p1 = &s[n];
//   switch (status)
//   {
//     case WFS_NONE: strncpy(p1, "not started", MSL - n); break;
//     case WFS_NO_CONNECTION: strncpy(p1, "not connected", MSL - n); break;
//     case WFS_CONNECTED: strncpy(p1, "connected (no IP)", MSL - n); break;
//     case WFS_GOT_IP: strncpy(p1, "got IP", MSL - n); break;
//     case WFS_RST_READY: strncpy(p1, "Restarted OK", MSL - n); break;
//     case WFS_ALL_OK: strncpy(p1, "all OK", MSL - n); break;
//   }
//   n += strlen(p1);
//   p1 = &s[n];
//   if (status >= WFS_GOT_IP)
//   {
//     snprintf(p1, MSL - n, ", IP=%d.%d.%d.%d ", wifiIP[0], wifiIP[1], wifiIP[2], wifiIP[3]);
//     n += strlen(p1);
//     p1 = &s[n];
//   }
//   strncpy(p1, "\r\n", MSL - n);
//   usb.send(s);
// }
// 
// /**
//  * Send current status for WiFi - client readable */
// void UWifi8266::sendStatusWiFi()
// {
//   const int MSL = 120;
//   char s[MSL + 4];
//   snprintf(s, MSL, "wfi %d %d %d %d  %d %d %d %d  %s %d %d %s\r\n", not goingToSleep , setup, status, inSleep, 
//            wifiIP[0], wifiIP[1], wifiIP[2], wifiIP[3], 
//            wifiMAC,
//            portNumber, strlen(wifiPW), wifiSSID);
//   usb.send(s);
//   if (localEcho)
//     // also to terminal
//     sendStatusWiFiHuman();
// }
// 
// void UWifi8266::sendStatusWiFiClients()
// {
//   const int MSL = 120;
//   char s[MSL + 4];
// //   uint32_t a[WIFI_MAX_CLIENTS];
// //   for (int i = 0; i < WIFI_MAX_CLIENTS; i++)
// //     if (clientActive[i])
// //       a[i] = hbTimerCnt - clientAlive[i];
// //     else
// //       a[i] = 0;
//   snprintf(s, MSL, "wfc %u %u  %u %u  %u %u  %u %u  %u %u\r\n" , 
//              clientRxMsgCnt[0], clientTxMsgCnt[0],
//              clientRxMsgCnt[1], clientTxMsgCnt[1],
//              clientRxMsgCnt[2], clientTxMsgCnt[2],
//              clientRxMsgCnt[3], clientTxMsgCnt[3],
//              clientRxMsgCnt[4], clientTxMsgCnt[4]/*,
//              wifiMsgGood, wifiMsgLost*/);
//   usb.send(s);
// //     wifiMsgLost = 0;
// //     wifiMsgGood = 0;
// }
// 
// /**
//  * Decode wifi message with connect parameters in format
//  * \param buf is string with:
//  * "wifi 1 port SSID", where
//  * \param 1 marks that connection shall be (re)initialized (1 = reinitialize, 0=close)
//  * \param port is the port number to open
//  * \param SSID is the wifi access point name
//  * \param buf is the string holding all these parameters */
// void UWifi8266::decodeWifi(char * buf)
// {
//   char * p1 = buf, *p2;
//   int v;
//   bool useWifi = not goingToSleep;
//   bool syntaxError = false;
//   if (buf[0] == 'e')
//     // echo all from 8266 board to USB
//     echo8266toUSB = true;
//   else if (buf[0] == 'n')
//     echo8266toUSB = false;
//   else
//   { // else first character is '0' or '1' (enable wifi)
//     useWifi = strtol(p1, &p1, 10);
//     syntaxError = *p1 == '\0';
//   }
//   if (not syntaxError)
//   { // read the remaining parameters
//     v = strtol(p1, &p1, 10);
//     if (*p1 > '\0' and v >1000)
//     {
//       if (v != portNumber)
//         portNumber = v;
//     }
//     else
//       usb.send("# wifi: port must be > 1000\r\n");
//     // find start of SSID
//     while (*p1 > 0 and *p1 != '"')
//       p1++;
//     if (*p1 == '"')
//     { // end SSRID at new line
//       p2 = ++p1;
//       while (*p2 > '\0' and *p2 != '"')
//         p2++;
//       if (*p2 == '"')
//       { // debug
//         //       const int MSL = 75;
//         //       char s[MSL];
//         // debug end
//         *p2++ = '\0';
//         strncpy(wifiSSID, p1, MAX_SSID_SIZE);
//         // now get password - if exist
//         wifiPW[0] = '\0';
//         // now 
//         // debug
//         //       snprintf(s, MSL, "#wifi got: p1=%s, p2=%s, of:%s:\r\n", p1, p2, buf);
//         //       usb.send(s);
//         // debug end
//         p1 = p2;
//         while (*p1 > 0 and *p1 != '"')
//           p1++;
//         if (*p1 == '"')
//         { // end SSRID at new line
//           p2 = ++p1;
//           while (*p2 > 0 and *p2 != '"')
//             p2++;
//           if (*p2 == '"')
//           { // remove quote and terminate
//             // debug
//             //           snprintf(s, MSL, "#wifi got: p1=%s, p2=%s, of:%s:\r\n", p1, p2, buf);
//             //           usb.send(s);
//             // debug end
//             *p2 = '\0';
//             strncpy(wifiPW, p1, MAX_SSID_SIZE);
//           }    
//           else
//           {
//             usb.send("# wifi: failed to find end quote PW\r\n");
//             syntaxError = true;
//           }
//         }
//         else
//         {
//           usb.send("# wifi: failed to find quoted PW\r\n");
//           syntaxError = true;
//         }
//       }
//       else
//       {
//         usb.send("# wifi: failed to find end quote for SSID\r\n");
//         syntaxError = true;
//       }
//     }
//     else
//       usb.send("# wifi: failed to find quoted SSID\r\n");
//     // debug
//   //   const int MSL = 100;
//   //   char s[MSL];
//   //   snprintf(s, MSL, "# wifi set (%s) - now SSID=%s, PW=%s, port=%d\r\n", buf, wifiSSID, wifiPW, portNumber);
//   //   usb.send(s);
//     // debug end
//   }
//   else
//     usb.send("# wifi: missing parameters\r\n");  
//   if (not syntaxError)
//     enableWifi(useWifi);
// }
// 
// 
// void UWifi8266::enableWifi(bool useWifi)
// {
//   goingToSleep = not useWifi;
//   if (useWifi)
//     // not running and not sleeping, so just start
//     setup = 1;
//   else
//   {
//     if (wifi.inSleep)
//       usb.send("# WIFI in sleep mode already\r\n");
//     else
//     { // not sleeping, so (re)start
//         setup = 10;
//     }
//   }
// }
// 
// 
// 
// int8_t UWifi8266::wait4Reply(uint32_t timeout_ms)
// {
//   int8_t result = 0;
//   const int MSL = 130;
//   char s[MSL];
//   switch (replyState)
//   {
//     case 1:
//       // a commad is send, start timeout counter
//       wifiWait = hbTimerCnt;
//       // wait for reply
//       replyState++;
//       break;
//     case 2: // got an OK
//       // waiting for reply
//       if (replyType == WFI_OK)
//       { // we got an OK, reset timer and proceed
//         result = 1;
//         if (localEcho)
//         {
//           snprintf(s, MSL, "#\r\n# ... wifi took %lu ms (to OK)\r\n", hbTimerCnt - wifiWait);
//           usb.send(s);
//         }
//         // reset fail counter
//         wifiFailCnt = 0;
//         //         usb.send("# WFI OK\r\n");
//         // sendStatusWiFi();
//       }
//       else if (replyType == WFI_ERR)
//       { // retry right away
//         result = -1;
//         wifiFailCnt++;
//         if (localEcho)
//         {
//           snprintf(s, MSL, "#\r\n# ... wifi took %lu ms (to fail %d times, setup state=%d)\r\n", hbTimerCnt - wifiWait, wifiFailCnt, setup);
//           usb.send(s);
//         }
//         usb.send("# WFI error\r\n");
//         // an error may be followed by more reply, so allow 8266 to send all, before posting new commands
//         wait4moreAfterError = 200;
//         //
//         if (wifiFailCnt > 2)
//         {
//           usb.send("# WFI error - end of retry - give up\r\n");
//           result = -2;
//           wifiFailCnt = 0;
//         }
//         //sendStatusWiFiHuman();
//       }
//       else if (replyType == WFI_BUSY)
//       { // retry af a short while
// //         usb.send("# - BUSY wait\r\n");
//         if (localEcho)
//         {
//           snprintf(s, MSL, "#\r\n# ... wifi took %lu ms (to busy)\r\n", hbTimerCnt - wifiWait);
//           usb.send(s);
//         }
//         // if busy again, then count (in state==3)
//         replyState++;
//         wifiBusy = hbTimerCnt;
//         // reset fail counter
//         wifiFailCnt = 0;
//         //         usb.send("# WFI busy\r\n");
//         //         sendStatusWiFiHuman();
//       }
//       else if (hbTimerCnt - wifiWait > timeout_ms)
//       { // giving up
//         status = WFS_NONE;
//         result = -2;
//         if (localEcho)
//           usb.send("# WFI timeout\r\n");
//         //sendStatusWiFiHuman();
//       }
//       break;
//     case 3:
//       // chip is busy, so wait a bit (1s) and then retry
//       if (hbTimerCnt - wifiBusy > 1000)
//       { // finished waiting - retry command
//         result = -1;
//         if (localEcho)
//         {
//           snprintf(s, MSL, "#\r\n# ... wifi took %lu ms (busy end wait)\r\n", hbTimerCnt - wifiBusy);
//           usb.send(s);
//         }
//       }
// //       else
// //         // wait a bit longer
// //         wifiBusy--;
//       break;
//     default:
//       usb.send("# wifi (wait4reply) state error\n");
//       replyState = 0;
//       break;
//   }
//   if (result != 0)
//   { // there is a conclusion - either retry or continue - or error
//     replyState = 0;
//     // and we are no longer waiting for a reply
//     replyType = WFI_FINISHED;
//   }
//   return result;
// }
// 
// /**
//  * Initialize wifi and set up socket server.
//  * Assumed to be called at every loop (until finished).
//  * When setup is finished setup==0 (failed) or setup=99 (success).
//  * Setup is executed only if not waiting for a reply.
//  * Result is set by the "receivedCharFromSer" function.
//  */
// void UWifi8266::serialSetup()
// {
//   const int MSL = 65;
//   char s[MSL];
//   int8_t w, p;
//   if (replyState == 0)
//   {
//     if (wait4moreAfterError > 0)
//     {
//       wait4moreAfterError--;
//     }
//     else
//     { // OK to proceed
//       // debug
//       if (setup < 16)
//       {
//         snprintf(s, MSL, "# %lu (%d), wfi setup state %d\n\r", hbTimerCnt, wait4moreAfterError, setup);
//         usb.send(s);
//       }
//       // debug end
//       switch (setup)
//       {
//         case 0: // do not setup
//           break;
//         case 1:
//           // Serial1.write("AT+RST\r\n");
//           // wait for confirmation
//           // wifiReplyState = 1;
//           setup = 2;
//           wifiFailCnt = 0;
//           wifiConnectTryCount++;
//           // debug
//           if (localEcho)
//           {
//             snprintf(s, MSL, "# ---- wifi setup to '%s', pw='%s', port %d ----\r\n", 
//                     wifiSSID, wifiPW, portNumber); 
//             usb.send(s);
//           }
//           // debug end
//           break;
//         case 2:
//           Serial1.write("AT+CWMODE=3\r\n");
//           // wait for confirmation
//           replyType = WFI_NO_REPLY_YET;
//           // debug
//           if (localEcho)
//             usb.send("# Send MODE=3 (AT+CWMODE=3)\r\n");
//           // debug end
//           break;
//         case 3:
//           // get MAC (and IP)
//           Serial1.write("AT+CIFSR\r\n");
//           // wait for confirmation
//           replyType = WFI_NO_REPLY_YET;
//           // debug
//           if (localEcho)
//             usb.send("# Send AT+CIFSR to get MAC\r\n");
//           // debug end
//           break;
//         case 4:
//           snprintf(s, MSL, "AT+CWJAP=\"%s\",\"%s\"\r\n", wifiSSID, wifiPW);
//           Serial1.write(s);
//           // wait for confirmation
//           replyType = WFI_NO_REPLY_YET;
//           // debug
//           if (localEcho)
//           {
//             snprintf(s, MSL, "# send: AT+CWJAP=\"%s\",\"%s\"\r\n", wifiSSID, wifiPW);
//             usb.send(s);
//             sendStatusWiFiHuman();
//           }
//           // debug end
//           break;
//   //       case 4:
//   //         { // ready to read IP - just skip this state
//   //           setup++;
//   //         }
//   //         break;
//         case 5:
//           // got connection
//           wifiConnectTryCount = 0;
//           // enable multiple connections (5)
//           Serial1.write("AT+CIPMUX=1\r\n");
//           // wait for confirmation
//           replyType = WFI_NO_REPLY_YET;
//           // debug
//           if (localEcho)
//             usb.send("# Send AT+CIPMUX=1 (multible connections)\r\n");
//           // debug end
//           break;
//         case 6:
//           // set as socket server
//           snprintf(s, MSL, "AT+CIPSERVER=1,%d\r\n", portNumber);
//           Serial1.write(s);
//           // wait for confirmation
//           replyType = WFI_NO_REPLY_YET;
//           // debug
//           if (localEcho)
//             usb.send("# Send CIPMUX=XX (XX=port number)\r\n");
//           // debug end
//           break;
//         case 7:
//           // get IP (and MAC)
//           Serial1.write("AT+CIFSR\r\n");
//           // wait for confirmation
//           replyType = WFI_NO_REPLY_YET;
//           if (status == WFS_GOT_IP)
//             status = WFS_ALL_OK;
//           // debug
//           if (localEcho)
//             usb.send("# Send AT+CIFSR to get IP\r\n");
//           // debug end
//           break;
//         case 8:
//           // just to set the last ALL_OK flag
//           // when a valid IP is received
//           if (status == WFS_GOT_IP)
//           {
//             status = WFS_ALL_OK;
//             wifiActive = true;
//           }
//           if (wifiIP[3] != state.deviceID)
//           {
//             snprintf(s, MSL, "# Robot ID=%d and IP4 LSb (%d.%d.%d.%d) is different\r\n", 
//                     state.deviceID, wifiIP[0], wifiIP[1], wifiIP[2], wifiIP[3]);
//             usb.send(s);
//           }
//           setup++;
//           break;
//         case 10:
//         case 11:
//         case 12:
//         case 13:
//         case 14:
//           // close connections
//           p = setup - 10;
//           if (clientActive[p])
//           { // close connection
//             snprintf(s, MSL, "AT+CIPCLOSE=%d\r\n", p);
//             Serial1.write(s);
//             replyType = WFI_NO_REPLY_YET;
//             usb.send("# closeing client\r\n");
//           }
//           else
//           {
//             setup++;
//   //           usb.send("# no client\r\n");
//           }
//           restartCnt = 0;
//           break;
//         case 15:
//           // restart wifi setup is needed,
//           // so delete the old first (disallow further connections)
//           // snprintf(s, MSL, "AT+CIPSERVER=0\r\n");
//           // rather do a restart
//           if (not goingToSleep)
//           {
//             snprintf(s, MSL, "AT+RST\r\n");
//             Serial1.write(s);
//             replyType = WFI_NO_REPLY_YET; 
//             usb.send("# resetting TCP server (setup state 15)\r\n");
//             wifiFailCnt = 0;
//             restartCnt++;
//           }
//           else
//             setup++;
//           break;
//         case 16:
//           // reload new configuration
//           if (not goingToSleep)
//           {
//   //           setup = 1;
//             if (localEcho and inSleep)
//               usb.send("# is trying to restart socket server after sleep\r\n");
//           }
//           else
//           {
//             setup = 0;
//             if (localEcho)
//               usb.send("# sending wifi to sleep (reboot to restart)\r\n");
//             enterSleepMode();
//           }
//           status = WFS_NONE;
//   //         sendStatusWiFi();
//           break;
//         case 99:
//           break;
//         default:
//           setup = 99;
//           // debug
//           sendStatusWiFi();
//           // sendStatusWiFiHuman();
//           //wifiStatus = WFS_GOT_IP;
//           break;
//       }
//       if (replyType == WFI_NO_REPLY_YET)
//         // activate reply state machine
//         replyState = 1;
//     }
//   }
//   else
//   { // wait for reply (OK, error or otherwise)
//     // wait4reply returns:
//     //  1  if OK received
//     //  -1 if if busy or if 8266 returned an error
//     //  -2 if a timeout (no reply in time, busy too long or other error)
//     //  0  if still waiting
//     w = wait4Reply(10000);
//     if (w == -2)
//     {
//       if (restartCnt > 5)
//         setup = 0; // gave up
//       else
//         // try another restart
//         setup = 15;
//       wifiActive = false;
//     }
//     else
//       // redo, continue or no change
//       setup += w;
//   }
// }
// 
// void UWifi8266::sendPendingToWifi()
// {
//   const int MSL = 80;
//   char s[MSL];
//   if (sendingState != WFD_SENDING and 
//     sendingState != WFD_SEND_REQ and 
//     not serTxAllSend)
//   {
//     int f = Serial1.availableForWrite(); // free write space
//     if (f > 20)
//     {
//       serTxSendCnt = serTxBufCnt - serTxPosition;
//       if (serTxSendCnt > f)
//       { // send maximum number of characters
//         serTxSendCnt = f;
//         if (echo8266toUSB)
//         {
//           snprintf(s, MSL, "# wifi send %d of %d only\r\n", serTxSendCnt, serTxBufCnt);
//           usb.send(s);
//         }
//       }
//       else
//       { // space for all characters - note as send in tx-counter
//         clientTxMsgCnt[serTxClient] += 1;
//         //         if (serTxPosition > 0)
//         if (echo8266toUSB)
//         {
//           snprintf(s, MSL, "# wifi send the all/rest %d of %d\r\n", serTxSendCnt, serTxBufCnt);
//           usb.send(s);
//         }
//       }
//       // create send command
//       snprintf(s, MSL, "AT+CIPSEND=%d,%d\r\n", serTxClient, serTxSendCnt);
//       // send request to send
//       Serial1.write(s);
//       // flag ready to send when OK
//       sendingState = WFD_SEND_REQ;
//       // start timeout counter
//       waitForSendOKtime = hbTimerCnt;
// //       usb.send("# send request to 8266\r\n");
//     }
//   }
// }
// 
// /**
//  * Got a new character from wifi channel
//  * Put it into buffer, and if a full line, then intrepid the result.
//  * \param n is the new character 
//  * \param cmd is pointer to rxBuffer (of size RX_BUF_SIZE)
//  * \param channel is pointer to store rx channel
//  * \returns true if cmd holds a command string
//  */
// bool UWifi8266::receivedCharFromSer(uint8_t n)
// { // got another character from usb host (command)
//   const int MSL = 180;
//   char s[MSL];
//   bool result = false;
// //   if (n >= ' ' or n == '\n')
//   { // do not use control characters
//     if (serRxBufCnt == 0 and (n <= ' ')) // or n > 126))
//       ; // ignore starting garbage
//     else
//     {
//       serRxBuf[serRxBufCnt] = n;
//       if (serRxBufCnt < RX_BUF_SIZE - 1)
//         serRxBufCnt++;
//       else
//       { // garbage in buffer, just discard
//         serRxBuf[serRxBufCnt] = 0;
//         //     const char * msg = "** Discarded (missing \\n?)\n";
//         snprintf(s, MSL, "# wifi rx buffer full:\r\n%s\r\n", serRxBuf);
//         usb.send(s);
//         serRxBufCnt = 0;
//         dataCharsCnt = 0;
//         replyType = WFI_FINISHED;
//       }
//     }
//   }
//   //   if (localEcho == 1)
//   //       // echo characters back to terminal
//   //       Serial1.write(n);
//   if (sendingState == WFD_SEND_REQ)
//   { // we are waiting for 8266 is ready to receive characters to send
//     if (serRxBuf[0] == '>')
//     { // and we are done with the received '>' character
//       serRxBufCnt = 0;
//       // deliver bytes to send - as prepared by sendPendingToWifi()
//       Serial1.write(&serTxBuf[serTxPosition], serTxSendCnt);
//       // flag that we are waiting for SENDOK
//       sendingState = WFD_SENDING;
//       serTxPosition += serTxSendCnt;
//       // are all in buffer send?
//       if (serTxPosition == serTxBufCnt)
//         serTxAllSend = true;
//     }
//   }
//   if ((n == '\n') and serRxBufCnt > 1)
//   { // check for known (some of them) reply types
//     char *p1, *p2, *p3, *p4;
//     int serChannel;
// //     const int MSL = 130;
// //     char s[MSL];
//     // terminate command string
//     serRxBuf[serRxBufCnt] = '\0';
//     // debug
// //     snprintf(s, MSL, "# wifi line:%s", serRxBuf);
// //     usb.send(s);
//     // debug end
//     // usb.send("(1)\r\n");
//     p1 = strstr(serRxBuf, "+IPD,"); // received data
//     p2 = strstr(serRxBuf, "OK");    // an acknowledge, either OK or SEND OK
//     p3 = strstr(serRxBuf, ",CONNECT");// probably 0,CONNECT
//     p4 = strstr(serRxBuf, ",CLOSED"); // propably '0,CLOSED'
//     //
//     if (p1 == serRxBuf) //(p1 != NULL)
//     { // received data - if not first data, then channel management data has priority
//       //usb.send("# wifi (1)\r\n");
//       //        usb.send(p1 + 5);
//       //        usb.send("(2)\r\n");
//       serChannel = strtol(p1 + 5, &p2, 10);
//       if (serChannel >= 0  and p2 != NULL and serChannel < WIFI_MAX_CLIENTS)
//       { // +IPD,0,21:command
//         int mcnt = 0;
//         p2++; // skip ','
//         lastChannel = serChannel;
//         //         usb.send(p1);
//         //         usb.send("(3)\r\n");
//         // get number of chars in data part
//         dataCharsCnt = strtol(p2, &p2, 10);
//         p2++; // skip the ':'
//         mcnt = strlen(p2); // length to '\n'
//         if (p1 != serRxBuf)
//         { // there is partial data prior to new data
//           // concatenate old and new data
//           // debug
//           snprintf(s, MSL, "# wifi concatenate rx:'%s' p1:'%s'\r\n", serRxBuf, p1);
//           usb.send(s);
//           // debug end
//           // new part of message count
//           if (mcnt < 0 or mcnt > serRxBufCnt)
//             usb.send("# wifi message length error\r\n");
//           else
//           {
//             memmove(p1, p2, mcnt);
//             // debug
//             snprintf(s, MSL, "# wifi concatenated rx:'%s'\r\n", serRxBuf);
//             usb.send(s);
//             // debug end
//             wifiCommand = serRxBuf;
//             result = true;
//           }
//         }
//         else
//         { // message starts after prelude part
//           // debug
//           if (*p2 == '+')
//           {
//             snprintf(s, MSL, "# wifi debug '%s'\r\n", serRxBuf);
//             usb.send(s);
//           }
//           // debug end
//           wifiCommand = p2;
//           result = true;
//         }
//         // first character must be usable - else skip until next \n
//         if (echo8266toUSB and wifiCommand != NULL)
//         { // debug echo to USB channel
//           snprintf(s, MSL, "# UWifi8266::rx ch=%d, m=%d, len=%d, p2len=%d, cnt=%d, *p2=%c [0x%x 0x%x], msg=%s\r\n", 
//                    serChannel, dataCharsCnt, strlen(serRxBuf), mcnt, serRxBufCnt, *p2, p2[0], p2[1], serRxBuf);
//           usb.send(s);
//         }        
//         clientRxMsgCnt[serChannel] += 1;
//         dataCharsCnt -= (mcnt + 1);
//       }
//       // we are in data reply mode - and there may be more
//       replyType = WFI_DATA;
//     }
//     else if (dataCharsCnt > 0 and replyType == WFI_DATA)
//     { // the message is just padded to previous message
//       // and has been read from 8266 block - mark as command
//       // usb.send("# wifi (2)\r\n");
//       wifiCommand = serRxBuf; 
//       // mark valid as command
//       result = true;
//       // debug
//       if (false)
//       {
//         snprintf(s, MSL, "# wifi m=%d len=%d cnt=%d chars more %s\r", dataCharsCnt, strlen(serRxBuf), serRxBufCnt, wifiCommand);
//         usb.send(s);
//       }
//       // debug end
//       // decrease number of bytes missing
//       dataCharsCnt -= strlen(wifiCommand);
//       // to avoid debug desaster
//       if (dataCharsCnt <= 0)
//       { // no more data
//         dataCharsCnt = 0;
//         replyType = WFI_FINISHED;
//       }
//     }
//     else if (p3 != NULL)
//     { // CONNECT received
//       int v = strtol(serRxBuf, &p3, 10);
//       if (*p3 == ',' and v >= 0 and v < WIFI_MAX_CLIENTS)
//       {
//         clientActive[v] = WIFI_MUST;
//         clientAlive[v] = hbTimerCnt;
//         clientRxMsgCnt[v] = 1;
//         clientTxMsgCnt[v] = 0;
//         // debug
//         usb.send("#activated wifi client\r\n");
//         // debug end
//       }
//       // debug
//       else
//         usb.send("# CONNECT failed to activate client\r\n");
//       // debug end
//     }
//     else if (p4 != NULL)
//     { // connection CLOSED
// //       usb.send("# wifi (3)\r\n");
//       int v = strtol(serRxBuf, &p4, 10);
//       if (*p4 == ',' and v >= 0 and v < WIFI_MAX_CLIENTS)
//       {
//         clientActive[v] = WIFI_NOT;
//         clientRxMsgCnt[v] = 0;
//         clientTxMsgCnt[v] = 0;
//         // debug
//         //         usb.send("#lost wifi client\r\n");
//         // debug end
//       }
//       // debug
//       else if (localEcho)
//       {
//         usb.send("# CLOSED failed to close wifi client\r\n");
//       }
//       // debug end
//     }
//     else if (strstr(serRxBuf, "+CIFSR:STAIP"))
//     { // receiving static IP
// //       usb.send("# wifi (4)\r\n");
//       if (localEcho)
//         usb.send("# got a +CIFSR:STAIP - reading IP\r\n");
//       p1 = strchr(serRxBuf, '"');
//       if (p1 != NULL)
//       {
//         for (int i = 0; i < 4; i++)
//         {
//           p1++;
//           wifiIP[i] = strtol(p1, &p1, 10);
//         }
//         if (wifiIP[0] > 0)
//           status = WFS_GOT_IP;
// //         status = WFS_ALL_OK;
//       }
//     }
//     else if (strstr(serRxBuf, "+CIFSR:STAMAC"))
//     {
// //       usb.send("# wifi (5)\r\n");
//       if (localEcho)
//         usb.send("# got a +CIFSR:STAMAC - reading MAC\r\n");
//       p1 = strchr(serRxBuf, '"');
//       strncpy(wifiMAC, ++p1, 18);
//       wifiMAC[17] = '\0';
//       // no space allowed in MAC - 
//       // happens sometime (communication error from 8266)
//       for (int i = 0; i < 17; i++)
//         if (wifiMAC[i] <= ' ')
//           wifiMAC[i] = '_';
//     }
//     // client status - up to 5 clients
//     else if (strstr(serRxBuf, "+CIPSTATUS:"))
//     { // +CIPSTATUS:0,"TCP","10.59.8.113",48935,1
// //       usb.send("# wifi (6)\r\n");
//       // not used - would be nice, some other day
//       snprintf(s, MSL, "# client status: %s\r\n", p1);
//       usb.send(s);
//       replyType = WFI_OK;
//     }
//     // client status - up to 5 clients
//     else if (strstr(serRxBuf, "STATUS:"))
//     { // +CIPSTATUS:0,"TCP","10.59.8.113",48935,1
//       // not used
// //       snprintf(s, MSL, "# client status: %s\r\n", p1);
// //       usb.send(s);
// //       usb.send("# wifi (7)\r\n");
//       status = WFS_ALL_OK;
//     }
//     else if (p2)
//     { // got an OK
// //       usb.send("# wifi (8)\r\n");
//       //       usb.send("# got an OK\r\n");
//       //       usb.send(serRxBuf);
//       //       usb.send("# (OK)\r\n");
// //       snprintf(s, MSL, "# OK: '%s'\r\n", serRxBuf);
// //       usb.send(s);
//       if (strstr(serRxBuf, "SEND OK"))
//       {
//         // debug
// //         uint32_t t = hbTimerCnt - wifiTiming;
// //         snprintf(s, MSL, "# wifi got SEND OK\r\n"); // %lu ms\r\n", t);
// //         usb.send(s);          
//         // debug end
//         sendingState = WFD_SEND_OK;
//         // count good messages
//         wifiMsgGood++;
//       }
//       replyType = WFI_OK;
//     }
//     else if (strstr(serRxBuf, "ERROR") or strstr(serRxBuf, "FAIL"))
//     {
//       if (true or localEcho)
//       {
//         snprintf(s, MSL, "# %lu err: '%s'\r\n", hbTimerCnt, serRxBuf);
//         usb.send(s);
//       }
//       //stop transmitting
//       serTxBufCnt = 0;
//       serTxAllSend = true;
//       replyType = WFI_ERR;
//     }
//     else if (strstr(serRxBuf, "busy"))
//     {
// //       usb.send("# wifi (10)\r\n");
//       if (localEcho)
//       {
//         snprintf(s, MSL, "# busy: '%s'\r\n", serRxBuf);
// //         usb.send("# got an busy...\r\n");
// //         usb.send(serRxBuf);
// //         usb.send("# (BUSY)\r\n");
//         usb.send(s);
//       }
//       replyType = WFI_BUSY;
//     }
//     else if (strstr(serRxBuf, "link is not valid"))
//     {
// //       usb.send("# wifi (11)\r\n");
//       //       usb.send("# got no client (link not valid)...\r\n");
//       //       usb.send(serRxBuf);
//       //       usb.send("# (no link)\r\n");
//       replyType = WFI_LINK_NOT_VALID;
//       // if a send is pending, then abort it
//       sendingState = WFD_NONE;
//     }
//     else if (true or replyType != WFI_FINISHED)
//     { // got data - and not waiting for a reply
// //       usb.send("# wifi (12)\r\n");
//       replyType = WFI_MESSAGE;
//       // debug
//       //       usb.send("# got message line:\r\n");
//       //       usb.send(serRxBuf);
//       // debug end
//       if (strstr(serRxBuf, "WIFI DISCONNECT"))
//       {
//         status = WFS_NO_CONNECTION;
//         // all clients just died
//         for (int i = 0; i < WIFI_MAX_CLIENTS; i++)
//           clientActive[i] = WIFI_NOT;
//         // send status to USB
//         usb.send("# got a DISCONNECT from wifi (IP timeout?)\r\n");
//         requestingClient = -1;
//         sendStatusWiFi();
//       }
//       else if (strstr(serRxBuf, "WIFI CONNECTED"))
//       {
//         status = WFS_CONNECTED;
//         usb.send("# got a WIFI CONNECTED to wifi (new IP?)\r\n");
//         // send status to USB
//         requestingClient = -1;
//         wifiActive = true;
//         inSleep = false;
//         sendStatusWiFi();
//       }
//       else if (strstr(serRxBuf, "WIFI GOT IP"))
//       {
// //         status = WFS_GOT_IP;
//         usb.send("# got a WIFI GOT IP to wifi (new IP?)\r\n");
//         // send status to USB
//         requestingClient = -1;
//         sendStatusWiFi();
//         if (setup <=0 or setup == 99)
//         { // reused last setup (or IP timeout), that is OK, repeat mode and fetch IP and MAC
//           setup = 5;
//           usb.send("# WIFI repeat last setup - setting server mode and fetch IP and MAC\r\n");
//         }
//       }
//       else if (strstr(serRxBuf, "ready"))
//       { // received after a restart AT+RST reply
//         status = WFS_RST_READY;
//         usb.send("# WIFI restarted (AT+RST reply)\r\n");
//         // set server as requested
//         setup = 1;
//         restartCnt = 0;
//       }
//       else 
//       {  // unknown message
//         //         const int MSL = 75;
//         //         char s[MSL];
//         //         snprintf(s, MSL, "# wifi got unknown 0x%x 0x%x 0x%x 0x%x (%d):'%s'\r\n", 
//         //                  serRxBuf[0], serRxBuf[2], serRxBuf[3], serRxBuf[4], serRxBufCnt, serRxBuf);
//         //         usb.send(s);
//         if (false)
//         {
//           snprintf(s, MSL, "# wifi 8266 1 reply %s\r\n", serRxBuf);
//           usb.send(s);
//           replyType = WFI_ERR;
//         }
//       }
//     }
//     else if (wifiCommand == NULL)
//     { // unknown just show result ignore
// //       usb.send("# wifi (13)\r\n");
//       if (true)
//       {
//         snprintf(s, MSL, "# wifi 8266 2 reply %s\r\n", serRxBuf);
//         usb.send(s);
//       }
//     }
//     if (replyType != WFI_DATA)
//     { // ready for new data from wifi board
//       // but there may still be a valid command in serRxBuf
//       serRxBufCnt = 0;
//       dataCharsCnt = 0;
//     }
//     //     if (localEcho == 1)
//     //     {
//     //       usb.send("\r\n>>");
//     //       //usb_serial_flush_output();
//     //     }
//   }
//   else if (false and n < ' ' and n > '\0')
//   { // all remaining characters
//     //     if (n != '\r' and n != '\n')
//     // debug
//     //     const int MSL = 70;
//     //     char s[MSL];
//     //     snprintf(s, MSL, "# got cnt=%d, a %d \\n=%d, \\r=%d\r\n", serRxBufCnt, n, '\n','\r');
//     //     usb.send(s);
//     // debug end
//     if (serRxBufCnt > 0) // and serRxBuf[serRxBufCnt-1] == n)
//     { // ignore carriage return always and newline as first character
//       if (dataCharsCnt > 0)
//         // but we have used a character from wifi board
//         dataCharsCnt--;
// //       if (n == '\r')
// //       {
// //         // debug
// // //         snprintf(s, MSL, "# wifi got cnt=%d, a %d \\r=%d, \\n=%d\r\n", serRxBufCnt, n, '\r','\n');
// // //         usb.send(s);
// //         if (echo8266toUSB)
// //         {
// //           snprintf(s, MSL, "# wifi msg (%lu) ch=%d, len=%d : %s\r\n", hbTimerCnt, lastChannel, strlen(serRxBuf), serRxBuf);
// //           usb.send(s);
// //         }
// //         // debug end
// //       }
//     }
//   }
// //   else if (serRxBufCnt >= RX_BUF_SIZE - 1)
// //   { // garbage in buffer, just discard
// //     serRxBuf[serRxBufCnt] = 0;
// // //     const char * msg = "** Discarded (missing \\n?)\n";
// //     snprintf(s, MSL, "# wifi rx buffer full:\r\n%s\r\n", serRxBuf);
// //     usb.send(s);
// //     serRxBufCnt = 0;
// //     dataCharsCnt = 0;
// //     replyType = WFI_FINISHED;
// //   }
//   if (result)
//   { // all but data ends by a in a \n
// //     if (replyType != WFI_DATA)
// //       dataCharsCnt = 0;
//     // debug
// //     snprintf(s, MSL, "# wifi end ch=%d cmd='%s'\r\n", lastChannel, wifiCommand);
// //     usb.send(s);
//     // debug end
//   }
//   return result;
// }
// 
// 
// void UWifi8266::clearRxBuffer()
// {
//   serRxBufCnt = 0;
//   serRxBuf[serRxBufCnt] = '\0';
//   wifiCommand = NULL;
// }
// 
// 
// 
// /**
//  * send string to wifi socket client.
//  * If last message still waits to be send, then nothing is send and function returns false.
//  * \param link is client number to send to [0..4]
//  * \param msg is string - maximum 100 chars 
//  * \param addCRLF ass line feed and carriage return at end 
//  * \returns true if send (buffered for send) */
// bool UWifi8266::wifiSend(int link, const char * msg, int cnt, bool addCRNL)
// {
//   const int MSL=150;
//   char s[MSL];
// //   int f = Serial1.availableForWrite(); // free write space
//   bool almostSend = false;
// //   if (sendingState == WFD_SEND_REQ or sendingState == WFD_SENDING)
// //   { // waiting to send last message
// // //     wifiMsgLost++;
// //     //     snprintf(s, MSL, "# %lu ms wifi busy - not send %s", hbTimerCnt, msg);
// //     //     usb.send(s);
// //   }
// //   else if (cnt >= TX_BUF_SIZE - 2)
// //   {
// //     wifiMsgLost++;
// //     usb.send("# line too big for wifi (>98) - not send\r\n");
// //   }
//   if (clientActive[link] == WIFI_NOT)
//   {  // client no longer alive - just drop it
//     almostSend = true;
//     wifiMsgLost++;
// //     usb.send("# link not active - not send\n");
//   } 
//   else if (serTxAllSend and status == WFS_ALL_OK)
//   { // ready to send a new message - put it in the tx-buffer
//     if (TX_BUF_SIZE > (cnt + 3))
//     { // save message in tx buffer - for tx when ready
//       strncpy((char*)serTxBuf, msg, cnt);
//       if (addCRNL)
//       { // add carrage return and line feed
//         serTxBuf[cnt++] = '\r';
//         serTxBuf[cnt++] = '\n';
//       }
//       serTxBuf[cnt] = '\0';
//       // save all details for this new message
//       serTxClient = link;
//       serTxPosition = 0;
//       serTxBufCnt = cnt;
//       serTxAllSend = false;
//       // debug
// //       snprintf(s, MSL, "#wifi (status %d) put %d chars into tx buffer\r\n", sendingState, cnt);
// //       usb_serial_write(s, strlen(s));      
//       // debug end
//       almostSend = true;
//     }
//     else
//     {
//       snprintf(s, MSL, "# wifi no space need %d has %d (skip)\r\n", cnt, TX_BUF_SIZE - 3);
//       usb.send(s);
//       wifiMsgLost++;
//     }
//   }
// //   else
// //   {  // debug
// //     snprintf(s, MSL, "#wifi (status %d) failed to tx %d:%s", sendingState, cnt, msg);
// //     usb.send(s);
// //     // debug end
// //   }
//   return almostSend;
// }
// 
// void UWifi8266::eePromLoadWifi()
// {
//   uint8_t n, flag;
//   flag = eeConfig.readByte();
//   // enabeled
//   bool useWifi = (flag & (1 << 0)) != 0;
//   // SSID
//   n = eeConfig.readByte();
//   if (eeConfig.isStringConfig())
//   { // do not use
//     eeConfig.skipAddr(n);
//   }
//   else
//   { // use
//     eeConfig.readBlock(wifiSSID, n);
//     wifiSSID[n] = '\0';
//   }
//   // read also password
//   int m = eeConfig.readByte();
//   if (m > 0 and m <= 16)
//   {
//     if (eeConfig.isStringConfig())
//     { // do not use
//       eeConfig.skipAddr(m);
//     }
//     else
//     { // use
//       eeConfig.readBlock(wifiPW, m);
//       wifiPW[m] = '\0';
//     }
//   }
//   else
//   {
//     if (not eeConfig.isStringConfig())
//       wifiPW[0] = '\0';
//     if (m > 0)
//       usb.send("# wifi ee load error\r\n");
//   }
//   // and port number
//   if (eeConfig.isStringConfig())
//     eeConfig.skipAddr(2);
//   else
//     portNumber = eeConfig.readWord();
//   // restart wifi
// //   if (setup == 0)
// //   { // just booted or wifi disabled
// //     if (wifiEnabled)
// //       setup = 1;
// //   }
// //   else
// //     // we are stopping or reloading setup
// //     setup = 10;
//   enableWifi(useWifi);
// }
// 
// void UWifi8266::eePromSaveWifi()
// { // save port number and SSID
//   uint8_t flag = 0;
//   int n = strlen(wifiSSID);
//   // flags
//   if (not goingToSleep)
//     flag +=  1 << 0;
//   eeConfig.pushByte(flag);
//   eeConfig.pushByte(n);
//   eeConfig.pushBlock(wifiSSID, n);
//   // password
//   n = strlen(wifiPW);
//   // write number of bytes in PW
//   eeConfig.pushByte(n);
//   if (n > 0)
//     eeConfig.pushBlock(wifiPW, n);
//   // write port number
//   eeConfig.pushWord(portNumber);
// }
// 
// void UWifi8266::restart()
// { // close all clients and restart 8266 chip
//   setup = 10;
// }
// 
// void UWifi8266::enterSleepMode()
// { // send command to sleep for 60 seconds
//   // will never return from sleep (requires "use wifi" and reboot)
//   Serial1.write("AT+GSLP=20000\r\n");
//   // wait for confirmation
//   replyType = WFI_NO_REPLY_YET;
//   // mark setup as in "no setup"
//   setup = 0;
//   // mark as in sleep - not waiting for reply
//   inSleep = true;
//   // debug
//   usb.send("# UWifi8266::enterSleepMode: setting wifi to sleep\n");
// }
