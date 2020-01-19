#!/usr/bin/env python

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
    parser.add_option("-p", "--port",action="store", type="int", dest="port", default=8123)
    parser.add_option("-g", "--group",action="store", type="string", dest="mcastgroup", default="225.0.0.250")
    parser.add_option("-t", "--ttl",action="store", type="int", dest="ttl", default=1)
    parser.add_option("-a", "--addr",action="store", type="string", dest="addr", default="127.0.0.1")

    (options, args) = parser.parse_args()

    addrinfo = socket.getaddrinfo(options.mcastgroup, None)[0]

    s = socket.socket(addrinfo[0], socket.SOCK_DGRAM)

    s.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_IF, socket.inet_aton(options.addr))

    # Set Time-to-live (optional)
    ttl_bin = struct.pack('@i', options.ttl)
    if addrinfo[0] == socket.AF_INET: # IPv4
        s.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl_bin)
    else:
        s.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_MULTICAST_HOPS, ttl_bin)

    with open(options.file, "r") as f:
        for line in f:
            s.sendto(line.strip(), (addrinfo[4][0], options.port))
            
