/***************************************************************************
 *   Copyright (C) 2014-2022 by DTU
 *   jca@elektro.dtu.dk            
 * 
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

#ifndef UMAT_H
#define UMAT_H

#include <stdint.h>
#include "main.h"

class UMatRot3x3
{
public:
  float d2w[3][3];
  float w2d[3][3];
  /**
   * Set rotation matrix as 
   * R = Ryaw * Rpitch * Rroll
   * \param all in radians, roll on x (fwd), pitch on y (left) yaw on z (up)
   */
  void set(float roll, float pitch, float yaw);
  /**
   * rotate a vector in drone coordinates to world coordinates
   * \param v in input values (x,y,z) x is fwd, y is left, z is up
   * \param u is u = R * v
   */
  void rotateD2W(float v[3], float u[3]);
  /**
   * rotate a vector in world coordinates to drone coordinates
   * - same as above, just with transposed matrix
   * \param v in input values (x,y,z) x is fwd, y is left, z is up
   * \param u is u = R' * v
   */
  void rotateW2D(float v[3], float u[3]);
  
};


#endif
