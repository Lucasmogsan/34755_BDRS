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

#ifndef CONTROL_TURN_H
#define CONTROL_TURN_H

#include <string.h>
#include <stdlib.h>
#include "ucontrolbase.h"
//#include "WProgram.h"


class UControlTurn : public UControlBase
{
public:
  /**
   * Constructor
   * \param id of controller
   * \param descr description of the controller use */
  UControlTurn(const char * idKey, const char * descr);
   /**
    * Set pointer to input and output of controller
    * \param referenceInput reference value in same scaling as measurement input
    * \param measurementInput is measured value that controller should try to keep be equal to reference
    * \param outputValue is an array of 2 values 0=left, 1=right speed reduction
    * */
   void setInputOutput(float * referenceInput, float * measurementInput, float outputValue[]);
   
   /**
    * control tick 
    * This will calculate the full controller. 
    * \param logextra flag to log detailed control values */
   void controlTick(bool logExtra = false);
protected:
  /**
   * Output of base controller */
  float baseOutput;
  /**
   * Pointer to real output - left and right */
  float * velReduc;
};

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


class UControlWallTurn : public UControlTurn
{
public:
  /** Constructor */
  UControlWallTurn(const char * idKey, const char * descr);
  /**
   * Set pointer to input and output of controller
   * \param referenceInput reference value in same scaling as measurement input
   * \param measurementInput is measured value of both IR sensors
   * \param outputValue is an array of 2 values 0=left, 1=right speed reduction
   * */
  void setInputOutput(float * referenceInput, float measurementInput[], float * outputValue, const float nearWallDistance);
  
  /**
   * control tick 
   * This will calculate the full controller. */
  void controlTick();
protected:
  /**
   * both IR measurements front used if close to bump into something */
  float * irInput;
  /**
   * most critical measured value */
  float combinedMeasuredValue;
  /**
   * if distance is more than this, then go straight */
  float maxTurnDistance;
};


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


class UControlEdge : public UControlTurn
{
public:
  /** Constructor */
  UControlEdge(const char* idKey, const char * descr);
  /**
   * Set pointer to input and output of controller
   * \param referenceInput reference value in same scaling as measurement input
   * \param leftSide,rightSide is measured value of left and right edge
   * \param leftValid,rightValid is valid flags for edge
   * \param outputValue is an array of 2 values 0=left, 1=right speed reduction
   * \param followLeft is true, if left edge value should be used for control
   * */
  void setInputOutput(float* referenceInput, float * leftSide, float * rightSide, 
                      bool * valid/*, bool* rightValid*/, 
                      float outputValue[], bool * followLeft);
  
  /**
   * control tick 
   * This will calculate the full controller. */
  void controlTick();
protected:
  /**
   * most critical measured value */
  float measuredValue;
  // calculated value
  float * edgePos[2];
  // is edge valid
  bool * edgeValid; //[2];
  // wich edge to follow true=left
  bool * edge;
  // hostory measuremns for better crossing line handling
  // 1 mm pr sample (1m/s gives every sample)
  //float edgePos1[2];//, edgePos2[2];
//   int invalidCnt;
//   float oldDist;
};

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


class UControlBalVel : public UControlBase
{
public:
  /** Constructor */
  UControlBalVel ( const char* idKey, const char * descr);
  /**
   * Set pointer to input and output of controller
   * \param referenceInput reference value in same scaling as measurement input
   * \param measurementInput is measured value of both left and right velocity
   * \param outputValue is tilt reference to balance controller
   * */
  void setInputOutput(float * referenceInput, float * measurementInput, float * outputValue);
  /**
   * control tick 
   * This will calculate the full controller. */
  void controlTick();
protected:
  /**
   * fastest velocity is taken as measured value */
  float measuredValue;
  // calculated value
  float * wheelVelocity[2];
};

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


class UControlPos : public UControlBase
{
  // Bør bruge velocity som maksimum hastighed under positionering
public:
  UControlPos ( const char* idKey, const char * descr);
  /**
   * Set pointer to input and output of controller
   * \param referenceInput reference value in same scaling as measurement input
   * \param measurementInput is measured value of both left and right velocity
   * \param outputValue is tilt reference to balance controller
   * */
  void setInputOutput(float * referenceInput, float* currentDistance, float* lineStartDistance, float* outputValue );
  /**
   * control tick 
   * This will calculate the full controller. */
  void controlTick();
protected:
  /**
   * fastest velocity is taken as measured value */
  float * distance; // current driven distance
  float * startDistance; // distance at start of line
  float * velRef;
  float lineDistance; // distance in this mission line
  float ctrlOutput;
};

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////


class UControlIrVel : public UControlBase
{
  // Bør bruge velocity som maksimum hastighed under positionering
public:
  UControlIrVel(const char* idKey, const char * descr);
  /**
   * Set pointer to input and output of controller
   * \param referenceInput reference value in same scaling as measurement input
   * \param measurementInput is measured value of both IR distance sensors
   * \param outputValue is tilt reference to balance controller
   * */
  void setInputOutput(float * referenceInput, float* currentDistance, float* outputValue );
  /**
   * control tick 
   * This will calculate the full controller. */
  void controlTick();
protected:
  /**
   * fastest velocity is taken as measured value */
  float * irDist; // IR distance from both sensors
  float measuredDist; // combined measured distance used as control input
};

#endif
