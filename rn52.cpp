#include "stegophone.h"
#include "rn52.h"

namespace StegoPhone
{
  RN52 *RN52::getInstance() {
    if (0 == _instance)
      _instance = new RN52();
    return _instance;
  }
  
  RN52::RN52() {
  }

  void RN52::init()
  {
      digitalWrite(StegoPhone::StegoPhone::rn52ENPin, true);
      //detect if RN52 is connected to uart. expect CMD\r\n
      char cmdTest[10];
      int cmdIdx = 0;
      while (Serial1.available() > 0) {
        int incomingByte = Serial1.read();
        if (cmdIdx < (sizeof(cmdTest)-1)) {
          cmdTest[cmdIdx++] = (char) incomingByte;
        }
      }
      cmdTest[cmdIdx++] = '\0';
    
      if (strcmp(cmdTest, "CMD\r\n") != 0) {
        this->exceptionOccurred = true;
      }
  }
  
  void RN52::rn52Command(const char *cmd) {
    if (this->exceptionOccurred) return;
    Serial1.write(cmd);
    Serial1.write('\n');
  }
  
  void RN52::rn52Exec(const char *cmd, char *buf, const int bufferSize, const int interDelay) {
    if (this->exceptionOccurred) return;
    rn52Command(cmd);
    delay(interDelay);
    Serial1.setTimeout(500);
    Serial1.readBytesUntil('\n', buf, bufferSize - 1);
  }
  
  unsigned short RN52::rn52Status() {  
    if (this->exceptionOccurred) return 0;
    Serial.println("QUERYING");
    char result[7];
    rn52Exec("Q", result, 7);
    for (int i=0; i<sizeof(result); i++)
      Serial.println((int) result[i]);
    result[4] = '\0';
    Serial.print("RN52:");
    Serial.println(result);
    Serial.println("---- END QUERY ----");
    return 0;
  }
  
  void RN52::rn52Debug(const char *cmd, const int interDelay, const int bufferSize) {
    if (this->exceptionOccurred) return;
    char cmdTest[bufferSize];
    int cmdIdx = 0;
    StegoPhone *stego = StegoPhone::getInstance();
    stego->displayMessageDual("RN52",cmd);
    rn52Command(cmd);
    delay(interDelay);
    stego->decimalOnPurple();
    while (Serial1.available() > 0) {
      int incomingByte = Serial1.read();
      if (cmdIdx < (sizeof(cmdTest)-1)) {
        cmdTest[cmdIdx++] = (char) incomingByte;
      } else {
        break;
      }
    }
    cmdTest[cmdIdx++] = '\0';
    stego->decimalOffPurple();
    Serial.print("RN52:");
    Serial.print(cmdTest);
  }
  
  RN52 *RN52::_instance = 0;
}
