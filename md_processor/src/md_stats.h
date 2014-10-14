#ifndef order_book_stats_h
#define order_book_stats_h

#include <stdint.h>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "md_types.h"

/**
 * @brief This is the statistics object which maintains a count of any
 * errors that are propagated in the md
 */
class OrderBookStats {
public:
	OrderBookStats() = default;

	void event_error() {
		++event_error_cnt;
	}

	void corrupt_error() {
		++corrupt_cnt;
	}
	void dup_orderid() {
		++dup_orderid_cnt;
	}
	void no_order_for_trade() {
		++no_order_for_trade_cnt;
	}
	void order_range() {
		++order_range_cnt;
	}
	void order_parse() {
		++order_parse_cnt;
	}
	void side_error() {
		++side_error_cnt;
	}
	void quantity_range()  {
		++quantity_range_cnt;
	}
	void quantity_parse() {
		++quantity_parse_cnt;
	}
	void price_range()  {
		++price_range_cnt;
	}
	void price_parse() {
		++price_parse_cnt;
	}
	void no_order_for_modify() {
		++no_order_for_mod_cnt;
	}
	void missing_price_level() {
		++missing_price_level_cnt;
	}
	void best_ask_le_best_bid() {
		++best_ask_le_best_bid_cnt;
	}

	/**
	 * Print out the stat onto the stream
	 * @param ostr
	 */
	void print(std::ostream &ostr) {

		int buff_len = 300;
		char buffer[buff_len];
		int cx;
		cx = snprintf(buffer, buff_len,
				"Error summary\nCorrupt event:%3d\nDuplicate order id:%3d\n", corrupt_cnt+event_error_cnt,
				dup_orderid_cnt);
		cx += snprintf(buffer + cx, buff_len - cx,
				"No order matching trade:%3d\nNo order id with cancel or modify:%3d\n",
				best_ask_le_best_bid_cnt, no_order_for_mod_cnt);
		cx += snprintf(buffer + cx, buff_len - cx,
				"No matching trade with order:%3d\nOrder range:%3d\nOrder syntax:%3d\n",
				no_order_for_trade_cnt, order_range_cnt,order_parse_cnt);
		cx += snprintf(buffer + cx, buff_len - cx,
				"Side error:%3d\n",
				side_error_cnt);
		cx += snprintf(buffer + cx, buff_len - cx,
				"Quantity range:%3d\nQuantity syntax:%3d\n",
				quantity_range_cnt,quantity_parse_cnt);
		cx += snprintf(buffer + cx, buff_len - cx,
				"Price range:%3d\nPrice syntax:%3d\n",
				price_range_cnt,price_parse_cnt);

		ostr << buffer;
	}

private:
	uint32_t corrupt_cnt = 0;
	uint32_t event_error_cnt = 0;
	uint32_t dup_orderid_cnt = 0;
	uint32_t no_order_for_trade_cnt = 0;
	uint32_t no_order_for_mod_cnt = 0;
	uint32_t best_ask_le_best_bid_cnt = 0;
	uint32_t order_range_cnt = 0;
	uint32_t order_parse_cnt = 0;
	uint32_t side_error_cnt = 0;
	uint32_t quantity_range_cnt = 0;
	uint32_t quantity_parse_cnt = 0;
	uint32_t price_range_cnt = 0;
	uint32_t price_parse_cnt = 0;
	uint32_t missing_price_level_cnt = 0;
};

/**
 * Singleton to the statistics for the book
 * c++11 makes this safe to initialise
 * @return the book stats
 */
static OrderBookStats& stats(){
  static OrderBookStats instance;
  return instance;
}

#endif
