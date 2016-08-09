/* *****************************************************************************
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
 * ****************************************************************************/

/**
 *  @file main.c
 *
 *  @brief main loop and calling any hardware init stuff.
 *  logic to handle bootloader entry and jumping to user code.
 */

#include "common.h"
#include "dfu.h"


#define SW_ONLY 		1
#define FLASH_ONLY 		2
#define SW_OR_FLASH 	0


static int checkUserJump(u8 sw)
{
	switch (sw) {
		case SW_ONLY: return readButtonState();
		case FLASH_ONLY: {
			return (!(checkUserCode(USER_CODE_FLASH0X8005000)
			       || checkUserCode(USER_CODE_FLASH0X8002000))
			);
		}
	}

	return (readButtonState() ||
			(!(checkUserCode(USER_CODE_FLASH0X8005000)
			|| checkUserCode(USER_CODE_FLASH0X8002000)))
	);
}

static u32 checkReset()
{
#ifdef BL_BUTTON_ALWAYS_WORKS
	return checkUserJump(SW_ONLY) | (GET_REG(RCC_CSR) & (RESET_ACTIVATION));
#else
	return (GET_REG(RCC_CSR) & (RESET_ACTIVATION));
#endif
}

int main() 
{
    int bl_start = 0;
    int bl_active = 0;

    systemReset();  // peripherals but not PC
    setupCLK(); 	// not USB, that is handled by USB portion
    setupLEDAndButton();

	// determine our reset method to see if we should even use the bootloader
	// NOTE: user must set RCC->CSR bit 24, otherwise this may always succeed
	if (checkReset()) {
		// do the startup LED quickflash
#ifndef DISABLE_STARTUP_FAST_BLINK
	 	strobePin(LED_BANK, LED_PIN, STARTUP_BLINKS, BLINK_FAST, LED_ON_STATE);
#endif

	 	// see if we should enter the bootloader....
#if (BOOTLOADER_WAIT == 0)
		// as there is no wait defined, there must a switch or other method to get
		// into the bootloader. we don't even want to activate it for a brief moment,
		// so only activate if the button is held high or no code is in user flash
	 	bl_start = checkUserJump(SW_OR_FLASH);
#else
	 	// load the bootloader for the specified bootloader wait time
	 	// this is specified in config.h for the selected platform
	 	bl_start = 1;
#endif
 	} else {
 		// even if invalid reset, if for some reason the flash is corrupted,
 		// we need to enter the bootloader. in this case, we get no quickflash =)
 		bl_start = checkUserJump(FLASH_ONLY);
 	}

	if (bl_start) {
		// for some reason we've entered the bootloader
		// if a button is pushed, or we don't have valid code in flash,
		// we want to stay in the bootloader
		bl_active = checkUserJump(SW_OR_FLASH);

		// only set up USB and flash if in the bootloader (as we are now)
		setupUSB();
    	setupFLASH();

    	// stay in the bootloader if we're waiting, or if we're forced active
    	int bl_wait = BOOTLOADER_WAIT;
    	while (bl_wait || bl_active) {
    		if (bl_wait) bl_wait--;

	        // is DFU in progress?
	        if (dfuUploadStarted()) {
	        	// wait until we're done
	        	while (!dfuUploadDone());
	        	// success flash, we also need to wait a little longer for manifest to be sent
	        	// otherwise it will be successful but we'll get "unable to read DFU status" error

	        	// flashing faster once we're done takes time, this serves as our wait
	        	strobePin(LED_BANK, LED_PIN, STARTUP_BLINKS, BLINK_FAST, LED_ON_STATE);

	            // uncomment the following line if you always want to execute code after completion
	            // break;

	            // uncomment the following line if you only want to stay in the bootloader if
	            // the switch is still pressed (used for toggle switches). deactivating the
	            // switch will then start the program. used for user-initiated code start
	            bl_active = checkUserJump(SW_ONLY);

	            // uncomment no lines if you want to stay in the bootloader after loading code
	            // keep in mind that the DFU state machine is broken so there's no point, really
	        } else {
	        	// while in bootloader, flash the LED slowly
	        	strobePin(LED_BANK, LED_PIN, 1, BLINK_SLOW, LED_ON_STATE);
	        }
	    }
	}

	if (checkUserCode(USER_CODE_FLASH0X8002000))  {
		jumpToUser(USER_CODE_FLASH0X8002000);
	} else {
		if (checkUserCode(USER_CODE_FLASH0X8005000)) {
			jumpToUser(USER_CODE_FLASH0X8005000);
		}  else {
			// Nothing to execute in either Flash or RAM
			strobePin(LED_BANK, LED_PIN, 10, BLINK_FAST, LED_ON_STATE);
			systemHardReset();
		}
	}

	return 0;
}