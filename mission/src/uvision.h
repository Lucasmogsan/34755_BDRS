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


#ifndef UVISION_H
#define UVISION_H

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

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
// forward declaration

class UVision{
  
public:
  /** setup and request data */
  void setup(int argc, char **argv);
  /** decode an unpacked incoming messages
   * \returns true if the message us used */
  bool decode(char * msg);
  /**
   * Stream to screen - if any screen is available */
  bool processImage(float seconds);
  /**
   * Close camera */
  void stop();
  /// camera open flag
  bool camIsOpen = false;
  /// stream images to client (e.g. over ssh)
  bool saveImage = false;
  /// Shutting down
  bool terminate = false;
  /// show image to X-terminal
  bool showImage = false;
  /**
    * images for manual function using slider */
  cv::Mat dest;
  cv::Mat source;
  int slider1;
  int slider2;
  /**
   * focal length for Sandberg camera in pixels */
  const int focalLength = 1008;
  const float golfBallDiameter = 0.043; // meter
  /**
   * camera position in robot coordinates (x (forward), y (left), z (up)) */
  const float camPos[3] = {0.13,-0.02, 0.23};       // in meters
  const float camTilt = 22 * M_PI / 180; // in radians
  cv::Mat1f camToRobot;
//   const float st = sin(camTilt);
//   const float ct = cos(camTilt);
//   const cv::Mat1f camToRobot( cos(camTilt),  0.f, st, camPos[0];
//                                          0.f ,  1.f, 0.f , camPos[1];
//                                          -st, 0.f, ct, camPos[2];
//                                          0.f ,  0.f, 0.f , 1.f);
  
private:
  /// buffer for captured image
  cv::Mat frame;
  /// openCV video capture function
  cv::VideoCapture cap;
  /// thread to keep buffer empty
  thread * listener = NULL; /// thread for listen loop
  static void startloop(UVision * vision); /// To spawn the listen loop as a separate thread, it needs to be static
  void loop(); /// endless loop keeping image buffer empty
  bool getNewestFrame(); /// function to get a fresh frame (newest) from camera  
  bool useFrame = false; /// flag to transfer newest image to 'frame' buffer
  bool gotFrame = false; /// flag for the newest image is available in 'frame'
  int frameSerial = 0;
  mutex dataLock;
  //
  //
  bool findBall = false;
  bool doFindBall();
  cv::Mat debugImg;
  /**
   * Bounding boc for found balls */
  vector<cv::Rect> ballBoundingBox;
  void ballProjectionAndTest();
  //
  bool findAruco = false;
  bool doFindAruco();
  // 
  /**
   * Calculates color distance between pix and col.
   * distance 0 is total match, else block distance for U and V only
   * \returns d = |du| + |dv| */
  int uvDistance(cv::Vec3b pix, cv::Vec3b col);
  
};

/**
 * Make this bridge connection visible to the rest of the software */
extern UVision vision;

#endif
