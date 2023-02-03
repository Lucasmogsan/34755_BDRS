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

#include "ucontrolturn.h"
#include "ueeconfig.h"
#include "ucontrol.h"
// #include "robot.h"
#include "ulinesensor.h"
#include "uencoder.h"

UControlTurn::UControlTurn(const char * idKey, const char * descr) : UControlBase(idKey, descr)
{
  velReduc = NULL;
//   velReduc[1] = NULL;
}

void UControlTurn::setInputOutput ( float* referenceInput, float* measurementInput, float outputValue[] )
{
  input = referenceInput;
  measurement = measurementInput;
  output = &baseOutput;
  gyro = NULL;
  velReduc = outputValue;
//   velReduc[1] = &outputValue[1];
  // this is sensitive to angle folding
  plusMinusPiCheck = true;
}

void UControlTurn::controlTick(bool logExtra)
{
  UControlBase::controlTick(logExtra);
//   const int MSL = 70;
//   char s[MSL];
//   snprintf(s, MSL, "#turn out=%g\n", baseOutput);
//   usb_send_str(s);
  if (velReduc != NULL)
  { // implement steering as reduction of current velocity for each wheel
    bool symmetric = fabs(baseOutput) < 0.3* fabs(control.mission_vel_ref) + 0.10;
    // debug
    if (not control.balance_active)
    { // symmetric in all cases may be good (except maybe in balance)
      symmetric = true;
    }
    // debug end
    if (symmetric)
    { // small turn, so keep right average velocity
      velReduc[0] =  baseOutput;
      velReduc[1] = -baseOutput;
    }
    else if (baseOutput > 0)
    { // turning left sharp turn
//       if (fabs(baseOutput) > 0.3* fabs(mission_vel_ref))
      { // sharp turn, so reduce inner wheel only
        velReduc[0] = baseOutput * 2.0;      
        //velReduc[1] = 0;
      }
    }
    else
    { // turning right sharp turn
//       if (fabs(baseOutput) > 0.3* fabs(mission_vel_ref))
      { // reduce 
        //velReduc[0] = 0;      
        velReduc[1] = -baseOutput * 2.0;
      }
    }
  }
  
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

UControlWallTurn::UControlWallTurn ( const char* idKey, const char * descr) : UControlTurn ( idKey, descr)
{
  irInput = NULL;
  maxTurnDistance = 0.5;
}


void UControlWallTurn::setInputOutput ( float* referenceInput, float measurementInput[], float* outputValue, const float maxWallDistance)
{
  UControlTurn::setInputOutput(referenceInput, measurementInput, outputValue);
  // input needs treatment
  // both values, index 0 is looking right, 1 is looking forward
  irInput = measurementInput;
  measurement = &combinedMeasuredValue;
  // not used 
  maxTurnDistance = maxWallDistance;
}

void UControlWallTurn::controlTick()
{ // use a combination of both sensors
  // should follow wall if closest - but short front distance turns too.
//   const int MSL=120;
//   char s[MSL];
  //
  // these distances is when the robot start to avoid a front obstacle
  const float headBounceDistLow = 0.25; // when not in balance, then sensor sees the floor (about 20-25cm is fine)
  const float headBounceDistBal = 0.35;
  if (irInput[1] < headBounceDistBal and control.balance_active and fabs(encoder.pose[3]) < 0.15)
  { // closing in from the front so start avoid to the left (sensor looking horizontal)
    // input is reference
    combinedMeasuredValue = irInput[1] - headBounceDistBal + *input;
  }
  else if (irInput[1] < headBounceDistLow and not control.balance_active)
  { // closing in from the front so start avoid to the left (sensor is looking somewhat down)
    combinedMeasuredValue = irInput[1] - headBounceDistLow + *input;
  }
  else if (irInput[0] < (*input + 0.07))
  { // do not turn too much to the right to get closer (7cm is OK dependent on controller)
    combinedMeasuredValue = irInput[0];
//     snprintf(s, MSL, "# wall %.3f %.3f %.3f<%g prefilt %f kp=%g\r\n", 
//              irInput[0], irInput[1], combinedMeasuredValue, maxTurnDistance,
//              preOut, kp
//             );
//     usb_send_str(s);
  }
  else
  { // IR sensor is probably missing first measurement
    // do nothing - go straight (measurement is equal to reference)
    combinedMeasuredValue = *input;
  }
  // result is implemented as normal turn.
  UControlTurn::controlTick();
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

UControlEdge::UControlEdge ( const char* idKey, const char * descr) : UControlTurn ( idKey, descr)
{
  edgePos[0] = NULL;
  edgePos[1] = NULL;
}


void UControlEdge::setInputOutput (float* referenceInput, float* leftSide, float * rightSide, 
                                   bool * valid/*, bool* rightValid*/, 
                                   float outputValue[], bool * followLeft)
{
  input = referenceInput;
  output = &baseOutput;
  measurement = &measuredValue;
  gyro = NULL;
  velReduc = outputValue;
  // both, 0 is looking right, 1 is looking forward
  edgePos[0] = leftSide;
  edgePos[1] = rightSide;
  edgeValid/*[0]*/ = valid;   // left and right are always either both valid or both invalid
//  edgeValid[1] = rightValid;  // this line not needed
  edge = followLeft;  
}

void UControlEdge::controlTick()
{ // use a combination of both sensors
  // should follow wall if closest - but short front distance turns too.
//   const int MSL = 60;
//   char s[MSL];
//   bool valid = true;
  /// konstant gain for development in line position over the last 1 (or 2mm) of travel
  /// to fast detect crossing line
  bool isOK = true;
//   const float kd = 2.0;
  if (*edgeValid and ls.crossingLineCnt == 0)// not xingW and not xingB)
  { // edges are valid and no sign of a corssing line
    if (*edge)
    { // following left edge (white or black is handled at sensor level)
//       float m2;
//       if (invalidCnt == 0)
//         m2 = *(edgePos[0]) + (*(edgePos[0]) - edgePos1[0]) * kd;
//       else
//         // history not valid
//         m2 = *(edgePos[0]);
//       if (m2 < -2.5)
//         invalidCnt++;
//       else
//         invalidCnt = 0;
      measuredValue = *(edgePos[0]);
    }
    else
    {
//       float m2;
//       if (invalidCnt == 0)
//         m2 = *(edgePos[1]) + (*(edgePos[1]) - edgePos1[1]) * kd;
//       else
//         // history not valid
//         m2 = *(edgePos[1]);
//       // debug trying to follow line senter, i.e. just between left and right value
//       if (m2 > 2.5)
//         invalidCnt++;
//       else
//         invalidCnt = 0;
      measuredValue = *(edgePos[1]);
    }
//     if (distance - oldDist > 0.001)
//     { // distance is 1mm (or more), so we save a value
//       // (to avoid turning at a crossing line)
//       oldDist = distance;
// //       edgePos2[0] = edgePos1[0];
// //       edgePos2[1] = edgePos1[1];
//       edgePos1[0] = *(edgePos[0]);
//        edgePos1[1] = *(edgePos[1]);
//     }
  }
  else
  { // no valid input - assume right course
//     measuredValue = *input;
    isOK = false;
//     invalidCnt++;
//     oldDist = 0.0;
  }
  // debug control details
  if (false)
  { // reset logger values to zero (just in case)
    memset(logger.dataloggerExtra, 0, sizeof(logger.dataloggerExtra));
    logger.dataloggerExtra[3] = 77.7;
    UControlTurn::controlTick(true); //true /* datalogExtra activate */);
  }
  else
    // debug end
    UControlTurn::controlTick(); 
  if (not isOK)
  { // do not use control value
    velReduc[0] = 0;
    velReduc[1] = 0;
    *output = 0;
  }
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

UControlBalVel::UControlBalVel ( const char* idKey, const char * descr) : UControlBase ( idKey, descr)
{
  wheelVelocity[0] = NULL;
  wheelVelocity[1] = NULL;
}

void UControlBalVel::setInputOutput ( float* referenceInput, float* measurementInput, float* outputValue )
{
  input = referenceInput;
  wheelVelocity[0] = &measurementInput[0];
  wheelVelocity[1] = &measurementInput[1];
  measurement = &measuredValue;
  gyro = NULL;
  output = outputValue;
}

void UControlBalVel::controlTick()
{ // control the velocity of fastest wheel,
  // so measurement is fastest wheel.
  if (*wheelVelocity[0] > *wheelVelocity[1])
    measuredValue = *wheelVelocity[0];
  else
    measuredValue = *wheelVelocity[1];
  // control and implement as normal
  UControlBase::controlTick();
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

UControlPos::UControlPos ( const char* idKey, const char * descr) : UControlBase ( idKey, descr)
{
}


void UControlPos::setInputOutput ( float* referenceInput, float* currentDistance, float* lineStartDistance, float* outputValue )
{
  input = referenceInput;
  distance = currentDistance;
  startDistance = lineStartDistance;
  measurement = &lineDistance;
  gyro = NULL;
  output = &ctrlOutput;
  velRef = outputValue;
}

void UControlPos::controlTick()
{ // measured distance is since start of this mission line
//    const int MSL = 90;
//    char s[MSL];
  lineDistance = *distance - *startDistance;
  // control and implement as normal
  UControlBase::controlTick();
  //if (not outLimitUse or (outLimitUse and mission_vel_ref < outLimit))
  { // limit velocity to maximum velocity reference
//      snprintf(s, MSL, "# limit %g > %g, m=%g, di=%g, st=%g\n", *output, mission_vel_ref, *measurement, *distance, *startDistance);
//      usb_send_str(s);
    if (*output > control.mission_vel_ref)
    { // limited positive velocity
      *velRef = control.mission_vel_ref;
      outLimitUsed = true;
//      usb_send_str("# limit +\n");
    }
    else if (*output < -control.mission_vel_ref)
    { // limited negative velocity
      *velRef = -control.mission_vel_ref;
      outLimitUsed = true;
//      usb_send_str("# limit -\n");
    }
    else
    { // normal control
      *velRef = *output;
    }
  }
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

UControlIrVel::UControlIrVel ( const char* idKey, const char * descr) : UControlBase ( idKey, descr)
{
  irDist = NULL;
  measuredDist = 0;
}


void UControlIrVel::setInputOutput ( float* referenceInput, float* currentDistance, float* outputValue )
{
  UControlBase::setInputOutput(referenceInput, &measuredDist, outputValue);
  irDist = currentDistance;
}

void UControlIrVel::controlTick()
{ // get measured distance depending on sensor configuration
  // mission_irSensor_use may be 0 (not used), 1 (wall follow - not here), 
  // 2 (use sensor 1 only), 
  // 3 (use both sensors (both assumed to point forward'ish))
  if (control.mission_irSensor_use == 3)
  {
    if (irDist[0] < irDist[1])
      measuredDist = irDist[0];
    else
      measuredDist = irDist[1];
  }
  else
    // must be 2, else this place controlTick is not called
    measuredDist = irDist[1];
  
  // control and implement as normal
  UControlBase::controlTick();
}
