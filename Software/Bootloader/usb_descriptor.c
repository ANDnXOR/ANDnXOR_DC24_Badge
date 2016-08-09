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
 *  @file usb_descriptor.c
 *
 *  @brief aka application descriptor; big static struct and callbacks for sending
 *  the descriptor.
 *
 */


#include "usb_descriptor.h"

u8 u8_usbDeviceDescriptorDFU[18] = {
      18,   /* bLength */
    0x01,   /* bDescriptorType */
    0x00,   /* bcdUSB, version 2.00 */
    0x02,
    0x00,   /* bDeviceClass : See interface */
    0x00,   /* bDeviceSubClass : See interface*/
    0x00,   /* bDeviceProtocol : See interface */
    bMaxPacketSize, /* bMaxPacketSize0 0x40 = 64 */
    VEND_ID1,   /* idVendor     (0110) */
    VEND_ID0,
    PROD_ID1,   /* idProduct (0x1001 or 1002) */
    PROD_ID0,
    0x00,   /* bcdDevice*/
    0x01,
    0x01,   /* iManufacturer : index of string Manufacturer  */
    0x02,   /* iProduct      : index of string descriptor of product*/
    0x03,   /* iSerialNumber : index of string serial number*/
    0x01    /*bNumConfigurations */
};

ONE_DESCRIPTOR usbDeviceDescriptorDFU = {
    u8_usbDeviceDescriptorDFU,
    18
};

u8 u8_usbFunctionalDescriptor[9] = {
    /******************** DFU Functional Descriptor********************/
       9,   /*blength = 9 Bytes*/
    0x21,   /* DFU Functional Descriptor*/
    0x03,   /*bmAttributes, bitCanDnload | bitCanUpload */
    0xFF,   /*DetachTimeOut= 255 ms*/
    0x00,
    (dummyTransferSize & 0x00FF),
    (dummyTransferSize & 0xFF00) >> 8, /* TransferSize = 1024 Byte*/
    0x10,                              /* bcdDFUVersion = 1.1 */
    0x01
};

ONE_DESCRIPTOR usbFunctionalDescriptor = {
    u8_usbFunctionalDescriptor,
    9
};

#define u8_usbConfigDescriptorDFU_LENGTH 45
u8 u8_usbConfigDescriptorDFU[u8_usbConfigDescriptorDFU_LENGTH] = {
    0x09,   /* bLength: Configuation Descriptor size */
    0x02,   /* bDescriptorType: Configuration */
    u8_usbConfigDescriptorDFU_LENGTH,   /* wTotalLength: Bytes returned */
    0x00,
    0x01,   /* bNumInterfaces: 1 interface */
    0x01,   /* bConfigurationValue: */
    0x00,   /* iConfiguration: */
    0x80,   /* bmAttributes: */
    0x32,   /* MaxPower 100 mA */
    /* 09 */

    /************ Descriptor of DFU interface 0 Alternate setting 0 *********/
    0x09,   /* bLength: Interface Descriptor size */
    0x04,   /* bDescriptorType: */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x00,   /* bNumEndpoints*/
    0xFE,   /* bInterfaceClass: DFU */
    0x01,   /* bInterfaceSubClass */
    0x02,   /* nInterfaceProtocol, switched to 0x02 while in dfu_mode */
    0x04,   /* iInterface: */

    /************ Descriptor of DFU interface 0 Alternate setting 1 *********/
    0x09,   /* bLength: Interface Descriptor size */
    0x04,   /* bDescriptorType: */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x01,   /* bAlternateSetting: Alternate setting */
    0x00,   /* bNumEndpoints*/
    0xFE,   /* bInterfaceClass: DFU */
    0x01,   /* bInterfaceSubClass */
    0x02,   /* nInterfaceProtocol, switched to 0x02 while in dfu_mode */
    0x05,   /* iInterface: */
	
    /************ Descriptor of DFU interface 0 Alternate setting 2 *********/
    0x09,   /* bLength: Interface Descriptor size */
    0x04,   /* bDescriptorType: */
    0x00,   /* bInterfaceNumber: Number of Interface */
    0x02,   /* bAlternateSetting: Alternate setting */
    0x00,   /* bNumEndpoints*/
    0xFE,   /* bInterfaceClass: DFU */
    0x01,   /* bInterfaceSubClass */
    0x02,   /* nInterfaceProtocol, switched to 0x02 while in dfu_mode */
    0x06,   /* iInterface: */	

    /******************** DFU Functional Descriptor********************/
    0x09,   /*blength = 7 Bytes*/
    0x21,   /* DFU Functional Descriptor*/
    0x03,   /*bmAttributes, bitCanDnload | bitCanUpload */
    0xFF,   /*DetachTimeOut= 255 ms*/
    0x00,
    (dummyTransferSize & 0x00FF),
    (dummyTransferSize & 0xFF00) >> 8, /* TransferSize = 1024 Byte*/
    0x10,                          /* bcdDFUVersion = 1.1 */
    0x01
    /***********************************************************/
    /*45*/
};

ONE_DESCRIPTOR usbConfigDescriptorDFU = {
    u8_usbConfigDescriptorDFU,
    u8_usbConfigDescriptorDFU_LENGTH
};

#define USB_STR_LANG_ID_LEN 4
u8 u8_usbStringLangId[USB_STR_LANG_ID_LEN] = {
    USB_STR_LANG_ID_LEN,
    0x03,
    0x09,
    0x04    /* LangID = 0x0409: U.S. English */
};

static const u8 u8_usbStringVendor[USB_VENDOR_STR_LEN] = {
    USB_VENDOR_STR_LEN,
    0x03,
    USB_VENDOR_MSG_STR
};

static const u8 u8_usbStringProduct[USB_PRODUCT_STR_LEN] = {
    USB_PRODUCT_STR_LEN,
    0x03,
    USB_PRODUCT_MSG_STR
};

static const u8 u8_usbStringSerial[USB_SERIAL_STR_LEN] = {
    USB_SERIAL_STR_LEN,
    0x03,
    USB_SERIAL_MSG_STR
};

static const u8 u8_usbStringAlt0[ALT0_STR_LEN] = {
	ALT0_STR_LEN,
	0x03,
	ALT0_MSG_STR
};


static const u8 u8_usbStringAlt1[ALT1_STR_LEN] = {
	ALT1_STR_LEN,
	0x03,
	ALT1_MSG_STR	
};


static const u8 u8_usbStringAlt2[ALT2_STR_LEN] = {
	ALT2_STR_LEN,
	0x03,
	ALT2_MSG_STR
};

u8 u8_usbStringInterface = 0;

ONE_DESCRIPTOR usbStringDescriptor[STR_DESC_LEN] = {
    { (u8 *)u8_usbStringLangId,  USB_STR_LANG_ID_LEN },
    { (u8 *)u8_usbStringVendor,  USB_VENDOR_STR_LEN },
    { (u8 *)u8_usbStringProduct, USB_PRODUCT_STR_LEN },
    { (u8 *)u8_usbStringSerial,  USB_SERIAL_STR_LEN },
    { (u8 *)u8_usbStringAlt0,    ALT0_STR_LEN },
    { (u8 *)u8_usbStringAlt1,    ALT1_STR_LEN },
	{ (u8 *)u8_usbStringAlt2,    ALT2_STR_LEN }
};

