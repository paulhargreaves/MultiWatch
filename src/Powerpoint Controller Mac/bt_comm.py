import sys, atexit

if sys.platform != 'win32' and sys.platform != 'darwin':
    # on LINUX
    try:
        import lightblue
    except:
        print "please install lightblue: "
        print "On Ubuntu..."
        print "sudo apt-get install python-lightblue"
        print "sudo apt-get install python-bluez"
        sys.exit(1)
else:
	import socket

class BT_Socket:
    def __init__(self, bdaddr, port=0x1001):
        atexit.register(self.__del__)
        if sys.platform == 'win32' or sys.platform == 'darwin':
            # Connect to L2Cap server if running on windows
            self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            addr = "localhost"
            port = 1115
        else:
            self._sock = lightblue.socket( lightblue.L2CAP )
            addr = bdaddr        
        print "trying to connect to:"
        print addr
        self._sock.connect((addr, port))
        if sys.platform == 'win32' or sys.platform == 'darwin':
			# Send L2Cap Socket Server a connect message
            self._sock.send('\x20\x00\x00\x00')
            self._sock.send(bdaddr.replace(":",""))
            # wait for an ACK
            buf = self._sock.recv(1024)
            
    def send(self, message):
        if sys.platform == 'win32' or sys.platform == 'darwin':
            # sending data
            self._sock.send('\x00\x00\x00\x00')
            self._sock.send(message)
            
            # wait for an ACK
            buf = self._sock.recv(1024)
            return buf
        else:
            self._sock.send(message)
            
    def receive(self, buffersize):
        if sys.platform == 'win32' or sys.platform == 'darwin':
            self._sock.send('\x01\x00\x00\x00')
            self._sock.send('\x00\x00\x00\x00')
            buf = self._sock.recv(buffersize)
            print len(buf)
            return buf
        else:
            return self._sock.recv(buffersize)
        
    def close(self):
        if sys.platform == 'win32' or sys.platform == 'darwin':
            self._sock.send('\x30\x00\x00\x00')
            self._sock.send(" ")
        return self._sock.close()

    def __del__(self):
        try:
            self._sock.close()
        except: pass
