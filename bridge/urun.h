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

#include <stdio.h>

#ifndef URUN_H
#define URUN_H
 
#include <thread>
 
/**
 * calculatin difference between two timestamps as a float - in seconds
 * \param newest is a timestamp created with gettimeofday(&timeval) of type timeval (defined in \<sys/time.h> ))
 * \param oldest is a timestamp created with gettimeofday(&timeval) of type timeval (defined in \<sys/time.h> ))
 * \returns difference in decimal seconds
 * */
// float getTimeDiff(timeval newest, timeval oldest);

inline int mini(int a, int b)
{
  if (a < b)
    return a;
  else
    return b;
}

inline int maxi(int a, int b)
{
  if (a > b)
    return a;
  else
    return b;
}


class URun;

/** start a thread in the provided object */
void runObj(URun * obj);

/**
 * Base class, that makes it easier to starta thread
 * from the method runObj
 * The run method should be overwritten in the real class */
class URun
{
public:
  URun()
  {
    th1 = nullptr;
    th1stop = true;
  }
  
  ////////////////////////////////////////////////
  
  virtual ~URun()
  {
//     printf("instance of run stopping\n");
    stop();
  }
  
  ////////////////////////////////////////////////

  static void runObj(URun * obj)
  { // called, when thread is started
    // transfer to the class run() function.
    obj->run();
    // call stop to ensure that start can be called again
    // in case the run loop has stopped by user code
    if (obj->isRunning())
      obj->stop();
  }
  
  ////////////////////////////////////////////////

  virtual void run()
  {
    printf("Should not show\n");
  }
  
  ////////////////////////////////////////////////
  /**
   * Start the thread and run until thStop is set */
  
  bool start()
  {
    th1stop = false;
    if (th1 == nullptr)
      th1 = new std::thread(runObj, this);
    return th1 != nullptr;
  }
  
  virtual void stop()
  {
    th1stop = true;
    if (th1 != nullptr)
    {
      if (th1->joinable())
        th1->join();
      delete th1;
      th1 = nullptr;
    }
  }
  
  bool isRunning()
  {
    return not th1stop;
  };
  
private:
  // read thread handle
  std::thread * th1;
  // set true to stop thread
  bool th1stop;
};


#endif
