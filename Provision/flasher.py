import csv
import math
import serial
import glob
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
		"-t", "--text",
		action="store",
		dest="text",
		help="Arbitrary Text to Flash")
	parser.add_option(
		"-d", "--device",
		action="store",
		dest="device", 
        help="Serial Device to Flash")
	parser.add_option(
		"-a", "--address",
		action="store",
		dest="address",
		type="int",
		help="Address in flash memory to write")
	parser.add_option(
		"-e", "--erase",
		action="store_true",
		dest="erase",
		help="Erase the chip")
	parser.add_option(
		"-g", "--gif",
		action="store_true",
		dest="gif",
		help="Animated Gif")
	parser.add_option(
		"-s", "--serial",
		action="store_true",
		dest="serial",
		help="Assign Serial")
	parser.add_option(
		"-v", "--verbose",
		action="store_true",
		dest="verbose",
		help="Verbose Output")


	(options, args) = parser.parse_args()

	if options.verbose:
		print("file:", options.filename)
		print("device:", options.device)
		print("address:", options.address)
		print("text:", options.text)
		print("Erase Set?", options.erase)

	ser = serial.Serial()
	ser.port=options.device
	ser.baudrate=115200
	ser.xonxoff=False
	ser.stopbits=serial.STOPBITS_ONE
	ser.open()

	if options.erase:
		print("Erasing Flash!")
		doErase(ser, options)
	else:
		if (options.filename):
			doFileTransfer(ser, options)
		elif (options.text):
			doTextTransfer(ser, options)
		elif (options.serial):
			doAssignSerial(ser, options)

	ser.close()

def waitForLine(ser, test, options):
	line = None
	print("---waiting for", test)
	#print("Serial data is: ", ser)
	#Wait for device to be ready
	while (line != test):
		line = ser.readline().rstrip().decode("latin_1") 		
#		line = ser.readline().rstrip()
		if options.verbose:		
			print ("<<", line)
def readSerial(ser, options):
	waitForLine(ser, "Ready", options)
	#Write some data
	print("** Sending read serial command")
	ser.write(b's')
	waitForLine(ser, "SERIAL", options)
	line = ser.readline().rstrip().decode("utf-8")
	print ("MCU Serial is", line)
	waitForLine(ser, "Done", options)
	return line

def doAssignSerial(ser, options):
	serialNumber = readSerial(ser, options)
	options.text = str(chr(int(findUniqueNodeId(serialNumber))))
	doTextTransfer(ser, options)

def findUniqueNodeId(serialNumber):
	nodeid = 1
	with open('nodes.csv', 'a+b') as csvfile:
		reader = csv.reader(csvfile, dialect=csv.excel)
		for row in reader:
			if row[1] == serialNumber:
				return row[0]
			nodeid += 1
		
		#if they got this far, the MCU is unique, write it to the file
		writer = csv.writer(csvfile, dialect=csv.excel)
		writer.writerow([nodeid, serialNumber])
	return nodeid

def doFileTransfer(ser, options):

	address = options.address
	print("Address:", int(address))

	#Read in the file	
	#with open(options.filename, 'r+') as f:
	#	contents = f.read()
	#	print("**File size = " + str(len(contents)))

	im = Image.open(options.filename)
	size = float(math.ceil((im.size[0] * im.size[1] / 8)))
	print("**Size (bytes): ", size)

	pages = int(math.ceil(size / 256))
	print("**Size: " + str(im.size) + " Pages: " + str(pages) + " Mode: " + str(im.mode))
	framei = 0

	for frame in ImageSequence(im):
		print("Flashing Frame", framei)

		waitForLine(ser, "Ready", options)
		
		#Write some data
		print("** Sending write command")
		ser.write(b'w')

		print("**Waiting for address")
		#Wait for the device to ask for an address
		waitForLine(ser, "Address:", options)
		#write the address
		if options.verbose:
			print("** Sending " + hex(address) + " as address")
		ser.write(str(address)+"\n")

		#Wait to send the page count
		waitForLine(ser, "Size(Bytes):", options)
		if options.verbose:
			print("** Sending "+str(size)+" as size")
		ser.write(str(size)+"\n")


		pixels = frame.load()
		
		#Draw frame to screen with 1s and 0s for debugging
		if options.verbose:
			for y in range(0, im.size[1]):
				for x in range(0, im.size[0]):
					bit = 0
					if (pixels[x,y] == 1 or pixels[x,y] == 255):
						bit = 1
					sys.stdout.write(str(bit))
				print("")

		waitForLine(ser, "OK GO", options)

		byteCount = 0
		byte = 0
		b = 0
		for y in range(0, im.size[1]):
			for x in range(0, im.size[0]):
				bit = 0
				if (pixels[x,y] == 1 or pixels[x,y] == 255):
					bit = 1
				byte |= bit			
				b += 1
						
				#A full byte as been constructed, send it
				if (b == 8):
					#clear the bit count
					b = 0

					#Write the byte to serial 
					if options.verbose:
						print(byteCount, ":", hex(byte))	
					ser.write(chr(byte))	
					byte = 0

					waitForLine(ser, "A", options)

				else:
					byte = byte << 1
					#print "\n"

		waitForLine(ser, "DONE", options)
		framei += 1
		address += pages * 256
		print("New address", hex(address))
	
	print("Done.")

def doTextTransfer(ser, options):
	address = options.address
	print("Address:", hex(address))

	print("Text:", options.text)
	print("Size:", len(options.text))

	waitForLine(ser, "Ready", options)
		
	#Write some data
	print("** Sending write command")
	ser.write(b'w')

	#Wait for the device to ask for an address
	waitForLine(ser, "Address:", options)
	#write the address
	if options.verbose:
		print("** Sending " + hex(address) + " as address")
	ser.write(str(address)+"\n")

	#Wait to send the page count
	waitForLine(ser, "Size(Bytes):", options)
	if options.verbose:
		print("** Sending "+str(len(options.text))+" as size")
	ser.write(str(len(options.text))+"\n")

	waitForLine(ser, "OK GO", options)

	#Write the text to serial one byte at a time
	for byte in options.text:
		if options.verbose:
			print("Writing:", byte)		
		ser.write(byte)
		waitForLine(ser, "A", options)

	waitForLine(ser, "DONE", options)	

#Erase the chip
def doErase(ser, options):	
	waitForLine(ser, "Ready", options)
	print("** Erasing chip")
	e = bytearray("e", encoding="ascii")
	ser.write(e)
	waitForLine(ser, 'Done', options)
	print("** Erasing Complete!")
	return

class ImageSequence:
    def __init__(self, im):
        self.im = im
    def __getitem__(self, ix):
        try:
            if ix:
                self.im.seek(ix)
            return self.im
        except EOFError:
            raise IndexError # end of sequence

def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result
if __name__ == "__main__":
	print(serial_ports())
	main(sys.argv[1:])

