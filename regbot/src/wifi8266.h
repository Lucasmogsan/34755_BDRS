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

#ifndef WIFI_8266_H
#define WIFI_8266_H

#include <string.h>
#include <stdlib.h>
//#include "WProgram.h"

//////////////////////////////////////

// class UWifi8266;
// extern UWifi8266 wifi;
// 
// /////////////////////////////////////
// 
// class UWifi8266
// {
// public:
//   // constructor
// //   UWifi8266();
//   // debug function
//   void sendStatusWiFiHuman();
//   // packed status of general wifi info
//   void sendStatusWiFi();
//   // wifi client info (rx,tx count)
//   void sendStatusWiFiClients();
//   // decode commands starting with 'wifi'
//   void decodeWifi(char * buf);
//   /** enable or disable wifi 
//    * If useWifi=false, then wifi is send to sleep. */
//   void enableWifi(bool useWifi);
//   // check for OK or otherwise from 8266 chip
//   int8_t wait4Reply(uint32_t timeout_ms);
//   // setup of all commands needed by 8266 chip, but keep running main loop,
//   // i.e. returns with new state, when setup = 0 or 99 then setup is finished
//   void serialSetup();
//   /** send string to wifi channel
//    * \param link is link to send to [0..4]
//    * \param str is string to send 
//    * \param cnt is number of characters to send
//    * \param addCRLF if true, then \\r + \\n is added 
//    * \returns true if not busy */
//   bool wifiSend(int link, const char * msg, int cnt, bool addCRNL = false);
//   /**
//    * send any pending part of transmit message */
//   void sendPendingToWifi();  
//   /**
//    * Handle incoming data from serial line (8266).
//    * \param n is new char from line
//    * \param wifiCommand (public) is pointer to data from serial line - received from a client.
//    * \param lastChannel (public) client channel [0..4] is returned here.
//    * \returns true if a line is received (terminated with \n. */
//   bool receivedCharFromSer(uint8_t n);
//   /**
//    * empty rx buffer - ready for more characters */
//   void clearRxBuffer();
//   /**
//    * make wifi chip enter sleep mode - either no wifi use or just
//    * to avoid making 100ms transmissions when measuring motor current (gives peeks on 5V supply) */
//   void enterSleepMode();
//   /**
//    * Restart 8266 chip, by first close all connected clients and then restart chip and server
//    * by setting setup to 11->15->16->1 ... started */
//   void restart();
//   
// 
// public:
//   static const int TX_BUF_SIZE = 1003;
//   uint8_t serTxBuf[TX_BUF_SIZE]; // wifi tx buffer (one message only)
//   uint8_t serTxClient;
//   int16_t serTxPosition = 0;
//   int16_t serTxBufCnt = 0;
//   bool serTxAllSend = true; /// wifi output data send to 8266 chip
//   int16_t serTxSendCnt = 0;
//   //bool waitForSendOK = false;
//   uint32_t waitForSendOKtime;
//   int8_t setup = 10; // default is - go to sleep
//   static const int WIFI_MAX_CLIENTS = 5; // 
//   // command for command entrepeter
//   char * wifiCommand = NULL;
//   // source for last command (0..4)
//   int8_t lastChannel = 0;
//   // restart count after timeout 
//   int restartCnt = 0;
//   uint16_t wait4moreAfterError = 0;
//   typedef enum
//   {
//     WIFI_NOT = 0, // not alive
//     WIFI_MUST,    // alive do not send status
//     WIFI_NO_HASH, // alive send all but no debug starting with #
//     WIFI_ALL      // alive send all (as to USB)
//   } WIFI_MGS_TYPE;
//   WIFI_MGS_TYPE clientActive[WIFI_MAX_CLIENTS] = {WIFI_NOT,WIFI_NOT,WIFI_NOT,WIFI_NOT,WIFI_NOT};
//   uint32_t clientAlive[WIFI_MAX_CLIENTS] = {0,0,0,0,0};
//   uint16_t clientRxMsgCnt[WIFI_MAX_CLIENTS] = {0,0,0,0,0};
//   uint16_t clientTxMsgCnt[WIFI_MAX_CLIENTS] = {0,0,0,0,0};
//   
//   typedef enum 
//   { // connection status - set by the corresponding keywords from device
//     WFS_NONE = 0,      // not started / closed
//     WFS_NO_CONNECTION, // from message
//     WFS_CONNECTED,     // claimed by message
//     WFS_GOT_IP,        // claimed by message
//     WFS_RST_READY,     // received a "ready", i.e. chicp restarted
//     WFS_ALL_OK         // got real IP number from 8266 (assumes all is OK)
//   } WifiStatus;
//   typedef enum 
//   { // command / reply state 
//     // set when sending command - and when reply received
//     WFI_FINISHED = 0,  // not expecting a reply
//     WFI_NO_REPLY_YET,  // expecting a reply
//     WFI_OK,            // got an OK reply
//     WFI_ERR,
//     WFI_BUSY,
//     WFI_DATA,
//     WFI_LINK_NOT_VALID,
//     WFI_MESSAGE
//   } WifiReply;
//   typedef enum 
//   {
//     WFD_NONE = 0,  // not in sending mode
//     WFD_SEND_REQ,  // waiting for OK to transfer data
//     WFD_SENDING,   // transferring data and sending
//     WFD_SEND_OK,    // all data send OK
//     WFD_SEND_FAILED
//   } WifiSending;
//   WifiStatus status = WFS_NONE;
//   WifiReply replyType = WFI_FINISHED;
//   WifiSending sendingState = WFD_NONE;
//   // debug
//   static const int RX_BUF_SIZE = 110;
//   char serRxBuf[RX_BUF_SIZE]; // wifi connection
//   int serRxBufCnt = 0;
//   
//   /// save wifi settings to ee prom
//   void eePromSaveWifi();
//   /// load wifi settings from ee-prom
//   void eePromLoadWifi();
//   /// debug flag to send all characters received from 8266 to USB connection
//   bool echo8266toUSB = false;
//   // is wifi disabled
//   bool wifiActive = false;
//   // is in sleep mode
//   bool goingToSleep = true; // default is not to use wifi
//   bool inSleep = false;
// 
//   uint8_t wifiConnectTryCount = 0;
//   
// protected:
//   uint8_t replyState = 0;
//   uint32_t wifiWait = 0;
//   uint32_t wifiBusy = 0;
//   uint8_t wifiFailCnt = 0;
//   uint8_t wifiIP[4]; // IP number from wifi
//   char wifiMAC[18] = "00:00:00:00:00:00"; // STATIC MAC as string
//   uint16_t portNumber = 24001;
// //  bool wifiWaitForSendOK = false;
//   uint32_t wifiMsgGood = 0, wifiMsgLost = 0;
//   //float wifiLoss = 0.0;
// //  uint32_t wifiWaitForSendOKtime;
//   static const int MAX_SSID_SIZE = 16;
// //  char wifiSSID[MAX_SSID_SIZE] = "WaveLAN IAU";
//   char wifiSSID[MAX_SSID_SIZE] = "device";
//   char wifiPW[MAX_SSID_SIZE] = "";
//   
//   ////////////////
// // private:
// //   uint32_t wifiTiming = 0;
// //   uint32_t wifiDelayMax = 0;
// //   uint32_t wifiDelayMax2 = 0;
// private:
//   int dataCharsCnt = 0;
// };


#endif
