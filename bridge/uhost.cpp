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
#include <netpacket/packet.h>
#include <unistd.h>
#include <math.h>

#include "uhost.h"
#include "uhandler.h"
#include "ubridge.h"
#include <linux/wireless.h>
#include <sys/ioctl.h>


UHostIp hostip;


void UHostIp::setup()
{
  setSourceID("host");
  displayIP = true;
  lastIpCnt = 0;
  gethostname(hostname, MHL);
  char s1[] = "ip name Host v4 IP";
  handler.handleCommand(this, s1, true);
  char s[] = "temp name CPU temperature.";
  handler.handleCommand(this, s, true);
  // start regular update
  start();
}


bool UHostIp::check_wireless(const char* ifname, char* protocol) 
{
  int sock = -1;
  struct iwreq pwrq;
  memset(&pwrq, 0, sizeof(pwrq));
  strncpy(pwrq.ifr_name, ifname, IFNAMSIZ);
  bool isWiFi = false;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
  {
    perror("UHostIp::check_wireless: socket");
  }
  else if (ioctl(sock, SIOCGIWNAME, &pwrq) != -1) 
  {
    if (protocol) 
      strncpy(protocol, pwrq.u.name, IFNAMSIZ);
    isWiFi = true;
  }
  if (sock >= 0)
    close(sock);
  return isWiFi;
}

bool UHostIp::findWifiMACs()
{
  struct ifaddrs * ifAddrStruct=NULL;
  struct ifaddrs * ifa=NULL;
  bool changed = false;
  //  
  getifaddrs(&ifAddrStruct);
  ipsCnt = 0;
  macCnt = 0;
  char temp[MHL2];
  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) 
  { // all interface names and types
    if (ifa->ifa_addr->sa_family == AF_PACKET ) 
    { // is a valid interface
      if (strcmp(ifa->ifa_name, "lo") != 0)
      { // not loop back - loop back is skipped
        // get MAC for this interface
        struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
        char protocol[IFNAMSIZ] = {'\0'};
        if (s->sll_halen > 0 and check_wireless(ifa->ifa_name, protocol))
        { // it has a MAC and is a wifi connection
          snprintf(temp, MHL, "%s %02x:%02x:%02x:%02x:%02x:%02x (%s)", ifa->ifa_name,
                    s->sll_addr[0], s->sll_addr[1], s->sll_addr[2],
                    s->sll_addr[3], s->sll_addr[4], s->sll_addr[5], protocol);
          if (strcmp(temp, macs[macCnt]) != 0)
          {
            changed = true;
            strncpy(macs[macCnt], temp, MHL2);
//             printf("#### found MAC %d: %s %s\n", macCnt, macs[macCnt], protocol);
          }
          if (macCnt < MIPS - 1)
            macCnt++;
        }
      }
      //
    }
//     if (macCnt == 0)
//       printf("# UHostIp::findWifiMACs: Found no wifi MACs\n");
  }
  return changed;
}

bool UHostIp::findIPs()
{
  struct ifaddrs * ifAddrStruct=NULL;
  struct ifaddrs * ifa=NULL;
  void * tmpAddrPtr=NULL;
  bool changed = false;
  //  
  getifaddrs(&ifAddrStruct);
  ipsCnt = 0;
  macCnt = 0;
  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) 
  { // all interface names and types
    if (ifa->ifa_addr->sa_family == AF_INET) 
    { // is a valid IP4 Address
      tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
      char addressBuffer[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
      if (strcmp(ifa->ifa_name, "lo") != 0)
      { // not loop back - loopback is skipped
        int sl = strlen(addressBuffer);
        if (sl > 0 and strncmp(ips[ipsCnt], addressBuffer, sl) != 0)
        {
          changed = true;
          snprintf(ips[ipsCnt], MHL, "%s", addressBuffer); 
          printf("#### found IP %d: %s %s\n", ipsCnt, ifa->ifa_name, ips[ipsCnt]);
        }
        if (ipsCnt < MIPS - 1)
          ipsCnt++;
      }
    }
  }
  return changed;
}


void UHostIp::run()
{
  int loop = 0;
  const int MSL = 100;
  char s[MSL];
  while (isRunning())
  {
    bool changed = findIPs();
    if (changed)
    { // update message
      updateIPlist();
    }
    changed = findWifiMACs();
    if (changed and macCnt > 0)
    { // make mac list message
      snprintf(maclist, MHL2, "mac %s\n", macs[0]);
      handler.handleCommand(this, maclist, true);  
      if (macCnt > 1)
        printf("# found more than 1 wifi interface! (ignored)\n");
    }
    // tell Regbot display about the host IP
    USource * regbot = bridge.findSource("regbot");
    if (regbot != nullptr)
    { // send display message with IP addresses to REGBOT
      snprintf(s, MSL, "regbot disp%s\n", ip4list);
      handler.handleCommand(this, s, true);
    }
    // measure CPU temperature
    snprintf(s, MSL, "temp %.2f\n", measureCPUtemp());
    handler.handleCommand(this, s, true);
    //
    loop++;
    // repeat every 2 seconds
    sleep(2);
  }    
}


void UHostIp::updateIPlist()
{
  const int MSL = 200;
  char s[MSL];
  // update IP list
  int n = 0;
  char * p1 = ip4list;
  for (int i = 0; i < ipsCnt; i++)
  {
    snprintf(p1, MHL2 - n, " %s", ips[i]);
    n += strlen(p1);
    p1 = &ip4list[n];
  }
  // make IP list message
  snprintf(s, MSL, "ip%s\n", ip4list );
  handler.handleCommand(this, s, true);  
}

float UHostIp::measureCPUtemp()
{
  FILE * temp;
  const char * tf = "/sys/class/thermal/thermal_zone0/temp";
  temp = fopen(tf, "r");
  float t = 0;
  if (temp != NULL)
  {
    const int MSL = 20;
    char s[MSL];
    char * p1 = s;
    int n = fread(p1, 1, 1, temp);
    int m = n;
    while (n > 0)
    {
      n = fread(++p1, 1, 1, temp);
      m += n;
    }
    s[m] = '\0';
    if (m > 3)
    {
      t = strtof(s, &p1);
    }
    //     printf("#got %d bytes (%g deg) as:%s\n", m, t/1000.0, s); 
    fclose(temp);
  }
  return t/1000.0;
}
