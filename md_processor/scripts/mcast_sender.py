#!/usr/bin/env python

port = 8123
mcastgroup = '225.0.0.250'
ttl = 1 

import time
import struct
import socket
import sys

'''
    Used to sample the csv or any text file and send it as multicast packets which we can
    write with tcpdump into a pcap file.
    
    Window 1 - tcpdump -B 1000000 -s0 -i em1 -w data/md-test-2.pcap \(udp port 8123 and dst net 225.0.0.250\)
    Window 2 - python mcast_sender.py -f ../data/md-test-2.csv
    
    Then with this pcap file we can use the pcap adapter
'''
if __name__ == '__main__':
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option("-f", "--file",action="store", type="string", dest="file")
    (options, args) = parser.parse_args()

    addrinfo = socket.getaddrinfo(mcastgroup, None)[0]

    s = socket.socket(addrinfo[0], socket.SOCK_DGRAM)

    # Set Time-to-live (optional)
    ttl_bin = struct.pack('@i', ttl)
    if addrinfo[0] == socket.AF_INET: # IPv4
        s.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl_bin)
    else:
        s.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_MULTICAST_HOPS, ttl_bin)

    with open(options.file, "r") as f:
        for line in f:
            s.sendto(line.strip(), (addrinfo[4][0], port))
            