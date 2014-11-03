import zmq
from random import randrange

from optparse import OptionParser
parser = OptionParser()
parser.add_option("-f", "--file",action="store", type="string", dest="file")
(options, args) = parser.parse_args()

context = zmq.Context()
socket = context.socket(zmq.PUSH)
socket.connect("tcp://localhost:5557")
socket.set_hwm(10000)

numLines=0
with open(options.file, "r") as g:
    numLines=sum(1 for _ in g)
        
if numLines > 0:
    socket.send_string("{0:d}".format(numLines))
        
with open(options.file, "r") as f:
  for line in f:
    socket.send_string(line.strip())
    

    
