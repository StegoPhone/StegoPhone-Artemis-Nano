//################################################################################################
//## StegoPhone : Steganography over Telephone
//## (c) 2020 Jessica Mulein (jessica@mulein.com)
//## All rights reserved.
//## Made available under the GPLv3
//################################################################################################

// HEADERS
#include <ArduinoJson.h>
#include <SPI.h>
#include <SparkFun_Alphanumeric_Display.h>
#include <SparkFun_Qwiic_Keypad_Arduino_Library.h>
#include <Wire.h>
//include <ArduinoBLE.h>
//================================================================================================


// HARDWARE HANDLES
//================================================================================================
HT16K33 displayBlue;
HT16K33 displayPurple;
KEYPAD keypad;
//================================================================================================

// PIN DEFINITIONS
//================================================================================================
const byte rn52ENPin = 0;          // active high
const byte rn52CMDPin = 1;         // active low
const byte rn52SPISel = 2;         //
const byte rn52InterruptPin = 7; // input no pull, active low 100ms
const byte keypadInterruptPin = 8; // input no pull, active low
const byte userLED = 19;
//================================================================================================

// ISR MODIFIED
//================================================================================================
volatile bool userLEDStatus = true;
volatile boolean rn52StatusUpdated = false; // updated by ISR if RN52 has an event
volatile boolean buttonAvailable = false; // used to keep track if there is a button on the stack
//================================================================================================

enum StegoState {
  Initializing,
  InitFailure,
  Ready
};

StegoState stegoStatus = Initializing;

void setUserLED(bool newValue) {
  userLEDStatus = newValue;
  digitalWrite(userLED, userLEDStatus);
}

void toggleUserLED() {
      setUserLED(!userLEDStatus);
}

void blinkForever(int interval = 1000) {
    while (1) {
      toggleUserLED();
      delay(interval);
    }
}

void displayClearDual() {
  displayBlue.clear();
  displayPurple.clear();
}

void displayMessage(HT16K33 &dsp, const char *str) {
  dsp.clear();
  dsp.print(str);
}

void displayMessage(HT16K33 &dsp, const char str) {
  dsp.clear();
  dsp.print(str);
}

void displayMessageDual(const char *strBlue, const char *strPurple) {
  displayClearDual();
  displayBlue.print(strBlue);
  displayPurple.print(strPurple);
}

void intReadKeypad() // keep this quick; no delays, prints, I2C allowed.
{
  buttonAvailable = true;
}

void intRN52Update()
{
  rn52StatusUpdated = true;
}

void rn52Command(const char *cmd) {
  Serial1.write(cmd);
  Serial1.write('\n');
}

void rn52Debug(const char *cmd, const int interDelay = 50, const int bufferSize = 1024) {
  char cmdTest[bufferSize];
  int cmdIdx = 0;
  displayMessageDual("RN52",cmd);
  rn52Command(cmd);
  delay(interDelay);
  displayPurple.decimalOn();
  while (Serial1.available() > 0) {
    int incomingByte = Serial1.read();
    if (cmdIdx < (sizeof(cmdTest)-1)) {
      cmdTest[cmdIdx++] = (char) incomingByte;
    } else {
      break;
    }
  }
  cmdTest[cmdIdx++] = '\0';
  displayPurple.decimalOff();
  Serial.print("RN52:");
  Serial.print(cmdTest);
}

void setup()
{
  Serial.begin(9600); // console/debug
  Serial1.begin(115200); // Connected to RN52
  pinMode(rn52ENPin, OUTPUT);
  pinMode(rn52CMDPin, OUTPUT);
  pinMode(rn52SPISel, OUTPUT);
  pinMode(rn52InterruptPin, INPUT); // Qwiic Keypad holds INT pin HIGH @ 3.3V, then LOW when fired.
  pinMode(keypadInterruptPin, INPUT); // Qwiic Keypad holds INT pin HIGH @ 3.3V, then LOW when fired.
  // Note, this means we do not want INPUT_PULLUP.
  pinMode(userLED, OUTPUT);

  digitalWrite(rn52ENPin, false);
  digitalWrite(rn52CMDPin, false); // cmd mode by default
  digitalWrite(rn52SPISel, false);
  digitalWrite(userLED, userLEDStatus);

  attachInterrupt(digitalPinToInterrupt(keypadInterruptPin), intReadKeypad, FALLING);
  // Note, INT on the Keypad will "fall" from HIGH to LOW when a new button has been pressed.
  // Also note, it will stay low while there are still button events on the stack.
  // This is useful if you want to "poll" the INT pin, rather than use a hardware interrupt.
  attachInterrupt(digitalPinToInterrupt(rn52InterruptPin), intRN52Update, FALLING);

  Wire.begin(); //Join I2C bus

  if (displayBlue.begin(0x70) == false) {
    stegoStatus = InitFailure;
    blinkForever();
  } else if (displayPurple.begin(0x72) == false) {
    displayMessage(displayBlue, "-DSP2");
    stegoStatus = InitFailure;
    blinkForever();
  }
  // displays ok

  displayMessageDual("STEG","BOOT");
  delay(500);

  if (keypad.begin() == false)   // Note, using begin() like this will use default I2C address, 0x4B.
  {                              // You can pass begin() a different address like so: keypad1.begin(Wire, 0x4A).
    displayMessageDual("KYPD","MSNG");
    stegoStatus = InitFailure;
    while (1);
  }

  // boot RN-52
  displayMessageDual("RN52","INIT");
  digitalWrite(rn52ENPin, true);
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
    stegoStatus = InitFailure;
    displayMessageDual("RN52","ERR");
    blinkForever();
  }

  displayMessageDual("STEG","RDY");
  stegoStatus = Ready;
  rn52Debug("Q", 50, 1024);
}

void loop(void)
{
  // intReadKeypad() is triggered via FALLING interrupt pin (digital pin 2)
  // it them updates the global variable buttonAvailable
  if (buttonAvailable) {
    keypad.updateFIFO();  // necessary for keypad to pull button from stack to readable register
    const byte pressedDigit = keypad.getButton();
    if (pressedDigit == -1) {
      Serial.println("BRK!");
      // fifo cleared
      buttonAvailable = false;
    } else {
      displayMessage(displayBlue, (char) pressedDigit);
      Serial.println((char) pressedDigit);
    }
    // redundant/temporary: https://twitter.com/JessicaMulein/status/1336283417831374850?s=20
    buttonAvailable = false;
  }

  if (rn52StatusUpdated) {
    Serial.println("RN52 event");
    toggleUserLED();
    rn52StatusUpdated = false;
  }

  switch (stegoStatus) {
    case Ready:
    // intentional fallthrough
    default:
      break;
  }
}
