import math
import sys, getopt
from PIL import Image
from optparse import OptionParser

def main(argv):
	parser = OptionParser()
	parser.add_option(
		"-f", "--file", 
		action="store",
		dest="filename",
        help="File to Write to bin", 
		metavar="FILE")
	parser.add_option(
		"-o", "--out",
		action="store",
		dest="out",
		help="Output file to write to",
		metavar="OUTFILE")
	parser.add_option(
		"-t", "--text",
		action="store",
		dest="text",
		help="Arbitrary Text to Flash")
	parser.add_option(
		"-a", "--address",
		action="store",
		dest="address",
		type="int",
		help="Address in flash memory to write")
	parser.add_option(
		"-g", "--gif",
		action="store_true",
		dest="gif",
		help="Animated Gif")
	parser.add_option(
		"-v", "--verbose",
		action="store_true",
		dest="verbose",
		help="Verbose Output")


	(options, args) = parser.parse_args()

	if options.verbose:
		print("file:", options.filename)
		print("outfile:", options.out)
		print("address:", options.address)
		print("text:", options.text)

	if (options.filename):
		writeFile(options)
	elif (options.text):
		writeText(options)

def writeFile(options):

	address = options.address
	print "Address:", hex(address)

	im = Image.open(options.filename)
	framei = 0;

	#determine pages per frame
	size = float(math.ceil((im.size[0] * im.size[1] / 8))) / 256
	pages = int(math.ceil(size))
	print "Size:",size,"Pages:",pages

	#read in the data
	with open(options.out, 'rb') as out:
		outdata = bytearray(out.read())

	#write metadata to the first 3 bytes, will offset by 8 bytes for future proof
	outdata[address], outdata[address+1] = im.size   #width, height
	count = 0
	for i in ImageSequence(im):
		count += 1
	outdata[address+2] = count        #frame count
	address += 256 #go to next page

	#write each frame 
	for frame in ImageSequence(im):
		print "Writing Frame", framei, "0x", hex(address)

		pixels = frame.load()
		
		#Draw frame to screen with 1s and 0s for debugging
		if options.verbose:
			for y in range(0, im.size[1]):
				for x in range(0, im.size[0]):
					bit = 0
					if (pixels[x,y] == 1 or pixels[x,y] == 255):
						bit = 1
					sys.stdout.write(str(pixels[x,y]))
				print ""

		offset = 0
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

					outdata[address + offset] = byte
					byte = 0
					offset += 1

				else:
					byte = byte << 1
					#print "\n"

		framei += 1

		#move the address inside the byte array by even page count
		address += pages * 256

	#write out the data
	with open(options.out, 'wb') as out:
		out.write(outdata)
	print "Done."

def writeText(options):
	address = options.address
	print "Address:", hex(address)
	print "Text:", options.text
	print "Size:", len(options.text)

	#read in the data
	with open(options.out, 'rb') as out:
		outdata = bytearray(out.read())
	
	offset = 0
	for c in options.text:
		outdata[options.address + offset] = c
		offset += 1

	#write out the data
	with open(options.out, 'wb') as out:
		out.write(outdata)

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

if __name__ == "__main__":
   main(sys.argv[1:])

