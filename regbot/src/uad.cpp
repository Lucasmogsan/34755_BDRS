 /***************************************************************************
  *   Copyright (C) 2019-2022 by DTU                             *
  *   jca@elektro.dtu.dk                                                    *
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
 
#include <stdio.h>
#include "ucontrol.h"
#include "ustate.h"
#include "ucommand.h"
#include "ueeconfig.h"
#include "uusb.h"
#include "usubss.h"

#include "ulinesensor.h"

#include "uad.h"

UAd ad;

void adc0_isr();
void adc1_isr();


void UAd::setup()
{
  int useADCresolution = 12;
  adc.adc0->setResolution ( useADCresolution); 
  adc.adc1->setResolution ( useADCresolution);
#ifdef REGBOT_HW41
  // support for 3.3V only  
  adc.adc0->setReference ( ADC_REFERENCE::REF_3V3);
  adc.adc1->setReference ( ADC_REFERENCE::REF_3V3);
  adc.adc0->setConversionSpeed ( ADC_CONVERSION_SPEED::MED_SPEED);
  adc.adc1->setConversionSpeed ( ADC_CONVERSION_SPEED::MED_SPEED);
#else
  adc.adc0->setReference ( ADC_REFERENCE::REF_1V2);
  adc.adc1->setReference ( ADC_REFERENCE::REF_1V2);
#ifdef REGBOT_HW4
  adc.adc0->setConversionSpeed ( ADC_CONVERSION_SPEED::MED_SPEED);
  adc.adc1->setConversionSpeed ( ADC_CONVERSION_SPEED::MED_SPEED);
#else
  adc.adc0->setConversionSpeed ( ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc.adc1->setConversionSpeed ( ADC_CONVERSION_SPEED::HIGH_SPEED);
#endif
#endif  
  // more pins
  pinMode ( PIN_BATTERY_VOLTAGE, INPUT ); // battery voltage (A9)
  pinMode ( PIN_LINE_SENSOR_0, INPUT ); // Line sensor sensor value
  pinMode ( PIN_LINE_SENSOR_1, INPUT ); // Line sensor sensor value
  pinMode ( PIN_LINE_SENSOR_2, INPUT ); // Line sensor sensor value
  pinMode ( PIN_LINE_SENSOR_3, INPUT ); // Line sensor sensor value
  pinMode ( PIN_LINE_SENSOR_4, INPUT ); // Line sensor sensor value
  pinMode ( PIN_LINE_SENSOR_5, INPUT ); // Line sensor sensor value
  pinMode ( PIN_LINE_SENSOR_6, INPUT ); // Line sensor sensor value
  pinMode ( PIN_LINE_SENSOR_7, INPUT ); // Line sensor sensor value
  //
  motorCurrentRawAD[0] = adc.analogRead ( PIN_LEFT_MOTOR_CURRENT );
  motorCurrentRawAD[1] = adc.analogRead ( PIN_RIGHT_MOTOR_CURRENT );
  batVoltRawAD = adc.analogRead ( PIN_BATTERY_VOLTAGE );
  // initialize low-pass filter for current offset
  //   motorCurrentMLowPass[0] = adc.analogRead ( PIN_LEFT_MOTOR_CURRENT ) * 100;
  //   motorCurrentMLowPass[1] = adc.analogRead ( PIN_RIGHT_MOTOR_CURRENT ) * 100;
    // enable interrupt for the remaining ADC oprations
  adc.adc0->enableInterrupts(adc0_isr); // ( ADC_0 );
  adc.adc1->enableInterrupts(adc1_isr); // ( ADC_1 );
  //
//   adc = new ADC();
  // info messages
  addPublistItem("ad", "Get raw AD values (ir1, ir2, battery, m1 current, m2 current)");
  addPublistItem("ls", "Get raw line sensor AD values (n1, n2, ls1 (l,h), ls2 ... ls8, ct1, ct2 (us))");
}


void UAd::tick()
{ // start AD cycle
  adcSeq = 0;
  adcHalf = false;
  // debug
  if (PIN_BUZZER > 0)
    digitalWriteFast(PIN_BUZZER, LOW);
  // debug end
  adc.startSingleRead(adcPin[0]); // + 400;
  adcStartTime = hb10us;
  adcStartCnt++;
  // safety things - it happends that the AD interrupt stops
  if (debug1adcIntCntLast == adcInt0Cnt)
  {
    adcIntErrCnt++;
    if (adcIntErrCnt % 100 == 0)
    { // disabled for 100ms
      const int MSL  = 54;
      char s[MSL];
      snprintf(s, MSL, "# ADC seq=%d, resetcnt=%d, reset\n", adcSeq, adcResetCnt);
      usb.send(s);
      // after a mission this could happen
      // I haven't found a better solution
      adc.adc0->enableInterrupts(adc0_isr); // ( ADC_0 );
      adc.adc1->enableInterrupts(adc1_isr); // ( ADC_1 );
      adcResetCnt++;
    }
  }
  else
    adcIntErrCnt = 0;
  debug1adcIntCntLast = adcInt0Cnt;
  //
  // service subscriptions
  subscribeTick();
}

void UAd::tickHalfTime()
{ // NB! called by timer interrupt
  if ( adcSeq >= ADC_NUM_ALL )
  {
    adcHalfStartTime = hb10us;
    adcSeq = 0;
    adc.startSingleRead ( adcPin[0] );
    adcHalf = true;
    adcHalfCnt++;
  }
  // debug
  if (PIN_BUZZER > 0)
    digitalWriteFast(PIN_BUZZER, HIGH);
  // debug end
}

void UAd::sendHelp()
{
//   const int MRL = 150;
//   char reply[MRL];
  usb.send("# AD converter -------\r\n");
  subscribeSendHelp();
}

bool UAd::decode(const char* buf)
{
  bool used = true;
  if (subscribeDecode(buf)) {}
  else
    used = false;
  return used;
}

void UAd::sendData(int item)
{
  if (item == 0)
    sendADraw();
  else if (item == 1)
    sendStatusLSRaw();
//   else if (item == 2)
//     sendStatusCurrentVolt();
}

void UAd::sendADraw()
{
  const int MSL = 100;
  char s[MSL];
  snprintf(s, MSL, "ad %d %d %d %d %d\n", 
           irRawAD[0],
           irRawAD[1],
           batVoltRawAD,
           motorCurrentRawAD[0],
           motorCurrentRawAD[1]
  );
  usb.send(s);
}

void UAd::sendStatusLSRaw()
{
  const int MRL = 250;
  char reply[MRL];
  //                       #1 #2   LS0    LS1    LS2    LS3    LS4    LS5    LS6    LS7    timing (us)
  snprintf(reply, MRL, "ls %d %d  %d %d  %d %d  %d %d  %d %d  %d %d  %d %d  %d %d  %d %d  %ld %ld\r\n",
           adcStartCnt, adcHalfCnt, 
           adcLSH[0], adcLSL[0], adcLSH[1], adcLSL[1], adcLSH[2], adcLSL[2], adcLSH[3], adcLSL[3], 
           adcLSH[4], adcLSL[4], adcLSH[5], adcLSL[5], adcLSH[6], adcLSL[6], adcLSH[7], adcLSL[7], 
           adcConvertTime * 10, adcHalfConvertTime * 10);
  usb.send(reply);
}



void UAd::eePromLoad()
{
  // deviceID = eeConfig.readWord();
}

void UAd::eePromSave()
{
  // eeConfig.pushWord(deviceID);
}


void UAd::adInterrupt(int a)
{
  uint16_t v;
  if (a == 0)
  {
    v = adc.readSingle ( ADC_0 );
    adcInt0Cnt++;
  }
  else
  {
    v = adc.readSingle ( ADC_1 );
    adcInt1Cnt++;
  }
  // debug logging destination
  if (false)
  {
    float * nv = logger.dataloggerExtra; // space for 20 floats
    nv[adcSeq] = v;
  }
  // debug end
  //uint8_t pin = ADC::sc1a2channelADC0[ADC0_SC1A&ADC_SC1A_CHANNELS];   // <- For ADC Debug
  //Serial.printf("ADC0 %d pin %d\n\r", v, pin);                        // <- For ADC Debug
  if ( adcSeq < ADC_NUM_IR_SENSORS )
  {
    *adcDest[adcSeq] = v;
  }
  else if ( adcSeq < ADC_NUM_NO_LS )
  { // low-pass filter battery and current sensor values at about 2ms time constant
    // result is in range 0..8196 (for measured between 0v and 1.2V)
    *adcDest[adcSeq] = ( ( *adcDest[adcSeq] ) >> 1 ) + v;
  }
  else if ( adcHalf )
  { // line sensor raw value
    adcLSH[adcSeq - ADC_NUM_NO_LS] = v;
  }
  else
  {
    adcLSL[adcSeq - ADC_NUM_NO_LS] = v;
  }
  adcSeq++;
  if ( adcSeq < ADC_NUM_ALL ) // start new and re-enable interrupt
  {
    adc.startSingleRead ( adcPin[adcSeq] );
  }
  else     // finished
  { // set LED for next read (has then about 0.5 ms to settle)
    if ( adcHalf )
    { // turn LEDs on
      adcHalfConvertTime = hb10us - adcHalfStartTime;
      digitalWriteFast ( PIN_LINE_LED_HIGH, ls.lineSensorOn );
      if ( state.robotHWversion > 2 )
        digitalWriteFast ( PIN_LINE_LED_LOW, ls.lineSensorOn );
      else
        digitalWriteFast ( OLD_PIN_LINE_LED, ls.lineSensorOn );
    }
    else
    { // turn LEDs off
      adcConvertTime = hb10us - adcStartTime;
      digitalWriteFast ( PIN_LINE_LED_HIGH, LOW );
      if ( state.robotHWversion > 2 )
        digitalWriteFast ( PIN_LINE_LED_LOW, LOW );
      else
        digitalWriteFast ( OLD_PIN_LINE_LED, LOW );
    }
    if (false)
    {
      const int MSL=50;
      char s[MSL];
      snprintf(s, MSL, "# ADC hf=%d, at=%ld, 10us=%ld\n", adcHalf, adcConvertTime, hb10us);
      usb.send(s);
    }
  }
}




// If you enable interrupts make sure to call readSingle() to clear the interrupt.
void adc0_isr()
{
  ad.adInterrupt(0);
}

//////////////////////////////////////////////////////////

void adc1_isr()
{
  ad.adInterrupt(1);
}
