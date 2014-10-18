#ifndef book_hash_h
#define book_hash_h

#include "basic_types.h"

/**
   *  @brief Implementation of the support structure to an order book.
   *
   *  Using std::unordered_map we can keep price levels indexed by price into
   *  maps of orders index by order id containing quantities.
   *
   *  i.e bidLevels[price][orderid] <- quantity
   *
   *  Advantages is that it is easy to implement and offers no sort overhead
   *  for every add and erase
   *
   *  Possible issue sum the quantities sequentially and we have to sort
   *  Create a sorted version on demand when we use the sorted_* method
   *  as below
   */
struct BookHash {
	// Order quantities mapped by order id key
	typedef typename std::unordered_map<OrderIdKeyType,QuantityValueType> Orders;
	// Each price level Bid/Ask of orders associated with orders by price key
	typedef typename std::unordered_map<PriceLevelKey,Orders> PriceLevels;
	typedef PriceLevels AskPriceLevels;
	typedef PriceLevels BidPriceLevels;

	PriceLevelKey get_top_bid();
	PriceLevelKey get_top_ask();

	typedef std::map<PriceLevelKey,BookHash::Orders,GreaterComp> SortedBids;
	typedef std::map<PriceLevelKey,BookHash::Orders,LessComp> SortedAsks;

	SortedBids & get_sorted_bids();
	SortedAsks & get_sorted_asks();

	void clear();

	// Bid levels as a unordered_map
    BidPriceLevels bidLevels;
    // Ask levels as a unordered_map
	AskPriceLevels askLevels;

private:
	// Bid levels as a vector of custom PriceLevel types
	SortedBids sortedBidLevels;
    // Ask levels as a vector of custom PriceLevel types
	SortedAsks sortedAskLevels;
};

/**
 *  @brief  Sort bid levels
 *  @param  book Container of with price levels
 *
 *  This is the specialized version of the sorting for BookHash explicitly.
 *
 *  We return the sorted map by simply adding the values out of the BookHash
 *
 *  So not efficient but it needs to be analysed in use case.
 *
 */
template<>
inline void sorted_bid(BookHash & book,BookHash::SortedBids &ordered) {
	for (auto& i : book.bidLevels) {
		ordered[i.first]=i.second;
	}
}

/**
 *  @brief  Sort ask levels
 *  @param  book Container of with price levels
 *
 *  This is the specialized version of the sorting for BookHash explicitly.
 *
 *  We return the sorted map by simply adding the values out of the BookHash
 *
 *  So not efficient but it needs to be analysed in use case.
 *
 */
template<>
inline void sorted_ask(BookHash & book,BookHash::SortedAsks &ordered) {
	for (auto& i : book.askLevels) {
		ordered[i.first]=i.second;
	}
}

/**
 *  @brief  Get a sorted map of bids
 *
 *  We return the sorted map by using the above sort
 *
 *  So not efficient but it needs to be analysed in use case.
 *
 */
inline BookHash::SortedBids & BookHash::get_sorted_bids() {
	sortedBidLevels.clear();
	sorted_bid(*this,sortedBidLevels);
	return sortedBidLevels;
}

/**
 *  @brief  Get a sorted map of asks
 *
 *  We return the sorted map by using the above sort
 *
 *  So not efficient but it needs to be analysed in use case.
 *
 */
inline BookHash::SortedAsks & BookHash::get_sorted_asks() {
	sortedAskLevels.clear();
	sorted_ask(*this,sortedAskLevels);
	return sortedAskLevels;
}


/**
 *  @brief  Get the top bid
 *  @return top bid price
 *
 *  As price levels are not sorted
 *  we need to sort first
 *
 */
PriceLevelKey BookHash::get_top_bid() {
	//auto sorted_bids = sorted_bid(*this);
	std::map<PriceLevelKey,BookHash::Orders,GreaterComp> sorted_bids;
	sorted_bid(*this,sorted_bids);

	auto top_bid = sorted_bids.begin();
	if (top_bid!=sorted_bids.end()) {
		return top_bid->first;
	}
	else {
		return PriceLevelKey();
	}
}

/**
 *  @brief  Get the top ask
 *  @return top bid price
 *
 *  As price levels are not sorted
 *  we need to sort first
 *
 */
PriceLevelKey BookHash::get_top_ask() {
	//auto sorted_asks = sorted_ask(*this);
	std::map<PriceLevelKey,BookHash::Orders,LessComp> sorted_asks;
	sorted_ask(*this,sorted_asks);
	auto top_ask = sorted_asks.begin();
	if (top_ask!=sorted_asks.end()) {
		return top_ask->first;
	}
	else {
		return PriceLevelKey();
	}
}

void BookHash::clear() {
	bidLevels.clear();
	askLevels.clear();
}


#endif
