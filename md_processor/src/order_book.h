#ifndef order_book_h
#define order_book_h

#include <stdint.h>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>
#include <cstring>

#include "md_stats.h"
#include "md_helper.h"

/**
   *  @brief The Order book. This is an entity which allows the basic add,
   *  modify, cancel and trade expected of an order book.
   *
   *  @tparam  Container  Type of underlying book data
   *  structure we can use and defaults the BookMap.
   */
template <typename Container=BookMap>
class OrderBook {
	typedef std::map<PriceLevelKey,typename Container::Orders,GreaterComp> SortedBids;
	typedef std::map<PriceLevelKey,typename Container::Orders,LessComp> SortedAsks;

public:
	/**
	 *  @brief  Add an order.
	 *  @param  order.
	 *
	 *  This function figures out which side and uses the levels on the
	 *  appropriate side to add new order.
	 *
	 *  Monitor the add order to see if it produces trade match
	 *
	 */
	void add(Order && order) {
		// Choose a side
		if (order.side == Side::Bid) {
			// Add bid order
			add_side(order, levels.bidLevels);
			// Monitor this add to see if it may cause a match
			monitor_bid_order_match(order);
		} else if (order.side == Side::Ask) {
			// Add ask order
			add_side(order, levels.askLevels);
			// Monitor this add to see if it may cause a match
			monitor_ask_order_match(order);
		}
	}

	/**
	 *  @brief  Modify an order.
	 *  @param  order Order details for modify
	 *
	 *  This function figures out which side and uses the levels on the
	 *  appropriate side to modify order.
	 *
	 */
	void modify(Order && order) {
		// Choose a side
		if (order.side == Side::Bid) {
			// Modify bid order
			modify_side(order, levels.bidLevels);
		} else if (order.side == Side::Ask) {
			// Modify ask order
			modify_side(order, levels.askLevels);
		}
	}

	/**
	 *  @brief  Cancel an order.
	 *  @param  order Order details for cancel
	 *
	 *  This function figures out which side and uses the levels on the
	 *  appropriate side to cancel order.
	 *
	 */
	void cancel(Order && order) {
		// Choose a side
		if (order.side == Side::Bid) {
			// Cancel bid order
			cancel_side(order, levels.bidLevels);
		}  else if (order.side == Side::Ask) {
			// Cancel ask order
			cancel_side(order, levels.askLevels);
		}
	}

	/**
	 *  @brief  Handle a trade to the book.
	 *  @param  trade Trade details for cancel
	 *
	 *  where we will capture last trade and calculate
	 *  maintain the total quantities ad this as recent price
	 *
	 *  Note: we are no summing at this point as we sum on demand
	 *  of the trade information in the book @see book_trade().
	 *
	 */
	void trade(Trade && trade) {
		if (trade.price != total_traded.price) {
			// Reset the price
			total_traded.total.clear();
			total_traded.price = trade.price;
			total_traded.side = trade.side;
		}

		// Add an addition quantity
		total_traded.total.push_back(trade.quantity);

		// Check to see if this was expected as part of the match
		monitor_trade(trade);
	}

	/**
	 *  @brief  Get the top bid from underlying Container book data structure.
	 *  @return price of top bid
	 *
	 */
	PriceLevelKey get_top_bid() noexcept {
		return levels.get_top_bid();
	}

	/**
	 *  @brief  Get the top ask from underlying Container book data structure.
	 *  @return price of top bid
	 *
	 */
	PriceLevelKey get_top_ask() noexcept {
		return levels.get_top_ask();
	}

	/**
	 *  @brief  Get the mid price from underlying Container book data structure.
	 *  @return mid price as a floating point type
	 *
	 */
	PriceType get_mid() noexcept  {
		return (get_top_ask() + get_top_bid()) / 2.0;
	}

private:
	    /**
	     *  @brief   Add an order for particular side.
	     *  @tparam  PriceLevels Type of underlying price data
	     *  @param   o Order object.
	     *  @param   sideLevels PriceLevels price level for particular side (Bid or Ask)
	     *
	     *  The price level is selected using <levels>[order.price] which can either
	     *  create or retrieve an existing price level
	     *
	     *  Next <price level>[order.orderid] is checked to make sure there is no duplicate
	     *  orderid. If it does we record this as a statistic and throw runtime.
	     *
	     *  The PriceLevel template is part of the underlying Container type template
	     *  parameter and could be Container::BidPriceLevels or Container::AskPriceLevels
	     */
	    template<typename PriceLevels>
	    void add_side(const Order & order,PriceLevels & sideLevels) {
	    	// Order id cannot exist yet
			if (sideLevels[order.price].count(order.orderid)==0) {
				// OK add the order
				sideLevels[order.price][order.orderid] = order.quantity;
			}
			else {
				// Already have this order then if must be duplicate
				stats().dup_orderid();
			}
	    }

	    /**
	     *  @brief   Modify an order for particular side.
	     *  @tparam  PriceLevels Type of underlying price data
	     *  @param   o Order object.
	     *  @param   sideLevels PriceLevels price level for particular side (Bid or Ask)
	     *
	     *  The price level is selected using <levels>[order.price] which must exist
	     *  and will record if missing
	     *
	     *  Next <price level>[order.orderid] is checked to make sure it exists for
	     *  orderid. If it does not we record this as missing and throw runtime.
	     *
		 *  The PriceLevel template is part of the underlying Container type template
	     *  parameter and could be Container::BidPriceLevels or Container::AskPriceLevels
	     */
	    template<typename PriceLevels>
	    void modify_side(const Order & order,PriceLevels & sideLevels) {
	    	// Price must be here for modify
			if (sideLevels.count(order.price)>0) {
				// Order id must exist too
				if (sideLevels[order.price].count(order.orderid)>0) {
					// OK we modify
					sideLevels[order.price][order.orderid] = order.quantity;
				}
				else {
					stats().no_order_for_modify();
				}
			}
			else {
				// Modify must have a price level
				stats().missing_price_level();
			}
	    }

	    /**
	     *  @brief   Cancel an order for particular side.
	     *  @tparam  PriceLevels Type of underlying price data
	     *  @param   o Order object.
	     *  @param   sideLevels PriceLevels price level for particular side (Bid or Ask)
	     *	     *
	     *  Removes the order from the price level.
	     *
	     *  When the orders are all cancelled at particular level then we remove
	     *  the price as it has effectively disappeared.
	     *
	     *  The price level is selected using <levels>[order.price] which must exist
	     *  and will record if missing
	     *
	     *  Next <price level>[order.orderid] is checked to make sure it exists for
	     *  orderid. If it does not we record this as missing and throw runtime.
	     *
		 *  The PriceLevel template is part of the underlying Container type template
	     *  parameter and could be Container::BidPriceLevels or Container::AskPriceLevels
	     */
	    template<typename PriceLevels>
	    void cancel_side(const Order & order,PriceLevels & sideLevels) {
	    	// Price must be here for cancel
			if (sideLevels.count(order.price)>0) {
				// Order id must exist too
				if (sideLevels[order.price].count(order.orderid)>0) {
					// Only erase completely if quantity is fully taken
					// I expect any cancel to be a complete cancel not
					// partial one
					if (sideLevels[order.price][order.orderid] <= order.quantity) {
						// Complete cancel
						(sideLevels[order.price]).erase(order.orderid);
					}
					else{
						// Partial cancel
						sideLevels[order.price][order.orderid] -= order.quantity;
					}
				}
				else {
					// are recored with this single statistic
					stats().no_order_for_modify();
				}

				// If Price level has no orders then it disappears
				if (sideLevels[order.price].empty()) {
					(sideLevels).erase(order.price);
				}
			}
			else {
				// Cancel must have a price level
				stats().missing_price_level();
			}
	    }

	    /**
	     *  @brief   Monitor a bid order match that may occur because of this order
	     *  @param   o Order object.
	     *
	     *
	     *  If we get an order it could be one that causes a match or it could be
	     *  an order that is not expected. i.e we got an order that caused a match
	     *  and we should have got a trade(s) to clear this match order but instead
	     *  we get a order event. So this means we are missing a trade and we record
	     *  this as an error statistic
	     */
	    void monitor_bid_order_match(const Order & order) {
	        if (matchedOrderPendingTrades.price>0) {
	        	// We should'have a new order we are expecting
	        	// a trade so flag this as bad
	            stats().best_ask_le_best_bid();
	            matchedOrderPendingTrades={};
	            throw std::runtime_error("No trade matching order");
	        }
	        // Check if a match is occurring
	        else if (!levels.bidLevels.empty()) {
	        	// Look at the top of the ask to see if we match
	        	auto top_ask = get_top_ask();
	        	if (top_ask > 0 && top_ask <= order.price) {
	            	// Got a match so monitor for coming trade
	                matchedOrderPendingTrades={order.price,order.orderid};
	            }
	        }
	    }

	    /**
	     *  @brief   Monitor a ask order match that may occur because of this order
	     *  @param   o Order object.
	     *
	     *
	     *  If we get an order it could be one that causes a match or it could be
	     *  an order that is not expected. i.e we got an order that caused a match
	     *  and we should have got a trade(s) to clear this match order but instead
	     *  we get a order event. So this means we are missing a trade and we record
	     *  this as an error statistic
	     */
	    void monitor_ask_order_match(const Order & order) {
	        if (matchedOrderPendingTrades.price>0) {
	        	// We should'have a new order we are expecting
	        	// a trade so flag this as bad
	            stats().best_ask_le_best_bid();
	            matchedOrderPendingTrades={};
	            throw std::runtime_error("No trade matching order");
	        }
	        // Check if a match is occurring
	        else if (!levels.askLevels.empty()) {
	        	// Look at the top of the bid to see if we match
	        	auto top_bid = get_top_bid();
	            if ( top_bid > 0 && top_bid >= order.price) {
	            	// Got a match so monitor for coming trade
	                matchedOrderPendingTrades={order.price,order.orderid};
	            }
	        }
	    }

	    /**
	     *  @brief   Monitor a trade
	     *  @param   trade Trade object.
	     *
	     *
	     *  If trade matches what we expect then OK clear the pending
	     *
	     */
	    void monitor_trade(const Trade & t) {
	    	// Did we get a trade which corresponds with pending match
	    	if (matchedOrderPendingTrades.price > 0.0) {
	    		// Order pending a trade
				if (t.side==Side::Ask && matchedOrderPendingTrades.price>=t.price) {
					// All good if we did
					matchedOrderPendingTrades={};
				}
				else if (t.side==Side::Bid && matchedOrderPendingTrades.price<=t.price) {
					// All good if we did
					matchedOrderPendingTrades={};
				}
				else {
					// Trade does not match the order
					stats().no_order_for_trade();
					throw std::runtime_error("No order matching trade");
				}
	    	}
	    }

	private:
	    // Container type
	    // The actual supporting book data structure options, briefly -
	    // 1. BookMap implemented with std::map and is implicitly sorted at runtime
	    //    and requires sequential sum for order quantities
	    // 2. BookHash implemented with std::unorder_map and needs to be sorted when needed
	    //    and requires sequential sum for order quantities
	    // 3. BookVector implemented with std::vector with more implementation to maintain sort
	    //    at runtime also make use vector instructions for sum for order quantities
	    Container levels;
	    // The total traded we maintain when monitoring trade event
	    // The price and vector of all the quantities at this price after reset
	    TotalTraded total_traded{};
	    // Monitor the trade that should happen, this is what is pending
	    // and should happen in next tarde event
	    PendingMatch matchedOrderPendingTrades{};

	public:
	    /**
	     *  @brief   Return a snap copy of the book data. We normally would not use all levels
	     *  		 and generally this a performance critical
	     *  @tparam  n Number of levels for this BookData default to 5
	     *  @param   Optional event if we use this to produce a book on a particular event.
	     *  @return	 BookData of n levels
	     *
	     *  Create a fixed BookData object which can then be published in a thread safe way
	     */
	    template <int n=5>
	    BookData<n>  book_data(Event event=Event::Unknown) {
	    	static_assert(n <= 20,"Order book may not be bigger than 20 levels");

	    	// Sort the bid levels, if they are not sorted already @see sorted_bid
	        SortedBids &bid_prices=levels.get_sorted_bids();
	    	auto bid_it = bid_prices.begin();
	    	auto bid_end = bid_prices.end();

	    	// Sort the ask levels, if they are not sorted already @see sorted_bid
	    	SortedAsks &ask_prices=levels.get_sorted_asks();
	    	auto ask_it = ask_prices.begin();
	    	auto ask_it_end = ask_prices.end();

	        BookData<n> book;
	        book.event=event;
	        // Forward iterate cycle
	        for (int i =0; i < n; i++) {
	        	// Check that we have that bid level otherwise this level is left default {0}
	        	if (bid_it != bid_end) {
	        		typename Container::Orders bidLevel;
	        		// Expand the bid level into price key and quantities
	        		std::tie(book.bdprice[i],bidLevel)=*(bid_it++);
	        		book.bdcontr[i]=bidLevel.size();
	        		book.bdquantity[i]=sum(bidLevel);
	        	}
	        	// Check that we have that ask level otherwise this level is left default {0}
	        	if (ask_it != ask_it_end) {
	        		typename Container::Orders askLevel;
	        		// Expand the bid level into price key and quantities
	        		std::tie(book.sdprice[i],askLevel)=*(ask_it++);
	        		book.sdcontr[i]=askLevel.size();
	        		book.sdquantity[i]=sum(askLevel);
	        	}
	        }

	    	return book;
	    }

	    /**
	     *  @brief   Return a snap copy of the book data. Noting that all we populate is the trade
	     *  		 part as this is all we are interested in
	     *  @tparam  n Number of levels for this BookData default to 5
	     *  @return	 BookData with trade info populated
	     *
	     *  Create a fixed BookData populated with latest trade information
	     */
	    template <int n=5>
	    BookData<n>  trade_data() {
	    	static_assert(n <= 20,"Order book may not be bigger than 20 levels");

	        BookData<n> book=book_data();
	        book.event=Event::Trade;
	        // The last trade and its traded quantity
	        book.last_trade={total_traded.side,total_traded.price,total_traded.total.back()};
	        // Sum of all quantities at this price
	        book.total_traded_quantity={total_traded.side,total_traded.price,sum(total_traded.total)};

	    	return book;
	    }

	    /**
	     *  @brief   Return a snap copy of the mid data top bid/ask.
	     *
	     *  @tparam  n Number of levels for this BookData default to 5
	     *  @param   Optional event if we use this to produce top prices on a particular event.
	     *  @return	 BookData of n levels
	     *
	     *  Create a fixed BookData object which can then be published in a thread safe way
	     */
	    template <int n=5>
	    BookData<n> mid_data(Event event=Event::Unknown) {
	    	static_assert(n <= 20,"Order book may not be bigger than 20 levels");

	        BookData<n> book;
	        book.event=Event::Mid;
			book.bdprice[0]=levels.get_top_bid();
			book.sdprice[0]=levels.get_top_ask();

	    	return book;
	    }
};

#endif
