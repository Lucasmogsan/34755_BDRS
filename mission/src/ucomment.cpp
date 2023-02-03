/*  
 * 
 * Copyright © 2022 DTU, 
 * Author:
 * Christian Andersen jcan@dtu.dk
 * Jacob Bechmann Pedersen
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

#include <string>
#include <string.h>
#include "ubridge.h"
#include "ucomment.h"

// create 
UComment comment;


// Bridge class:
void UComment::setup()
{ /// subscribe to # information (debug text messages)
  bridge.tx(":# subscribe -1\n");
}


bool UComment::decode(char* msg)
{ // catch all messages starting with '#'
  // from any source
  bool used = true;
  const char * p1 = strchrnul(msg, ':');
  if (strncmp(p1, ":#", 2) == 0)
  { // show message
    printf("%s\n", msg);
  }
  else
    used = false;
  //
  return used;
}


