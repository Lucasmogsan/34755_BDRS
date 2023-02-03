/***************************************************************************
*   Copyright (C) 2016 by DTU (Christian Andersen)                        *
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

#include <iostream>
#include <sys/time.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
// local files
#include "userverport.h"
#include "ujoy.h"
#include "uhandler.h"
// #include "uoled.h"
#include "ubridge.h"
#include "uconsole.h"
#include "uini.h"
#include "uhost.h"
#include "uros.h"

using namespace std;

char ** argvs;
int argcs;



////////////////////////////////////////////////////////////////////



void restart()
{
  printf("# Going to reboot ...\r\n\n\n\n");
  execve(argvs[0], argvs, NULL);
  printf("# Never here - this process should be overwritten by now.\n");
}



bool readCommandLineParameters(int argc, char ** argv, 
                               bool * deamon, 
                               int * port,
                               int * usbdev, int usbdevSize,
                               char jsdev[], int jsdevSize,
                               char simhost[], int simhostSize,
                               int * simport,
                               int * usbdevFront,
                               char filepath[], int filepathsize
)
{
  // are there mission parameters
  bool isHelp = false;
  *deamon = false;
  // default values
  *usbdev = 0;
  *usbdevFront = 1;
  strncpy(jsdev, "/dev/input/js0", usbdevSize);
  strncpy(simhost, "localhost", simhostSize);
//   for (int i = 0; i < argc; i++)
//   {
//     fprintf(stderr,"# %d %s\n", i, argv[i]);
//   }
  for (int i = 1; i < argc; i++)
  {
    if (isHelp)
      break;
    char * c = argv[i];
    if (c[0] == '-')
    {
      if (c[1] == '-')
        c++;
      switch (c[1])
      {
        case 'h':
          isHelp = true;
          break;
        case 'd':
          *deamon = true;
          break;
        case 'p':
          *port = strtol(argv[++i], NULL, 10);
          break;
        case 'i':
          *simport = strtol(argv[++i], NULL, 10);
          break;
        case 'u':
          *usbdev = strtol(argv[++i], nullptr, 10);
          break;
        case 'f':
          *usbdevFront = strtol(argv[++i], nullptr, 10);
          break;
        case 'l':
          strncpy(filepath, argv[++i], filepathsize);
          break;
        case 's':
          strncpy(simhost, argv[++i], simhostSize);
          break;
        case 'j':
          strncpy(jsdev, argv[++i], jsdevSize);
          break;
        default:
          printf("# Unused parameter '%s'\n", argv[i]);
          break;
      }
    }
    else
      printf("Parameter option should start with a '-', not '%s'\n", c);
  }
  // print config
  if (not isHelp)
  { // mostly debug
    printf("\n# Using the following parameters:\n");
    printf("# usbdev1 = /dev/ttyACM%d\n", *usbdev);
    printf("# usbdev2 = /dev/ttyACM%d\n", *usbdevFront);
    printf("# port    = %d\n", *port);
    printf("# gamepad = %s\n", jsdev);
    printf("# simhost = %s\n", simhost);
    printf("# simport = %d\n", *simport);
    printf("# daemon  = %d\n", *deamon);
    printf("# log+ini = %s\n", filepath);
    printf("# ------------------------\n");
  }
  return isHelp;
}


int main ( int argc,char **argv ) 
{ // save application name for restart
  argvs = argv;
  argcs = argc;
  bool asDaemon = false;
  bool help = false;
  const int MPL = 100;
  int usbDev;
  int usbDevFront;
  char simHost[MPL];
  char jsDev[MPL];
  int port = 24001;
  int simport = 0;
  const int MFPL = 200;
  char filepath[MFPL];
  snprintf(filepath, MFPL, "%s", std::getenv("PWD"));
  help = readCommandLineParameters(argc, argv, &asDaemon, &port, &usbDev, MPL, jsDev, MPL, simHost, MPL, &simport, &usbDevFront, filepath, MFPL);
  if (port == simport)
  {
    printf("#### Port can't be the same for simulator input %d and output %d!\n", simport, port);
    help = true;
  }
  if (help)
  { // command line help
    printf("#  \n");
    printf("# Bridge from a REGBOT to a socket connection\n");
    printf("# parameters:\n");
    printf("#  -h     : this help\n");
    printf("#  -d     : run as daemon - without listening to keyboard, default is %d\n", asDaemon);
    printf("#  -p N   : set output port number to N (default is %d)\n", port);
    printf("#  -i N   : set input  port number for simulator REGBOT, default is %d (use USB)\n", simport);
    printf("#  -s str : set input host name/IP for simulated REGBOT, default is '%s'\n", simHost);
    printf("#  -u str : set USB device to drive Teensy, default is '/dev/ttyACM%d'\n", usbDev);
    printf("#  -f str : set USB device to front Teensy, default is '/dev/ttyACM%d'\n", usbDevFront);
    printf("#  -j str : set joystick device name, default is '%s'\n", jsDev);
    printf("#  -l str : set set path for ini-file and logfiles, is '%s'\n", filepath);
    printf("#  \n");
  }
  else
  { // not help 
    handler.setup();
    ini.setup(filepath);
    dataa.setup(filepath);
    server.setup(port);
    bridge.setup();
    console.setup();
    joy.setup(jsDev);
    rosif.setup();
    hostip.setup();
    teensy1.setup(usbDev, simport, simHost, "ttyACM0");
    teensy2.setup(usbDev, simport, simHost, "ttyACM1");
    //
    // give names to some common data items
    handler.handleCommand(&console, (char*)"ini:# name Text message from ini-file loader", true);
//     handler.handleCommand(&console, (char*)"drive:# name Text message from drive interface", true);
//     handler.handleCommand(&console, (char*)"front:# name Text message from front interface", true);
    handler.handleCommand(&console, (char*)"bridge:# name Text message from bridge interface", true);
    handler.handleCommand(&console, (char*)"gamepad:# name Text message from gamepad interface", true);
    handler.handleCommand(&console, (char*)"console:# name Text message from console interface", true);
    //
    // load commands from ini-file
    ini.loadSystemIni();
    if (rosif.isActive())
      rosif.start();
    // Open connection if set to active
    teensy1.activate();
    teensy2.activate();
    // main loop
    sleep(2);
    console.run(asDaemon);
    //
    printf("Stopping ...\n");
    hostip.stop();  // stop checking for IP change
    joy.stop();     // delete joystick interface
    handler.stop(); // stop handling messages
    server.stop();  // delete socket server
    teensy1.stop();   // delete Teensy interface
    teensy2.stop();   // delete Teensy interface
    bridge.stop();  // stop the rest (database, items and subscriptions)
  }
  printf("Finished\n");
  if (restartBridge)
  {
    printf("Will restart in a moment\n");
    restart();
  }
}

