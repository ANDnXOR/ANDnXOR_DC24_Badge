import csv
import math
import serial
from time import sleep
import sys, getopt
from PIL import Image
from optparse import OptionParser

def main(argv):
	parser = OptionParser()
	parser.add_option(
		"-f", "--file", 
		action="store",
		dest="filename",
        help="File to Write to Flash", 
		metavar="FILE")
	parser.add_option(
		"-d", "--device",
		action="store",
		dest="device", 
        help="Serial Device to Flash")
	parser.add_option(
		"-v", "--verbose",
		action="store_true",
		dest="verbose",
		help="Verbose Output")
	parser.add_option(
		"-a", "--address",
		action="store",
		dest="address",
		type="int",
		help="Address in flash memory to write")
	parser.add_option(
		"-o", "--offset",
		action="store",
		dest="offset",
		type="int",
		default=0,
		help="Offset within bin file to start writing")


	(options, args) = parser.parse_args()

	ser = serial.Serial()
	ser.port=options.device
	ser.baudrate=115200
	ser.xonxoff=False
	ser.stopbits=serial.STOPBITS_ONE



	doFileTransfer(ser, options)

def waitForLine(ser, test, options):
	line = None

	#Wait for device to be ready
	while (line != test):
		line = ser.readline().rstrip().decode("latin_1") 		
#		line = ser.readline().rstrip()
		#if options.verbose:		
		#	print("<<", line)

def readSerial(ser, options):
	ser.Open();
	waitForLine(ser, "Ready", options)
	#Write some data
	print("** Sending read serial command")
	ser.write(b's')
	line = ser.readline().rstrip().decode("utf-8")
	print("MCU Serial is", line)
	waitForLine(ser, "Done", options)
	return line
	ser.close()

def doFileTransfer(ser, options):

	if ser.isOpen():
		ser.close()
	ser.open()
	print(ser.isOpen())

	with open(options.filename, "rb") as f:
		binData = bytearray(f.read())	


	size = len(binData) - options.offset
	print("Flashing File", options.filename)
	print("\tSize (bytes): ", size)
	waitForLine(ser, "Ready", options)
		
	#Write some data
	print("\tSending write command")
	ser.write(b'w')

	print("\tWaiting for address")
	#Wait for the device to ask for an address
	waitForLine(ser, "Address:", options)
	#write the address
	if options.verbose:
		print("\tSending " + hex(options.address) + " as address")
	ser.write(bytearray(str(options.address), encoding = "ascii"))
	ser.write(b'\n')
	#Wait to send the page count
	waitForLine(ser, "Size(Bytes):", options)
	ser.write(str(size).encode('latin_1'))
	ser.write(b'\n')

	#Wait for go signal
	waitForLine(ser, "OK GO", options)

	print("\t***Flashing, please wait***")
	
	#write bytes
	count = 0
	for byte in binData:
		#skip offset bytes
		if count >= options.offset:
			ser.write(bytes([byte]))
			#sleep(0.001)
			waitForLine(ser, "A", options)
			if options.verbose:		
				print(count, ":", hex(byte))
		count += 1
		pct = (100 * count / size)
		if (pct > 100): 
			pct = 100
		sys.stdout.write("\r\t%3.1f%%" % pct)
		sys.stdout.flush()

	waitForLine(ser, "DONE", options)
	print("Done.")
	ser.close()

if __name__ == "__main__":
   main(sys.argv[1:])

