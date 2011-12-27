#!/usr/bin/env python

r"""
This module implements the Pulse protocol, documented in /doc/pulse_protocol.odt
It provides several functions that produce strings containing pulse messages.
Each function provides a different level of abstraction.

The simplest way to generate a message is to use one of the high-level
functions like email_notification().
"""

import struct
import time
from datetime import datetime
from bt_comm import BT_Socket
import sys

unittesting = False

seq_num = 0

use_old_packet_format = False

def int_from_buffer(buffer):
    #print "lenght of buffer"
    #print len(buffer)
    if (len(buffer) < 4):
        print "Got error in response. Try restarting L2CAPServer and try again."
        sys.exit(1)
    ret = 0
    ret = (ord(buffer[3]) & 0xFF)
    #print "C2"
    ret <<= 8;
    ret |= (ord(buffer[2]) & 0xFF);
    #print "C3"
    ret <<= 8;
    ret |= (ord(buffer[1]) & 0xFF);
    ret <<= 8;
    ret |= (ord(buffer[0]) & 0xFF);
    return ret;

def determine_packet_format(socket):
    global use_old_packet_format
    use_old_packet_format = True
    print "."
    if (message_get_int_response(socket, cmd_query_main_app()) < 363):
        # Need to determine if we're in VLF
        print "."
        if (message_get_int_response(socket, cmd_query_vlf()) > 0):
            # IN VLF, use the old packet format
            print "." # Do nothing. Why can't python just have a blank IF statement?
        else:
            # Either in an OLD main app or in a new version, but we queried using the old format
            use_old_packet_format = False
            if (message_get_int_response(socket, cmd_query_main_app()) <= 363):
                use_old_packet_format = True
                print "."
            else:
                use_old_packet_format = False
    print "."
    print "Using Old packet format: "
    print use_old_packet_format

def get_in_vlf():
    return use_old_packet_format

def message_get_int_response(socket, msg):
    socket.send(msg)
    resp = socket.receive(4)
    return int_from_buffer(resp)


def get_seq_num():
    """Returns the current sequence number and increments it"""
    global seq_num, unittesting
    cur = seq_num
    seq_num = (seq_num+1)%256

    # Hack to make unit testing deterministic
    if unittesting: return 0
    return cur

def to_p_str(s):
    r"""
    Returns a string with contents that conforms to the p_string format.

    A p_string is basically a combination C-string and Pascal string. It has a
    leading length byte and a trailing zero byte. The length counts the ending
    zero, but not itself.

    TODO: This description is out of date.  Now it just creates c-strings.

    >>> to_p_str("xxx")
    '\x04xxx\x00'
    """
    if len(s) > 254: raise ArgumentError("string too long")
    return s + '\0'

def pulse_message(endpoint, body):
    r"""
    Returns a string that contains a complete Pulse Protocol message.

    >>> pulse_message(1, 'xxx')
    '\x04\x07\x01\x00xxx'
    """

    # In this version of the protocol, the header is always 4 bytes long + 4 bytes for time_t
    header_len = 8
    msg_len = header_len + len(body)

    if use_old_packet_format == True:        
        header = chr(header_len) + chr(msg_len) + chr(endpoint) + chr(get_seq_num()) + struct.pack("<I", int(time.time()))
    else:
        # TODO: Make the length byte actually be a uint16_t
        header = chr(header_len) + chr(endpoint) + chr(msg_len) + chr(0) + struct.pack("<I", int(time.time()))
    return header+body

# Alert Types
FLASH = 1
DBL_FLASH = 2
VIBRATE = 4
DBL_VIBRATE = 8

# Notification Types
class NotificationType:
    EMAIL = 0
    SMS = 1
    CALENDAR = 2
    PHONE = 3

def notification(notification_type, alert_type=0, *args):
    header = '\x00\x00\x00'+chr(notification_type) + '\x01\x00\x00\x00\x00\x14\x14\x14' + '\x00\x00\x00\x00'   # was 1 byte now is 8  old: chr(alert_type)
    body = "".join([to_p_str(s) for s in args])

    # Notifications go to endpoint 1
    return pulse_message(1, header+body)


def email_notification(sender, subject, body, alert_type=0):
    r"""
    Returns a pulse message containing an email notification.

    >>> email_notification("Bob", "Hello", "What's up?", VIBRATE)
    "\x04\x1e\x01\x00\x00\x04\x04Bob\x00\x06Hello\x00\x0bWhat's up?\x00"
    """
    return notification(NotificationType.EMAIL, alert_type, sender, subject, body)

def sms_notification(sender, body, alert_type=0):
    return notification(NotificationType.SMS, alert_type, sender, body)

def calendar_notification(name, location, time, alert_type=0):
    return notification(NotificationType.CALENDAR, alert_type, name, location, time)

def phone_notification(phone_number, name, alert_type=0):
    return notification(NotificationType.PHONE, alert_type, phone_number, name)

def command(id, para1, para2, data=""):
    # Force these ints to be the right size. Python's lack of types is annoying to the
    # C programmer in me.  /me trying to learn to be "pythonic"
    cmd = struct.pack("<HII", (id & 0xFFFF), (para1 & 0xFFFFFFFF), (para2 & 0xFFFFFFFF))
    st = pulse_message(2, cmd+data)
    return st

def cmd_erase_spiflash_sector(sector):
    return command(32, sector, ~sector)

def cmd_erase_os():
    return command(40, 0x00000002, 0x00008000)

def cmd_erase_resources(size):
    return command(40, 0x00000003, size)

def cmd_set_bt_sniff_off():
    return command(90, 0x00000000, 0x00000000)

def cmd_query_main_app():
    return command(0, 0x00000004, 0x00000000)

def cmd_query_vlf():
    return command(0, 0x00000000, 0x00000000)

def cmd_send_time():
    tm = time.localtime();
    struct_tm = struct.pack("<iiiiiiiii", tm.tm_sec, tm.tm_min, tm.tm_hour,
    tm.tm_mday, tm.tm_mon - 1, tm.tm_year - 1900, tm.tm_wday + 1, tm.tm_yday, tm.tm_isdst)
    return command(50, int(time.time()), 0xffffffff, struct_tm)

def cmd_reboot_watch():
    return command(255, 0x5265626f, 0x6f744d65)

def cmd_query_vital(vital):
    return command(0, vital, 0)

def write_spiflash(addr, data):
    if len(data) > 220:
        raise ValueError("data too long: %d bytes" % len(data))
    return pulse_message(3, struct.pack("<I", 0x00000000) + struct.pack("<I", addr) + data)

#if __name__=="__main__":
#    import doctest

#    unittesting=True
#    doctest.testmod(verbose=True)
