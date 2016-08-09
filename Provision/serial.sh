#!/bin/bash

UDEVICE=$1

#Flash a unique node id to the device
python flasher.py -s -d $UDEVICE -a 0x95000
