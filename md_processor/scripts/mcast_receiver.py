#!/usr/bin/env python

import time
import struct
import socket
import sys

'''
    Used to sample the csv or any text file and send it as multicast packets which we can
    write with tcpdump into a pcap file.
    
    Window 1 - python mcast_receiver.py --addr=127.0.0.1 -g 225.0.0.250 
    Window 2 - python mcast_sender.py -f ../data/md-test-2.csv
    
    Then with this pcap file we can use the pcap adapter
'''
if __name__ == '__main__':
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option("-p", "--port",action="store", type="int", dest="port", default=8123)
    parser.add_option("-g", "--group",action="store", type="string", dest="mcastgroup", default="225.0.0.250")
    parser.add_option("-t", "--ttl",action="store", type="int", dest="ttl", default=1)
    parser.add_option("-a", "--addr",action="store", type="string", dest="addr", default="127.0.0.1")

    (options, args) = parser.parse_args()

    # Look up multicast group address in name server and find out IP version
    addrinfo = socket.getaddrinfo(options.mcastgroup, None)[0]

    # Create a socket
    s = socket.socket(addrinfo[0], socket.SOCK_DGRAM)

    # Allow multiple copies of this program on one machine
    # (not strictly needed)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # Bind it to the port
    s.bind((options.mcastgroup, options.port))

    group_bin = socket.inet_pton(addrinfo[0], addrinfo[4][0])
    # Join group
    if addrinfo[0] == socket.AF_INET: # IPv4
        mreq = struct.pack('4s4s', socket.inet_aton(options.mcastgroup), socket.inet_aton(options.addr))
        s.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)
    else:
        mreq = group_bin + struct.pack('@I', socket.inet_aton(options.addr))
        s.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_JOIN_GROUP, mreq)

    # Loop, printing any data we receive
    while True:
        data, sender = s.recvfrom(1500)
        while data[-1:] == '\0': data = data[:-1] # Strip trailing \0's
        print (str(sender) + '  ' + repr(data))
 
