/*
 * Oled 0.9" display interface
 ***************************************************************************
 *   Copyright (C) 2017 by DTU (Christian Andersen)                        *
 *   jca@elektro.dtu.dk                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Lesser General Public License for more details.                   *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <math.h>

#include "uoled.h"
#include "ArduiPi_OLED_lib.h"
#include "Adafruit_GFX.h"
#include "ArduiPi_OLED.h"
#include "ArduiPi_OLED_lib.h"


// Config Option
struct s_opts
{
  int oled;
  int verbose;
} ;

//int sleep_divisor = 1 ;

// default options values
s_opts opts = {
  OLED_ADAFRUIT_SPI_128x64,  // Default oled 0.95"
//  OLED_SH1106_SPI_128x64,    // 1.3" display
  true                                                                         // Not verbose
};

// #define NUMFLAKES 10
// #define XPOS 0
// #define YPOS 1
// #define DELTAY 2

// #define LOGO16_GLCD_HEIGHT 16 
// #define LOGO16_GLCD_WIDTH  16 


UOled::UOled()
{
  displayFound = false;
  displayIP = true;
  lastIpCnt = 0;
  gethostname(hostname, MHL);
  // init and say -booting
  initDisplay();
  // start display IP thread
  for (int i = 0; i < MAX_OLED_LINES; i++)
    oledLines[i][0] = '\0';
  oldTempTime.now();
  start();
}

UOled::~UOled()
{
  printf("closing o-led\n");
  if (displayFound)
    // Free PI GPIO ports
    display->close();
  // deallocate display
  free(display);
}


bool UOled::initDisplay()
{
  displayFound = false;
#ifdef armv7l
  display = new ArduiPi_OLED();
  //opts.oled = OLED_SH1106_SPI_128x64; // default option (128x64)
  // SPI change parameters to fit to your LCD
  displayFound = display->init(OLED_SPI_DC,OLED_SPI_RESET,OLED_SPI_CS, opts.oled);
  if (not displayFound)
  {
    printf("UOled::initDisplay SPI access failed - aborting\n");
  }
#endif
  if (displayFound)
  {  // display setup using SPI connection
    printf("Oled is: %s\n", oled_type_str[opts.oled]);
    //
    display->begin();
    // init done - clear local buffer
    display->clearDisplay();   // clears the screen  buffer
    // print welcome
    display->setTextSize(1);
    display->setTextColor(WHITE);
    display->setCursor(0,0);
    display->printf("%s booting\n", hostname);
    display->display();        // display it (clear display)
  }
  else
  {
    printf("UOled::initDisplay - no dispaly found - using console\n");
  }
  return displayFound;
}


void UOled::displayIPs()
{
  int ipsCnt;
  const int MIPSLEN = 100;
  struct ifaddrs * ifAddrStruct=NULL;
  struct ifaddrs * ifa=NULL;
  void * tmpAddrPtr=NULL;
  //  
  getifaddrs(&ifAddrStruct);
  ipsCnt = 0;
  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) 
  {
    // printf("found IP %d\n", ipsCnt);
    if (ifa->ifa_addr->sa_family == AF_INET ) 
    { // is a valid IP4 Address
      if (ips[ipsCnt] == NULL)
        ips[ipsCnt] = (char*)malloc(MIPSLEN);
      tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
      if (strcmp(ifa->ifa_name, "lo") != 0)
      { // not loop back - loopback is skipped
        snprintf(ips[ipsCnt], MIPSLEN, "%s", addressBuffer); 
        if (ipsCnt < MIPS - 1)
          ipsCnt++;
      }
      //
      //printf("#### found IP %d: %s %s\n", ipsCnt, ifa->ifa_name, addressBuffer);
      //
    } 
    else // if (ifa->ifa_addr->sa_family == AF_INET6) 
    { // check it is IP6
      // is a valid IP6 Address
      //         printf("ip V6 - not used\n");
      // not used
    } 
  }
  if (ipsCnt != lastIpCnt)
  { // display if relevant only
    //     if (ifAddrStruct!=NULL) 
    if (displayFound)
    {
      display->clearDisplay();   // clears the screen  buffer
      display->display();        // display it (the clear display)
//       display->setTextSize(1);



      display->setTextColor(WHITE);
//       display->setCursor(0,0);
//       display->printf("%s\n", hostname);
    }
    displayVoltageAndName(0.0, 0.0, 0.0);
    //
//     fprintf(stdout, "-----------------------\n");
//     fprintf(stdout, "Sending to display:\n");
//     fprintf(stdout, "%s\n", hostname);
    int maxIPcnt = ipsCnt;
    if (ipsCnt < lastIpCnt)
      maxIPcnt = lastIpCnt;
    for (int i = 0; i < maxIPcnt; i++)
    { /*// limit 10 chars in name
      ips[i][10] = '\0';*/
      if (i >= ipsCnt)
        ips[i][0] = '\0';
      printLine(i+1, ips[i]);
      fprintf(stdout, "%d %s", i, ips[i]);
    }
    lastIpCnt = ipsCnt;
    //     if (displayFound)
//       display->display();
//     fprintf(stdout, "-----------------------\n");
  }
}


void UOled::run()
{
  int loop = 0;
  clear();
  while (not th1stop)
  {
    if (displayIP)
    {
      displayIPs();
    }
    sleep(1);
    if (loop > 4 and not displayFound)
      // stop looking for display, terminate this thread
      th1stop = true;
    loop++;
  }    
}

void UOled::clear()
{
  if (displayFound)
  {
    display->clearDisplay();   // clears the screen  buffer
    for (int i = 0; i < MAX_OLED_LINES; i++)
    {
      oledLines[i][0] = '\0';
      display->printf("%s\n", oledLines[i]);
    }
    display->display();        // display it (the clear display)
    displayIP = true;
    lastIpCnt = -1;
    displayIPs();
    redisplay();
  }
}

void UOled::redisplay()
{
  if (displayFound)
  {
    display->clearDisplay();   // clears the screen  buffer
    display->setCursor(0, 0);
    for (int i = 0; i < MAX_OLED_LINES; i++)
      display->printf("%s\n", oledLines[i]);
    display->display();        // display it (the clear display)
    //
    std::printf("-------on OLED---------\n");
  }
  else
  {
    std::printf("-----no display------\n");
  }
  for (int i = 0; i < MAX_OLED_LINES; i++)
    std::printf("%d %s\n", i, oledLines[i]);
  std::printf("-----------------------\n");
}


void UOled::printLine(int lineNumber, const char* lineText)
{
//   displayIP = false;
  if (lineNumber >= 0 and lineNumber < MAX_OLED_LINES)
  {
    strncpy(oledLines[lineNumber], lineText, MAX_OLED_CHARS);
    oledLines[lineNumber][MAX_OLED_CHARS-1] = '\0';
  }
  redisplay();
}

void UOled::displayVoltageAndName(float batteryVoltage, float time, float cpuTemp)
{
//   printf("UOled::displayVoltageAndName: setting to %fV\n", batteryVoltage);
  if (fabsf(batteryVoltage - oldBatteryVoltage) > 0.05 or
    (fabsf(cpuTemp - oldTemperature) > 1 and oldTempTime.getTimePassed() > 10)    )
  {
    snprintf(oledLines[0], MAX_OLED_CHARS, "%.1fV %.0fC %s ", batteryVoltage, cpuTemp, hostname);
//     snprintf(oledLines[2], MAX_OLED_CHARS, "%.1f", time);
    // limit charcount in name
    oledLines[0][MAX_OLED_CHARS - 1] = '\0';    
    oldBatteryVoltage = batteryVoltage;
    oldTemperature = cpuTemp;
    oldTempTime.now();
    oldRegbotTime = time;
    redisplay();
    if (batteryVoltage < 10.2 and batteryVoltage > 6 and (time - batteryTime) > 20)
    {
        const int MSL=100;  
        char s[MSL];
        snprintf(s, MSL, "espeak \"%s say: battery voltage is low %.1f\" -ven+f4 -s130 -a20 &", hostname, batteryVoltage); 
        system(s);
        batteryTime = time;
    }
  }
}

