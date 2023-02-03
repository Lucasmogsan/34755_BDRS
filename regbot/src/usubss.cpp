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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "usubss.h"
#include "usubs.h"
#include "uusb.h"


USubss::USubss()
{
}

bool USubss::subscribeDecode(const char * keyLine)
{ // if the subscribe command is for any of my data, then return true.
  bool used = false;
  const char * p1 = keyLine;
  bool newSubscription = strncmp(p1, "sub ", 4) == 0;
  if (newSubscription)
    p1 += 4;
  for (int i = 0; i < (int)subs.size(); i++)
  {
    used = subs[i]->decode(p1, newSubscription);
    if (used)
      break;
  }
  return used;
}

void USubss::subscribeTick()
{ // check for time to send data
  for (int i = 0; i < (int)subs.size(); i++)
  {
    if (subs[i]->tick())
    {
      sendData(i);
    }
  }
}

void USubss::stopSubscriptions()
{
  for (int i = 0; i < (int)subs.size(); i++)
  {
    subs[i]->stopSubscription();
  }
}

void USubss::subscribeSendHelp()
{
  for (int i = 0; i < (int)subs.size(); i++)
    subs[i]->sendHelpLine();
}

void USubss::sendPublishList(int & listNum)
{
  for (int i = 0; i < (int)subs.size(); i++)
    subs[i]->sendPublishList(listNum);
}

void USubss::addPublistItem(const char* key, const char * helpLine)
{
  subs.push_back(new USubs(key, helpLine));
}
