'''
Created on Oct 08, 2014

@author: ctait
'''

class Singleton:
    """
    Python implementation for a singleton used by OrderStats
    """
    def __init__(self, klass):
        self.klass = klass
        self.instance = None
    def __call__(self, *args, **kwds):
        if self.instance == None:
            self.instance = self.klass(*args, **kwds)
        return self.instance
