/******************************************************************************
 * The MIT License
 *
 * Copyright (c) 2010 LeafLabs LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *****************************************************************************/

/**
 *  @file config.h
 *
 *  @brief bootloader settings and macro defines
 *
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H


#include "common.h"


// uncomment to not do the fast blink at startup
// #define DISABLE_STARTUP_FAST_BLINK

// Speed controls for strobing the LED pin
#define BLINK_FAST 					0x33000
#define BLINK_SLOW 					0x100000

// default wait times in blink_slow periods if not explicitly defined
#define WAIT_SHORT 					4
#define WAIT_LONG 					15

// do we enter bootloader on certain reset methods only?
#define RESET_ACTIVATION 			RESET_POR | RESET_EXT
// how about the button? can it override this? uncomment if so
#define BL_BUTTON_ALWAYS_WORKS

// The USB clock bit is the same for all boards
#define RCC_APB1ENR_USB_CLK   		0x00800000 

// used by the bootloader for something
#define LARGEST_FLASH_PAGE_SIZE 	0x800

// Jump locations for legacy (0x8005000) and new / smaller (0x8002000) bootloader
#define USER_CODE_FLASH0X8005000   	((u32)0x08005000)
#define USER_CODE_FLASH0X8002000	((u32)0x08002000)

// Upload to RAM has been removed / depreacted so these values a not used any more
#define USER_CODE_RAM     			((u32)0x20000C00)

// RAM_END, set ram end to the end of ram on the device wth the least RAM (STM32F103C)
// btw whoever did this, this is poor as shit practice, come on now
#define RAM_END           			((u32)0x20005000)


/* Porting information Please read.
	The following defines are used to set up the hardware of your target board.
	See http://www.st.com/web/en/resource/technical/document/reference_manual/CD00171190.pdf

	Generally, two GPIO pins need to be defined; these are for the LED and the Button.
	
	The following is required:
	- LED_BANK        - this is the GPIO port for the LED, e.g. GPIOA, GPIOB, ...
	- LED_PIN         - this is the pin number e.g. for PB1, set the value to 1
	- LED_ON_STATE    - the GPIO state to light the LED.
				        if the GPIO is connected to cathode, set 0. if anode, set 1.
	
	- BUTTON_BANK     - this is the GPIO port for the button, e.g. GPIOA, GPIOB, ...
	- BUTTON_PIN      - this is the pin number e.g. for PBC14, set the value to 14
	- BUTTON_ON_STATE - set to the required value for button activation. a pull-up or
						pull-down will be set as appropriate to make the button work.
						set to 0 if your button shorts to GND on press, 1 if to VCC
*/

#if defined TARGET_MAPLE_MINI

	#define HAS_MAPLE_HARDWARE 1

	#define LED_BANK         GPIOB
	#define LED_PIN          1
	#define LED_ON_STATE	 1	

	/* On the Mini, BUT is PB8 */
	#define BUTTON_BANK      GPIOB
	#define BUTTON_PIN       8
	#define BUTTON_ON_STATE  1
	
	/* USB Disc Pin Setup.   USB DISC is PB9 */
	#define USB_DISC_BANK	 GPIOB
	#define USB_DISC_PIN     9
	
#elif defined TARGET_MAPLE_REV3

	#warning "Target MAPLE_REV3"

// Flag that this type of board has the custom maple disconnect hardware
	#define HAS_MAPLE_HARDWARE 1
	
	#define LED_BANK         GPIOB
	#define LED_PIN          1
	#define LED_ON_STATE	 1

	#define BUTTON_BANK      GPIOB
	#define BUTTON_PIN       8
	#define BUTTON_ON_STATE  1	

	/* USB Disc Pin Setup.   USB DISC is PB9 */
	#define USB_DISC_BANK    GPIOB
	#define USB_DISC_PIN     9
	
#elif defined TARGET_MAPLE_REV5

// Flag that this type of board has the custom maple disconnect hardware
	#define HAS_MAPLE_HARDWARE 1

	#define LED_BANK         GPIOA
	#define LED_PIN          5
	#define LED_ON_STATE	 0

	/* On the Mini, BUT is PB8 */
	#define BUTTON_BANK      GPIOC
	#define BUTTON_PIN       9

	/* USB Disc Pin Setup.   USB DISC is PB9 */
	#define USB_DISC_BANK    GPIOB
	#define USB_DISC_PIN     9
	
#elif defined TARGET_GENERIC_F103_PC13
	
	#define LED_BANK		 GPIOC 
	#define LED_PIN 	 	 13
	#define LED_ON_STATE	 0

	// Button (if you have one)
	#define BUTTON_BANK      GPIOC
	#define BUTTON_PIN       14
	#define BUTTON_ON_STATE  1
	
	
#elif defined TARGET_GENERIC_F103_PG15
	
	#define LED_BANK		 GPIOG 
	#define LED_PIN 		 15
	#define LED_ON_STATE	 1

	// Button (if you have one)
	#define BUTTON_BANK      GPIOC
	#define BUTTON_PIN       14
	#define BUTTON_ON_STATE  1
	
#elif defined TARGET_GENERIC_F103_PD2
	
	#define LED_BANK		 GPIOD 
	#define LED_PIN 		 2
	#define LED_ON_STATE	 1

	// Button (if you have one)
	#define BUTTON_BANK      GPIOC
	#define BUTTON_PIN       14
	#define BUTTON_ON_STATE  1
	
#elif defined TARGET_GENERIC_F103_PD1
	
	#define LED_BANK		 GPIOD 
	#define LED_PIN 		 1
	#define LED_ON_STATE	 1

	// Button (if you have one)
	#define BUTTON_BANK      GPIOC
	#define BUTTON_PIN       14
	#define BUTTON_ON_STATE  1

#elif defined TARGET_GENERIC_F103_PA1
	
	#define BOOTLOADER_WAIT  WAIT_LONG

	#define LED_BANK		 GPIOA 
	#define LED_PIN 		 1
	#define LED_ON_STATE	 1

	// Button (if you have one)
	#define BUTTON_BANK      GPIOC
	#define BUTTON_PIN       14
	#define BUTTON_ON_STATE  1

	#define USB_DISC_HARDWIRED 1

#elif defined TARGET_GENERIC_F103_PB9
	
	#define LED_BANK		 GPIOB 
	#define LED_PIN 		 9
	#define LED_ON_STATE	 1

	// Button (if you have one)
	#define BUTTON_BANK      GPIOC
	#define BUTTON_PIN       14
	#define BUTTON_ON_STATE  1

#elif defined TARGET_GENERIC_F103_PE2
	
	#define LED_BANK		 GPIOE 
	#define LED_PIN 		 2
	#define LED_ON_STATE	 1

#elif defined TARGET_GENERIC_F103_PA9
	
	#define LED_BANK		 GPIOA 
	#define LED_PIN 		 9
	#define LED_ON_STATE	 1
	
#elif defined TARGET_GENERIC_F103_PE5
	
	#define LED_BANK		 GPIOE 
	#define LED_PIN 		 5
	#define LED_ON_STATE	 1	
	
	#define BUTTON_BANK      GPIOD
	#define BUTTON_PIN       2
	#define BUTTON_ON_STATE  1
	
#elif defined TARGET_GENERIC_F103_PE5_BUTTON_PA0
	
	#define LED_BANK		 GPIOE 
	#define LED_PIN 		 5
	#define LED_ON_STATE	 1	
	
	#define BUTTON_BANK      GPIOA
	#define BUTTON_PIN       0
	#define BUTTON_ON_STATE  1	

#elif defined TARGET_GENERIC_F103_PB7
	
	#define LED_BANK		 GPIOB 
	#define LED_PIN 		 7
	#define LED_ON_STATE	 1	

#elif defined TARGET_GENERIC_F103_PB0
	
	#define LED_BANK		 GPIOB 
	#define LED_PIN 		 0
	#define LED_ON_STATE	 1	
	#define BOOTLOADER_WAIT  30

#elif defined TARGET_STBEE
	
	#define HAS_MAPLE_HARDWARE 1

	#define LED_BANK		 GPIOD 
	#define LED_PIN 		 4
	#define LED_ON_STATE	 0	

	/* BUTTON is PA0 (pull down) */
	#define BUTTON_BANK		 GPIOA
	#define BUTTON_PIN 		 0
	#define BUTTON_ON_STATE	 1

	/* USB Disc Pin Setup.   USB DISC is PD3 */
	#define USB_DISC_BANK	 GPIOD
	#define USB_DISC_PIN     3

	/* CRISTAL 12MHz */
	#define XTAL12M
	
#elif defined TARGET_NAZE32
	
	#define LED_BANK		 GPIOB 
	#define LED_PIN 		 3
	#define LED_ON_STATE	 0

#elif defined TARGET_ANDNXOR_DC24_BENDER
	
	#define BOOTLOADER_WAIT  2
	#define USB_VENDOR_STR_LEN 16
	#define USB_VENDOR_MSG_STR 'A', 0, 'N', 0, 'D', 0, '!', 0, 'X', 0, 'O', 0, 'R', 0
	#define USB_PRODUCT_STR_LEN 10
    #define USB_PRODUCT_MSG_STR 'D', 0, 'C', 0, '2', 0, '4', 0

	/* lightsense LED on PB0:K, PB1:A */
	#define LED_BANK 		 GPIOB
	#define LED_PIN 		 1
	#define LED_ON_STATE 	 1
	#define LED_PIN_K 		 0 	// mcu-attached cathode

	/* SWITCH is */
	#define BUTTON_BANK		 GPIOC
	#define BUTTON_PIN 		 14
	#define BUTTON_ON_STATE	 0

	/* USB Disc Pin Setup.   USB DISC is PA2 */
	#define USB_DISC_BANK	 GPIOA
	#define USB_DISC_PIN     2

#elif defined TARGET_TC_GRIPDAPTOR
	
	#define BOOTLOADER_WAIT  0

	/* P2 orange LED on PB3 */
	#define LED_BANK		 GPIOB 
	#define LED_PIN 		 3
	#define LED_ON_STATE	 1

	/* SWITCH is PB2 (pull down) */
	#define BUTTON_BANK		 GPIOB
	#define BUTTON_PIN 		 2
	#define BUTTON_ON_STATE	 1

	/* USB Disc Pin Setup.   USB DISC is PA8 */
	#define USB_DISC_BANK    GPIOA
	#define USB_DISC_PIN     8

	#define CUSTOM_VID_PID
	#define VEND_ID0 0xBA
	#define VEND_ID1 0xD5
	#define PROD_ID0 0xAD
	#define PROD_ID1 0xA3

	#define USB_VENDOR_STR_LEN 24
	#define USB_VENDOR_MSG_STR 't', 0, 'r', 0, 'u', 0, 'e', 0, 'C', 0, 'o', 0, 'n', 0, 't', 0,'r', 0, 'o', 0, 'l', 0

	#define USB_PRODUCT_STR_LEN 32
	#define USB_PRODUCT_MSG_STR 'G', 0, 'R', 0, 'i', 0, 'P', 0, ' ', 0, 'B', 0, 'o', 0, 'o', 0, 't', 0, 'l', 0, \
								'o', 0, 'a', 0, 'd', 0, 'e', 0, 'r', 0

	#define USB_SERIAL_STR_LEN 16
	#define USB_SERIAL_MSG_STR '1', 0, '2', 0, '3', 0, '4', 0, '3', 0, '2', 0, '1', 0
	
	#define CUSTOM_ALT_STRINGS
	#define ALT0_STR_LEN 0x64
	#define ALT0_MSG_STR 'G',0,'r',0,'I',0,'P',0,' ',0,'B',0,'o',0,'o',0,'t',0,'l',0,'o',0,'a',0,'d',0,'e',0,'r',0, \
						 ' ',0,'v',0,VER_MAJOR,0,'.',0,VER_MINOR,0,' ',0,' ',0,'U',0,'p',0,'l',0,'o',0,'a',0,'d',0, \
						 ' ',0,'t',0,'o',0,' ',0,'R',0,'A',0,'M',0,' ',0,'n',0,'o',0,'t',0,' ',0,'s',0,'u',0,'p',0, \
						 'p',0,'o',0,'r',0,'t',0,'e',0,'d',0

	#define ALT1_STR_LEN 0x60
	#define ALT1_MSG_STR 'G',0,'r',0,'I',0,'P',0,' ',0,'B',0,'o',0,'o',0,'t',0,'l',0,'o',0,'a',0,'d',0,'e',0,'r',0, \
						 ' ',0,'v',0,VER_MAJOR,0,'.',0,VER_MINOR,0,' ',0,' ',0,'U',0,'p',0,'l',0,'o',0,'a',0,'d',0, \
						 ' ',0,'t',0,'o',0,' ',0,'F',0,'l',0,'a',0,'s',0,'h',0,' ',0,'0',0,'x',0,'8',0,'0',0,'0',0, \
						 '5',0,'0',0,'0',0,'0',0

	#define ALT2_STR_LEN 0x60
	#define ALT2_MSG_STR 'G',0,'r',0,'I',0,'P',0,' ',0,'B',0,'o',0,'o',0,'t',0,'l',0,'o',0,'a',0,'d',0,'e',0,'r',0, \
						 ' ',0,'v',0,VER_MAJOR,0,'.',0,VER_MINOR,0,' ',0,' ',0,'U',0,'p',0,'l',0,'o',0,'a',0,'d',0, \
						 ' ',0,'t',0,'o',0,' ',0,'F',0,'l',0,'a',0,'s',0,'h',0,' ',0,'0',0,'x',0,'8',0,'0',0,'0',0, \
						 '2',0,'0',0,'0',0,'0',0

#else
	
	#error "No config for this target or no target specified"

#endif


/* some somewhat sensible defaults */
#define STARTUP_BLINKS 4

#ifndef BOOTLOADER_WAIT
#ifdef BUTTON_BANK
	#define BOOTLOADER_WAIT  WAIT_SHORT
#else
	#define BOOTLOADER_WAIT  WAIT_LONG
#endif
#endif

#ifndef CUSTOM_VID_PID
#define VEND_ID0 0x1E
#define VEND_ID1 0xAF
#define PROD_ID0 0x00
#define PROD_ID1 0x03
#endif

#ifndef USB_VENDOR_STR_LEN
#define USB_VENDOR_STR_LEN 0x12
#define USB_VENDOR_MSG_STR 'L', 0, 'e', 0, 'a', 0, 'f', 0, 'L', 0, 'a', 0, 'b', 0, 's', 0
#endif

#ifndef USB_PRODUCT_STR_LEN
#define USB_PRODUCT_STR_LEN 0x14
#define USB_PRODUCT_MSG_STR 'M', 0, 'a', 0, 'p', 0, 'l', 0, 'e', 0, ' ', 0, '0', 0, '0', 0, '3', 0
#endif

#ifndef USB_SERIAL_STR_LEN
#define USB_SERIAL_STR_LEN 0x10
#define USB_SERIAL_MSG_STR 'L', 0, 'L', 0, 'M', 0, ' ', 0, '0', 0, '0', 0, '3', 0
#endif

#ifndef CUSTOM_ALT_STRINGS
#define ALT0_STR_LEN 0x66
#define ALT0_MSG_STR 'S',0,'T',0,'M',0,'3',0,'2',0,' ',0,'B',0,'o',0,'o',0,'t',0,'l',0,'o',0,'a',0,'d',0,'e',0, \
				     'r',0,' ',0,'v',0,VER_MAJOR,0,'.',0,VER_MINOR,0,' ',0,' ',0,'U',0,'p',0,'l',0,'o',0,'a',0, \
				     'd',0,' ',0,'t',0,'o',0,' ',0,'R',0,'A',0,'M',0,' ',0,'n',0,'o',0,'t',0,' ',0,'s',0,'u',0, \
				     'p',0,'p',0,'o',0,'r',0,'t',0,'e',0,'d',0

#define ALT1_STR_LEN 0x62
#define ALT1_MSG_STR 'S',0,'T',0,'M',0,'3',0,'2',0,' ',0,'B',0,'o',0,'o',0,'t',0,'l',0,'o',0,'a',0,'d',0,'e',0, \
				     'r',0,' ',0,'v',0,VER_MAJOR,0,'.',0,VER_MINOR,0,' ',0,' ',0,'U',0,'p',0,'l',0,'o',0,'a',0, \
				     'd',0,' ',0,'t',0,'o',0,' ',0,'F',0,'l',0,'a',0,'s',0,'h',0,' ',0,'0',0,'x',0,'8',0,'0',0, \
				     '0',0,'5',0,'0',0,'0',0,'0',0

#define ALT2_STR_LEN 0x62
#define ALT2_MSG_STR 'S',0,'T',0,'M',0,'3',0,'2',0,' ',0,'B',0,'o',0,'o',0,'t',0,'l',0,'o',0,'a',0,'d',0,'e',0, \
				     'r',0,' ',0,'v',0,VER_MAJOR,0,'.',0,VER_MINOR,0,' ',0,' ',0,'U',0,'p',0,'l',0,'o',0,'a',0, \
				     'd',0,' ',0,'t',0,'o',0,' ',0,'F',0,'l',0,'a',0,'s',0,'h',0,' ',0,'0',0,'x',0,'8',0,'0',0, \
				     '0',0,'2',0,'0',0,'0',0,'0',0
#endif

#endif
