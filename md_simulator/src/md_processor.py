'''
Created on Oct 08, 2014

@author: ctait

Python implementation 
'''

import sys
import md_handling

if __name__ == '__main__':
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option("-f", "--file",
                  action="store", type="str", 
                  dest="file", default="md.csv")
    (options, args) = parser.parse_args()
    
    md = md_handling.MDHandler()
    counter=0
    with open(options.file, 'rb') as fp:
        for message in iter(fp.readline, ''):
            md.processMessage(message)
            counter += 1
            if counter % 10 == 0:
                md.printCurrentOrderBook(sys.stdout)

    md.stop()

    md.printStats(sys.stderr)
    

    
