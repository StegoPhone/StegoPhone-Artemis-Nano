//################################################################################################
//## StegoPhone : Steganography over Telephone
//## (c) 2020 Jessica Mulein (jessica@mulein.com)
//## All rights reserved.
//## Made available under the GPLv3
//################################################################################################

#ifndef _RN52_H_
#define _RN52_H_

#include <Arduino.h>

namespace StegoPhone
{
  class RN52
  {
    public:
      static RN52 *getInstance();
      void init();
      
      void rn52Command(const char *cmd);
      
      void rn52Exec(const char *cmd, char *buf, const int interDelay = 100, const int bufferSize = 1024);
      
      unsigned short rn52Status();
      
      void rn52Debug(const char *cmd, const int interDelay = 50, const int bufferSize = 1024);

    protected:
      static RN52 *_instance;
      RN52();
      bool exceptionOccurred = false;
  };
}

#endif // _RN52_H_
