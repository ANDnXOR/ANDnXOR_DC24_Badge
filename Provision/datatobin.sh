#!/bin/bash

FILE=ANDnXOR_Badge-Flash.bin
WRITER=ANXWrite.py

#Create 700kB random file skipping first 4096 bytes
dd bs=1024 count=700 seek=4 < /dev/urandom > $FILE

#Flash data version this should be last as a _simple_ test
python $WRITER -t $'\x0C' -a 0x19500 -o $FILE

#Ensure 0xFF ends up at special byte for node id (#NANDIssues)
python $WRITER -t $'\xFF' -a 0x95000 -o $FILE

#flash the main logo and animation
python $WRITER -f assets/AND\!XOR.bmp         -a 0x1000 -v -o $FILE
python $WRITER -f assets/AND\!XOR-ripples.gif -a 0x1500 -o $FILE -v

#major lazer
python $WRITER -f assets/major-lazer.bmp   -a 0x5200 -o $FILE

#cyber pathogen
python $WRITER -f assets/cyberpathogen.xbm -a 0x5700 -o $FILE

#Rick Roll
python $WRITER -f assets/rick-long-dithered-short.gif -a 0x5B00 -o $FILE -v

#Flying Toasters
python $WRITER -f assets/Toast.xbm   -a 0x13C00 -v -o $FILE
python $WRITER -f assets/toaster.xbm -a 0x13E00 -v -o $FILE

#Matrix
python $WRITER -f assets/matrix-d.xbm -a 0x14000 -v -o $FILE
python $WRITER -f assets/matrix-e.xbm -a 0x14200 -o $FILE
python $WRITER -f assets/matrix-f.xbm -a 0x14400 -o $FILE
python $WRITER -f assets/matrix-c.xbm -a 0x14600 -o $FILE
python $WRITER -f assets/matrix-o.xbm -a 0x14800 -o $FILE
python $WRITER -f assets/matrix-n.xbm -a 0x14A00 -o $FILE

#Airplane mode
python $WRITER -f assets/AirplaneMode.xbm -a 0x14C00 -o $FILE

#No Service
python $WRITER -f assets/NoService.xbm -a 0x14E00 -o $FILE

#Netscape
python $WRITER -f assets/netscape-dithered.gif -a 0x15000 -o $FILE

#Lycos
python $WRITER -f assets/lycologo-bw.gif -a 0x19800 -o $FILE -v

#Flames
python $WRITER -f assets/Flames-bw.gif -a 0x1D500 -o $FILE -v



#Flappy DEFCON
python $WRITER -f assets/pipetop.png           -a 0x25300 -o $FILE -v
python $WRITER -f assets/pipebottom.png        -a 0x25500 -o $FILE -v
python $WRITER -f assets/defcon_up-small.png   -a 0x25700 -o $FILE -v
python $WRITER -f assets/defcon_down-small.png -a 0x25900 -o $FILE -v

#Asteroids
python $WRITER -f assets/ship0.png   -a 0x25B00 -o $FILE -v
python $WRITER -f assets/ship45.png  -a 0x25D00 -o $FILE -v
python $WRITER -f assets/ship90.png  -a 0x25F00 -o $FILE -v
python $WRITER -f assets/ship135.png -a 0x26100 -o $FILE -v
python $WRITER -f assets/ship180.png -a 0x26300 -o $FILE -v
python $WRITER -f assets/ship225.png -a 0x26500 -o $FILE -v
python $WRITER -f assets/ship270.png -a 0x26700 -o $FILE -v
python $WRITER -f assets/ship315.png -a 0x26900 -o $FILE -v

#Dodge
python $WRITER -f assets/virus.png  -a 0x26B00 -o $FILE -v
python $WRITER -f assets/lock.png   -a 0x26D00 -o $FILE -v
python $WRITER -f assets/edge.png   -a 0x26F00 -o $FILE -v
python $WRITER -f assets/floppy.png -a 0x27100 -o $FILE -v

#Ski
python $WRITER -f assets/ski.png      -a 0x27300 -o $FILE -v
python $WRITER -f assets/skileft.png  -a 0x27500 -o $FILE -v
python $WRITER -f assets/skiright.png -a 0x27700 -o $FILE -v
python $WRITER -f assets/skiflag.png  -a 0x27900 -o $FILE -v
python $WRITER -f assets/skirock.png  -a 0x27B00 -o $FILE -v
python $WRITER -f assets/skitree.png  -a 0x27D00 -o $FILE -v

#Party
python $WRITER -f assets/party.gif          -a 0x27F00 -o $FILE -v
python $WRITER -f assets/party-bender.gif   -a 0x80100 -o $FILE -v
python $WRITER -f assets/party-fry.gif      -a 0x35300 -o $FILE -v
python $WRITER -f assets/party-zoidburg.gif -a 0x42000 -o $FILE -v
python $WRITER -f assets/party-toad.gif     -a 0x48400 -o $FILE -v

#knight rider
python $WRITER -f assets/knightrider.gif -a 0x4D000 -o $FILE

#hackers
python $WRITER -f assets/hackers.gif -a 0x4E900 -o $FILE -v

#war games
python $WRITER -f assets/wargames.gif -a 0x58F00 -o $FILE -v

#Ninja
python $WRITER -f assets/ninja-p1-idle1.png 	-a 0x89300 -o $FILE -v
python $WRITER -f assets/ninja-p2-idle1.png 	-a 0x89500 -o $FILE -v
python $WRITER -f assets/ninja-p1-idle2.png 	-a 0x89700 -o $FILE -v
python $WRITER -f assets/ninja-p2-idle2.png 	-a 0x89900 -o $FILE -v
python $WRITER -f assets/ninja-punch.png 		-a 0x89B00 -o $FILE -v
python $WRITER -f assets/ninja-kick.png 		-a 0x89D00 -o $FILE -v
python $WRITER -f assets/ninja-shield.png 		-a 0x89F00 -o $FILE -v
python $WRITER -f assets/ninja-right.png 		-a 0x8A100 -o $FILE -v
python $WRITER -f assets/ninja-down.png 		-a 0x8A300 -o $FILE -v
python $WRITER -f assets/ninja-up.png 			-a 0x8A500 -o $FILE -v
python $WRITER -f assets/ninja-p1-punch1.png 	-a 0x8A700 -o $FILE -v
python $WRITER -f assets/ninja-p2-punch1.png 	-a 0x8A900 -o $FILE -v
python $WRITER -f assets/ninja-p1-punch2.png 	-a 0x8AB00 -o $FILE -v
python $WRITER -f assets/ninja-p2-punch2.png 	-a 0x8AD00 -o $FILE -v
python $WRITER -f assets/ninja-p1-kick1.png 	-a 0x8AF00 -o $FILE -v
python $WRITER -f assets/ninja-p2-kick1.png 	-a 0x8B100 -o $FILE -v
python $WRITER -f assets/ninja-p1-kick2.png 	-a 0x8B300 -o $FILE -v
python $WRITER -f assets/ninja-p2-kick2.png 	-a 0x8B500 -o $FILE -v
python $WRITER -f assets/ninja-p1-shield.png 	-a 0x8B700 -o $FILE -v
python $WRITER -f assets/ninja-p2-shield.png 	-a 0x8B900 -o $FILE -v
python $WRITER -f assets/ninja-p1-shield2.png 	-a 0xA5C00 -o $FILE -v
python $WRITER -f assets/ninja-p2-shield2.png 	-a 0xA5E00 -o $FILE -v
python $WRITER -f assets/ninja-p1-dead.png 		-a 0x8BB00 -o $FILE -v
python $WRITER -f assets/ninja-p2-dead.png 		-a 0x8BE00 -o $FILE -v
python $WRITER -f assets/ninja-dt.png			-a 0xA5900 -o $FILE -v

#Purisa Font
python $WRITER -f assets/purisa-a.png 				-a 0x8C100 -o $FILE
python $WRITER -f assets/purisa-b.png 				-a 0x8C300 -o $FILE 
python $WRITER -f assets/purisa-c.png 				-a 0x8C500 -o $FILE 
python $WRITER -f assets/purisa-d.png 				-a 0x8C700 -o $FILE 
python $WRITER -f assets/purisa-e.png		 		-a 0x8CA00 -o $FILE 
python $WRITER -f assets/purisa-f.png 				-a 0x8CC00 -o $FILE 
python $WRITER -f assets/purisa-g.png 				-a 0x8CF00 -o $FILE 
python $WRITER -f assets/purisa-h.png 				-a 0x8D200 -o $FILE 
python $WRITER -f assets/purisa-i.png 				-a 0x8D400 -o $FILE 
python $WRITER -f assets/purisa-j.png		 		-a 0x8D600 -o $FILE 
python $WRITER -f assets/purisa-k.png 				-a 0x8D800 -o $FILE 
python $WRITER -f assets/purisa-l.png 				-a 0x8DA00 -o $FILE 
python $WRITER -f assets/purisa-m.png	 			-a 0x8DC00 -o $FILE 
python $WRITER -f assets/purisa-n.png 				-a 0x8DF00 -o $FILE 
python $WRITER -f assets/purisa-o.png 				-a 0x8E100 -o $FILE 
python $WRITER -f assets/purisa-p.png		 		-a 0x8E300 -o $FILE 
python $WRITER -f assets/purisa-q.png	 			-a 0x8E500 -o $FILE 
python $WRITER -f assets/purisa-r.png		 		-a 0x8E800 -o $FILE 
python $WRITER -f assets/purisa-s.png		 		-a 0x8EA00 -o $FILE 
python $WRITER -f assets/purisa-t.png 				-a 0x8EC00 -o $FILE 
python $WRITER -f assets/purisa-u.png 				-a 0x8EE00 -o $FILE 
python $WRITER -f assets/purisa-v.png		 		-a 0x8F000 -o $FILE 
python $WRITER -f assets/purisa-w.png 				-a 0x8F200 -o $FILE 
python $WRITER -f assets/purisa-x.png 				-a 0x8F500 -o $FILE 
python $WRITER -f assets/purisa-y.png 				-a 0x8F700 -o $FILE 
python $WRITER -f assets/purisa-z.png 				-a 0x8F900 -o $FILE 
python $WRITER -f assets/purisa-0.png 				-a 0x8FB00 -o $FILE 
python $WRITER -f assets/purisa-1.png 				-a 0x8FD00 -o $FILE 
python $WRITER -f assets/purisa-2.png 				-a 0x8FF00 -o $FILE 
python $WRITER -f assets/purisa-3.png			 	-a 0x90100 -o $FILE 
python $WRITER -f assets/purisa-4.png 				-a 0x90300 -o $FILE 
python $WRITER -f assets/purisa-5.png 				-a 0x90500 -o $FILE 
python $WRITER -f assets/purisa-6.png			 	-a 0x90700 -o $FILE 
python $WRITER -f assets/purisa-7.png 				-a 0x90900 -o $FILE 
python $WRITER -f assets/purisa-8.png 				-a 0x90B00 -o $FILE 
python $WRITER -f assets/purisa-9.png 				-a 0x90D00 -o $FILE 
python $WRITER -f assets/purisa-period.png 			-a 0x90F00 -o $FILE 
python $WRITER -f assets/purisa-exclamation.png		-a 0x91100 -o $FILE 
python $WRITER -f assets/purisa-comma.png 			-a 0x91300 -o $FILE 
python $WRITER -f assets/purisa-question.png 		-a 0x91500 -o $FILE 
python $WRITER -f assets/purisa-space.png			-a 0x91700 -o $FILE 
python $WRITER -f assets/purisa-lparen.png 			-a 0x9AC00 -o $FILE 
python $WRITER -f assets/purisa-rparen.png	 		-a 0x9AE00 -o $FILE 
python $WRITER -f assets/purisa-lbracket.png 		-a 0x9B000 -o $FILE 
python $WRITER -f assets/purisa-rbracket.png 		-a 0x9B200 -o $FILE 
python $WRITER -f assets/purisa-lbrace.png	 		-a 0x9B400 -o $FILE 
python $WRITER -f assets/purisa-rbrace.png	 		-a 0x9B600 -o $FILE 
python $WRITER -f assets/purisa-lt.png		 		-a 0x9B800 -o $FILE 
python $WRITER -f assets/purisa-gt.png		 		-a 0x9BA00 -o $FILE 
python $WRITER -f assets/purisa-forward.png			-a 0x9BC00 -o $FILE 
python $WRITER -f assets/purisa-backward.png 		-a 0x9BF00 -o $FILE 
python $WRITER -f assets/purisa-pipe.png	 		-a 0x9C100 -o $FILE 
python $WRITER -f assets/purisa-semi.png 			-a 0x9C300 -o $FILE 
python $WRITER -f assets/purisa-colon.png 			-a 0x9C500 -o $FILE 


python $WRITER -f assets/bender.png -a 0xA6000 -o $FILE -v
python $WRITER -f assets/defcon.png -a 0x91900 -o $FILE
python $WRITER -f assets/eff.png -a 0x93E00 -o $FILE
python $WRITER -f assets/nayan.gif -a 0x91D00 -o $FILE
python $WRITER -f assets/hackaday.png -a 0x94200 -o $FILE
python $WRITER -f assets/pirateflag.gif -a 0x96000 -o $FILE
python $WRITER -f assets/rememberme.gif -a 0x9C700 -o $FILE
