/***************************************************************************
 *   Copyright (C) 2016 by DTU                             *
 *   jca@elektro.dtu.dk                                                    *
 *
 * Interface to IMU MPU9150 mounted on spark-fun board
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




#ifndef UIMU2_H
#define UIMU2_H

#include "usubss.h"
// #include "mpu9250.h"
#include "MPU9250_asukiaaa.h"
#include "MadgwickAHRS.h"
#include "umat.h"


// I2C address 0x69 could be 0x68 depends on your wiring.
#define ADDRESS_MPU				   0x68
// #define ADDRESS_COM          0x0C // not used (Magnetometer)

class ULog;

class UImu2 : public USubss
{
public:
  // Functions
  void setup();
  bool decode(const char * cmd);
  void sendHelp();
  void tick();
  /**
   * save scale and offset */
  void eePromSave();
  /**
   * load scale and offset */
  void eePromLoad();
  void initMpu();  
  
  uint32_t gyroOffsetStartCnt = 1000;
  bool     gyroOffsetDone;
//   bool boardOK = true;
  
  float gyro[3] = {0};
  float acc[3] = {0};
  int16_t magRaw[3] = {0};
  float mag[3] = {0};
  
  UMatRot3x3 boardOrientation;
  float rX = 0, rY = 0, rZ = 0; //radians
  
  
  inline float getRoll()  { return mdgw.getRoll();}
  inline float getPitch() { return mdgw.getPitch();  }
  inline float getYaw() { return mdgw.getYaw(); }
  inline float getRollRadians() { return mdgw.getRollRadians(); }
  inline float getPitchRadians() { return mdgw.getPitchRadians(); }
  inline float getYawRadians() { return mdgw.getYawRadians(); };  

protected:
  /**
   * send data to subscriber or requester over USB 
   * @param item is the item number corresponding to the added subscription during setup. */
  void sendData(int item);
  
// complentary tilt filter
public:
//   float boardTiltOffset = 0;
  float tiltu1  = 0; // old value for complementary filter
  float accAng;   // for debug
  float gyroTiltRate; 
  void estimateTilt();
  int imuAvailable = 10; // count down on error
  
private:
  void sendStatusGyro();
  void sendStatusAcc();
  void sendAccOffset();
  void sendStatusMag();
  void sendStatusMagRaw();
  void sendGyroOffset();
  void sendImuPose();
  void sendBoardOrientation();
  
  void decodeMagCalibrationValues(const char * values);
  void decodeAccelerationCalibrate(const char * values);
  void sendStatusMagOffset();
protected:
  float offsetGyro[3] = {0};
  
  float accOffset[3] = {0};
  float accScale[3] = {1,1,1};
  bool simpleAccCal = false;
  int simpleAccCnt = -1;
  float simpleAccVal[3];
  
  float magOffset[3] = {0};
  float magRot[9] = {1,0,0,  0,1,0,  0,0,1};
  
  bool tiltOnly = false;
private:
  //
  MPU9250_asukiaaa mpu{ADDRESS_MPU};
  Madgwick mdgw;
  // last read from IMU
  uint32_t lastRead = 0;
  uint32_t lastReadMag = 0;
  uint32_t lastDebug = 0;
  //
  bool useMag = false;
  int magErr = 0;
  bool useMadgwich = false;
  uint32_t tickCnt = 0;
  //
  int sampleTime10us = 100;
  uint32_t imuSec, imuuSec, imuSecm, imuuSecm;
  //
  friend class ULog;
};
  
extern UImu2 imu2;

#endif // UIMU_H
