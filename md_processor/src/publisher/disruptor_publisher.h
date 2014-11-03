#ifndef disruptor_publisher_h
#define disruptor_publisher_h

#include <sys/time.h>

#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>

#include <disruptor/ring_buffer.h>
#include <disruptor/event_publisher.h>
#include <disruptor/event_processor.h>
#include <disruptor/exception_handler.h>

#include "book_data_event.h"

#include "md_publisher.h"

using namespace disruptor;

/**
 * @brief md publisher hub. This brings together all the components
 * which implements the kind of publishing we want to achieve.
 *
 * Uses a disruptor messaging queue which is included and all copyright
 * honoured
 *
 * So internally this uses an ring buffer and handlers(consumers) of
 * this data implement handling classes which can operate on a consumer
 * thread and this off loads the publish to one or many consumers.
 *
 * We are only using one consumer here which you will see in book_data_events.h
 *
 */
template<int N=5>
class disruptor_publisher : public md_publisher<N> {
public:
	disruptor_publisher(PrintType printType) : buffer_size(1024*100),
					book_data_factory{},
					//ring_buffer(&book_data_factory, buffer_size,
					//kSingleThreadedStrategy, kBusySpinStrategy),
					ring_buffer(&book_data_factory, buffer_size,
										kSingleThreadedStrategy, kBlockingStrategy),
					sequence_to_track(0),
					barrier(ring_buffer.NewBarrier(sequence_to_track)),
				 	book_data_handler(printType),
					book_data_exception_handler{},
					processor(&ring_buffer,
							  (SequenceBarrierInterface*)barrier.get(),
							  &book_data_handler,
							  &book_data_exception_handler),
					consumer(std::ref<BatchEventProcessor<BookDataEvent>>(processor)),
					translator(new BookDataEventTranslator),
					publisher(&ring_buffer)
	{
	}


	/**
	 * Stop the processing and wait for the consumer thread to finish
	 */
	void stop() {
		processor.Halt();
		consumer.join();
	}

	/**
	 * Offer the book data to the translator
	 * @param book_data
	 */
	void offer(BookData<5> && book_data) {
		// One copy  of data needed before translator
		translator->book_data = std::forward<BookData<5>>(book_data);
		// OnTranslate called in PublishEvent
		publisher.PublishEvent(translator.get());
	}

private:
	// Ring buffer size
	int buffer_size;
	// The events produced for the ring buffer
	BookDataEventFactory book_data_factory;
	// The ring buffer used in disruptor
	RingBuffer<BookDataEvent> ring_buffer;
	// Sequence number tracking of messages
	std::vector<Sequence*> sequence_to_track;
	// producer/consumer sequence barrier
	std::unique_ptr<ProcessingSequenceBarrier> barrier;
	// The handler which with action on the data message
	BookDataBatchHandler book_data_handler;
	// Exception handling
	IgnoreExceptionHandler<BookDataEvent> book_data_exception_handler;
	// Processor which gathers batches of data messages put on ring buffer
	BatchEventProcessor<BookDataEvent> processor;
	// The thread our off loaded consumer runs in
	std::thread consumer;
	// Custom translator for data messages i.e book_data
	std::unique_ptr<BookDataEventTranslator> translator;
	EventPublisher<BookDataEvent> publisher;
};

#endif
