import os, sys, time
import pulseprotocol as proto
from bt_comm import BT_Socket

#Establishing connection with the watch
btmac = sys.argv[1]
try:
	socket = BT_Socket(btmac, 0x1001)
except:
	print "Bad mac address or, if on Windows, the L2CAPServer might not be wokring"
	sys.exit()

proto.determine_packet_format(socket)
socket.send(proto.cmd_set_bt_sniff_off())
socket.receive(4)

while(1):
	#Receive the commain from the watch
	text=socket.receive(256)
	#If it's a valid command (Note: non-valid commands will have a length of 2)
	if(len(text) is 4):
		#transform it into an int
		key=proto.int_from_buffer(text)
		# Always send back the message if in valid ascii range
                if(key >= 128 and key <= 255):
                      print "Message received. Sending response ", key
                      socket.send(chr(key)) # send the response back
                if(key is 69):
                     print "Next slide request received ", key 
                     os.system("./sendkey.osascript ' '")
                elif(key <= 128):
                     print "Unknown key=", key
	#	elif (key is 0):
	#		SendKeys.SendKeys("""{LEFT}""")
        print "looping..."
        # wend

#Closes conncetions		
socket.close()
