//################################################################################################
//## StegoPhone : Steganography over Telephone
//## (c) 2020 Jessica Mulein (jessica@mulein.com)
//## All rights reserved.
//## Made available under the GPLv3
//################################################################################################

// HEADERS
#include "stegophone.h"

void setup()
{
  StegoPhone::StegoPhone *stegoHandle = StegoPhone::StegoPhone::getInstance();
  stegoHandle->init();
}

void loop(void)
{
  StegoPhone::StegoPhone *stegoHandle = StegoPhone::StegoPhone::getInstance();
  stegoHandle->loop();
}
