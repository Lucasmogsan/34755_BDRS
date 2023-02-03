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

#ifndef USUBS_H
#define USUBS_H


class USubs
{
public:
  
  USubs(const char * key, const char * help);
  /**
   * @brief Decode a subscription command
   * @param keyline is the keyword for this message and further parameters
   * @param newSubscription is true, if this is a subscription command, else data request
   * @returns true if used
   */
  bool decode(const char * keyline, bool newSubscription);
  /**
   * @brief decode a one time request (no parameters)
   * @param key is the request key, followed by an i
   * @returns true if used
   */
//   bool decode(const char * key);
  /**
   * @brief tick is servise of subscription
   * @returns true if message is to be send
   */
  bool tick();
  /**
   * @brief sendHelpLine sends help line for this key
   */
  void sendHelpLine();
  /**
   * @brief sendHelpLine sends help line for this key
   * \param listNum is the publist list number of last item - to be increased
   */
  void sendPublishList(int & listNum);
  /**
   * Stop all pulished items */
  void stopSubscription()
  {
    subN = 0;
  }
  /**
   * @brief MKL and msgKey is the message key for this subscription
   */
  const char * msgKey;
  /**
   * @brief helpText is text to be returned for a help request
   */
  const char * helpText;
  /**
   * @brief keySize is size of key, to make compare faster
   */
  int keySize;
  /// interval count so far
  int subCnt = 0;
  /// interval in ms (0 = not avtive)
  int subN = 0;
  /// explicit request data
  bool dataRequest = false;
  
};

#endif // USUBS_H
