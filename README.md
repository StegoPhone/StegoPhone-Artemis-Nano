
# StegoPhone : Steganography over Telephone
```
for Sparkfun Artemis Nano
(c) 2020 Jessica Mulein (jessica@mulein.com)
All rights reserved.
Made available under the GPLv3
```

- SparkFun Artemis Nano (https://www.sparkfun.com/products/15443)
- RN-52 Bluetooth via UART (https://www.sparkfun.com/products/12849)

* Qwiic - I2C to Keypad, and 2x Alphanumeric display
* GPIO09 - TX to RN-52 UART RX
* GPIO10 - RX to RN-52 UART TX
* GPIO08 - Keypad INT (Active LOW)
* GPIO00 - Device Enable to RN-52 (HIGH for >=1s to power on, can leave high or low after)
* GPIO01 - RN-52 GPIO9 (CMD enable LOW, DATA enable HIGH. LOW at boot)
* GPIO02 - RN-52 SPI SS
* GPIO13 - RN-52 MISO
* GPIO12 - RN-52 MOSI
* GPIO11 - RN-52 SCK

# I2C Devices

* SparkFun Qwiic Keypad 0x4B
* SparkFun Qwiic Alphanumeric Display 0x70 (Blue)
* SparkFun Qwiic Alphanumeric Display 0x72 (Purple)

```
    0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- 4b -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: 70 -- 72 -- -- -- -- --
```
