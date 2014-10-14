
#include <stdint.h>
#include <sys/time.h>
#include <algorithm>
#include <array>

#include <cstddef>
#include <cstdio>
#include <cstdlib>

#include <iostream>
#include <limits>
#include <string>
#include <tuple>

#include <utility>


#include "md_types.h"
#include "md_stats.h"
#include "md_helper.h"

#include "strtk/strtk.hpp"
#include "json_spirit/json_spirit_reader_template.h"

struct type_parser {

/**
 * Handle floating types
 *
 * Using strtk included and all copyright honoured
 *
 * Handles all types of integers included signed
 *
 * @param itr
 * @param end
 * @param result
 * @param
 * @return
 */
template <typename T>
static bool make_type(json_spirit::Array::iterator & itr, const json_spirit::Array::iterator & end, T &result,
		typename std::enable_if<std::is_floating_point<T>::value >::type* = 0 ) {
	result=itr->get_real();
	return true;
}

/**
 * Handle a integer type
 *
 * Using strtk included and all copyright honoured
 *
 * Handles all types of integers included signed
 *
 * @param itr
 * @param end
 * @param result
 * @param
 * @return True if its an integer OK
 */
template <typename T>
static bool make_type(json_spirit::Array::iterator & itr, const json_spirit::Array::iterator & end, T &result,
		typename std::enable_if<std::is_integral<T>::value >::type* = 0) {
	result=itr->get_int();
	return true;
}

/**
 * Handle char* strings
 * @param itr
 * @param end
 * @param t
 * @return True if have a string OK
 */
static bool make_type(json_spirit::Array::iterator & itr, const json_spirit::Array::iterator & end, char* t) {
	if (end == itr) return false;
	std::string s = itr->get_str();
	std::string::iterator sit = s.begin();
	std::string::iterator eit = s.end();
	while (eit != sit && *t) {
		*(t++)=*(sit++);
	}
	*t='\0';
	return true;
}

/**
 * Handle single char
 *
 * @param itr
 * @param end
 * @param t
 * @return True if one char OK
 */
static bool make_type(json_spirit::Array::iterator & itr, const json_spirit::Array::iterator & end, char& t) {
	if (end == itr) return false;
	t=itr->get_str()[0];
	return true;
}

/**
 * Make type implementation for Event enums
 * need specific handling
 *
 * @param itr
 * @param end
 * @param side
 * @return True if syntax OK
 */
static bool make_type(json_spirit::Array::iterator & itr, const json_spirit::Array::iterator & end, Event & event) {
	if (end == itr) return false;
	if (itr->get_str()=="A")
		event=Event::Add;
	else if (itr->get_str()=="M")
		event=Event::Modify;
	else if (itr->get_str()=="X")
		event=Event::Cancel;
	else if (itr->get_str()=="T")
		event=Event::Trade;
	else {
		event=Event::Unknown;
		return false;
	}

	return true;
}

/**
 * Make type implementation Side enums
 * need specific handling
 *
 * @param itr
 * @param end
 * @param side
 * @return True if syntax OK
 */
static bool make_type(json_spirit::Array::iterator & itr, const json_spirit::Array::iterator & end, Side & side) {
	if (end == itr) return false;
	if (itr->get_str()=="B")
		side=Side::Bid;
	else if (*itr=="S")
		side=Side::Ask;
	else {
		side=Side::Unknown;
		return false;
	}

	return true;
}

/**
 * Specialisation for Event type parsing
 *
 * @param itr
 * @param event
 * @return
 */
template <typename Iterator>
static bool parse_type(Iterator& message_iter,const Iterator& end,Event & event) {
    return make_type(message_iter,end, event);
}

/**
 * Specialisation for Side parsing
 *
 * @param itr
 * @param side
 * @return True if syntax OK
 */
template <typename Iterator>
static bool parse_type(Iterator& message_iter,const Iterator& end,Side & side) {
    return make_type(message_iter,end, side);
}

/**
 * Parse to extract a variable list and apply parsing result type
 *
 * @param itr
 * @param t
 * @return True if syntax OK
 */
template <typename T, typename Iterator, typename... Rest>
static bool parse_type(Iterator& message_iter,const Iterator& end,
                  T& t,
                  Rest&... rest)
{
	return parse_type(message_iter,end,t) && parse_type(++message_iter,end,rest...);
}

/**
 * Parse to extract a single templated result type
 *
 * @param itr
 * @param t
 * @return True if syntax OK
 */
template <typename T, typename Iterator>
static bool parse_type(Iterator& message_iter,const Iterator& end,
                  T& t)
{
	return make_type(message_iter,end,t);
}

/**
 * Extracts the event type from the text message event
 * @param message_iter
 * @return True if syntax OK
 */
template <typename Iterator>
static auto get_event(Iterator &message_iter, const Iterator &end) -> Event {
	Event event{Event::Unknown};
	parse_type(message_iter,end,event);

    if (!contains(event,'A','M','X','T')) {
        stats().event_error();
        throw std::runtime_error("Corruption");
    }

    return event;
}

/**
 * Extracts the order object properties from the text message event
 *
 * @param message_iter
 * @return
 */
template <typename Iterator>
static auto get_order(Iterator &message_iter, const Iterator &end) -> Order {
	// Order id extract
	OrderIdKeyType orderid{};
    if(likely(parse_type(++message_iter,end,orderid))) {
	   if (!(orderid > 0 && orderid <= max_order_id)) {
		   stats().order_range();
		   throw std::runtime_error("Order id range");
	   }
    }
    else {
    	stats().order_parse();
    	throw std::runtime_error("Order id syntax");
    }

    // Side extract
    Side side{Side::Unknown};
	if (unlikely(!parse_type(++message_iter,end,side))) {
    	stats().side_error();
    	throw std::runtime_error("Order side syntax");
    }

    // Quantity extract
    QuantityValueType quantity{};
	if (likely(parse_type(++message_iter,end,quantity))) {
		if (!(quantity > 0 && quantity <= max_order_quantity)) {
			stats().quantity_range();
			throw std::runtime_error("Order quantity range");
		}
	}
	else {
		stats().quantity_parse();
		throw std::runtime_error("Order quantity syntax");
	}

    // Price extract
	PriceLevelKey price{};
	if (likely(parse_type(++message_iter,end,price))) {
		if (!(price > 0 && price <= max_order_price)) {
			stats().price_range();
			throw std::runtime_error("Order price range");
		}
	}
	else {
		stats().price_parse();
		throw std::runtime_error("Order price syntax");
	}

	// I am a pod so optimizer do job
    return Order{orderid,side,quantity,price};
}

/**
 * Extracts the trade properties from the text message event
 *
 * @param message_iter
 * @return
 */
template <typename Iterator>
static auto get_trade(Iterator &message_iter, const Iterator &end) -> Trade {
    // Side extract
    Side side{Side::Unknown};
	if (unlikely(!parse_type(++message_iter,end,side))) {
    	stats().side_error();
    	throw std::runtime_error("Order side syntax");
    }

    QuantityValueType quantity{};
	if (likely(parse_type(++message_iter,end,quantity))) {
		if (quantity<=0) {
			stats().quantity_range();
			throw std::runtime_error("Trade quantity range"+quantity);
		}
	}
	else {
		stats().quantity_parse();
		throw std::runtime_error("Trade quantity syntax");
	}

	PriceLevelKey price{};
	if (likely(parse_type(++message_iter,end,price))) {
		if (price<=0) {
			stats().price_parse();
			throw std::runtime_error("Trade price range"+price);
		}
	}
	else {
		stats().price_parse();
		throw std::runtime_error("Trade price is syntax");
	}

    return Trade{side,quantity,price};
}

};


