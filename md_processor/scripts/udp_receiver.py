#!/usr/bin/env python

import time
import struct
import socket
import sys

'''
    Used to sample the csv or any text file and send it as udp packets
    
    Window 1 - python udp_receiver.py --addr=192.168.1.235 --port=7017 --expected=3023
    Window 2 - python udp_sender.py -f ../data/md-test-2.csv --addr=192.168.1.234 -p 6016
'''
if __name__ == '__main__':
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option("-p", "--port",action="store", type="int", dest="port", default=8123)
    parser.add_option("-a", "--addr",action="store", type="string", dest="addr", default="127.0.0.1")
    parser.add_option("-e", "--expected",action="store", type="int", dest="expected")

    (options, args) = parser.parse_args()

    # Create a socket
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Bind it to the port
    s.bind((options.addr, options.port))
    s.settimeout(10.0)

    # Loop, printing any data we receive
    count = 0
    while count < options.expected:
        data, sender = s.recvfrom(1500)
        count += 1
    print ("received {:d} messages".format(count))
 
