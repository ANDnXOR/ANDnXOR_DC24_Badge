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
 *  @file common.h
 *
 *  @brief toplevel include for bootloader source files
 *
 *
 */

#ifndef __COMMON_H
#define __COMMON_H


#include "stm32_lib/stm32f10x_type.h"
#include "stm32_lib/cortexm3_macro.h"

#include "config.h"
#include "hardware.h"
#include "usb.h"

#define VER_MAJOR 			'1'
#define VER_MINOR 			'1'

#define RESET_POR 			(1 << 27) 	// power-on reset
#define RESET_EXT			(1 << 26) 	// reset button via reset pin
#define RESET_ANY 			0xfe000000 	// all reset types


#ifndef RESET_ACTIVATION
#define RESET_ACTIVATION 	RESET_POR | RESET_EXT
#endif


typedef void (*FuncPtr)(void);


#endif
