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

#ifndef CONTROL_BASE_H
#define CONTROL_BASE_H

#include <string.h>
#include <stdlib.h>
//#include "WProgram.h"
#include "ulog.h"
#include "usubss.h"

class UControl;

// whenever mission control type is changed
// then old values may be invalid
//extern bool historyInvalid;

class UTransferFunctionPZ
{
public:
  /**
   * constructor */
  UTransferFunctionPZ();
  /**
   * Init discrete control parameters */
  void initControl();
  /**
   * Reset control - i.e. set all old values to zero */
  void resetControl();
  /**
   * Set parameters */
  void setParamsTauTau(float tau_numerator, float tau_denominator, float limit);
  /**
   * decode from 3 values in a string 
   * \param line format is "1 0.22 0.022" for "use=true tauNumerator=0.22 tauDenominator=0.022"
   * \returns pointer to next unused character on line, or NULL if format error */
  const char * decodeFromString(const char * line);
  /**
   * Set parameters */
//   void setParamsTauAlpha(float tau, float alpha, float limit);
  /**
  * Get parametres to string */
  int getToString(char * buf, int bufCnt);
  /**
   * Do calculate control */
  void controlTick();
  /**
   * save to ee-prom (or string) the configuration
   * of this controller part */
  void eePromSave();
  /**
   * read configuration from ee-prom (or string)
   * to this controller part */
  void eePromLoad();
  
public:
  /** 
   * Should this regulator be used at all */
  bool inUse;
  /** differentiator time constant bay be used direct, if a gyro (differentiated) input exist.*/
  float tau_num;
protected:
  /**
   * Pole time constant for filter */
  float tau_den;
  /**
   * Sample time - is constant in this case */
  const float sampleTime = 0.001;
//   float limit;
//   bool filter_use;
//   bool output_limit;
private:
  /** control factor for numerator */
  float ze[2];
  /** control factor for denominator */
  float zu[2];
public:
  /** current control input values [0]=current [1]=last value */
  float x[2];
  /** current control output values [0]=current [1]=last value */
  float y[2];
};

class UTransferFunctionI
{
public:
  /**
   * constructor */
  UTransferFunctionI();
  /**
   * Init discrete control parameters */
  void initControl();
  /**
   * Reset control - i.e. set all old values to zero */
  void resetControl();
  /**
   * Set parameters */
  void setParamsIntegrator(float tau, float limit);
  /**
   * decode from 3 values in a string 
   * \param line format is "1 0.22 4.5" for "use=true tau=0.22 limit=4.5"
   * \returns pointer to next unused character on line, or NULL if format error */
  const char * decodeFromString(const char * line);
  /**
   * Get parametres to string */
  int getToString(char * buf, int bufCnt);
  /**
   * Do calculate control */
  void controlTick();
  /**
   * save tau and limit to e-prom - if in use */
  void eePromSave();
  /**
   * read configuration from ee-prom (or string)
   * to this controller part */
  void eePromLoad();
  
public:
  bool inUse;
  // should input to integrator be added to outpot - generation a zero
  bool andZero;
protected:
//   float tau_num;
  float tau_den;
  float sampleTime;
  float limit;
//   bool filter_use;
  bool limit_use;
private:
  /** control factor for numerator */
  float ze[2];
  /** control factor for denominator */
  float zu[2];
public:
  /** current control input values [0]=current [1]=last value */
  float x[2];
  /** current control output values [0]=current [1]=last value */
  float y[2];
};

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

class UControlBase : public USubss
{
public:
  /**
   * constructor 
   * \param control ID
   * \param descr description of controller use */
  UControlBase(const char * idKey, const char * descr);
  /**
   * set PWM port of frekvens */
  void setup();
  /**
   * send command help */
  void sendHelp();
  /**
   * Init control parameters 
   * must be called when sample period is changed */
  void initControl();
  /**
   * Send subscribed data */
  void sendData(int item) override;
  /**
  * init regulator from string 
  * \param line is a formatted string:
  *     "rxx use kp  ui ti iLimit  u1 t1n t1d  u2 t2n t2d   u3 t3n t3d u3i t3i lim3  ...
  *          ffuse kp4 u4 t4n t4d  lim_use limit rate_use rate_limit"
  * where the part in quotes are the line parameter, and:
  * use is controller active, else factor one from in to out
  * kp is proportional gain
  * ui ti iLimit is integrator use, time constant, and i limit (1e10 is no limit)
  * u1 t1n, t1d is Lead fwd: use, numerator,denominator time constants in  (t1n*s + 1)/(t1d*s + 1)
  * u2 t2n, t2d is Lead back use, numerator,denominator time constants in  (t2n*s + 1)/(t2d*s + 1) 
  * u3 t3n, t3d is pre-filter use, numerator,denominator time constants in  (t3n*s + 1)/(t3d*s + 1)
  * u3i t3i, lim3 is pre-filter i_term: use, tau_i, and limit
  * ffuse kp4 is feed forward use and proportional gain 
  * u4 t4n, t4d is feed forward use, numerator,denominator time constants in  (t4n*s + 1)/(t4d*s + 1)
  * lim_use is output limiting active
  * u_limit is output limit value
  * rate_use is rate limit active
  * rate_limit for reference value >1e10 marks do not use
  * \returns true initial key matches this regulator */
   bool decode(const char * line);
   /**
    * Set pointer to input and output of controller
    * \param referenceInput reference value in same scaling as measurement input
    * \param measurementInput is measured value that controller should try to keep be equal to reference
    * \param outputValue is output of the controller - affecting the measurement
    * \param gyro is differentiated measurement input, that is used by lead in back-branch, (if not NULL) in place of the pole-zero Lead filter
    * */
   void setInputOutput(float * referenceInput, float * measurementInput, float * outputValue, float * gyroInput = NULL);
   /**
    * Get regulator parameters */
   void sendRegulatorParams();
   /**
    * reset controller, i.e. set all old values to zero */
   void resetControl();
   /**
    * control tick 
    * This will calculate the full controller. */
   void controlTick(bool logExtra = false);
   /**
    * save configiration to ee-prom (or string) */
   void eePromSave();
   /**
    * load configiration from ee-prom (or string) */
   void eePromLoad();
   /** is this key me? */
   bool isMe(const char * id);
   /**
    * add control values to log
    * \param item is is for log set of values */
   void toLog(ULog::logItem item);
   /**
    * flag output limit, but do not implement the limit
    * The output limit flag is set as appropriate.
    * */
   inline void setDoImplementOutputLimitFlag(bool value)
   {
     doImplementOutputLimit = value;
   };
   inline float getOutLimit()
   {
     return outLimit;
   }
   
public:
   /** use regulator, if false then no connection from input to output */
   bool use;
   /** is the output limit in use, this can flag that 
    * integrator should not integrate - as in balance velosity */
   bool outLimitUsed;
   
protected:
  static const int MKL = 6;
  /** key when receiving or sending settings about this controller */
  char key[MKL];
  const char * description;
  /// proportional gain
  float kp;
  /// feed forward
  bool ffUse;
  float ffKp;
  /// output limit
  bool outLimitUse;
  float outLimit;
  /// rate limiter
//   bool rateLimitUse;
//   float rateLimit;
  /** input output */
  float * input; /// input reference value
  float * measurement; /// measurement value
  float * gyro; /// extra input for balance controller (uses tau_num only then)
  float * output; /// actuator output
  bool plusMinusPiCheck; // is reference an angle, there may be need of a folding check
  bool doImplementOutputLimit = true;
public:
  /** controller elements */
  UTransferFunctionPZ preFilt;
  UTransferFunctionI postFiltI;
  UTransferFunctionPZ ffFilt;
  UTransferFunctionI integrator;
  UTransferFunctionPZ leadFwd;
  UTransferFunctionPZ leadBack;
protected:
  /// control varaibles (interim values used when logging control data
  float ffOut;   /// feed forward output
  float preOut;  /// pre filter output
  float backEst; /// estimated measurement (after potential lead)
  float eu;       /// error (ref - measurement)
  float u1, u;    /// control signal before and after post-integrator and limit
  
  friend UControl;
};

#endif
