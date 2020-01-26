
#ifndef md_handler_h
#define md_handler_h

#include <iostream>

#include "md_types.h"
#include "md_parsers.h"
#include "md_order_book.h"
#include "md_publishers.h"
#include "arguments.h"

/**
   *  @brief Implementation of the md handler
   *
   *  Controls the print publisher and message processing which
   *  is sent to the order book
   *
   */
template <typename Parser=list_parser, typename BookContainer=BookMap,typename Publisher=print_publisher<>>
class md_handler {
public:
    void stop() {
    	if (publisher.get()) {
    		publisher->stop();
    	}
    }

    void start(PrintType printType, const arguments& args) {
    	if (not std::is_same<null_publisher, Publisher>::value) {
    		publisher=std::make_unique<Publisher>(printType,args);
    	}
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
    template <typename TokenContainer>
    void process_message(TokenContainer && tokens) {
		auto message_iter = tokens.begin();
		auto end = tokens.end();
        Event event = Parser::get_event(message_iter,end);

		switch (event) {
		default:
			break;
		case Event::Add:
			ob.add(Parser::get_order(message_iter,end));
			if (publisher.get()) {
				publisher->offer(ob.mid_data());
				publisher->offer(ob.book_data());
			}
			break;
		case Event::Modify:
			ob.modify(Parser::get_order(message_iter,end));
			if (publisher.get()) {
				publisher->offer(ob.mid_data());
				publisher->offer(ob.book_data());
			}
			break;
		case Event::Cancel:
			ob.cancel(Parser::get_order(message_iter,end));
			if (publisher.get()) {
				publisher->offer(ob.mid_data());
				publisher->offer(ob.book_data());
			}
			break;
		case Event::Trade:
			ob.trade(Parser::get_trade(message_iter,end));
			if (publisher.get()) {
				publisher->offer(ob.trade_data());
				publisher->offer(ob.mid_data());
			}
			break;
		case Event::Snapshot:
			bool traded = ob.snapshot_trades(Parser::get_trades(message_iter,end));
			ob.snapshot_orders(Parser::get_orders(message_iter,end));
			if (publisher.get()) {
				if (unlikely(traded)) {
					publisher->offer(ob.trade_data());
				}
				else {
					publisher->offer(ob.book_data());
				}
			}
			break;
		}
    }

    /**
     * Offers the book data off for publishing
     */
    void publishOrderBook() {
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
    	OrderBook<BookContainer> ob;
    	// The publisher which handles book data messages
    	std::unique_ptr<Publisher> publisher;
};

typedef md_handler<> MDHandler;

#endif
