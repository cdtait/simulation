import zmq
from random import randrange

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-f", "--file",action="store", type="string", dest="file")
(options, args) = parser.parse_args()

import sys
import zmq

REQUEST_TIMEOUT = 1000
REQUEST_RETRIES = 3
SERVER_ENDPOINT = "tcp://localhost:5555"

def retry_send(line,expectedReply,client,poll):
    retries_left = REQUEST_RETRIES
    while retries_left:
        client.send_string(line.strip())
        expect_reply = True
        while expect_reply:
            socks = dict(poll.poll(REQUEST_TIMEOUT))
            if socks.get(client) == zmq.POLLIN:
                reply = client.recv()
                if not reply:
                    break
                elif reply == expectedReply:
                    retries_left = 0
                    expect_reply = False
                    break
            else:
                # Socket is confused. Close and remove it.
                client.setsockopt(zmq.LINGER, 0)
                client.close()
                poll.unregister(client)
                retries_left -= 1
                if retries_left == 0:
                    print "Server seems to be offline, abandoning"
                    break
                # Create new connection
                client = context.socket(zmq.REQ)
                client.connect(SERVER_ENDPOINT)
                poll.register(client, zmq.POLLIN)
                client.send_string(line.strip())

if __name__ == '__main__':
    context = zmq.Context(1)
    
    client = context.socket(zmq.REQ)
    client.connect(SERVER_ENDPOINT)
    
    poll = zmq.Poller()
    poll.register(client, zmq.POLLIN)
    
    numLines=0
    with open(options.file, "r") as g:
        numLines=sum(1 for _ in g)
        
    if numLines > 0:
        retry_send("{0:d}".format(numLines),str(numLines),client,poll)

    lineNumber=1
    with open(options.file, "r") as f:
      for line in f:
        retry_send(line,str(lineNumber),client,poll)
        lineNumber+=1
    
    client.setsockopt(zmq.LINGER, 0)
    client.close()
    poll.unregister(client)

    print "Done"
    context.term()
