/*  
 * 
 * Copyright © 2022 DTU, Christian Andersen jcan@dtu.dk
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


#ifndef UBRIDGE_H
#define UBRIDGE_H

#include <iostream>
#include <sys/time.h>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <thread>
#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>

using namespace std;
// forward declaration

class UBridge{
  
public:
  /** setup and connect to this socket
   * also starts the listen loop */
  void setup(const char ip[], const char port[], int argc, char **argv);
  /**
   * Shutdown connection */
  ~UBridge();
  /**
   * Send command lines to hardware (via bridge) */
  void tx(const char * msg);
  /** Stop connection to bridge */
  void stop(); 
  
public:
  /// connected to hardware through bridge 
  bool connected;
  const char * host; /// host string
  const char * hostport; /// port string
  bool terminate = false; // shutdown flag
  
private:
  addrinfo * servinfo = nullptr; /// socket info
  int sockfd; /// Socket file descriptor
  mutex sendMtx; /// to avoid too many are sending at the same time
  thread * listener = NULL; /// thread for listen loop
  static void startloop(UBridge * bridge); /// To spawn the listen loop as a separate thread, it needs to be static
  void loop(); /// endless loop listening for incoming
  /// unpack message and check CRC
  void unpackMessage(char * msg);
  /// distribute incoming messages for decoding
  void decode(char * msg);
  /// catch CTRL-C from keyboard
  struct sigaction sigIntHandler;
  // testflag to test code without the bridge (e.g. vision)
  bool usebridge = true;
};

/**
 * Make this bridge connection visible to the rest of the software */
extern UBridge bridge;

#endif
