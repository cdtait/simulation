#ifndef basic_types_h
#define basic_types_h

#include <assert.h>
#include <stdint.h>
#include <cstddef>
#include <iterator>
#include <array>
#include <map>
#include <unordered_map>
#include <vector>
#include <utility>
#include <numeric>
#include <algorithm>

#include "md_helper.h"

// Predefine max levels. The BookMap,BookHash,BookVector are not themselves limited in level
// this limit is only here for copying or print convenience
constexpr uint32_t max_levels=20;
// Quantity as unsigned 32 bit, could be 16 and we get better vectorized performance
typedef uint32_t QuantityValueType;
// Order id as unsigned 32 bit
typedef uint32_t OrderIdKeyType;
// Price as unsigned 32 bit, represented in examples always as integer
// maybe replace with double PriceType below if required
typedef uint32_t PriceLevelKey;
// Price as double.
typedef double PriceType;
// 100 million order ids in day as example for range check
constexpr OrderIdKeyType max_order_id=1e8;
// 500 as example
constexpr QuantityValueType max_order_quantity=500;
// 2000 as example
constexpr PriceLevelKey max_order_price=2000;

/**
 *  @brief The enumerated side type as char Bid or Ask
 *
 */
enum class Side : char {
	Bid='B',
	Ask='S',
	Unknown='U'
};

/**
 *  @brief The enumerated event type as char Add,Modify or Cancel
 *
 */
enum class Event : char {
	Add='A',
	Modify='M',
	Cancel='X',
	Trade='T',
	Mid='M',
	Snapshot='S',
	Unknown='U'
};

/**
 *  @brief The enumerated print type as char text human readable output or csv
 *
 */
enum class PrintType : char {
	Trading='T',
	Betting='B',
	CSV='C',
	None='N',
	Unknown='U'
};


/**
 *  @brief The order properties extracted from event
 *
 */
typedef struct {
	OrderIdKeyType orderid;
	Side side;
	QuantityValueType quantity;
	PriceLevelKey price;
} Order;

typedef std::vector<Order> Orders;

/**
 *  @brief The trade properties extracted from event
 *
 */
typedef struct {
	Side side;
	QuantityValueType quantity;
	PriceLevelKey price;
} Trade;

typedef std::vector<Trade> Trades;

/**
 *  @brief Pending match order
 *
 */
typedef struct {
	PriceLevelKey price;
	OrderIdKeyType orderid;
} PendingMatch;

// Arrays used to represent different types in BookData
template <int N> using IArrray = std::array<int, N>;
template <int N> using DArrray = std::array<double, N>;

/**
   *  @brief The BookData that is snap copied from OrderBook to publish
   *  or further processed
   *
   *  What we have here is a fixed memory chunk that should be efficient to transport
   *
   *  Could pack __attribute__((packed, aligned(8))) but may not add any efficiencies
   *  Could binary encode but again may not yield overall considering decode and network packet header
   *  If it goes on the wire internally infiniband RDMA  will work OK
   *
   *  @tparam  levels as int  defaults to 5
   *  structure we can use and defaults the BookMap.
   */
template <int levels=5>
struct BookData {
	BookData() = default;
	BookData(BookData&& b) = default;

	BookData&
    operator=(BookData&& b) = default;

	// Number of bid orders
	IArrray<levels> bdcontr{};
	// Bid order quantities
	IArrray<levels> bdquantity{};
	// Bid order prices
	DArrray<levels> bdprice{};
	// Ask order prices
	DArrray<levels> sdprice{};
	// Ask order quantities
	IArrray<levels> sdquantity{};
	// Number of ask orders
	IArrray<levels> sdcontr{};
	// Last trade we seen
	Trade last_trade{};
	// Total traded quantity at last traded price
	Trade total_traded_quantity{};
	// Event that cause the publish
	Event event{Event::Unknown};
};

/**
 *  @brief   This is the default sum template used by the map containers
 *  @tparam  Orders This is the BookContainers internal data structure for the orders
 *  @param   orders Orders type used in the sum
 *  @return	 the sum of the order quantities
 *
 *  Sum the quantities in a levels this is an implementation default for map types
 */
template<typename Orders>
inline auto sum(Orders & orders) -> QuantityValueType {
	auto f = [](QuantityValueType total,typename Orders::value_type q){return total+q.second; };
	return std::accumulate(orders.begin(), orders.end(), 0, f);
}

/**
 *  @brief   This is the specialized sum template used by vector of quanties as the BookVector has
 *  @tparam  Orders This is the BookContainers internal data structure for the orders
 *  @param   orders Orders type used in the sum
 *  @return	 the sum of the order quantities
 *
 *  Sum the quantities in a levels this is an implementation default vector
 *  if we cannot do @see vectorization
 */
inline auto sum(std::vector<QuantityValueType> & orders) -> QuantityValueType {
	auto f = [](QuantityValueType total, QuantityValueType q){return total+q; };
	return std::accumulate(orders.begin(), orders.end(), 0, f);
}

// Sorting methods used by Bid and Ask price levels
typedef typename std::less<PriceLevelKey> LessComp;
typedef typename std::greater<PriceLevelKey> GreaterComp;

/**
*  @brief The total traded holds a price with a a vector of quantity values
*/
typedef struct {
	Side side{};
	PriceLevelKey price{};
	std::vector<QuantityValueType> total;
} TotalTraded;

/**
*  @brief Special hash functor for use if we used the side as a key into a unordered_map
*
*  Currently not used
*/
typedef std::hash<Side> SideHashType;
namespace std
{
	template<>
	struct hash<Side>
	{
	  size_t
	  operator()(Side val) const noexcept
	  {
		  return static_cast<size_t>(val);
	  }
	};
}

#endif
