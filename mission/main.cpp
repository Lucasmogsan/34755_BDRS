/*  
 
 Copyright © 2022 DTU, Christian Andersen jcan@dtu.dk
 
 The MIT License (MIT)  https://mit-license.org/
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the “Software”), to deal in the Software without restriction, 
 including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software 
 is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies 
 or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
 FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 THE SOFTWARE. */


#include <iostream>
#include "src/ubridge.h"
#include "src/uvision.h"
#include "src/upose.h"
#include "src/ucomment.h"
#include "src/ustate.h"
#include "src/uplay.h"
#include "src/uevent.h"
#include "src/ujoy.h"

// to avoid writing std:: 
using namespace std;


bool setup(int argc, char **argv)
{ // check for command line parameters
  for (int i = 1; i < argc; i++)
  { // check for command line parameters
    // for process debug
    if (strcmp(argv[i], "help") == 0)
    { 
      printf("-----\n# User mission command line help\n");
      printf("# usage:\n#   ./user_mission [help] [ball] [show] [aruco] [videoX]\n-----\n");
      return false;
    }
  }
  // connect to robot hardware using bridge
  bridge.setup("127.0.0.1", "24001", argc, argv);
  if (true or bridge.connected)
  {  /// call setup for data structures
    pose.setup();
    comment.setup();
    state.setup();
    vision.setup(argc, argv);
    event.setup();
    joy.setup();
    printf("# Setup finished OK\n");
  }
  else
    printf("# Setup failed\n");
  sound.say("me?... I am a depressed robot.", 0.1);
//   while (sound.isSaying())
//     usleep(100000);
  // a bit of background music
  sound.play("/home/local/Music/music.mp3", 0.05);
  return true;
}

void step1()
{
  sound.say(". Step one.", 0.3);
  // remove old mission
  bridge.tx("regbot mclear\n");
  // clear events received from last mission
  event.clearEvents();
  // add mission lines
  bridge.tx("regbot madd vel=0.2:time=1\n");
  bridge.tx("regbot madd tr=0.1:time=1,turn=-90\n");
  bridge.tx("regbot madd :time=1\n");
  // start this mission
  bridge.tx("regbot start\n");
  // wait until finished
  //
  cout << "Waiting for step 1 to finish (event 0 is send, when mission is finished)\n";
  event.waitForEvent(0);
//   sound.say(". Step one finished.");
}

void step2()
{
//   sound.say(". Step two. Press button two for right, button three for left", 0.3);
  bool go_left = true;
  while (true)
  { // wait for decision (button 2 (right) or 3 (left))
    if (joy.button(2))
    {
      go_left = false;
      sound.say(". OK. Going right.", 0.3);
      break;
    }
    else if (joy.button(3))
    {
      go_left = true;
      sound.say(". OK. Going left.", 0.3);
      break;
    }
    else // wait
      usleep(5000);
  }
  // remove old mission
  bridge.tx("regbot mclear\n");
  // clear events received from last mission
  event.clearEvents();
  // add mission lines
  if (go_left)
    bridge.tx("regbot madd vel=0.2,tr=0.1:turn=90\n");
  else
    bridge.tx("regbot madd vel=0.2,tr=0.1:turn=-90\n");
  // drive a bit straight for correct end heading
  bridge.tx("regbot madd :dist=0.2\n"); 
  // start this mission
  bridge.tx("regbot start\n");
  // wait until finished
  cout << "Waiting for step 1 to finish (event 0 is send, when mission is finished)\n";
  event.waitForEvent(0);
  sound.say(". Step two finished.");
}

int main(int argc, char **argv) 
{
  if (setup(argc, argv))
  { // start mission
    std::cout << "# Robobot mission starting ...\n";
    //
    step1();
    step2();
    //
    std::cout << "# Robobot mission finished ...\n";
    // remember to close camera
    vision.stop();
    sound.say("I am finished... sorry danish.", 0.2);
    while (sound.isSaying())
      sleep(1);
    bridge.tx("regbot mute 1\n");
  }
  return 0;
}
