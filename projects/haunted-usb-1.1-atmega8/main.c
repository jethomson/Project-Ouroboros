/*
   Haunted USB 1.1 with support for ATmega8
   A few modifications added to the original by Jonathan Thomson.
   2011: added support for ATmega betemcu board in main.c and usbconfig.h. added Makefile_linux
   2019: updated the V-USB driver and a minor change to main.c so it would compile.
   https://jethomson.wordpress.com/2011/08/18/project-ouroboros-reflashing-a-betemcu-usbasp-programmer/

   Disclaimer: This device provides keyboard input to a computing device outside of user control. 
   It is intended for experimentation and demonstration purposes only. It should never be used in 
   a situation that can result in loss of data, property, or finances. It should never be used in 
   a situation that could cause harm to a person via operation or failure. Usage of this device 
   is beyond my control and I am not responsible for any damages resulting.

*/

/* 
DonP 20080624

Attribution and License:

 In accordance with the licensing information and guidelines for the
 OBJECTIVE DEVELOPMENT USB driver code for microcontrollers (which this
 is based on) I am including the following:

    This is source code for my "Haunted, Mystery USB Device".
	Copyright (C) 2008  Donald Papp (donp@imakeprojects.com)

 (More specifically, the portions changed and written entirely by me
 - as well as the circuitboard design to go with this code, which is not
 included here - are copyrighted by me.  However, they are open sourced under
 the GPL.)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.


History:
	This source code began as the program for the Thermostat based on AVR USB driver
	by Christian Starkjohann.  (c) 2006 by OBJECTIVE DEVELOPMENT Software GmbH.

	That code served as the basis for Garrett Mace to make his Stealth USB CapsLocker
	(http://www.macetech.com/blog/?q=node/46) which served as partial inspiration to me.

	Learning from that code (and using it as a base) this code is now the
	source for the HAUNTED, MYSTERY USB DEVICE (working name).
	(http://www.imakeprojects.com/Projects/haunted-usb/)


June 24, 2008
		- Altered the timing and delays of the injected keyboard events.
		- Added functionality for an external LED which is flickered in a
		malfunctioning-looking way.
		- Added events (other than capslock toggling) and code to select between them.
		- Added prng() function for use as an RNG.

July 10, 2008
		- Modified code so that CAPS is toggled 2x right off the bat everytime so you
		can see it's working when you plug it in.  Does this via a global variable
		that is set and never unset.
		As long as 'sendInitialCapsLock' is >0 then the random delay and selection of key
		to send is overridden.  The var is decremented once the capslock keypress is built
		into a report.  We want caps lock toggled twice shortly after startup so that
		you can see it's working. (Either blinks on, or blinks off, depending on the
		initial state.)

*/

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>

#include "usbconfig.h"
#include "usbdrv/usbdrv.h"
#include "usbdrv/oddebug.h"


/* ------------------------------------------------------------------------- */

static uchar    reportBuffer[2];    /* buffer for HID reports */
static uchar    idleRate;           /* in 4 ms units */
static uchar    reportCount;        /* current report */
static unsigned int TimerDelay;     /* counter for delay period */

// DonP
// Whether to override normal delay and random key in reports.
static uchar sendInitialCapsLock;

/* ------------------------------------------------------------------------- */

PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { /* USB report descriptor */
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x06,                    // USAGE (Keyboard)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
	0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
	0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
	0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
	0x75, 0x01,                    //   REPORT_SIZE (1)
	0x95, 0x08,                    //   REPORT_COUNT (8)
	0x81, 0x02,                    //   INPUT (Data,Var,Abs)
	0x95, 0x01,                    //   REPORT_COUNT (1)
	0x75, 0x08,                    //   REPORT_SIZE (8)
	0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
	0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
	0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
	0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
	0xc0                           // END_COLLECTION
};


// -------------------------------------------------------------------------
// DonP 20080424
// Pretty good and fast pseudo-random number generator
// From Glen Worstell, in entry A3650 in CC AVR design contest
long int prng(long int seed)  // Call with seed value 1<= seed <= 0x7FFFFFFF-1
{
	// 0x7FFFFFFF is a magic number related to the prng function
	seed=(seed>>16) + ((seed<<15) & 0x7FFFFFFF) - (seed>>21) - ((seed<<10) & 0x7FFFFFFF);
	if( seed<0 )
		seed += 0x7FFFFFFF;
	return seed;
}
// End DonP
// -------------------------------------------------------------------------


/* We use a simplifed keyboard report descriptor which does not support the
 * boot protocol. We don't allow setting status LEDs and we only allow one
 * simultaneous key press (except modifiers). We can therefore use short
 * 2 byte input reports.
 * The report descriptor has been created with usb.org's "HID Descriptor Tool"
 * which can be downloaded from http://www.usb.org/developers/hidpage/.
 * Redundant entries (such as LOGICAL_MINIMUM and USAGE_PAGE) have been omitted
 * for the second INPUT item.
 */

static void buildReport(void)
{
	uchar   key = 0;
	uchar   randomkey = 0;

	// DonP 20080624
	// Random number generation
	long int x;
	// Use some value as a seed
	x = prng((long int)rand());
	// x is a 32 bit number
	x &= 0x0000FF00; // Axe all but these 8 bits
	x = (x >> 8); // Shift over to the LSB side
	// x should now be between 0 and 255
	// Using the indicated bits seems to work better
	// for lower values and seeds.
	x = (x / 32);	// x should be 0-7 now
	// 0x39 Caps lock
	// 0x49 Insert
	// 0x2A Backspace
	// 0x2C Space
	// 0x53 Numlock
	// 0x2B Tab
	// 0x4C Delete
	if( x == 0 )
		randomkey = 0x39;
	if( x == 1 )
		randomkey = 0x49;
	if( x == 2 )
		randomkey = 0x2A;
	if( x == 3 )
		randomkey = 0x2C;
	if( x == 4 )
		randomkey = 0x53;
	if( x == 5 )
		randomkey = 0x2B;
	if( x == 6 )
		randomkey = 0x4C;
	if( x == 7 )
		randomkey = 0x39;	// Caps Lock again to round it out
	// End DonP


	if(reportCount == 0)
	{
		// DonP - If the report struct is ready for more (reportCount=0)
		// then override it if necessary.  Otherwise leave it alone and 
		// the existing 'randomkey' value will be built into a report below.
		//
		// Also, remember that a keypress means a code was
		// present in one report, then absent from a subsequent one.
		// So if we sent 0x39 in the last one, let this next one be '0'.
		// We do this by only overriding when it's time for a report.
		// Otherwise it builds in the default '0' (empty) value.

		if( sendInitialCapsLock > 0 )
		{
			/*
			/// DEBUG ONLY --- REQUIRES LED ON PC1 of ATmega8 //
			DDRC |= _BV(1);	// Set PC1 as output (1), leave others alone
			PORTC &= ~_BV(1); // red LED on
			_delay_ms(100);
			PORTC |= _BV(1); // red LED off
			////////////////////////////////////////////////
			*/

			// We always enter this function 1 time right off the bat, so ignore it
			// if that's the case (actually, make it an empty report).
			if( sendInitialCapsLock!=3 )
			{
				randomkey = 0x39;		// CapsLock
			}
			else
			{
				randomkey = 0x00;
			}

			sendInitialCapsLock--;
		}

	key = randomkey;
	}


	reportCount++;
	reportBuffer[0] = 0;    /* no modifiers */
	reportBuffer[1] = key;
}

static void timerPoll(void)
{
	static unsigned int timerCnt;

#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
	if(TIFR & (1 << TOV1))
	{
		TIFR = (1 << TOV1); /* clear overflow */
#elif defined(__AVR_ATmega8__)
	if (TIFR & (1 << OCF1A))
	{
		// clear interrupt
		TIFR = (1 << OCF1A);
#endif
		/* check for end of pseudorandom delay */
		if( ++timerCnt >= TimerDelay )
		{
			//TimerDelay = 2835 + rand();    /* 1/63s * 63 * 30 + 0...32767 */
			TimerDelay = 315; // DonP Test, 5 seconds constant

			// DonP - ensure delay is overridden to ~1 second if this flag is set.
			// Actually only if it's greater than 1 otherwise the next event AFTER
			// the initial caps lock toggle is 1 second after the last toggle.
			// In other words if the flag is 1 the NEXT delay is what we want to be normal.
			// (Not the one after the flag is 0.)
			if( sendInitialCapsLock > 1)
				TimerDelay = 60;

			timerCnt = 0;
			reportCount = 0; /* start report */
		}
	}
}

/* ------------------------------------------------------------------------- */

static void timerInit(void)
{

#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
	// (16.5 MHz/1024 [ticks/sec])*[1 interrupt/256 ticks] = 62.94 interrupts/sec
	TCCR1 = 0x0b;  // start timer, 1024 prescale
#elif defined(__AVR_ATmega8__)
	// since atmega8 uses a different clock rate, use CTC mode to generate
	// the same number of interrupts per second.
	TCCR1B |= (1 << WGM12); // put timer1 in CTC mode
	OCR1A = (F_CPU/8)/63;   // number of prescaled ticks per interrupt
	TCCR1B |= (1 << CS11);  // start timer, 8 prescale
#endif
}


/* ------------------------------------------------------------------------- */
/* ------------------------ interface to USB driver ------------------------ */
/* ------------------------------------------------------------------------- */

uchar	usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;

usbMsgPtr = reportBuffer;
	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
		if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			/* we only have one report type, so don't look at wValue */
			buildReport();
			return sizeof(reportBuffer);
		}else if(rq->bRequest == USBRQ_HID_GET_IDLE){
			usbMsgPtr = &idleRate;
			return 1;
		}else if(rq->bRequest == USBRQ_HID_SET_IDLE){
			idleRate = rq->wValue.bytes[1];
		}
	}else{
		/* no vendor specific requests implemented */
	}
	return 0;
}

#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
/* ------------------------------------------------------------------------- */
/* ------------------------ Oscillator Calibration ------------------------- */
/* ------------------------------------------------------------------------- */

/* Calibrate the RC oscillator to 8.25 MHz. The core clock of 16.5 MHz is
 * derived from the 66 MHz peripheral clock by dividing. Our timing reference
 * is the Start Of Frame signal (a single SE0 bit) available immediately after
 * a USB RESET. We first do a binary search for the OSCCAL value and then
 * optimize this value with a neighboorhod search.
 * This algorithm may also be used to calibrate the RC oscillator directly to
 * 12 MHz (no PLL involved, can therefore be used on almost ALL AVRs), but this
 * is wide outside the spec for the OSCCAL value and the required precision for
 * the 12 MHz clock! Use the RC oscillator calibrated to 12 MHz for
 * experimental purposes only!
 */
static void calibrateOscillator(void)
{
uchar       step = 128;
uchar       trialValue = 0, optimumValue;
int         x, optimumDev, targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);

	/* do a binary search: */
	do{
		OSCCAL = trialValue + step;
		x = usbMeasureFrameLength();    /* proportional to current real frequency */
		if(x < targetValue)             /* frequency still too low */
			trialValue += step;
		step >>= 1;
	}while(step > 0);
	/* We have a precision of +/- 1 for optimum OSCCAL here */
	/* now do a neighborhood search for optimum value */
	optimumValue = trialValue;
	optimumDev = x; /* this is certainly far away from optimum */
	for(OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++){
		x = usbMeasureFrameLength() - targetValue;
		if(x < 0)
			x = -x;
		if(x < optimumDev){
			optimumDev = x;
			optimumValue = OSCCAL;
		}
	}
	OSCCAL = optimumValue;
}
/*
Note: This calibration algorithm may try OSCCAL values of up to 192 even if
the optimum value is far below 192. It may therefore exceed the allowed clock
frequency of the CPU in low voltage designs!
You may replace this search algorithm with any other algorithm you like if
you have additional constraints such as a maximum CPU clock.
For version 5.x RC oscillators (those with a split range of 2x128 steps, e.g.
ATTiny25, ATTiny45, ATTiny85), it may be useful to search for the optimum in
both regions.
*/

void    usbEventResetReady(void)
{
	calibrateOscillator();
	eeprom_write_byte(0, OSCCAL);   /* store the calibrated value in EEPROM */
}
#endif






/* ------------------------------------------------------------------------- */
/* --------------------------------- main ---------------------------------- */
/* ------------------------------------------------------------------------- */

int main(void)
{
uchar   i;

#if defined(__AVR_ATtiny85__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny25__)
uchar   calibrationValue;
	calibrationValue = eeprom_read_byte(0); /* calibration value from last time */
	if(calibrationValue != 0xff){
		OSCCAL = calibrationValue;
	}
#endif
	odDebugInit();
	usbDeviceDisconnect();
	for(i=0;i<20;i++){  /* 300 ms disconnect */
		_delay_ms(15);
	}
	usbDeviceConnect();

	wdt_enable(WDTO_1S);
	timerInit();
	//TimerDelay = 630; /* initial 10 second delay */
	TimerDelay = 200; /* initial ~3 second delay */


	// DonP - 20080710
	// By setting the 'sendInitialCapsLock' override flag to 4, after powerup
	// it will wait 3 seconds (initial time delay) then toggle the CAPS lock twice.
	// Non-zero 'sendInitialCapsLock' means:
	// 1. Override key selection to be alternately the 'CapsLock' event, or an empty report.
	// 2. Override random interval delay (after initial 3 second delay) to be 1 second.
	// 3. Decrement the 'sendInitialCapsLock' value by 1.

	sendInitialCapsLock = 3; // DonP - set flag to override key selection this many times+1.
	// (+1 because buildreport runs at powerup once and we want to ignore that.)


	usbInit();
	sei();
	for(;;){    /* main event loop */
		wdt_reset();
		usbPoll();

		/* A USB keypress cycle is defined as a scancode being present in a report, and
		then absent from a later report. To press and release the Caps Lock key, instead of
		holding it down, we need to send the report with the Caps Lock scancode and
		then an empty report. */

		if(usbInterruptIsReady() && reportCount < 2){ /* we can send another key */
			buildReport();
			usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
		}
		timerPoll();
	}
	return 0;
}
