#ifndef data_event_h
#define data_event_h

#include "disruptor/interface.h"
#include "md_types.h"

using namespace disruptor;

/**
 *  @brief Implementation of data event that will be moved
 *  to into the disruptor ring buffer used by the md_publisher.
 *
 */
class BookDataEvent {
 public:
	BookDataEvent(const int64_t& value = 0) : value_(value) {}

	/**
	 * Sequence number for ring buffer
	 * @return
	 */
    int64_t value() const {
    	return value_;
    }

    /**
     * Set the sequence number
     * @param value the current sequence number
     */
    void set_value(const int64_t& value) {
    	value_ = value;
    }

    /**
     * The book_data we hodl against the vent
     * @return the book_data
     */
    BookData<5>&& book_data() {
    	return std::move(book_data_);
    }

    /**
     * Set the book value
     * @param book_data
     */
    void book_data(BookData<5>&& book_data) {
    	book_data_ = std::move(book_data);
    }

 private:
    // The book data we hold
    BookData<5> book_data_;
    // Sequence number
    int64_t value_;
};

/**
 *  @brief Implementation of data event factor which creates
 *  the elements for the ring buffer.
 *
 */
class BookDataEventFactory : public EventFactoryInterface<BookDataEvent> {
 public:
	virtual ~BookDataEventFactory() = default;

	/**
	 * Craete the
	 * @param size the ring buffer size
	 * @return the batch of events
	 */
    virtual BookDataEvent* NewInstance(const int& size) const {
        return new BookDataEvent[size];
    }
};

/**
 *  @brief This is a handler which receives an event put onto the ringbuffer
 *
 *  This is acting as a simple printer to std::cout.
 *  Can print readable text and CSv formats by assigning the appropriate
 *  print functions.
 *
 *  This is operating in the reader thread(s) where the translators
 *  are operating in a writer thread
 *
 */
class BookDataBatchHandler : public EventHandlerInterface<BookDataEvent> {
 public:
	/**
	 * Select the correct printing functions either for CSV or txt
	 * @param printType the type of printing we want
	 */
		BookDataBatchHandler(PrintType printType) :
			EventHandlerInterface<BookDataEvent>()
	{
			if (printType==PrintType::Trading) {
				print_trade_f=print_trade_txt<5>;
			    print_book_f=print_book_txt<5>;
			    print_mid_f=print_mid_txt<5>;
			}
			else if (printType==PrintType::Betting) {
				print_trade_f=print_trade_betting<5>;
				print_book_f=print_book_betting<5>;
				print_mid_f=print_mid_betting<5>;
			}
			else if (printType==PrintType::CSV) {
				print_trade_f=print_trade_csv<5>;
				print_book_f=print_book_csv<5>;
				print_mid_f=print_mid_csv<5>;
			}
	}
	virtual ~BookDataBatchHandler() = default;

	/**
	 * When the event is receieve by a consumer(handler)
	 * we want to print out the value
	 * @param sequence
	 * @param end_of_batch
	 * @param event the book_data wrapped event
	 */
    virtual void OnEvent(const int64_t& sequence,
                         const bool& end_of_batch,
                         BookDataEvent* event) {
        if (event) {
        	if (event->book_data().event==Event::Trade) {
        		print_trade_f(std::cout,event->book_data());
        	}
        	else if (event->book_data().event==Event::Mid) {
        		print_mid_f(std::cout,event->book_data());
        	}
        	else {
        		print_book_f(std::cout,event->book_data());
        	}
        }
    };

    virtual void OnStart() {}
    virtual void OnShutdown() {}

 private:
    // Trade event printing function
	PrintFunctionType<5>::Funcp print_trade_f;
	// Book event printing function
	PrintFunctionType<5>::Funcp print_book_f;
	// Mid price printing function
	PrintFunctionType<5>::Funcp print_mid_f;
};

/**
 *  @brief This is a translator which moves the internals of the
 *  current BookData objects to the ringbuffer event just before it is
 *  advertised to the event handlers.
 *
 *  This is operating in the writer thread where the event handler(s)
 *  are operating in a reader thread(s)
 *
 */
class BookDataEventTranslator : public EventTranslatorInterface<BookDataEvent> {
 public:
	virtual ~BookDataEventTranslator() = default;

	/**
	 * Adds the book_data to the event to offer to the ring buffer
	 * @param sequence ring_buffer sequence tracking
	 * @param event the disruptor wrapped event
	 * @return the event with the book_data set
	 */
    virtual BookDataEvent* TranslateTo(const int64_t& sequence, BookDataEvent* event) {
        event->set_value(sequence);
        event->book_data(std::move(book_data));

        return event;
    };

    // Current book we want to add assume set before TranslateTo happens
    BookData<5> book_data;
};

#endif
