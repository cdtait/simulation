'''
Created on Oct 08, 2014

@author: ctait
'''
from __future__ import print_function

import logging
import order_book
import md_logging as fl

class MDHandler(object):
    '''
    This is a prototype of the md handler
    which implements the specification outlined in requirements
    '''
    def __init__(self):
        self.ob = order_book.OrderBook()
        self.md_logger=fl.MDLogger()

    def stop(self):
        self.md_logger.stop()
        
    def processMessage(self, message):
        (event,nmessage) = order_book.get_event(message,self.ob.stats)
        if event == 'A':
            self.ob.add(nmessage)
        elif event == 'M':
            self.ob.modify(nmessage)
        elif event == 'X':
            self.ob.cancel(nmessage)
        elif event == 'T':
            self.ob.trade(nmessage)

    def printCurrentOrderBook(self, ostr):
        import copy
        bdc=copy.deepcopy(self.ob.buyLevels.items())
        sdc=copy.deepcopy(self.ob.sellLevels.items())

        logging.info((bdc,sdc))

    def printStats(self, ostr):
        print(self.ob.stats,file=ostr)
        