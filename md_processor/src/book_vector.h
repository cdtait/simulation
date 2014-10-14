#ifndef book_vector_h
#define book_vector_h

#include "basic_types.h"

#include <functional>
#include "vectorclass.h"
#include "vectormath_exp.h"

/**
   *  @brief Implementation of the support structure to an order book.
   *
   *  Using std::vector we can keep price levels indexed by another vector
   *  which holds the prices an the index of this vector corresponds
   *  exactly to the index of the orders price. the vector of orders are
   *  indexed again bid another vector of order ids an the index of these
   *  correspond exactly to the index of the quantity of the orderid.
   *
   *  std::pair is also possible but wanted to be different from maps
   *  as much as possible.
   *
   *  std::vector<std::pair<OrderIdKeyType,QuantityValueType>> order_index;
   *
   *  Also we could use the underlying vector as a basis to make_heap
   *  in the PriceLevels struct an either maintain a sort or just sort
   *  on demand certainly could maintain head of price levels this
   *  would require the idea of std::pair to be in place also.
   *
   *  Advantages is that it is sorted and holds all the data in a contiguous
   *  memory chunk. This mean we can take advantage of vector instructions
   *  to do sum on columns much easier than that of the maps
   *
   * It requires more code around to implement the methods of the maps
   * which are need by the order book such as the [] operator look up and
   * maintenance of the vector
   */
struct BookVector {

	/**
	   *  @brief Represent the orders that can be added at a price level
	   *
	   *  Uses 2 vectors. One to hold the key of order ids and the other
	   *  the quantity. The position of the key is the same as the position
	   *  of the quantity.
	   *
	   *  Note std::pair could have been used to do the mapping too
	   *  i.e std::vector<std::pair<OrderIdKeyType,QuantityValueType>> order_index;
	   *  But this is just an experiment
	   *
	   */
	typedef struct {
		// Order quantities mapped by following order id key vector
		std::vector<QuantityValueType> orders;
		// Order id keys
		std::vector<OrderIdKeyType> order_index;

		/**
		 *  @brief  Enquire to see if any orders exist
		 *
		 *  This looks at the orders_index to see if empty
		 *
		 */
		auto empty() -> decltype(order_index.empty()) const {
			return order_index.empty();
		}

		/**
		 *  @brief  Enquire to see how many orders
		 *
		 *  This looks at the orders_index get the size
		 *
		 */
		auto size() -> decltype(order_index.size()) const {
			return order_index.size();
		}

		/**
		 *  @brief  Find how many of a particular order id exist
		 *  @param  order id the order is we are looking for
		 *
		 *  Looks at the orders_index get the count
		 *
		 */
		auto count(const OrderIdKeyType& orderid) -> decltype(std::count(std::begin(order_index), std::end(order_index), orderid)) {
			return std::count(order_index.begin(), order_index.end(), orderid);
		}

		/**
		 *  @brief  Remove an order
		 *  @param  order id the order is we want to remove
		 *
		 *  Removes the order id and corresponding quantity
		 *
		 */
		void erase(const OrderIdKeyType& orderid) {
			auto iter_indxpos = std::find(std::begin(order_index), std::end(order_index), orderid);

			if (iter_indxpos != std::end(order_index)) {
				orders.erase(std::begin(orders)+(iter_indxpos-std::begin(order_index)));
				order_index.erase(iter_indxpos);
				assert(order_index.size()==orders.size());
			}
		}

		/**
		 *  @brief  operator to get or create the order id
		 *  		and quantity element in the vector
		 *  @param  order id the order access
		 *  @return The modifiable reference to the
		 *          quantity for the order id
		 *
		 */
		QuantityValueType&
	    operator[](const OrderIdKeyType& orderid)
	    {
			auto iter_pos = std::find(std::begin(order_index), std::end(order_index), orderid);
			if (iter_pos != std::end(order_index)) {
				return orders[iter_pos-std::begin(order_index)];
			}
			else {
				order_index.push_back(orderid);
				orders.emplace_back(QuantityValueType{});
				assert(order_index.size()==orders.size());
				return orders.back();
			}
	    }

		/**
		 *  @brief  operator to get the order quantity
		 *  		element in the vector
		 *  @param  orderid the order access key
		 *  @return The read only reference to the quantity
		 *
		 */
		const QuantityValueType&
	    operator[](const OrderIdKeyType& orderid) const
	    {
			auto iter_pos = std::find(std::begin(order_index), std::end(order_index), orderid);
			if (iter_pos != std::end(order_index)) {
				return orders[iter_pos-std::begin(order_index)];
			}
			else {
				throw std::out_of_range("Order id:"+orderid);
			}
	    }
	} Orders;

	/**
	   *  @brief Represent the price level of the book
	   *
	   *  Uses 2 vectors. One to hold the key of price and the other
	   *  the orders of Orders type as discussed above.
	   *  The position of the key is the same as the position of the orders.
	   *
	   *  Note std::pair could have been used to do the mapping too
	   *  i.e std::vector<std::pair<PriceLevelKey,Orders>> level_index;
	   *  Also we could use heap functions to help sort get min/max element
	   *  in constant time
	   *
	   *  But this is just an experiment and we could arrive back at std::map
	   *  if we did!
	   *
	   */
	typedef struct {
		// Orders mapped by following price key vector
		std::vector<Orders> levels;
		std::vector<PriceLevelKey> level_index;

		/**
		 *  @brief  Enquire to see if any price levels exist
		 *
		 *  This looks at the level_index to see if empty
		 *
		 */
		auto empty() -> decltype(level_index.empty()) const {
			return level_index.empty();
		}

		/**
		 *  @brief  Enquire to see how many price levels
		 *
		 *  This looks at the level_index get the size
		 *
		 */
		auto size() -> decltype(level_index.size()) const {
			return level_index.size();
		}

		/**
		 *  @brief  Find how many of a particular price exist (should be 1)
		 *  @param  order id the order is we are looking for
		 *
		 *  Looks at the level_index get the count
		 *
		 */
		auto count(const PriceLevelKey& price) -> decltype(std::count(std::begin(level_index), std::end(level_index), price)) {
			return std::count(std::begin(level_index), std::end(level_index), price);
		}

		/**
		 *  @brief  Remove a price level
		 *  @param  price the price level is we want to remove
		 *
		 *  Removes the price key and corresponding orders
		 *
		 */
		void erase(const PriceLevelKey& price) {
			auto iter_indxpos = std::find(std::begin(level_index), std::end(level_index), price);

			if (iter_indxpos != std::end(level_index)) {
				levels.erase(std::begin(levels)+(iter_indxpos-std::begin(level_index)));
				level_index.erase(iter_indxpos);
				// !!! Warning no transaction !!!
				assert(level_index.size()==levels.size());
			}
		}

		/**
		 *  @brief  operator to get or create the price key
		 *  		and Orders element in the vector
		 *  @param  price key for the orders to get or create
		 *  @return The modifiable reference to the
		 *          orders for the price
		 *
		 */
		Orders&
	    operator[](const PriceLevelKey& price)
	    {
			auto iter_pos = std::find(std::begin(level_index), std::end(level_index), price);
			if (iter_pos != std::end(level_index)) {
				return levels[iter_pos-std::begin(level_index)];
			}
			else {
				level_index.push_back(price);
				levels.emplace_back(Orders());
				// !!! Warning no transaction !!!
				assert(level_index.size()==levels.size());
				return levels.back();
			}
	    }

		/**
		 *  @brief  operator to get the orders
		 *  		element in the vector
		 *  @param  price key for orders
		 *  @return The read only reference to the orders
		 *
		 */
		const Orders&
	    operator[](const PriceLevelKey& price) const
	    {
			auto iter_pos = std::find(std::begin(level_index), std::end(level_index), price);
			if (iter_pos != std::end(level_index)) {
				return levels[iter_pos-std::begin(level_index)];
			}
			else {
				throw std::out_of_range("Price:"+price);
			}
	    }

	} PriceLevels;

	/**
	 *  @brief  Get the top bid
	 *  @return top bid price
	 *
	 *  get the top by using a max search
	 *
	 */
	PriceLevelKey get_top_bid() {
		auto iter_pos = std::max_element(bidLevels.level_index.begin(),
						bidLevels.level_index.end());
		if (iter_pos != std::end(bidLevels.level_index)) {
			return *iter_pos;
		}
		else {
			return PriceLevelKey{};
		}
	}

	/**
	 *  @brief  Get the top bid
	 *  @return top ask price
	 *
	 *  get the top by using a min search
	 *
	 */
	PriceLevelKey get_top_ask() {
		auto iter_pos = std::min_element(askLevels.level_index.begin(),
				askLevels.level_index.end());
		if (iter_pos != std::end(askLevels.level_index)) {
			return *iter_pos;
		}
		else {
			return PriceLevelKey{};
		}
	}

	typedef PriceLevels AskPriceLevels;
	typedef PriceLevels BidPriceLevels;

	typedef std::map<PriceLevelKey,BookVector::Orders,GreaterComp> SortedBids;
	typedef std::map<PriceLevelKey,BookVector::Orders,LessComp> SortedAsks;

	SortedBids & get_sorted_bids();
	SortedAsks & get_sorted_asks();

	// Bid levels as a vector of custom PriceLevel types
    BidPriceLevels bidLevels;
    // Ask levels as a vector of custom PriceLevel types
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
 *  This is the specialized version of the sorting for BookVector explicitly.
 *
 *  We return the sorted map by simply adding the values out of the BookVector
 *
 *  So not efficient but it needs to be analysed in use case.
 *
 */
template<>
inline void sorted_bid(BookVector & book,BookVector::SortedBids & ordered) {
	for (decltype(book.bidLevels.levels.size()) i=0; i< book.bidLevels.levels.size(); i++) {
		ordered[book.bidLevels.level_index[i]]=book.bidLevels.levels[i];
	}
}


/**
 *  @brief  Sort ask levels
 *  @param  book Container of with price levels
 *
 *  This is the specialized version of the sorting for BookVector explicitly.
 *
 *  We return the sorted map by simply adding the values out of the BookVector
 *
 *  So not efficient but it needs to be analysed in use case.
 *
 */
template<>
inline void sorted_ask(BookVector & book,BookVector::SortedAsks & ordered) {
	for (decltype(book.askLevels.levels.size()) i=0; i< book.askLevels.levels.size(); i++) {
		ordered[book.askLevels.level_index[i]]=book.askLevels.levels[i];
	}
}

inline BookVector::SortedBids & BookVector::get_sorted_bids() {
	sortedBidLevels.clear();
	sorted_bid(*this,sortedBidLevels);
	return sortedBidLevels;
}

inline BookVector::SortedAsks & BookVector::get_sorted_asks() {
	sortedAskLevels.clear();
	sorted_ask(*this,sortedAskLevels);
	return sortedAskLevels;
}
/**
 *
 * The following code is use to vectorize the sums of quantities and adds further to the experiment
 * It may not work on all machine but I would assume that the any modern architecture is used
 *
 */

// AVX2 instruction capability assumed
template <typename>
struct VecType {
	enum{ size = 4  };
	typedef Vec4uq value_type;
};

// When QuantityValueType is 32 bits (default) we use can Vec8ui vectorsize = 8
template<> struct VecType<uint32_t>
{
	enum{ size = 8 };
	typedef Vec8ui	value_type;
};

// but if we can reduce to 16 bits then Vec16us vectorsize = 16 is possible
template<> struct VecType<uint16_t>
{
	enum{ size = 16 };
	typedef Vec16us	value_type;
};

// TODO Also floats and doubles if needed.

/**
 *  @brief  Vectorize the sum od quantities
 *  @param  vector of quantities
 *
 *  Using VCL as included in vectorclass sub dir
 *
 */
QuantityValueType vector_sum(std::vector<QuantityValueType> vec) {
	const int datasize = vec.size();
	decltype(vec.data()) data = vec.data();

	typedef VecType<QuantityValueType>  VecT;
	const int regularpart = datasize & (-VecT::size);

	// Vector load and sum
	VecT::value_type sum1(0), temp;

	int i;
	for (i = 0; i < regularpart; i += VecT::size) {
		temp.load(data+i);
		sum1 += temp;
	}

	QuantityValueType sum = horizontal_add(sum1);

	// Sum remaining element with do not fit a vector add
	// i.e if 13 elements and VecT::size is 8 then we
	// add the remaining 5 here
	for (; i < datasize; i++) {
		sum += vec.data()[i];
	}

	return sum;
}

/**
 *  @brief  Specialized sum calling the vector_sum function
 *  @param  vector of orders for a price level
 *  @return the sum of quantities
 *
 */
template<>
auto sum(BookVector::Orders & orders) -> QuantityValueType {
	return vector_sum(orders.orders);
}

#endif
