'''
Created on Oct 08, 2014

@author: ctait
'''

import Queue
import logging
import multiprocessing
import order_book
try:
    import threading
except ImportError: #pragma: no cover
    threading = None
    
class QueueHandler(logging.Handler):
    """
    This handler sends events to a queue. Typically, it would be used together
    with a multiprocessing Queue to centralise logging to file in one process
    (in a multi-process application), so as to avoid file write contention
    between processes.

    This code is new in Python 3.2, but this class can be copy pasted into
    user code for use with earlier Python versions.
    """

    def __init__(self, queue):
        """
        Initialise an instance, using the passed queue.
        """
        logging.Handler.__init__(self)
        self.queue = queue

    def enqueue(self, record):
        """
        Enqueue a record.

        The base implementation uses put_nowait. You may want to override
        this method if you want to use blocking, timeouts or custom queue
        implementations.
        """
        self.queue.put_nowait(record)

    def prepare(self, record):
        """
        Prepares a record for queuing. The object returned by this method is
        enqueued.

        The base implementation formats the record to merge the message
        and arguments, and removes unpickleable items from the record
        in-place.

        You might want to override this method if you want to convert
        the record to a dict or JSON string, or send a modified copy
        of the record while leaving the original intact.
        """
        # The format operation gets traceback text into record.exc_text
        # (if there's exception data), and also puts the message into
        # record.message. We can then use this to replace the original
        # msg + args, as these might be unpickleable. We also zap the
        # exc_info attribute, as it's no longer needed and, if not None,
        # will typically not be pickleable.
        self.format(record)
        record.msg = record.message
        record.args = None
        record.exc_info = None
        return record

    def emit(self, record):
        """
        Emit a record.

        Writes the LogRecord to the queue, preparing it for pickling first.
        """
        try:
            self.enqueue(self.prepare(record))
        except (KeyboardInterrupt, SystemExit): #pragma: no cover
            raise
        except:
            self.handleError(record)

if threading:
    class QueueListener(object):
        """
        This class implements an internal threaded listener which watches for
        LogRecords being added to a queue, removes them and passes them to a
        list of handlers for processing.
        """
        _sentinel = None

        def __init__(self, queue, *handlers):
            """
            Initialise an instance with the specified queue and
            handlers.
            """
            self.queue = queue
            self.handlers = handlers
            self._stop = threading.Event()
            self._thread = None

        def dequeue(self, block):
            """
            Dequeue a record and return it, optionally blocking.

            The base implementation uses get. You may want to override this method
            if you want to use timeouts or work with custom queue implementations.
            """
            return self.queue.get(block)

        def start(self):
            """
            Start the listener.

            This starts up a background thread to monitor the queue for
            LogRecords to process.
            """
            self._thread = t = threading.Thread(target=self._monitor)
            t.setDaemon(True)
            t.start()

        def prepare(self , record):
            """
            Prepare a record for handling.

            This method just returns the passed-in record. You may want to
            override this method if you need to do any custom marshalling or
            manipulation of the record before passing it to the handlers.
            """
            return record

        def handle(self, record):
            """
            Handle a record.

            This just loops through the handlers offering them the record
            to handle.
            """
            record = self.prepare(record)
            for handler in self.handlers:
                handler.handle(record)

        def _monitor(self):
            """
            Monitor the queue for records, and ask the handler
            to deal with them.

            This method runs on a separate, internal thread.
            The thread will terminate if it sees a sentinel object in the queue.
            """
            q = self.queue
            has_task_done = hasattr(q, 'task_done')
            while not self._stop.isSet():
                try:
                    record = self.dequeue(True)
                    if record is self._sentinel:
                        break
                    self.handle(record)
                    if has_task_done:
                        q.task_done()
                except Queue.Empty:
                    pass
            # There might still be records in the queue.
            while True:
                try:
                    record = self.dequeue(False)
                    if record is self._sentinel:
                        break
                    self.handle(record)
                    if has_task_done:
                        q.task_done()
                except Queue.Empty:
                    break

        def enqueue_sentinel(self):
            """
            This is used to enqueue the sentinel record.

            The base implementation uses put_nowait. You may want to override this
            method if you want to use timeouts or work with custom queue
            implementations.
            """
            self.queue.put_nowait(self._sentinel)

        def stop(self):
            """
            Stop the listener.

            This asks the thread to terminate, and then waits for it to do so.
            Note that if you don't call this before your application exits, there
            may be some records still left on the queue, which won't be processed.
            """
            self._stop.set()
            self.enqueue_sentinel()
            self._thread.join()
            self._thread = None 

class TradeOrderBookQueueListener(QueueListener):
    """
    This operates in the logging thread and listens to
    record event which in can then make printable for
    the formatter
    """
    def __init__(self, queue, *handlers):
        QueueListener.__init__(self, queue, *handlers)

    def prepare(self , record):
        """
        Transform here
        """
        (bl,sl)=record.msg
        record.msg = order_book.OrderBook.to_string(bl,sl)
        
        return record
        
class TradeOrderBookQueueHandler(QueueHandler):
    """
    We ant to avoid doing any text transform of the record object
    we with format in the other thread as above in prepare
    This is operating in the main thread
    """
    def __init__(self, queue):
        QueueHandler.__init__(self,queue)

    def prepare(self, record):
        """
        Leave any transform
        """
        record.args = None
        record.exc_info = None
        return record
    
import sys

class MDLogger(object):
    '''
    The md logger creation and control uses an eevent queue to pass
    messages to the TradeOrderBookQueueHandler via the TradeOrderBookQueueListener
    operating in a different thread
    '''
    def __init__(self):       
        que = multiprocessing.Queue(-1) # no limit on size
        queue_handler = TradeOrderBookQueueHandler(que)

        #logging.config.fileConfig('logging.conf')

        root = logging.getLogger()
        root.setLevel(logging.DEBUG)
        root.addHandler(queue_handler)
        
        formatter = logging.Formatter('%(threadName)s: %(asctime)s - %(name)s - %(levelname)s - %(message)s')
        handler = logging.StreamHandler(sys.stdout)
        handler.setFormatter(formatter)
        
        self.listener = TradeOrderBookQueueListener(que, handler)
        self.listener.start()

    def stop(self):
        self.listener.stop()
        