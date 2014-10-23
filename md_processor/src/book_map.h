#ifndef book_map_h
#define book_map_h

#include "basic_types.h"

/**
   *  @brief Implementation of the support structure to an order book.
   *
   *  Using std::maps we can keep price levels indexed by price into
   *  maps of orders index by order id containing quantities.
   *
   *  i.e bidLevels[price][orderid] <- quantity
   *
   *  Advantages is that it is easy to implement and requires no sorting
   *
   *  Possible issue is we are doing sorting that is not required for
   *  each add and erase and also we sum the quantities sequentially
   */
struct BookMap {
	// Order quantities mapped by order id key
	typedef typename std::map<OrderIdKeyType,QuantityValueType> Orders;
	// Each price level Bid/Ask of orders associated with orders by price key
	typedef typename std::map<PriceLevelKey,Orders,GreaterComp> BidPriceLevels;
	typedef typename std::map<PriceLevelKey,Orders,LessComp> AskPriceLevels;
	typedef BidPriceLevels SortedBids;
	typedef AskPriceLevels SortedAsks;

	PriceLevelKey get_top_bid();
	PriceLevelKey get_top_ask();

	SortedBids & get_sorted_bids() {
		return bidLevels;
	}

	SortedAsks & get_sorted_asks() {
		return askLevels;
	}

	void clear();

	// Bid levels as a map
    BidPriceLevels bidLevels;
    //Ask levels as a map
    AskPriceLevels askLevels;
};

/**
 *  @brief  Sort bid levels
 *  @param  book BookContainer of with price levels
 *
 *  This is the default version of the sorting.
 *
 *  As BookMap levels already are maps of orders this
 *  template is selected for BookMap
 *
 */
template<typename BookContainer>
inline void  sorted_bid(BookContainer & book,std::map<PriceLevelKey,typename BookContainer::Orders,GreaterComp> &ordered) {
}

/**
 *  @brief  Sort ask levels
 *  @param  book BookContainer of with price levels
 *
 *  This is the default version of the sorting.
 *
 *  As BookMap levels already are maps of orders this
 *  template is selected for BookMap
 *
 */
template<typename BookContainer>
inline void sorted_ask(BookContainer & book,std::map<PriceLevelKey,typename BookContainer::Orders,LessComp> &ordered) {
}

/**
 *  @brief  Get the top bid
 *  @return top bid price
 *
 *  As price levels are maintained in sort
 *  all we need to do is return the top
 *
 */
PriceLevelKey BookMap::get_top_bid() {
	auto top_bid = bidLevels.begin();
	if (top_bid!=bidLevels.end()) {
		return bidLevels.begin()->first;
	}
	else {
		return PriceLevelKey();
	}
}

/**
 *  @brief  Get the top ask
 *  @return top bid price
 *
 *  As price levels are maintained in sort
 *  all we need to do is return the top
 *
 */
PriceLevelKey BookMap::get_top_ask() {
	auto top_ask =  askLevels.begin();
	if (top_ask!=askLevels.end()) {
		return askLevels.begin()->first;
	}
	else {
		return PriceLevelKey();
	}
}

void BookMap::clear() {
	bidLevels.clear();
	askLevels.clear();
}

#endif
