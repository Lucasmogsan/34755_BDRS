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

#include "ucontrolbase.h"
#include "ueeconfig.h"

#include "ustate.h"

//#include "motor_controller.h"

//bool historyInvalid = false;

////////////////////////////////////////////////////////////

UTransferFunctionPZ::UTransferFunctionPZ()
{
//   sampleTime = 0.001;
  inUse = false;
  tau_den = 1.0;
  tau_num = 1.0;
}


void UTransferFunctionPZ::setParamsTauTau(float tau_numerator, float tau_denominator, float out_limit)
{
  tau_num = tau_numerator;
  tau_den = tau_denominator;
  inUse = out_limit <= 0.0;
//   limit = out_limit;
//   output_limit = out_limit < 1e6 and out_limit > 0.0;
  initControl();
}

////////////////////////////////////////////////////////////

void UTransferFunctionPZ::initControl()
{ // include pole zero init
  if (inUse)
  { /* Pole - Zero 1. order filter 
    * Gd:=(td * s + 1)/(al*td*s + 1);
    * denominator (nævner)
    * Gzd2:=expand(Gzd*z^(-1));
    *
    *      (2*al*td + T)u + (T- 2*al*td) u/z
    *      tau_den = td*al;
    * numerator (tæller)
    * Gzn2:=expand(Gzn*z^(-1));
    *                                 
    *      (T + 2*td)e + (T - 2*td) e/z
    *      tau_num = td
    *
    * controller code: u[0] =  -u[1]*zu[1] + e*ze[0] + e[1] * ze[1];
    */
    zu[0] = 2 * tau_den + sampleTime; // denominator 
    zu[1] = (sampleTime - 2 * tau_den) / zu[0];
    ze[0] = (sampleTime + 2 * tau_num) / zu[0]; // numerator
    ze[1] = (sampleTime - 2 * tau_num) / zu[0];
  }
  else
  { // no function (transfer = 0)
    zu[0] = 1;
    zu[1] = 1; // subtract old value - if any
    ze[0] = 0; // do not use input
    ze[1] = 0;
  }
}

void UTransferFunctionPZ::resetControl()
{
  x[0] = 0;
  x[1] = 0;
  y[0] = 0;
  y[1] = 0;
}

////////////////////////////////////////////////////////////

/**
 * simple filter 1. order with output limit*/
void UTransferFunctionPZ::controlTick()
{
  if (inUse)
  {
//     if (historyInvalid)
//     { // do not use history
//       x[1] = 0;
//       y[1] = 0;
//     }
    y[0] = -zu[1] * y[1] + ze[0] * x[0] + ze[1] * x[1];
//     if (output_limit)
//     {
//       if (y[0] > limit)
//         y[0] = limit;
//       else if (y[0] < -limit)
//         y[0] = -limit;
//     }
    y[1] = y[0];
    x[1] = x[0];
  }
  else
    y[0] = 0.0;
}


const char * UTransferFunctionPZ::decodeFromString(const char * line)
{
  const char * p1 = line;
  char * p2;
  inUse = strtol(p1, &p2, 0);
  if (p2 > p1)
  {
    p1 = p2;
    tau_num = strtof(p1, &p2);
  }
  if (p2 > p1) 
  {
    p1 = p2;
    tau_den = strtof(p1, &p2);
  }
  initControl();
  if (p2 > p1)
    return p2;
  else
    return NULL;
}

int UTransferFunctionPZ::getToString ( char* buf, int bufCnt )
{
  snprintf(buf, bufCnt, "%d %g %g ", inUse, tau_num, tau_den);
  return strlen(buf);
}

void UTransferFunctionPZ::eePromSave()
{
  if (inUse)
  {
    eeConfig.pushFloat(tau_num);
    eeConfig.pushFloat(tau_den);
  }
}

void UTransferFunctionPZ::eePromLoad()
{
  if (inUse)
  {
    if (state.robotIDvalid())
    {
      tau_num = eeConfig.readFloat();
      tau_den = eeConfig.readFloat();
    }
    else
      eeConfig.skipAddr(4 + 4);
  }
  initControl();
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

UTransferFunctionI::UTransferFunctionI()
{
  sampleTime = 0.001;
  inUse = false;
  tau_den = 1.0;
  limit = 1e6;
  limit_use = false;
  andZero = true;
}

////////////////////////////////////////////////////////////

void UTransferFunctionI::setParamsIntegrator(float tau, float i_limit)
{
//   tau_num = 1.0;
  tau_den = tau;
//   filter_use = i_limit <= 0.0;
  limit = i_limit;
  initControl();
}

////////////////////////////////////////////////////////////

const char * UTransferFunctionI::decodeFromString(const char * line)
{ // from e.g. "1 0.22 4.77"
  const char * p1 = line;
  const char * p2 = p1;
  inUse = strtol(p1, (char**)&p2, 0);
  if (p2 > p1)
  {
    p1 = p2;
    tau_den = strtof(p1, (char**)&p2);
  }
  if (p2 > p1)
  {
    p1 = p2;
    limit = strtof(p1, (char**)&p2);
    if (limit < 1e6 and limit >= 0.0)
      limit_use = true;
  }
  if (p2 > p1)
  {
    p1 = p2;
    andZero = strtol(p1, (char**)&p2, 0);
  }
  initControl();
  if (p2 > p1)
    return p2;
  else
    return NULL;
}

int UTransferFunctionI::getToString(char * buf, int bufCnt)
{
  float v;
  if (limit_use)
    v = limit;
  else
    v = 1e6;
  snprintf(buf, bufCnt, "%d %g %g %d ", inUse, tau_den, v, andZero);
  return strlen(buf);
}

////////////////////////////////////////////////////////////

void UTransferFunctionI::initControl()
{ // include pole zero init
  if (inUse)
  { // first order integrator
    /*
     * GC(s) = u/e = 1/(tau_i s)
     * Gzd2:=expand(Gzd*z^(-1));
     *                        
     *    (2*ti)u + (2*ti)u/z
     *                        
     * Gzn2:=expand(Gzn*z^(-1));
     *                
     *    T*e + T*e/z 
     *                
     *  u[0] = u[1] + T/(2*ti) * e[0] + T/(2*ti) * e[1]
     * 
     */
    zu[0] = 2 * tau_den; // denominator
    zu[1] = -1;
    ze[0] = sampleTime / zu[0]; // numerator
    ze[1] = sampleTime / zu[0];
    limit_use = limit < 99;
  }
  else
  { // no added value from integrator
    zu[0] = 1;
    zu[1] = 1;
    ze[0] = 0;
    ze[1] = 0;
  }
}

void UTransferFunctionI::resetControl()
{
  x[0] = 0;
  x[1] = 0;
  y[0] = 0;
  y[1] = 0;
}


////////////////////////////////////////////////////////////

/**
 * simple filter 1. order */
void UTransferFunctionI::controlTick()
{
  if (inUse)
  {
//     if (historyInvalid)
//     { // do not use history if invalid
//       x[1] = 0;
//       y[1] = 0;
//     }
    y[0] = -zu[1] * y[1] + ze[0] * x[0] + ze[1] * x[1];
    if (limit_use)
    { // integrator limitor in use
      if (y[0] > limit)
        y[0] = limit;
      else if (y[0] < -limit)
        y[0] = -limit;
    }
    y[1] = y[0];
    x[1] = x[0];
  }
  else
    y[0] = 0.0;
}

void UTransferFunctionI::eePromSave()
{
  if (inUse)
  {
    eeConfig.pushFloat(tau_den);
    eeConfig.pushFloat(limit);
  }
}

void UTransferFunctionI::eePromLoad()
{
  if (inUse)
  {
    if (state.deviceID > 0)
    {
      tau_den = eeConfig.readFloat();
      limit = eeConfig.readFloat();
    }
    else
      eeConfig.skipAddr(4 + 4);
  }
  initControl();
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


UControlBase::UControlBase(const char * idKey, const char * descr)
{
  use = false;
  kp = 1.0;
  ffKp = 0.0;
//   rateLimitUse = false;
  outLimitUse = false;
  outLimit = 1e6;
  strncpy(key, idKey, MKL);
  ffUse = false;
  ffKp = 1.0;
  input = NULL;
  measurement = NULL;
  gyro = NULL;
  output = NULL;
  plusMinusPiCheck = false;
  description = descr;
  setup();
}

void UControlBase::setup()
{
  addPublistItem(key, "Get params");
}


void UControlBase::sendHelp()
{
  const int MSL = 100;
  char s[MSL];
  snprintf(s, MSL, "# \t%s params \t%s\r\n", key, description);
  usb.send(s);
  subscribeSendHelp();
}

////////////////////////////////////////////////////////////////////////

void UControlBase::resetControl()
{ // clear all old values
  preFilt.resetControl();
  postFiltI.resetControl();
  ffFilt.resetControl();
  integrator.resetControl();
  leadFwd.resetControl();
  leadBack.resetControl();
}

void UControlBase::initControl()
{ // calculate discrete control parameters
  // for all controllers with dynamics
  preFilt.initControl();
  postFiltI.initControl();
  ffFilt.initControl();
  integrator.initControl();
  leadFwd.initControl();
  leadBack.initControl();  
}


////////////////////////////////////////////////////////////////////////

bool UControlBase::decode(const char * line)
{
  const char * p1 = line;
  char * p2;
  bool used = true;
  if (subscribeDecode(line)) {}
  else if (strncmp(line, key, strlen(key)) == 0)
  {
    const int MSL = 100;
    char s[MSL];
    snprintf(s, MSL, "#decode %s controller\n", key);
    usb.send(s);
    
    
    // format
    // cbal 1 -3 0 1 9999.99 1 0 1 1 1 0.029 0.001 0 1 1 1 0.1 5 1 0 1 0 1 1 1 99.99
    // 1 cbal 1 // use (any part of controller)
    // 2     -3 // Kp
    // 3      0 1 9999.99 1 // integrator (use, tau, limit, (and_zero - not implemented))
    // 7      0 1 1 // lead forward (use, zero, pole)
    // 10     1 0.029 0.001 // lead backward (use, zero, pole)
    // 13     0 1 1 // pre-filt (use, zero, pole)
    // 16     1 0.1 5 1 // post integrator (use, tau, limit, and_zero)
    // 20     0 1 // feed forward (use, Kff)
    // 22     0 1 1 // feed forward pole-zero (use zero pole)
    // 25     1 99.99 // output limit (use, limit)
    p1 += strlen(key);
    use = strtol(p1, &p2, 10);
    if (p2 > p1)
    {
      p1 = p2;
      kp = strtof(p1, &p2);
    }
//     snprintf(s, MSL, "# base 3: p1=%s p2=%s (%d), kp=%g\n\r", p1, p2, p2>p1, kp);
//     usb_send_str(s);
    if (p2 > p1)
    {
//       usb_send_str("# base 4\n\r");
      p1 = integrator.decodeFromString(p2);
      if (p1 != NULL)
      {
//         usb_send_str("# base 5\n\r");
        p1 = leadFwd.decodeFromString(p1);    
      }
      if (p1 != NULL)
        p1 = leadBack.decodeFromString(p1);    
      if (p1 != NULL)
        p1 = preFilt.decodeFromString(p1);    
      if (p1 != NULL)
        p1 = postFiltI.decodeFromString(p1);    
      if (p1 != NULL)
        ffUse = strtol(p1, &p2, 0);
      if (p1 != NULL and p2 > p1)
      {
        ffKp = strtof(p1 = p2, &p2);
//         snprintf(s, MSL, "# base 7: p1=%s p2=%s (%d), ffkp=%g\n\r", p1, p2, p2>p1, ffKp);
//         usb_send_str(s);
      }
      if (p1 != NULL and p2 > p1)
        p1 = ffFilt.decodeFromString(p1 = p2);    
      if (p1 != NULL)
        outLimitUse = strtol(p1, &p2, 0);
      if (p1 != NULL and p2 > p1)
      {
        outLimit = strtof(p1 = p2, &p2);
//         usb_send_str("# base 8\n\r");
      }
//       if (p1 != NULL and p2 > p1)
//         rateLimitUse = strtol(p1 = p2, &p2, 0);
//       if (p1 != NULL and p2 > p1)
//       {
//         rateLimit = strtof(p1 = p2, &p2);
//         used = true; 
//       }
    }
//     s[0] = '#';
//     s[1] = ' ';
//     int n = getRegulator(&s[2], MSL - 2);
//     p2 = &s[n+2];
//     strncpy(p2, "\n\r", MSL - n - 2);
//     usb_send_str(s, true);
  }
  else
    used = false;
//   usb_send_str("# base 99\n\r");
  return used;
}


void UControlBase::sendData(int item)
{
  if (item == 0)
    sendRegulatorParams();
}

void UControlBase::sendRegulatorParams()
{
  const int bufCnt = 270;
  char buf[bufCnt];
  char * p1 = buf;
  int n;
  n = snprintf(p1, bufCnt, "%s %d %g ", key, use, kp);
  p1 = &buf[n];
  n += integrator.getToString(p1, bufCnt - n);
  p1 = &buf[n];
  n += leadFwd.getToString(p1, bufCnt - n);
  p1 = &buf[n];
  n += leadBack.getToString(p1, bufCnt - n);
  p1 = &buf[n];
  n += preFilt.getToString(p1, bufCnt - n);
  p1 = &buf[n];
  n += postFiltI.getToString(p1, bufCnt - n);
  p1 = &buf[n];
  n += snprintf(p1, bufCnt - n, "%d %g ", ffUse, ffKp);
  p1 = &buf[n];
  n += ffFilt.getToString(p1, bufCnt - n);
  p1 = &buf[n];
  n += snprintf(p1, bufCnt - n, "%d %g\r\n", outLimitUse, outLimit);
  usb.send(buf);
}


void UControlBase::setInputOutput(float * referenceInput, float * measurementInput, float * outputValue, float * gyroInput /*= NULL*/)
{
  input = referenceInput;
  measurement = measurementInput;
  output = outputValue;
  gyro = gyroInput;
}

void UControlBase::controlTick(bool logExtra)
{
//   const int MSL = 120;
//   char s[MSL];
  
  if (not use)
  { // no controller - output is zero
    *output = 0.0;
  }
  else
  { // controller is in use
//     float ffOut;   /// feed forward output
//     float preOut;  /// pre filter output
//     float backEst; /// estimated measurement (after potential lead)
//     float eu;       /// error (ref - measurement)
//     float u1, u;       /// control signal
//     if (rateLimit)
//     { // input should be rate limited - acceleration limit for a velocity controller
//       float accSampleLimit = rateLimit * SAMPLETIME;
//       float diff = *input - rateLimited;
//       if (diff > accSampleLimit)
//         rateLimited += accSampleLimit;
//       else if (diff < -accSampleLimit)
//         rateLimited -= accSampleLimit;
//       else
//         rateLimited = *input;
//     }
//     else
//       rateLimited = *input;
    // feed forward
    if (ffUse)
    { // feed forward part is in use
      ffOut = *input * ffKp;
      if (ffFilt.inUse)
      { // should also be filtered
        ffFilt.x[0] = ffOut;
        ffFilt.controlTick();
        ffOut = ffFilt.y[0];
      }
    }
    else
      ffOut = 0.0;
    // pre filter (pole zero part)
    if (preFilt.inUse)
    { // pre filter (pole zero part) is in use
      preFilt.x[0] = *input;
      preFilt.controlTick();
      preOut = preFilt.y[0];
    }
    else
      preOut = *input;
    // handle feed back part (lead - and potentially gyro
    if (leadBack.inUse)
    {
      if (gyro != NULL)
        // lead is actually a P-D controller (probably balance only)
        backEst = *measurement + leadBack.tau_num * *gyro;
      else
      { // a real lead
        if (logExtra)
        {
          logger.dataloggerExtra[0] = leadBack.x[1];
          logger.dataloggerExtra[1] = leadBack.y[1];
        }
        float a = *measurement - leadBack.x[1];
        if (a > M_PI)
        { // adjust old value of x and y to match
          leadBack.x[1] += 2.0 * M_PI;
          leadBack.y[1] += 2.0 * M_PI;
        }
        else if (a < - M_PI)
        { // adjust old value of x and y to match
          leadBack.x[1] -= 2.0 * M_PI;
          leadBack.y[1] -= 2.0 * M_PI;
        }
        leadBack.x[0] = *measurement;
        leadBack.controlTick();
        backEst = leadBack.y[0];
        if (logExtra)
        {
          logger.dataloggerExtra[2] = leadBack.x[1];
          logger.dataloggerExtra[3] = leadBack.y[1];
        }
      }
    }
    else
    {
      backEst = *measurement;
    }
    // if input is angle that could fold at +/- pi, then
    // change reference input after pre-filter to same 
    // (half) revolution as measurements
    if (plusMinusPiCheck)
    {
      float a = preOut - *measurement;
      if (a > M_PI)
        preOut -= 2.0 * M_PI;
      else if (a < -M_PI)
        preOut += 2.0 * M_PI;
    }
    // now error can be calculated
    eu = preOut - backEst;
    // multiply with proportional gain
    eu *= kp;
    // Lead filter in forward branch
    if (leadFwd.inUse)
    {
      if (logExtra)
      {
        logger.dataloggerExtra[4] = leadFwd.x[1];
        logger.dataloggerExtra[5] = leadFwd.y[1];
      }
      leadFwd.x[0] = eu;
      leadFwd.controlTick();
      eu = leadFwd.y[0];
      if (logExtra)
      {
        logger.dataloggerExtra[6] = leadFwd.x[1];
        logger.dataloggerExtra[7] = leadFwd.y[1];
      }
    }
    // integrator 
    if (integrator.inUse)
    { // add value from integrator
      integrator.x[0] = eu;
      if (not outLimitUsed)
        // stop integrator if output is saturated
        integrator.controlTick();
      u1 = eu + integrator.y[0];
    }
    else
      u1 = eu; // no itegrator
    // add any feed forward term
    u1 += ffOut;
    // post filter integrator
    if (postFiltI.inUse)
    { // pre filter (pole zero part) is in use
      postFiltI.x[0] = u1;
      postFiltI.controlTick();
      if (postFiltI.andZero)
        u = u1 + postFiltI.y[0];
      else
        // no zero, so pole all the way
        u = postFiltI.y[0];
    }
    else
      u = u1;
    /// also add value from feed forward
    *output = u;
    outLimitUsed = false;
    if (outLimitUse)
    { // limiting activated
      if (u > outLimit)
      { // above maximum
        if (doImplementOutputLimit)
          *output = outLimit;
        outLimitUsed = true;
      }
      else if (u < -outLimit)
      { // below minimum
        if (doImplementOutputLimit)
          *output = -outLimit;
        outLimitUsed = true;
      }
    }
  }
}


void UControlBase::eePromSave()
{
  uint16_t f = 0; // use flags 
  if (use)                 f |= (1 << 0);
  if (integrator.inUse)    f |= (1 << 1);
  if (leadFwd.inUse)       f |= (1 << 2);
  if (leadBack.inUse)      f |= (1 << 3);
  if (preFilt.inUse)       f |= (1 << 4);
  if (postFiltI.inUse)     f |= (1 << 5);
  if (ffUse)               f |= (1 << 6);
  if (ffFilt.inUse)        f |= (1 << 7);
  if (outLimitUse)         f |= (1 << 8);
  if (postFiltI.andZero)   f |= (1 << 9);
  eeConfig.pushWord(f);
  if (use)
  {
    eeConfig.pushFloat(kp);
    integrator.eePromSave();
    leadFwd.eePromSave();
    leadBack.eePromSave();
    preFilt.eePromSave();
    postFiltI.eePromSave();
    if (ffUse)
    {
      eeConfig.pushFloat(ffKp);
      ffFilt.eePromSave();
    }
    if (outLimitUse)
      eeConfig.pushFloat(outLimit);
//     if (rateLimitUse)
//       eeConfig.pushFloat(rateLimit);
  }
}

void UControlBase::eePromLoad()
{
//   const int MSL = 100;
//   char s[MSL];
  uint16_t f; // use flags 
  f = eeConfig.readWord();
  use =                 f & (1 << 0);
  integrator.inUse =    f & (1 << 1);
  leadFwd.inUse =       f & (1 << 2);
  leadBack.inUse =      f & (1 << 3);
  preFilt.inUse =       f & (1 << 4);
  postFiltI.inUse =     f & (1 << 5);
  ffUse =               f & (1 << 6);
  ffFilt.inUse =        f & (1 << 7);
  outLimitUse =         f & (1 << 8);
  postFiltI.andZero =   f & (1 << 9);
  // snprintf(s, MSL, "# load %s flags 0x%x (10 bits used)\n", key, f);
  // usb_send_str(s);
  if (use)
  {
    kp = eeConfig.readFloat();
//     snprintf(s, MSL, "# load kp=%g\n", kp);
//     usb_send_str(s);
    integrator.eePromLoad();
    leadFwd.eePromLoad();
    leadBack.eePromLoad();
    preFilt.eePromLoad();
    postFiltI.eePromLoad();
    if (ffUse)
    {
      if (state.deviceID > 0)
        ffKp = eeConfig.readFloat();
      else
        eeConfig.skipAddr(4);
      ffFilt.eePromLoad();
//       snprintf(s, MSL, "# load ffkp=%g\n", ffKp);
//       usb_send_str(s);
    }
    if (outLimitUse)
    {
      if (state.deviceID > 0)
        outLimit = eeConfig.readFloat();
      else
        eeConfig.skipAddr(4);
    }
    //     if (rateLimitUse)
//       rateLimit = eeConfig.readFloat();
  }
}

bool UControlBase::isMe ( const char* id )
{
  return strncmp(key, id, strlen(key)) == 0;
}

/**
 * \param ref is reference input to controller
 * \param m is measurement value that should match ref
 * \param m2 is measurement after lead in feedback
 * \param ff is feed forward value
 * \param r2 is ref after pre-filter
 * \param up is error after Kp and lead in forward branch (p part of output)
 * \param ui is integrator value - after integrator limit 
 * \param u1 is output after add of feed forward and before post integrator
 * \param u  is output after post integrator and limit
 * */
void UControlBase::toLog (ULog::logItem item)
{
  float v[CTRL_LOG_SIZE];
  v[0] = *input;
  v[1] = *measurement;
  v[2] = backEst;
  v[3] = ffOut;
  v[4] = preOut;
  v[5] = (preOut - backEst) * kp;
  v[6] = eu;
  v[7] = integrator.y[0];
  v[8] = u1;
  v[9] = *output;
  logger.addToLog(item, v, sizeof(v));
//   usb_send_str("#control values to log\r\n");
}


