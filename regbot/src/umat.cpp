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

#include <math.h>
#include "umat.h"

void UMatRot3x3::rotateD2W(float v[3], float u[3])
{
  u[0] = v[0] * d2w[0][0] + v[1] * d2w[0][1] + v[2] * d2w[0][2];
  u[1] = v[0] * d2w[1][0] + v[1] * d2w[1][1] + v[2] * d2w[1][2];
  u[2] = v[0] * d2w[2][0] + v[1] * d2w[2][1] + v[2] * d2w[2][2];
}

void UMatRot3x3::rotateW2D(float v[3], float u[3])
{
  u[0] = v[0] * w2d[0][0] + v[1] * w2d[0][1] + v[2] * w2d[0][2];
  u[1] = v[0] * w2d[1][0] + v[1] * w2d[1][1] + v[2] * w2d[1][2];
  u[2] = v[0] * w2d[2][0] + v[1] * w2d[2][1] + v[2] * w2d[2][2];
}

void UMatRot3x3::set(float gamma, float beta, float alpha)
{
  float cg = cos(gamma);
  float sg = sin(gamma);
  float cb = cos(beta);
  float sb= sin(beta);
  float ca = cos(alpha);
  float sa = sin(alpha);
  d2w[0][0] = ca*cb;
  d2w[0][1] = ca*sb*sg - sa*cg;
  d2w[0][2] = ca*sb*cg + sa*sg;
  d2w[1][0] = sa*cb;
  d2w[1][1] = sa*sb*sg + ca*cg;
  d2w[1][2] = sa*sb*cg - ca*sg;
  d2w[2][0] = -sb;
  d2w[2][1] = cb*sg;
  d2w[2][2] = cb*cg;
  // and inverse (transposed)
  w2d[0][0] = d2w[0][0];
  w2d[0][1] = d2w[1][0];
  w2d[0][2] = d2w[2][0];
  w2d[1][0] = d2w[0][1];
  w2d[1][1] = d2w[1][1];
  w2d[1][2] = d2w[2][1];
  w2d[2][0] = d2w[0][2];
  w2d[2][1] = d2w[1][2];
  w2d[2][2] = d2w[2][2];
}
