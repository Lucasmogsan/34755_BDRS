
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


#ifndef UPLAY_H
#define UPLAY_H

#include <thread>
#include <mutex>
#include <unistd.h>

using namespace std;

/**
 * Class to start a thread that plays a 
 * sound track using default player (play).
 * The music runs until finished or when explicitly stopped.
 * \method start()   Start by calling 'start()'.
 * \method stopPlaying()  Stop process by calling 'stopPlaying()'.
 * */
class UPlay
{
public:
  /** destructor */
  ~UPlay()
  {
    stop();
  }
  /**
   * \param thisfile is the mp3 file - with full path - to play.
   * \param volume is an optional volume factor from 0 (silent) to 1 (full).
  * */
  void play(const char * thisfile, float volume = 0.01)
  {
    setVolume(volume);
    setFile(thisfile);
    start();
    usleep(10000);
  }
  /**
   * thread part, never call this directly, use start()
   * */
  virtual void run();
  void start();
  void stop();
  /**
   * Test if we are playing anything */
  bool isPlaying();
  /**
   * Name of file to play.
   * NB! the file must hard coded or in a percistent string.
   * */
  void setFile(const char * file);
  /**
   * Set volume (0..100) */
  void setVolume(float level);
  
protected:
  // default music.mp3 is a symbolic link to some music
  static const int MFL = 1500;
  char fileToPlay[MFL] = "/home/chr/Music/music.mp3";
  float volume = 0.1;
  thread * player = nullptr;
  static void startloop(UPlay * thisPlayer); /// To spawn the listen loop as a separate thread, it needs to be static
  int playingCount = 0;
};

/**
 * Another class to also speak a text. */

class USay : public UPlay
{
public:
  /**
   * Run the conversion from text to wav */
  void convertAndPlay();
  /**
   * Test if we are playing anything */
  bool isSaying();
  /**
   * Say this sentence, i.e. convert to a wav file and play this file 
   * \param sentence is the text to say.
   * \param volume is an optional volume factor from 0 (silent) to 1 (full).
   * \return false if playing is busy, else true */
  bool say(const char * sentence, float volume = 0.1);
    
protected:
  bool saying = false;
};

extern USay sound;

#endif
