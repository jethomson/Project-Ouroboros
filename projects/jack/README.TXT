This is a demostration program written for hacking the betemcu USBasp programmer.
http://jethomson.wordpress.com/2011/08/18/project-ouroboros-reflashing-a-betemcu-usbasp-programmer

If you want to reuse this code you MUST read USB-IDs-for-free.txt and change 
#define USB_CFG_VENDOR_NAME and #define USB_CFG_SERIAL_NUMBER in usbconfig.h
accordingly.

Because the code is triggered by the LED indicators changing it won't work in older versions of Linux that don't keep the Caps Lock and other lock keys in sync across multiple keyboards.

Disclaimer: This device provides keyboard input to a computing device outside of user control. It is intended for experimentation and demonstration purposes only. It should never be used in a situation that can result in loss of data, property, or finances. It should never be used in a situation that could cause harm to a person via operation or failure. Usage of this device is beyond my control and I am not responsible for any damages resulting.

The idea for this project came a comment at the Stealth USB CapsLocker article at
http://macetech.com/blog/?q=node/46

 * Description: Simulates a USB HID keyboard that prints out "All work and no
 *              play makes Jack a dull boy." This code is a modified version
 *              of Frank Zhao's "USB Business Card".
 *              I added support for an ATmega8 betemcu board and replaced the
 *              V-USB driver with a newer version.
 * Instructions: Compile and flash the program to your USB AVR device, open an
 *               empty text file, and press Caps Lock or Num Lock to activate.
 * Author: Jonathan Thomson
 * Based on: Frank Zhao's "USB Business Card" -- http://frank.circleofcurrent.com/cache/usbbusinesscard.htm
 *           Christian Starkjohann's "V-USB" -- http://www.obdev.at/products/vusb/index.html
 * Creation Date: 2011-10-18
 * Copyright: (c) 2011 Jonathan Thomson
 * License: GNU GPL v2 (see License.txt), GNU GPL v3
 * V-USB License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (usbdrv/CommercialLicense.txt)
