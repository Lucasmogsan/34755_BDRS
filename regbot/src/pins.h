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

#ifndef PINS_H
#define PINS_H

// REGBOT_HW4 will be defined in makefile, if Teensy 3.5 is selected.

#define OLD_PIN_LINE_LED        32 // For version < 3
#define OLD_PIN_START_BUTTON    11 // For version 1

// Pin defines for Regbot v3 and older

#ifndef REGBOT_HW4
// include Teensy 4.1 (REGBOT_HW41)
// Main pins
#ifdef REGBOT_HW41
#define PIN_LED_STATUS          11
#define PIN_LED_DEBUG           13 // (LED_BUILTIN)
#define PIN_START_BUTTON        37
#define PIN_LINE_LED_HIGH       12
#define PIN_LINE_LED_LOW        33
#define PIN_DISABLE2            51 // (not used)
#define PIN_POWER_IR            36
#define PIN_POWER_ROBOT         1
#define PIN_MUTE  		          0 // set to 0 for mute
#else
#define PIN_LED_STATUS          13
#define PIN_LED_DEBUG           13 // (LED_BUILTIN)
#define PIN_START_BUTTON        6
#define PIN_LINE_LED_HIGH       18
#define PIN_LINE_LED_LOW        25
#define PIN_DISABLE2            11
#define PIN_POWER_IR            32
#define PIN_POWER_ROBOT         33
#define PIN_BUZZER		           100 // not usable in this HW
#define PIN_MUTE  		           100 // not usable in this HW
#endif
// ADC pins
// #define ADC_NUM_IR_SENSORS      2
// #define ADC_NUM_NO_LS           5
// #define ADC_NUM_ALL             (ADC_NUM_NO_LS + 8)
#ifdef REGBOT_HW41
#define PIN_IR_RAW_1            A15
#define PIN_IR_RAW_2            A16
#define PIN_BATTERY_VOLTAGE     A17
#define PIN_LEFT_MOTOR_CURRENT  A1
#define PIN_RIGHT_MOTOR_CURRENT A0
#define PIN_LINE_SENSOR_0       A6
#define PIN_LINE_SENSOR_1       A13
#define PIN_LINE_SENSOR_2       A7
#define PIN_LINE_SENSOR_3       A12 
#define PIN_LINE_SENSOR_4       A8
#define PIN_LINE_SENSOR_5       A11 
#define PIN_LINE_SENSOR_6       A9
#define PIN_LINE_SENSOR_7       A10
#else
#define PIN_IR_RAW_1            A1
#define PIN_IR_RAW_2            A0
#define PIN_BATTERY_VOLTAGE     A9
#define PIN_LEFT_MOTOR_CURRENT  A10
#define PIN_RIGHT_MOTOR_CURRENT A11
#define PIN_LINE_SENSOR_0       A12
#define PIN_LINE_SENSOR_1       A13
#define PIN_LINE_SENSOR_2       A15   // A19
#define PIN_LINE_SENSOR_3       A16   // A15
#define PIN_LINE_SENSOR_4       A17   // A16
#define PIN_LINE_SENSOR_5       A18   // A20
#define PIN_LINE_SENSOR_6       A19   // A17
#define PIN_LINE_SENSOR_7       A20   // A18
#endif

// Motor Controller pins
#ifdef REGBOT_HW41
#define PIN_LEFT_DIR            2 
#define PIN_LEFT_PWM            3
#define PIN_RIGHT_PWM           5
#define PIN_RIGHT_DIR           4
#define PIN_LEFT_ENCODER_A      29
#define PIN_LEFT_ENCODER_B      28
#define PIN_RIGHT_ENCODER_A     31
#define PIN_RIGHT_ENCODER_B     30
#define PIN_LEFT_FAULT          38
#define PIN_RIGHT_FAULT         32
#define M1ENC_A         PIN_LEFT_ENCODER_A
#define M1ENC_B         PIN_LEFT_ENCODER_B
#define M2ENC_A         PIN_RIGHT_ENCODER_A
#define M2ENC_B         PIN_RIGHT_ENCODER_B
#else // Teensy 3.2 (HW 2, 3, 4, 5)
#define PIN_LEFT_DIR            2
#define PIN_LEFT_PWM            3
#define PIN_RIGHT_PWM           4
#define PIN_RIGHT_DIR           12
#define PIN_LEFT_ENCODER_A      20
#define PIN_LEFT_ENCODER_B      19
#define PIN_RIGHT_ENCODER_A     22
#define PIN_RIGHT_ENCODER_B     21
#define M1DIR           PIN_LEFT_DIR // M1IN2 - Teensy Digital 2 (direction)
#define M1PWM           PIN_LEFT_PWM // M1IN1 - Teensy Digital 3 (PWM)
#define M2DIR3          PIN_RIGHT_DIR // M2IN2 - Teensy Digital 8 (direction) hardware 3 only
#define M2PWM3          PIN_RIGHT_PWM // M2IN1 - Teensy Digital 4 (PWM) hardware 3
#define M12DIS          PIN_DISABLE2 // M1+M2 D2  enable both motors - hardware 3 only
#define M1ENC_A         PIN_LEFT_ENCODER_A // Encoder A - Teensy Digital 20
#define M1ENC_B         PIN_LEFT_ENCODER_B // Encoder B - Teensy Digital 19
#define M2ENC_A         PIN_RIGHT_ENCODER_A // Encoder A - Teensy Digital 22
#define M2ENC_B         PIN_RIGHT_ENCODER_B // Encoder B - Teensy Digital 21
#endif
// Legacy pins (used for HW 2,5)
#define M1DIS1          4 // M1D2  - Teensy Digital 4 (disable (hiz))) - hardware < 3
#define M2DIS1          10 // M2D2  - Teensy Digital 10 (disable (hi-z)) - hardware < 3
#define M2PWM1          9 // M2IN1 - Teensy Digital 9 (PWM) hardware < 3
#define SLEW            7 // SLEW  - Teensy Digital 7  -  hardware 2, 5 only, hardware 3 fixed high
#define M2DIR1          8 // M2IN2 - Teensy Digital 8 (direction) hardware < 3 only

// Servo pins
#ifdef REGBOT_HW41
#define PIN_SERVO1      10 
#define PIN_SERVO2       9
#define PIN_SERVO3       8
#define PIN_SERVO4       7
#define PIN_SERVO5       6
#define PIN_BUZZER		   6 // Shared with SERVO5!

#else
#define PIN_SERVO1      5 // Teensy pin for servo 1
#define PIN_SERVO2      9 // Teensy pin for servo 2 - not valid on version 3.0 or earlier (used for motor enable)
#define PIN_SERVO3      10 // Teensy pin for servo 3 - not valid on version 3.0 or earlier (used for motor enable)
#define PIN_SERVO4       5 // not used here
#define PIN_SERVO5       9 // not used here
#define SERVO_PIN_0     A14 // IO pin on servo board
#define SERVO_PIN_1     24  // IO pin on servo board
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Pin Definitions for Regbot PCB version v4.1 2018 (Teensy 3.5)
#elif defined(REGBOT_HW4)
// Main pins
#define PIN_LED_STATUS          11
#define PIN_LED_DEBUG           13
#define PIN_START_BUTTON        26
#define PIN_LINE_LED_HIGH       12
#define PIN_LINE_LED_LOW        25
#define PIN_DISABLE2            27
#define PIN_POWER_IR            24
#define PIN_POWER_ROBOT         28

// // ADC pins
// #define ADC_NUM_IR_SENSORS      2
// #define ADC_NUM_NO_LS           5
// #define ADC_NUM_ALL             (ADC_NUM_NO_LS + 8)
//
#define PIN_IR_RAW_1            A1
#define PIN_IR_RAW_2            A0
#define PIN_BATTERY_VOLTAGE     A14
#define PIN_LEFT_MOTOR_CURRENT  A2
#define PIN_RIGHT_MOTOR_CURRENT A3
//
#define PIN_LINE_SENSOR_0       A12
#define PIN_LINE_SENSOR_1       A13
#define PIN_LINE_SENSOR_2       A15
#define PIN_LINE_SENSOR_3       A16
#define PIN_LINE_SENSOR_4       A17
#define PIN_LINE_SENSOR_5       A18
#define PIN_LINE_SENSOR_6       A19
#define PIN_LINE_SENSOR_7       A20

// Motor Controller pins
#define PIN_LEFT_DIR            6
#define PIN_LEFT_PWM            3
#define PIN_RIGHT_PWM           4
#define PIN_RIGHT_DIR           2

#define PIN_LEFT_ENCODER_A      23
#define PIN_LEFT_ENCODER_B      22
#define PIN_RIGHT_ENCODER_A     20
#define PIN_RIGHT_ENCODER_B     21
#define M1DIR           PIN_LEFT_DIR // M1IN2 - Teensy Digital (direction)
#define M1PWM           PIN_LEFT_PWM // M1IN1 - Teensy Digital (PWM)
#define M2DIR3          PIN_RIGHT_DIR // M2IN2 - Teensy Digital (direction) hardware 3 only
#define M2PWM3          PIN_RIGHT_PWM // M2IN1 - Teensy Digital (PWM) hardware 3
#define M12DIS          PIN_DISABLE2 // M1+M2 D2  enable both motors - hardware 3 only
#define M1ENC_A         PIN_LEFT_ENCODER_A // Encoder A - Teensy Digital
#define M1ENC_B         PIN_LEFT_ENCODER_B // Encoder B - Teensy Digital
#define M2ENC_A         PIN_RIGHT_ENCODER_A // Encoder A - Teensy Digital
#define M2ENC_B         PIN_RIGHT_ENCODER_B // Encoder B - Teensy Digital
// Legacy pins (only used < 3)
#define M1DIS1          4 // M1D2  - Teensy Digital 4 (disable (hiz))) - hardware < 3
#define M2DIS1          10 // M2D2  - Teensy Digital 10 (disable (hi-z)) - hardware < 3
#define M2PWM1          9 // M2IN1 - Teensy Digital 9 (PWM) hardware < 3
#define SLEW            7 // SLEW  - Teensy Digital 7  -  hardware 1 og 2 only, hardware 3 fixed high
#define M2DIR1          8 // M2IN2 - Teensy Digital 8 (direction) hardware < 3 only

// Servo pins
#define PIN_SERVO1      5 // Teensy pin for servo 1
#define PIN_SERVO2      9 // Teensy pin for servo 2
#define PIN_SERVO3      10 // Teensy pin for servo 3
#define PIN_SERVO4      29 // Teensy pin for servo 4 - on 4.1 only
#define PIN_SERVO5      30 // Teensy pin for servo 5 - on 4.1 only
#define SERVO_PIN_0     PIN_SERVO4 // before 4.1
#define SERVO_PIN_1     PIN_SERVO5 // before 4.1
#define PIN_BUZZER		30 // 
#define PIN_MUTE  		 0 // not usable in this HW

#endif

#endif
