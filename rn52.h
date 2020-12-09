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
  enum RN52Status {
    Offline,
    Present,
    Error
  }

  class RN52
  {
    public:
      static RN52 *getInstance();
      bool init();

      void loop(bool rn52StatusUpdated);
      
      void rn52Command(const char *cmd);
      
      void rn52Exec(const char *cmd, char *buf, const int interDelay = 100, const int bufferSize = 1024);
      
      unsigned short rn52Status(char *hexArray);
      
      void rn52Debug(const char *cmd, const int interDelay = 50, const int bufferSize = 1024);

      bool ExceptionOccurred();

    protected:
      static RN52 *_instance;
      RN52();
      ~RN52();
      bool exceptionOccurred = false;
      
      byte *_rn52SerialBuffer;
      int _rn52SerialBufferSize;
      int _rn52SerialBufferDataLength;
  };
}

#endif // _RN52_H_
