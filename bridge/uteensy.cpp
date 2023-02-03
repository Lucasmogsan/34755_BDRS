/**
 * Interface with Teensy
 *  
 * ************************************************************************
*   Copyright (C) 2017-2022 by DTU (Christian Andersen)                        *
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

#include <sys/time.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <termios.h>

#include "uteensy.h"
#include "uhandler.h"
#include "udataitem.h"
#include "ubridge.h"

using namespace std;

UTeensy teensy1;
UTeensy teensy2;

// declare the static variable in the UTeensy class
int UTeensy::usbDevIsOpen[MAX_USB_DEVS] = {false};
mutex UTeensy::usbDevOpenList;

/** constructor */
// UTeensy::UTeensy(/*UBridge * bridge*/) // : USource(bridge)
// {
//   teensyConnectionOpen = false;
//   botComTx = NULL;
//   botComRx = NULL;
//   start();
// }
/** destructor */
UTeensy::~UTeensy()
{ // stop all activity before close
  // is done in run()
  closeCommLog();
}

void UTeensy::setup(int usbDevNum, int simport, char * simhost, const char * id)
{
  teensyConnectionOpen = false;
  botComTx = NULL;
  botComRx = NULL;
  usbdeviceNum = usbDevNum;
  simHost = simhost;
  simPort = simport;
  setSourceID(id);
  sourceNum = -2; 
//   if (shouldBeActive)
//     start();
  initialized = true;
}


void UTeensy::openCommLog(const char * path)
{
  if (botComRx == NULL)
  {
    const int MLL = 200;
    char l[MLL];
    snprintf(l, MLL, "log_rx_%s", sourceID);
    botComRx = new ULogFile();
    botComRx->setLogName(l, "txt");
    botComRx->setLogPath(path);
    botComRx->openLog();
    if (botComRx->isOpen())
      printf("UTeensy::openCommLog - opened logfile %s\n", botComRx->getLogFileName());
    snprintf(l, MLL, "log_tx_%s", sourceID);
    botComTx = new ULogFile();
    botComTx->setLogName(l, "txt");
    botComTx->setLogPath(path);
    botComTx->openLog();
    if (botComTx->isOpen())
      printf("UTeensy::openCommLog - opened logfile %s\n", botComTx->getLogFileName());
  }
}

void UTeensy::closeCommLog()
{
  if (botComRx != NULL)
  {
    delete botComRx;
    botComRx = NULL;
  }
  if (botComTx != NULL)
  {
    delete botComTx;
    botComTx = NULL;
  }
}

/**
  * send a string to the serial port */
void UTeensy::sendString(const char * message, int msTimeout)
{ // this function may be called by more than one thread
  // so make sure that only one send at any one time
  int t = 0;
  bool lostConnection = false;
  const char * cmd = message;
  // remove any source information as this is not relevant for the Teensy
  const char * p1 = cmd;
  while (*p1 > ' ')
  { // loop for a ':' before any space
    if (*p1 == ':')
    {// is a source specifier, remove this part
      p1++;
      cmd = p1;
      break;
    }
    p1++;
  }
//   if (cmd[0] == '#')
//     printf("# UTeensy::sendString: trying to send '%s' to Teensy\n", cmd);
// is not called by interrupts, so no need for now
//   txLock.lock();
  if (teensyConnectionOpen and cmd[0] != '#')
  { // simulator or not
    int n = strlen(cmd);
    // add CRC code
    const int MCL = 4;
    char crc[MCL];
    const char * p1 = cmd;
    bool gotNewline = false;
    int sum = 0;
    for (int i = 0; i < n; i++)
    { // do not count \t, \r, \n etc
      // as these gives problems for systems with auto \n or \n\r or similar
      if (*p1 >= ' ')
        sum += *p1;
      if (*p1 == '\n')
        gotNewline = true;
      p1++;
    }
    snprintf(crc, MCL, ";%02d", (sum % 99) + 1);
    //
    sendLock.lock();
    sendCnt++;
    if (socket.connected)
    { // send string to socket (REGBOT simulator)
      int m = socket.sendData(crc);
      m = socket.sendData(cmd);
      if (m < n)
        printf("# failed to send all data to simulator, send %d/%d of '%s'\n", m, n, cmd);
      if (not gotNewline)
        socket.sendData("\n");
    }
    else
    { // try send string to USB
      int m = write(usbport, crc, 3);
      int d = 0;
      while ((d < n) and (t < msTimeout))
      { // want to send n bytes to usbport within timeout period
        m = write(usbport, &cmd[d], n - d);
        if (m < 0)
        { // error - an error occurred while sending
          switch (errno)
          { // may be an error, or just nothing send (buffer full)
            case EAGAIN:
              //not all send - just continue
              //printf("UnblockedIo::blockSend: waiting - nothing send %d/%d\n", d, length);
              usleep(1000);
              t += 1;
              break;
            default:
              perror("UTeensy::send (closing connection): ");
              lostConnection = true;
              break;
          }
          // dump the rest on most errors
          if (lostConnection)
            break;
        }
        else
          // count bytes send
          d += m;
      }
      if (not gotNewline)
        write(usbport, "\n", 1);
    }
    sendLock.unlock();
    // debug logging
    {
      UTime t;
      t.now();
      if (botComTx != NULL)
        botComTx->toLog(t, cmd);
    }
  }
  // 
  //usleep(4000); // Short sleep helps communication be more reliable when many command lines are send consecutively.
//   txLock.unlock();
  if (lostConnection)
  {
    close(usbport);
    teensyConnectionOpen = false;
    usbDevIsOpen[usbdeviceNum] = false;
    justConnected = false;
    usbport = -1;
  }
  else
    lastTxTime.now();
}

////////////////////////////////////////////////////////////////////////

/**
  * receive thread */
void UTeensy::run()
{ // read thread for REGBOT messages
  int n = 0;
  rxCnt = 0;
  bool err = false;
  int readIdleLoops = 0;
  UTime t;
  t.now();
  // get robot name
  while (isRunning())
  {
//     if (handler == nullptr)
//     { // wait until handler is ready
//       sleep(1);
// //       printf("Teensy %d %s started - no handler yet\n", source, sourceID);
//       continue;
//     }
    if ((teensyConnectionOpen and not gotActivityRecently and lastRxTime.getTimePassed() > 10) or
        (justConnected and justConnectedTime.getTimePassed() > 20.0))
    { // connection timeout or failed to get connection name within 10 seconds, probably a wrong device
      // - shut down connection and try another
      teensyConnectionOpen = false;
      printf("# UTeensy::run - no relevant activity, shutting down %s\n", sourceID);
      printf("# UTeensy::run but open=%d, gotAct=%d, lastTime=%f, just=%d, justTime=%g\n",
             teensyConnectionOpen, gotActivityRecently, lastRxTime.getTimePassed(), justConnected, justConnectedTime.getTimePassed());
      sendStr("leave\n"); // stop streaming
      sendStr("disp disconnected\n");
      // don't close while sending
      sendLock.lock();
      if (socket.connected)
        socket.closeSocket();
      else
      { // close USB
        teensyConnectionOpen = false;
        close(usbport);
        usbport = -1;
        // mark this device as not relevant if we never got an ID (still in 'justConnected' mode)
        if (justConnected)
          usbDevIsOpen[usbdeviceNum] = 2;
        else
          usbDevIsOpen[usbdeviceNum] = false;
        // try a new number next time
        usbdeviceNum = (usbdeviceNum + 1) % MAX_USB_DEVS;
        justConnected = false;
      }
      sendLock.unlock();
      // try another device
      usbdeviceNum = (usbdeviceNum + 1) % MAX_USB_DEVS;
    }
    else if (not teensyConnectionOpen)
    { // wait a second (or 2)
//       printf("Teensy connection is not open - %d trying to open %s\n", usbdeviceNum, sourceID);
      sleep(1);
      // then try to connect
      openToTeensy();
      if (teensyConnectionOpen) 
      { // request base data
//         printf("# UTeensy::run - just connected to %s\n", sourceID);
        justConnected = true;
        justConnectedTime.now();
        initMessageTypes();
        // assume there is activity - in order not to
        // get an error right away
        gotActivityRecently = true;
        lastRxTime.now();
      }
    }
    else
    { // we are connected
      if (justConnected and t.getTimePassed() > 0.5)
      { // no name is received yet, so try again
        // justconnected flag is cleared when receiving a 'dname' message from Teensy
        // debug
        printf("# UTeensy::run: just connected: sending 'idi' to teensy at %s\n", getTeensyDeviceName(usbdeviceNum));
        // debug end
//         sendLock.lock();
        sendStr("idi\n");
        sendStr("sub hbt 499\n");
//         sendLock.unlock();
        t.now();
      }
      if (gotActivityRecently and lastRxTime.getTimePassed() > 2)
      { // are loosing data - may be just temporarily
        gotActivityRecently = false;
      }
      if (socket.connected)
      { // from simulator
        n = socket.readChar(&rx[rxCnt], &err); // read(usbport, &rx[rxCnt], 1);
        if (err)
        { // read failed (other than just no data)
          perror("Teensy::run socket port error");
          usleep(100000);
          sendLock.lock();
          // don't close while sending
          teensyConnectionOpen = false;
          socket.closeSocket();
          sendLock.unlock();
        }
        else
          gotActivityRecently = true;
      }
      else
      { // from USB
        n = read(usbport, &rx[rxCnt], 1);
        if (n < 0 and errno == EAGAIN)
        { // no data
          n = 0;
        }
        else if (n < 0)
        { // other error - close connection
          perror("Teensy::run port error");
          usleep(100000);
          sendLock.lock();
          // don't close while sending
          teensyConnectionOpen = false;
          close(usbport);
          sendLock.unlock();
          usbport = -1;
        }
      }
      if (n == 1)
      { // got a new character
        if (rxCnt > 0 or (rxCnt == 0 and rx[0] == ';'))
        {
          rxCnt++;
//           printf("# UTeensY::run: got new line\n");
        }
        if (rx[rxCnt-1] == '\n')
        { // terminate string
          rx[rxCnt] = '\0';
//           printf("# UTeensy::run: *** got:%s\n", rx);
          // handle new line of data
          // debug
          {
            UTime t;
            t.now();
            if (botComRx != NULL)
            {
              printf("# Teensy::run - is logging?\n");
              botComRx->toLog(t, rx);
            }
          }
//           decodeFromTeensy(rx);
          handler.handleCommand(this, rx, false);
          gotActivityRecently = true;
          lastRxTime.now();
          // reset receive buffer
          rxCnt = 0;
          n = 0;
          gotCnt++;
        }
      }
      else if (n == 0)
      {  // no data, so wait a bit
        usleep(1000);
        readIdleLoops++;
      }
      else
      { // debug n!= 0 and n!= 1
        printf("# Teensy::run: got n=%d chars, when asking for 1!\n", n);
      }
    } // connected
  }
  if (teensyConnectionOpen)
  {  // stop any active subscriptions
//     sendLock.lock();
    sendStr("leave\n");
//     sendLock.unlock();
    printf("#sending 'leave' to %s (%s)\n", sourceID, getTeensyDeviceName(usbdeviceNum));
  }
  if (socket.connected)
    socket.closeSocket();
  else if (usbport != -1)
  {
    close(usbport);
    usbport = -1;
  }
  teensyConnectionOpen = false;
//   printf("Teensy:: thread stopped and port closed\n");
}  

/**
  * Open the connection.
  * \returns true if successful */
bool UTeensy::openToTeensy()
{
  if (simPort > 1000)
  { // open connection to simulator
    string s = to_string(simPort);
    socket.createSocket(s.c_str(), simHost);
    socket.tryConnect();
    teensyConnectionOpen = socket.connected;
  }
  else
  { // should use USB port
    //   printf("Regbot::openToRegbot opening\n");
//     printf("# UTeensy::openToTeensy %s (%d %d %d %d %d)\n", getTeensyDeviceName(usbdeviceNum),
//            usbDevIsOpen[0], usbDevIsOpen[1], usbDevIsOpen[2], usbDevIsOpen[3], usbDevIsOpen[4]);
    usbDevOpenList.lock();
    if (usbDevIsOpen[usbdeviceNum])
    { // move to next device
      printf("# Teensy::openToTeensy dev=%d is open already\n", usbdeviceNum);
      usbdeviceNum = (usbdeviceNum + 1) % MAX_USB_DEVS;
      usbport = -1;
      usbDevOpenList.unlock();
      return false;
    }
    else
    { // not open already - try
      usbDevIsOpen[usbdeviceNum] = true;
      printf("# Teensy::openToTeensy dev=%d is not open already - opening\n", usbdeviceNum);
      // make reservation
      usbport = open(getTeensyDeviceName(usbdeviceNum), O_RDWR | O_NOCTTY | O_NDELAY);
      if (usbport == -1)
      { // open failed
        usbDevIsOpen[usbdeviceNum] = false;
        if (connectErrCnt < MAX_USB_DEVS)
        { // don't spam with too many error messages
          const int MSL = 100;
          char s[MSL];
          snprintf(s, MSL, "# UTeensy::openToTeensy open %s failed (%d %d %d %d %d) :", getTeensyDeviceName(usbdeviceNum),
                   usbDevIsOpen[0], usbDevIsOpen[1], usbDevIsOpen[2], usbDevIsOpen[3], usbDevIsOpen[4]        
          );
          perror(s);
        }
        // move to next
        usbdeviceNum = (usbdeviceNum + 1) % MAX_USB_DEVS;
        //         printf("# UTeensy::openToTeensy next try %s (already open = %d, usbdeviceNum=%d)\n", 
//                   getTeensyDeviceName(usbdeviceNum), usbDevIsOpen[usbdeviceNum], usbdeviceNum);
        connectErrCnt++;
      }
      else
      { // set connection to non-blocking
        printf("# UTeensy::openToTeensy opened %s\n", getTeensyDeviceName(usbdeviceNum));
        int flags;
        if (-1 == (flags = fcntl(usbport, F_GETFL, 0)))
          flags = 0;
        fcntl(usbport, F_SETFL, flags | O_NONBLOCK);    
    // #ifdef armv7l
        struct termios options;
        tcgetattr(usbport, &options);
        options.c_cflag = B115200 | CS8 | CLOCAL | CREAD; //<Set baud rate
        options.c_iflag = IGNPAR;
        options.c_oflag = 0;
        options.c_lflag = 0;
        tcsetattr(usbport, TCSANOW, &options);
    // #endif
        tcflush(usbport, TCIFLUSH);
        connectErrCnt = 0;
      }
      teensyConnectionOpen = usbport != -1;
    }
    usbDevOpenList.unlock();
  }
  return teensyConnectionOpen;
}

/////////////////////////////////////////////////


void UTeensy::initMessageTypes()
{ // set data types
  char s4[] = "pose name Robot pose [time(sec), x(m), y(m), h(rad)].";
  handler.handleCommand(this, s4, true);
  char s5[] = "hbt name Heartbeat [time (sec), ID, version, battery voltage (V), ...].";
  handler.handleCommand(this, s5, true);
  const int MSL = 200;
  char s[MSL];
  snprintf(s, MSL, "%s name Keyword for communication with this Teensy connection (source %d)", sourceID, sourceNum);
  handler.handleCommand(this, s, true);
}

bool UTeensy::decode(const char * key, const char * params, USource * client)
{
  bool used =  false;
  USource * mee = this;
  if (strcmp(key, sourceID) == 0)
  { // send without the destination Keyword
    // debug
//     if (strcmp(key, "drive") ==0)
//       printf("# UTeensy::decode key='%s', params='%s', client=%s\n", key, params, client->sourceID);
    // debug end
    bool forDataMgmt = true;
    if (UDataItem::reservedKeyword(params))
    {
      if (strncmp(params, "logopen", 7) == 0)
      { // 
        openCommLog(bridge.getLogPath());
      }
      else if (strncmp(params, "logclose", 8) == 0)
        closeCommLog();
      else if (strncmp(params, "meta", 4) == 0)
      {
        const int MSL = 200;
        char s[MSL];
        snprintf(s, MSL, "# Teensy connected=%d, just connected=%d\n", teensyConnectionOpen, justConnected);
        client->sendStr(s);
      }
    }
    else
      forDataMgmt = false;
    if ((not forDataMgmt) or strncmp(params, "help", 4) == 0)
    { // the outgoing data for this device - including help
//       sendLock.lock();
      sendStr(params);
//       sendLock.unlock();
      used = true;
    }
  }
  else if (client == mee)
  {
//     printf("# UTeensy::decode: a message from this device %s %s\n", key, params);
    if (strcmp(key, "dname") == 0)
    {
//       printf("# UTeensy::decode: got %s %s", key, params);
      const char * p1 = params;
      while (*p1 > ' ')
        p1++;
      int n = p1 - params;
      if (n < MAX_ID_LENGTH)
      {
        strncpy(sourceID, params, n);
        sourceID[n] = '\0';
        if (justConnected)
        { // reload ini-file for this interface
          const int MSL = 50;
          char s[MSL];
          printf("# UTeensy::decode - just connected - got 'dname' reload-ini %s\n", sourceID);
          snprintf(s, MSL, "ini reload %s\n", sourceID);
          handler.handleCommand(this, s, true);
          //
          // potentially rename host request
          while (isspace(*p1))
            p1++;
          saveRobotName(p1);
          //
          justConnected = false;
        }
      }
    }
  }
  // debug
//   if (strcmp(key, "drive") == 0)
//     printf("# UTeensy::decode (not for mgmt end (used=%d)) key='%s', params='%s', client=%s\n", used, key, params, client->sourceID);
  // debug end
  return used;
}

void UTeensy::sendDeviceDetails(USource* toClient)
{
  const int MSL = 200;
  char s[MSL];
//   char s2[MSL];
  if (simPort > 1000)
    snprintf(s, MSL, "source %d %s:dev %d %s:%d %d %d %d %d %d\n", sourceNum, sourceID, isActive(), simHost, simPort, gotCnt, errCnt, sendCnt, botComRx!=nullptr, botComTx!=nullptr);
  else
    snprintf(s, MSL, "source %d %s:dev %d %s %d %d %d %d %d\n", sourceNum, sourceID, isActive(), getTeensyDeviceName(usbdeviceNum), gotCnt, errCnt, sendCnt, botComRx!=nullptr, botComTx!=nullptr);
  toClient->sendLock.lock();
  toClient->sendStr(s);
  toClient->sendLock.unlock();
//   toClient->sendStr("# device info format: id':dev' active device rxCnt errCnt sendCnt logRx logTx\n");
}


const char * UTeensy::getTeensyDeviceName(int devNum)
{
  snprintf(usbDevName, MAX_DEV_NAME_LENGTH, "/dev/ttyACM%d", devNum);
  return usbDevName;
}


void UTeensy::saveRobotName(const char * newName)
{
//   printf("# UTeensy::renameHostTest: hertil\n");
  FILE * sc;
  sc = fopen("robotname","w");
  fprintf(sc,"%s\n", newName);
  fclose(sc);
}

void UTeensy::activate(bool toActive)
{
  shouldBeActive = toActive;
  if (initialized and shouldBeActive and not isRunning())
    start();
  else if (initialized and isRunning() and not shouldBeActive)
    stop();
}

