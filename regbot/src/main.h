/***************************************************************************
 *   Copyright (C) 2022 by DTU                                             *
 *   jca@elektro.dtu.dk                                                    *
 *
 *   Main function for small regulation control object (regbot)
 *   build on a teensy 3.1 72MHz ARM processor MK20DX256 - or any higher,  *
 *   intended for 31300/1 Linear control 1
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

#ifndef REGBOT_MAIN_H
#define REGBOT_MAIN_H


#define MISSING_SYSCALL_NAMES

#include <core_pins.h>
#include "pins.h"


extern volatile uint32_t hb10us;     /// heartbeat timer count (10 us)
extern volatile uint32_t hbTimerCnt; /// sample time count - typically ms (not assumed to overflow)
extern volatile uint32_t tsec;  /// time that will not overrun
extern volatile uint32_t tusec; /// time that will stay within [0...999999]

#endif
