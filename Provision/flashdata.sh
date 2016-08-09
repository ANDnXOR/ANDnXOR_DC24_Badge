#!/bin/bash

UDEVICE=$1
FILE=$2

#Flash the data starting after settings
python3 ANXFlash.py -f $FILE -d $UDEVICE -a 0x1000 -o 0x1000

#Flash a unique node id to the device
python flasher.py -s -d $UDEVICE -a 0x95000
