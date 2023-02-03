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

#ifndef USUBSS_H
#define USUBSS_H

#include "usubs.h"
#include <vector>

class USubs;

class USubss
{
protected:
  USubss();
  
  /**
   * @brief Decode a subscription command
   * @param keyline is the keyword for this message and further parameters
   * @returns true if used
   */
  bool subscribeDecode(const char * keyline);
  /**
   * @brief sendHelpLine sends help line for this key
   */
  void subscribeSendHelp();
  /**
   * @brief sendHelpLine sends help line for this key
   * \param listNum is the publist list number of last item - to be increased
   */
  void sendPublishList(int & listNum);
  /**
   * add subscription key */
  void addPublistItem(const char * key, const char * helpLine);
  /**
   * send data now from one of the subscription items
   * single request or as subscribed */
  virtual void sendData(int item) {};
public:
  /**
   * @brief tick is service of subscription
   * \param n is next item to check
   */
  void subscribeTick();
  /**
   * Stop all subscriptions */
  void stopSubscriptions();
  
protected:
  /// 
  std::vector<USubs*> subs;
};

#endif // USUBS_H
