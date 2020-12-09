//################################################################################################
//## StegoPhone : Steganography over Telephone
//## (c) 2020 Jessica Mulein (jessica@mulein.com)
//## All rights reserved.
//## Made available under the GPLv3
//################################################################################################

#include <stddef.h>
#include "stegophone.h"

namespace StegoPhone
{
  StegoPhone *StegoPhone::_instance = 0;

  StegoPhone::StegoPhone() {
    this->userLEDStatus = true;
    this->rn52InterruptOccurred = false; // updated by ISR if RN52 has an event
    this->keypadInterruptOccurred = false; // used to keep track if there is a button on the stack

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
    digitalWrite(userLED, StegoPhone::userLEDStatus);
  
    attachInterrupt(digitalPinToInterrupt(keypadInterruptPin), intReadKeypad, FALLING);
    // Note, INT on the Keypad will "fall" from HIGH to LOW when a new button has been pressed.
    // Also note, it will stay low while there are still button events on the stack.
    // This is useful if you want to "poll" the INT pin, rather than use a hardware interrupt.
    attachInterrupt(digitalPinToInterrupt(rn52InterruptPin), intRN52Update, FALLING);
  
    Wire.begin(); //Join I2C bus
  }
  
  StegoPhone *StegoPhone::getInstance() {
    if (0 == _instance)
      _instance = new StegoPhone();
    return _instance;
  }

  void StegoPhone::setup() {
    this->stegoStatus = StegoState::InitializationStart;
      
    // at a minimum we need the alphanumeric display to indicate what we are doing
    // otherwise, all we have is the on-board LED we'll use to blink for activity
    if (this->displayBlue.begin(0x70) == false) {
      this->stegoStatus = StegoState::InitializationFailure,
      this->blinkForever();
    } else if (displayPurple.begin(0x72) == false) {
      // have one display, but not both. able to display error on one
      this->displayMessage(displayBlue, "-DSP2");
      this->stegoStatus = StegoState::InitializationFailure;
      this->blinkForever();
    }
    // displays are ok
  
    this->stegoStatus = StegoState::DisplayInitialized;
    this->displayMessageDual("STEG","BOOT");
    delay(500);
  
    if (keypad.begin() == false)   // Note, using begin() like this will use default I2C address, 0x4B.
    {                              // You can pass begin() a different address like so: keypad1.begin(Wire, 0x4A).
      this->displayMessageDual("KYPD","MSNG");
      this->stegoStatus = StegoState::InitializationFailure;
      this->blinkForever();
    }
    // keypad input is ok
  
    // boot RN-52
    this->displayMessageDual("RN52","INIT");
    RN52 *rn52 = RN52::getInstance();
    // try to initialize
    if (rn52->init()) {
      this->displayMessageDual("RN52","ERR");
      this->stegoStatus = StegoState::InitializationFailure;
      this->blinkForever();
    } else {
      this->displayMessageDual("STEG","RDY");
      this->stegoStatus = Ready; 
    }
  }

  void StegoPhone::loop()
  {
      // blink if you can hear me
      if (this->keypadInterruptOccurred || this->rn52InterruptOccurred)
        this->toggleUserLED();

      // process any keypad inputs first
      if (this->keypadInterruptOccurred) {
        this->keypad.updateFIFO();  // necessary for keypad to pull button from stack to readable register
        const byte pressedDigit = keypad.getButton();
        if (pressedDigit == -1) {
          // fifo cleared
        } else {
          // TODO: do something with these digits
          this->displayMessage(displayBlue, (char) pressedDigit);
        }
        this->keypadInterruptOccurred = false;
      }

      // give the RN52 a chance to handle its inputs
      RN52 *rn52 = RN52::getInstance();
      rn52->loop(this->rn52InterruptOccurred);
      this->rn52InterruptOccurred = false;
    
      switch (this->stegoStatus) {
        case Ready:
        // intentional fallthrough
        default:
          break;
      }
  }

  void StegoPhone::setUserLED(bool newValue) {
    this->userLEDStatus = newValue;
    digitalWrite(userLED, this->userLEDStatus);
  }
  
  void StegoPhone::toggleUserLED() {
        setUserLED(!this->userLEDStatus);
  }
  
  void StegoPhone::blinkForever(int interval) {
      while (1) {
        this->toggleUserLED();
        delay(interval);
      }
  }
  
  void StegoPhone::displayClearDual() {
    this->displayBlue.clear();
    this->displayPurple.clear();
  }
  
  void StegoPhone::displayMessage(HT16K33 dsp, const char *str) {
    dsp.clear();
    dsp.print(str);
  }
  
  void StegoPhone::displayMessage(HT16K33 dsp, const char str) {
    dsp.clear();
    dsp.print(str);
  }
  
  void StegoPhone::displayMessageDual(const char *strBlue, const char *strPurple) {
    this->displayClearDual();
    this->displayBlue.print(strBlue);
    this->displayPurple.print(strPurple);
  }

  void StegoPhone::decimalOnBlue() {
    this->displayBlue.decimalOn();
  }

  void StegoPhone::decimalOnPurple() {
    this->displayPurple.decimalOn();
  }

  void StegoPhone::decimalOffBlue() {
    this->displayBlue.decimalOff();
  }

  void StegoPhone::decimalOffPurple() {
    this->displayPurple.decimalOff();
  }

  // intReadKeypad() is triggered via FALLING interrupt pin (digital pin 2)
  // it them updates the global variable keypadInterruptOccurred
  void StegoPhone::intReadKeypad() // (static isr) keep this quick; no delays, prints, I2C allowed.
  {
    StegoPhone::getInstance()->keypadInterruptOccurred = true;
  }

  void StegoPhone::intRN52Update() // (static isr)
  {
    StegoPhone::getInstance()->rn52InterruptOccurred = true;
  }
}
