 /***************************************************************************
  *   Copyright (C) 2019-2022 by DTU                             *
  *   jca@elektro.dtu.dk                                                    *
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
 
//#include <core_pins.h>
// #include "udrive.h"
#include "uimu2.h"
#include "ueeconfig.h"
#include "uencoder.h"
#include "ustate.h"




UImu2 imu2;



void UImu2::setup()
{
  initMpu();
  // board orientation (around x, y, z axis)
  // NB! ACC and gyro only
  boardOrientation.set(0,M_PI,0);
  // subscription
  addPublistItem("gyro",  "Get current gyro value as 'gyro gx gy gz' (deg/s)");
  addPublistItem("gyroo", "Get gyro offset 'gyroo ox oy oz'");
  addPublistItem("acc",   "Get accelerometer values 'acc ax ay az' (m/s^2)");
  addPublistItem("acco",  "Get accelerometer offset values 'acco xo yo zo' (m/s^2)");
  addPublistItem("mag",   "Get magnetometer values 'mag mx my mz' (uT)");  
  addPublistItem("magw",  "Get magnetometer raw values 'magw mx my mz' (uT)");  
  addPublistItem("mago",  "Get magnetometer offset values 'mago xo yo zo r11 r12 r13 r21 ... r33' (uT)");
  addPublistItem("imupose", "Get IMU-based pose 'imupose roll pitch yaw (radians)");
  addPublistItem("board", "Get orientation of IMU board (board rX rY rZ)");
}

void UImu2::initMpu()
{
  #ifdef REGBOT_HW4
  Wire.begin ( I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_INT, I2C_RATE_1000 );
  #else
  #ifdef REGBOT_HW41
  Wire.begin();
  Wire.setClock(1000000);
  //  Wire.setClock(400000);
  #else
  Wire.begin ( I2C_MASTER, 0x00, I2C_PINS_16_17, I2C_PULLUP_EXT, I2C_RATE_1000 );
  #endif
  #endif
  /* Initialize and configure IMU */
  mpu.setWire(&Wire);
  uint8_t id = 0;
  uint8_t retval = mpu.readId(&id);
  if (retval != 0)
  { // could not read from IPU
    usb.send("# Error initializing communication with IMU\n");
    imuAvailable = 0;
  }
  else
  { // print who am i
    const int MSL = 100;
    char s[MSL];
    snprintf(s, MSL, "# MPU9250 'who_am_i'=%d (%x)\n", id, id);
    usb.send(s);
    //
    mpu.beginAccel(ACC_FULL_SCALE_4_G);
    mpu.beginGyro(GYRO_FULL_SCALE_1000_DPS);
    if (useMag)
      mpu.beginMag(MAG_MODE_CONTINUOUS_100HZ);
  }
  // Madgwick filter
  // NB! sample time may change, but is not reflected here (@todo)
  mdgw.begin(1.0/state.SAMPLETIME);
}


bool UImu2::decode(const char* cmd)
{
  bool found = true;
  if (strncmp(cmd, "gyroc", 5) == 0)
  {
    gyroOffsetDone = false;
  }
  else if (strncmp(cmd, "acccal2", 7) == 0)
    simpleAccCal = true;
  else if (strncmp(cmd, "acccal ", 7) == 0)
    decodeAccelerationCalibrate(&cmd[7]);
  else if (strncmp(cmd, "magcal ", 7) == 0)
    decodeMagCalibrationValues(&cmd[7]);
  else if (strncmp(cmd, "board ", 6) == 0)
  {
    char * p1 = (char*)&cmd[6];
    rX = strtof(p1, &p1);
    rY = strtof(p1, &p1);
    rZ = strtof(p1, &p1);
    boardOrientation.set(rX, rY, rZ);
//     boardTiltOffset = rY;
  }
  else if (strncmp(cmd, "imuon ", 6) == 0)
  {
    char * p1 = (char*)&cmd[6];
    int e = strtol(p1, &p1, 10);
    int m = strtol(p1, &p1, 10);
    int mag = strtol(p1, &p1, 10);
    if (e == 1)
    { // start IMU (initialize)
      imuAvailable = 10;
      if (imuAvailable > 0)
      { // this do not work
        // initMpu();
        tickCnt = 0;
        usb.send("# initializing MPU9250\n");
      }
      else
      {
        usb.send("# start requesting MPU9250 data\n");
      }
    }
    else
    {
      imuAvailable = 0;
      // reset fails to 
//       mpu.Reset();
      usb.send("# Stopped using MPU9250\n");
    }
    // use extended 3D filter
    // else tilt estimate only
    useMadgwich = m == 1;      
    if (not useMag and mag == 1)
    {
      magErr = 0;
      mpu.beginMag();      
    }
    useMag = mag == 1;
  }
  else if (subscribeDecode(cmd)) {}
  else
    found = false;
  return found;
}

void UImu2::sendHelp()
{
  const int MRL = 300;
  char reply[MRL];
  usb.send("# IMU -------\r\n");
  // info requests
  snprintf(reply, MRL, "# \tgyroc \tStart gyro calibration (finished=%d)\r\n", gyroOffsetDone);
  usb.send(reply);
  usb.send(            "# \tmagcal \tSet magnetometer calibration values (offset[3] rotate/scale[9])\n");
  usb.send(            "# \tacccal \tSet accelerometer calibration values (offset[3] scale[3])\n");
  usb.send(            "# \tacccal2 \tmake simple acceleration calibration with horizontal board\n");
  usb.send(            "# \tboard rX rY rZ \tSet IMU board orientation angles (in radians)\r\n");
  usb.send(            "# \timuon E F M \tEnable IMU (E=1), Madgwick (F=1), use magnetometer (M=1) \r\n");
  usb.send(reply);
  subscribeSendHelp();
//   usb.send("#   sub YYY N \tSubscribe every N tick to YYY = message key without the 'i'.\r\n");
}

void UImu2::tick()
{ // read data - first time will fail
  tickCnt++;
  if (tickCnt < 20)
  {
    const int MSL = 100;
    char s[MSL];
    snprintf(s, MSL,"# UImu2::tick %lu, sampleTime = %d, imuavail=%d\n", tickCnt, sampleTime10us, imuAvailable);
    usb.send(s);
  }
  if (imuAvailable > 0)
  {
    int32_t nt = hb10us;
    int dt = nt - lastRead;
    if (true /*dt >= 90 and dt < 10000*/)
    { // data should be imuAvailable
      // there seems to be a problem, if dt <= 100 (1ms).
      lastRead = nt;
      sampleTime10us = dt;
      imuSec = tsec;
      imuuSec = tusec;
      if (mpu.accelUpdate() == 0)
      { // rectify raw data - except magnetometer
//         int16_t * a = mpu.getAcc();
        float accw[3];
        accw[0] = mpu.accelX();
        accw[1] = mpu.accelY();
        accw[2] = mpu.accelZ();
        
        if (simpleAccCal)
        {
          if (simpleAccCnt < 0)
          {
            simpleAccCnt = 500;
            simpleAccVal[0] = accw[0];
            simpleAccVal[1] = accw[1];
            simpleAccVal[2] = accw[2];
          }
          else if (simpleAccCnt > 0)
          {
            simpleAccVal[0] += accw[0];
            simpleAccVal[1] += accw[1];
            simpleAccVal[2] += accw[2];
          }
          else
          { // finished (simpleAccCnt == 0)
            const int MSL = 200;
            char s[MSL];
            snprintf(s, MSL, "# simple cal: %g %g %g\n", simpleAccVal[0], simpleAccVal[1], simpleAccVal[2]);
            usb.send(s);
            simpleAccCal = false;
            // ACC measurements are in g
            // NB - should include board orientation.
            accOffset[0] = simpleAccVal[0] / 500;
            accOffset[1] = simpleAccVal[1] / 500;
            accOffset[2] = simpleAccVal[2] / 500 - 1; // 0.9816;
            accScale[0] = 1; // 9.816
            accScale[1] = 1;
            accScale[2] = 1;
            sendAccOffset();
          }
          simpleAccCnt--;
        }
        // implement calibration
        float accc[3];
        for (int i = 0; i < 3; i++)
          accc[i] = accw[i] * accScale[i] - accOffset[i];
        // compensate for board orientation
        boardOrientation.rotateD2W(accc, acc);
        
        //
      }
      // Gyro
      if (mpu.gyroUpdate() == 0)
      { // gyro
        float gyroc[3];
        //         int16_t * g = mpu.getGyro();
        if (gyroOffsetDone)
        { // production
          gyroc[0] = mpu.gyroX() - offsetGyro[0];
          gyroc[1] = mpu.gyroY() - offsetGyro[1];
          gyroc[2] = mpu.gyroZ() - offsetGyro[2];
          //           gyro[1] = float(g[1] - offsetGyro[1];
//           gyro[2] = float(g[2] - offsetGyro[2]) * mpu.getGyroScale();
        }
        else
        { // calibrate
          // use raw values
          gyroc[0] = mpu.gyroX();
          gyroc[1] = mpu.gyroY();
          gyroc[2] = mpu.gyroZ();
          // start calibration
          if (tickCnt < gyroOffsetStartCnt)
          { // zero offset before 1000 summations
            offsetGyro[0] = 0;
            offsetGyro[1] = 0;
            offsetGyro[2] = 0;
          }
          else if (tickCnt <= gyroOffsetStartCnt + 1000)
          { // not finished offset calculation
            // summation over 1 second
            offsetGyro[0] += gyroc[0];
            offsetGyro[1] += gyroc[1];
            offsetGyro[2] += gyroc[2];
            if ((tickCnt - gyroOffsetStartCnt) % 100 == 0)
            {
              const int MSL = 150;
              char s[MSL];
              snprintf(s, MSL,"# UImu2::gyrooffset n=%lu, gx=%g sumgx=%g\n", tickCnt - gyroOffsetStartCnt, gyroc[0], offsetGyro[0]);
              usb.send(s);
            }
            if (tickCnt == gyroOffsetStartCnt + 1000)
            { // set average offset
              offsetGyro[0] /= 1000;
              offsetGyro[1] /= 1000;
              offsetGyro[2] /= 1000;
              usb.send("# gyro offset finished\r\n");
              gyroOffsetDone  = true;
            }
          }
          else
            // redo of calibrate requested
            gyroOffsetStartCnt = tickCnt + 10;
        }
        //
        // convert for board orientation
        boardOrientation.rotateD2W(gyroc, gyro);
        
        if (imuAvailable < 10)
          imuAvailable++;
      }
      else
      {
        imuAvailable--;
        if (imuAvailable == 0)
          usb.send("# message failed to read from MPU9250 10 times in a row, stopped trying\n");
      }
      // magnetometer
      dt = nt - lastReadMag;
      if (useMag and dt > 1000)
      { // we are reading mag every 10ms
        //
        lastReadMag = nt;
        // magnetometer
        if (mpu.magUpdate() == 0)
        {
//           if (mpu.new_mag_data())
          {
            imuSecm = imuSec;
            imuuSecm = imuuSec;
//             int16_t * m = mpu.getMag();
            // NB! magnetometer in 9250, the magnetometer has other axes
            // x = y, y=x, z= -z
            magRaw[0] =  mpu.magX(); //m[1];  // to x axis as gyro and acc
            magRaw[1] =  mpu.magY(); //m[0];  // to y-axis as gyro and acc
            magRaw[2] =  mpu.magZ(); //-m[2];  // to z-axis as gyro and acc
            //
            // offset and other correction
            float x,y,z;
            x = float(magRaw[0]) - magOffset[0];
            y = float(magRaw[1]) - magOffset[1];
            z = float(magRaw[2]) - magOffset[2];
            mag[0] = x * magRot[0] + y * magRot[1] + z * magRot[2];
            mag[1] = x * magRot[3] + y * magRot[4] + z * magRot[5];
            mag[2] = x * magRot[6] + y * magRot[7] + z * magRot[8];
            
          }
        }
        else
        {
          magErr++;
          if (magErr > 10)
          {
            useMag = false;
            usb.send("# magnetometer access error - stops using\n");
          }
        }
        // update Madgwick filter
        if (useMadgwich)
        { // all too slow for Teensy 3.2
          mdgw.update(acc[0], acc[1], acc[2],
                      gyro[0], gyro[1], gyro[2],
                      mag[0], mag[1], mag[2]);
        }
      }
    }
    if (not useMadgwich)
    { //  complementary tilt filter only
      estimateTilt();
      tiltOnly = true;
    }
  }
  // subscription service
  subscribeTick();
}

void UImu2::sendData(int item)
{
  if (item == 0)
    sendStatusGyro();
  else if (item == 1)
    sendGyroOffset();
  else if (item == 2)
    sendStatusAcc();
  else if (item == 3)
    sendAccOffset();
  else if (item == 4)
    sendStatusMag();
  else if (item == 5)
    sendStatusMagRaw();
  else if (item == 6)
    sendStatusMagOffset();
  else if (item == 7)
    sendImuPose();
  else if (item == 8)
    sendBoardOrientation();
}

void UImu2::sendBoardOrientation()
{
  const int MSL = 120;
  char s[MSL];
  snprintf(s, MSL, "board %g %g %g\n", rX, rY, rZ);
  usb.send(s);
}


////////////////////////////////////////////////

void UImu2::eePromSave()
{
  uint8_t f = 1;
  f |= useMadgwich << 1;
  eeConfig.pushByte(f);
  eeConfig.pushFloat(offsetGyro[0]);
  eeConfig.pushFloat(offsetGyro[1]);
  eeConfig.pushFloat(offsetGyro[2]);
  // accelerometer
  for (int i = 0; i < 3; i++)
    eeConfig.pushFloat(accOffset[i]);
  // magnetometer
  for (int i = 0; i < 3; i++)
    eeConfig.pushFloat(magOffset[i]);
  for (int i = 0; i < 9; i++)
    eeConfig.pushFloat(magRot[i]);
  for (int i = 0; i < 3; i++)
    eeConfig.pushFloat(accScale[i]);
  // imu board mounting orientation (in radians)
  eeConfig.pushFloat(rX);
  eeConfig.pushFloat(rY);
  eeConfig.pushFloat(rZ);
}

void UImu2::eePromLoad()
{
  uint8_t f = eeConfig.readByte();
  //bool useImu = f & 0x01;
  // IMU should not be turned off by configuration
  useMadgwich = (f & 0x02) > 0;
  //
  offsetGyro[0] = eeConfig.readFloat();
  offsetGyro[1] = eeConfig.readFloat();
  offsetGyro[2] = eeConfig.readFloat();
  gyroOffsetDone = true;
  //
  for (int i = 0; i < 3; i++)
    accOffset[i] = eeConfig.readFloat();
  //
  for (int i = 0; i < 3; i++)
    magOffset[i] = eeConfig.readFloat();
  for (int i = 0; i < 9; i++)
    magRot[i] = eeConfig.readFloat();
  for (int i = 0; i < 3; i++)
    accScale[i] = eeConfig.readFloat();
  rX = eeConfig.readFloat();
  rY = eeConfig.readFloat();
  rZ = eeConfig.readFloat();
  boardOrientation.set(rX, rY, rZ);
}

////////////////////////////////////////////

void UImu2::sendStatusMag()
{
  const int MRL = 250;
  char reply[MRL];
//   int16_t * m = mpu.getMag();
  snprintf(reply, MRL, "mag %g %g %g %lu.%03lu\r\n",
           mag[0], mag[1], mag[2], imuSecm, imuuSecm/1000);
  usb.send(reply);
}

void UImu2::sendStatusMagRaw()
{
  const int MRL = 250;
  char reply[MRL];
  snprintf(reply, MRL, "magw %d %d %d %lu.%03luf\r\n",
           magRaw[0], magRaw[1], magRaw[2], imuSec, imuuSec/1000);
  usb.send(reply);
}

void UImu2::decodeMagCalibrationValues(const char* values)
{
  float v[12];
  const char * p1 = values, * p2;
  bool fail = false;
  for (int i = 0; i < 12; i++)
  {
    v[i] = strtof(p1, (char **)&p2);
    fail = (p1 == p2);
    if (fail)
      break;
    p1 = p2;
  }
  if (not fail)
  {
    for (int i = 0; i < 3; i++)
      magOffset[i] = v[i];
    for (int i = 0; i < 9; i++)
      magRot[i] = v[i+3];
    usb.send("# new magnetometer values loaded\n");
  }
  else
    usb.send("# not enough parameters for magcal message (expected 12)\n");
}

void UImu2::decodeAccelerationCalibrate(const char* values)
{
  float v[6];
  const char * p1 = values, * p2;
  bool fail = false;
  for (int i = 0; i < 6; i++)
  {
    v[i] = strtof(p1, (char **)&p2);
    fail = (p1 == p2);
    if (fail)
      break;
    p1 = p2;
  }
  if (not fail)
  {
    for (int i = 0; i < 3; i++)
      accOffset[i] = v[i];
    for (int i = 0; i < 3; i++)
      accScale[i] = v[i+3];
    usb.send("# new accelerometer calibrate values loaded\n");
  }
  else
    usb.send("# not enough parameters for acccal message (expected 6)\n");
}

void UImu2::sendStatusMagOffset()
{
  const int MRL = 250;
  char reply[MRL];
  snprintf(reply, MRL, "mago %g %g %g  %g %g %g %g %g %g %g %g %g\r\n",
           magOffset[0], magOffset[1], magOffset[2], 
           magRot[0], magRot[1], magRot[2], magRot[3], magRot[4], magRot[5], magRot[6], magRot[7], magRot[8]);
  usb.send(reply);
}

void UImu2::sendStatusGyro()
{
  const int MRL = 250;
  char reply[MRL];
  snprintf(reply, MRL, "gyro %f %f %f %lu.%03lu\r\n",
           gyro[0], gyro[1], gyro[2], imuSec, imuuSec/1000);
  usb.send(reply);
}

void UImu2::sendStatusAcc()
{
  const int MRL = 250;
  char reply[MRL];
  snprintf(reply, MRL, "acc %f %f %f %lu.%03lu\r\n",
           acc[0], acc[1], acc[2], imuSec, imuuSec/1000);
  usb.send(reply);
}

void UImu2::sendGyroOffset()
{
  const int MRL = 250;
  char reply[MRL];
  snprintf(reply, MRL, "gyroo %f %f %f\r\n",
           offsetGyro[0], 
           offsetGyro[1], 
           offsetGyro[2]);
  usb.send(reply);
}

void UImu2::sendImuPose()
{
  const int MRL = 150;
  char reply[MRL];
  if (tiltOnly)
    snprintf(reply, MRL, "imupose %f %f %f\r\n",
           0.0, encoder.pose[3], 0.0);
  else
    snprintf(reply, MRL, "imupose %f %f %f\r\n",
             getRollRadians(), getPitchRadians(), getYawRadians());
  usb.send(reply);
}

void UImu2::sendAccOffset()
{
  const int MRL = 250;
  char reply[MRL];
  snprintf(reply, MRL, "acco %f %f %f %f %f %f\r\n",
           accOffset[0], accOffset[1], accOffset[2],
           accScale[0], accScale[1], accScale[2]
          );
  usb.send(reply);
}


/**
 * estimate tilt angle, as complementary filter with gyro and acc 
 *       1     tau s                     1
 * Gyro ---  ----------  + acc_pitch --------
 *       s    tau s + 1              tau s + 1
 *
 *     1        T/(T+2.*tau) + *T/(T+2.*tau) * z^-1
 * --------- = -------------------------------------
 *  tau s + 1     1 + (T-2.*tau)/(T+2.*tau) * z^-1
 *
 * T = 0.001;
 * tau = 1.0;
 *  */
/// tilt angle estimator
// float tiltu1  = 0; // old value for complementary filter
// float accAng;   // for debug
// float gyroTiltRate; 
void UImu2::estimateTilt()
{ // use actual sample time
  float T = sampleTime10us * 1e-5;
  float tau = 1.0; // seems to give good response
  float b = T/(T + 2 * tau);
  float a = -(T - 2 * tau)/(T + 2 * tau);
  float u; // input to filter
  float est; // estimated angle
  // gyro mounted on top plate!
  accAng = atan2f(-float(acc[0]),-float(acc[2]));
  // offset with value that makes the robot balance
  // accAng -= rY; // rY is board rotation on yAxis (tilt offset)
  // New and old angle must be in same revolution
  if ((accAng - encoder.pose[3]) > M_PI)
    accAng -= 2*M_PI;
  else if ((accAng - encoder.pose[3]) < -M_PI)
    accAng += 2*M_PI;
  // gyro is running in mode 2 (0= 250 grader/sek, 1 = 500 deg/s, 2=1000 deg/s 3=2000 deg/s)
  gyroTiltRate = gyro[1] * M_PI / 180.0; // radianer pr sekund
  // add gyro and accelerometer reading
  u = accAng + gyroTiltRate * tau;
  if (true) // imuGyro[0] < 245 and imuGyro[0] > -245)
  { // gyro not saturated
    // filter
    if (accAng > 0.0 and encoder.pose[3] < -M_PI/2.0)
      est = a * (encoder.pose[3] + 2 * M_PI) + b * u + b * tiltu1; 
    else if (accAng < 0.0 and encoder.pose[3] > M_PI/2.0)
      est = a * (encoder.pose[3] - 2 * M_PI) + b * u + b * tiltu1;
    else
      est = a * encoder.pose[3] + b * u + b * tiltu1;
  }
  else
    // else use angle as is from accelerometer
    est = accAng;
  // debug
  if (false and tickCnt % 100 == 0)
  {
    const int MSL = 200;
    char s[MSL];
    snprintf(s, MSL, "# est tilt:: at %lu.%03lu accAng=%.4f, gyro=%.4f, u=%.4f, est=%.4f, u1=%.4f, a=%.4f, b=%.4f, T=%.4f\n", 
             tsec, tusec/1000, accAng, gyroTiltRate, u, est, tiltu1, a, b, T);
    usb.send(s);
  }
  // debug end
  //
  if (est > M_PI)
  { // folded 
    est -= 2 * M_PI;
    // save last value of u in right angle space
    tiltu1 = accAng - 2 * M_PI + gyroTiltRate * tau;
  }
  else if (est < -M_PI)
  { // folded
    est += 2 * M_PI;
    tiltu1 = accAng - 2 * M_PI + gyroTiltRate * tau;
  }
  else
  { // no folding
    tiltu1 = u;
  }
  //
  encoder.pose[3] = est; // exfav[0]; // est;
}
