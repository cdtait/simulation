
#ifndef md_handler_h
#define md_handler_h

#include <iostream>

#include "md_types.h"
#include "parsers.h"
#include "order_book.h"
#include "md_publisher.h"

/**
   *  @brief Implementation of the md handler
   *
   *  Controls the print publisher and message processing which
   *  is sent to the order book
   *
   */
template <typename Parser=type_parser, typename Container=BookMap>
class md_handler {
public:
    void stop() {
        publisher->stop();
    }

    void start(PrintType printType) {
    	publisher=std::make_unique<md_publisher>(printType);
    }

    /**
     * Get the event and determines which methods on order
     * book is suitable for the event type
     *
     * If the publisher is enabled then we can offer the
     * data events for publishing
     *
     * @param iter
     */
    template <typename Iterator>
    void processMessage(Iterator& message_iter,const Iterator& end) {
        Event event = Parser::get_event(message_iter,end);

			switch (event) {
			default:
				break;
			case Event::Add:
				ob.add(Parser::get_order(message_iter,end));
				if (publisher.get())
					publisher->offer(ob.mid_data());
				break;
			case Event::Modify:
				ob.modify(Parser::get_order(message_iter,end));
				if (publisher.get())
					publisher->offer(ob.mid_data());
				break;
			case Event::Cancel:
				ob.cancel(Parser::get_order(message_iter,end));
				if (publisher.get())
					publisher->offer(ob.mid_data());
				break;
			case Event::Trade:
				ob.trade(Parser::get_trade(message_iter,end));
				if (publisher.get()) {
					publisher->offer(ob.trade_data());
					publisher->offer(ob.mid_data());
				}
				break;
			}
    }

    /**
     * Offers the book data off for publishing
     * @param ostr Not use now as offloaded
     */
    void printCurrentOrderBook(std::ostream &ostr) {
    	if (publisher.get())
    		publisher->offer(ob.book_data());
     }

    /**
     * Prints the stats onto the stream given
     * @param ostr output stream
     */
    void printStats(std::ostream &ostr) {
        stats().print(ostr);
    }

	private:
    	// OrderBook template for a particular container
    	OrderBook<Container> ob;
    	// The publisher which handles book data messages
    	std::unique_ptr<md_publisher> publisher;
};

typedef md_handler<> MDHandler;

#endif
