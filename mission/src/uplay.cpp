
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


#include "uplay.h"
#include "utime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ubridge.h"


USay sound;

/**
 * Class to start a thread that plays a 
 * sound track using default player (play).
 * The music runs until finished or when explicitly stopped.
 * \method start()   Start by calling 'start()'.
 * \method stopPlaying()  Stop process by calling 'stopPlaying()'.
 * */
void UPlay::run()
{ // soundfile is in 'fileToPlay'
  const int MSL = 300;
  char s[MSL];
  UTime t;
  t.now();
  playingCount++;
  bridge.tx("regbot mute 0\n");
  // start the playing - with low priority (nice 14)
  int e = snprintf(s, MSL, "nice -n14 play -q -v%.3f %s", volume, fileToPlay);
  printf("# playing using system call: %s\n", s);
  // -v0.1 gives low amplitude (10%)
  system(s);
  if (t.getTimePassed() < 1)
  {
    printf("# ---- file not found? (err=%d) '%s'\r\n", e, fileToPlay);
    printf("-- playing mp3 requires libsox-fmt-mp3 or libsox-fmt-all to be installed. \n");
  }
  playingCount--;
  if (playingCount <= 0)
    // mute the amplifier, when all sounds are finished
    bridge.tx("regbot mute 1\n");
}

void UPlay::start()
{
  if (player == nullptr)
  {
    printf("#Creating player thread\n");
    player = new thread(startloop, this);
    printf("#Created player thread\n");
  }
}

void UPlay::startloop(UPlay* thisPlayer)
{
  printf("# Starting player\n");
  thisPlayer->run();
}


void UPlay::stop()
{
  if (player != nullptr)
  { // kill any play process
    system("pkill play");
    player->join();
    player = nullptr;
  }
}
  /**
   * Test if we are playing anything */
bool UPlay::isPlaying()
{
  int e = system("pgrep play");
  return e == 0;
}
  /**
   * Name of file to play.
   * NB! the file must hard coded or in a percistent string.
   * */
void UPlay::setFile(const char * file)
{
  strncpy(fileToPlay, file, MFL);
}
  /**
   * Set volume (0..100) */
void UPlay::setVolume(float level)
{
  volume = level;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

/**
 * Run the conversion from text to wav */
void USay::convertAndPlay()
{ // convert 
  int e = system("nice -n13 text2wave aa.txt -o aa.wav\n");
  if (e == 0)
  {
//       printf("USay:: all is fine\n");
    setFile("aa.wav");
    UPlay::run();
  }
  else
    printf("USay:: text2wave returned %d\n", e);
  saying = false;
}
/**
 * Test if we are playing anything */
bool USay::isSaying()
{
  if (saying)
    return isPlaying();
  else
    return false;
}
/**
  * Say this sentence, i.e. convert to a wav file and play this file 
  * \param sentence is the text to say.
  * \param volume is an optional volume from 0% (silent) to 100%.
  * \return false if playing is busy, else true */
bool USay::say(const char * sentence, float volume)
{
  bool isOK = false;
  if (strlen(sentence) > 2 and not saying)
  {
    saying = true;
    FILE * a;
    setVolume(volume);
    a = fopen("aa.txt", "w");
    if (a != NULL)
    {
      fprintf(a, "%s\n", sentence);
      fclose(a);
      convertAndPlay();
      isOK = true;
    }
    else
    {
      printf("# USay::say: failed to save sentence to play\n");
      saying = false;
    }
  }
  else if (saying)
    printf("USay:: is busy (can not say '%s')\n", sentence);
  return isOK;
}
  
  
