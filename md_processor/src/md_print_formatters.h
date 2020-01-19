#ifndef print_types_h
#define print_types_h

#include "md_basic_types.h"

// TODO Split this into separate PrintFormatters

static constexpr const int print_levels=5;

// Trading presentation
static constexpr const char*  readable_trading_header=
{R"(|-----------------------------------------|
|         Bid        |         Ask        |
|-----------------------------------------|
|Level| Num | Qty |Price|Price| Qty | Num |
|-----------------------------------------|
)"};
static constexpr const char* readable_level_format={"|%5d|%5d|%5d|%5.0f|%5.0f|%5d|%5d|\n"};
static constexpr const unsigned int readable_level_format_len=44;
static constexpr const char* readable_trade_format={"Total traded:%d@%d Last trade:%c %d@%d\n"};
static constexpr const unsigned int readable_trade_format_len=52;
static constexpr const char* readable_trailer={"|-----------------------------------------|\n"};

static constexpr const char* readable_trading_mid_format={"Bid:%.2f Ask:%.2f Mid:%.2f\n"};
static constexpr const unsigned int readable_mid_format_len=50;

static constexpr const char* non_header={""};
static constexpr const char* non_trailer={""};

// Betting presentation
static constexpr const char*  readable_betting_header=
{R"(|-----------------------------------------|
|         Back          |       Lay       |
|-----------------------------------------|
|Level| Num | Qty |Price|Price| Qty | Num |
|-----------------------------------------|
)"};

static constexpr const char* readable_betting_mid_format={"Back:%.2f Lay:%.2f Mid:%.2f\n"};
static constexpr const unsigned int readable_betting_mid_format_len=51;

// CSV presentation
static constexpr const char* csv_level_format={"%d,%d,%.2f,%.2f,%d,%d,"};
static constexpr const unsigned int csv_level_format_len=60;

static constexpr const char* csv_trade_format={"S,%c,%d,%d,"};
static constexpr const unsigned int csv_trade_len=22;

static constexpr const char* csv_mid_format={"%.2f,%.2f,%.2f\n"};
static constexpr const unsigned int csv_mid_format_len=40;

/**
 * Prints the book in readable text format
 * @param ostr
 * @param book
 */
template <int n=5>
void print_book_base(std::ostream & ostr,BookData<n> && book) {
	int buff_len = (readable_level_format_len*n)+1;
	char buffer[buff_len+1];
	int cx =0;
	for (int i=0; i < n; i++) {
		cx += ::snprintf(buffer+cx,buff_len-cx,readable_level_format,
				i,book.bdcontr[i],book.bdquantity[i],
				book.bdprice[i],book.sdprice[i],
				book.sdquantity[i],book.sdcontr[i]);
	}
	ostr << buffer;
}

/**
 * Prints the book in readable text format
 * @param ostr
 * @param book
 */
template <int n=5>
void print_book_txt(std::ostream & ostr,BookData<n> && book) {
	ostr << readable_trading_header;
	print_book_base(ostr,std::forward<BookData<n>>(book));
	ostr << readable_trailer;
}

/**
 * Prints the book in readable text format
 * @param ostr
 * @param book
 */
template <int n=5>
void print_book_betting(std::ostream & ostr,BookData<n> && book) {
	ostr << readable_betting_header;
	print_book_base(ostr,std::forward<BookData<n>>(book));
	ostr << readable_trailer;
}


/**
 * Prints the book in csv format
 *
 * @param ostr
 * @param book
 */
template <int n=5>
void print_book_csv_base(std::ostream & ostr,BookData<n> && book) {
	int buff_len = (csv_level_format_len*n)+1;
	char buffer[buff_len];
	int cx =0;
	for (int i=0; i < n; i++) {
		cx += ::snprintf(buffer+cx,buff_len-cx,csv_level_format,
				book.bdcontr[i],book.bdquantity[i],
				book.bdprice[i],book.sdprice[i],
				book.sdquantity[i],book.sdcontr[i]);
	}
	ostr << buffer << "\n";
}

/**
 * Prints the book in csv format including trade defaults
 *
 * @param ostr
 * @param book
 */
template <int n=5>
void print_book_csv(std::ostream & ostr,BookData<n> && book) {
	ostr << "S,U,0,0,";
	print_book_csv_base(ostr,std::forward<BookData<n>>(book));
}



/**
 * Prints the trade in readable format
 * @param ostr
 * @param book
 */
template <int n=5>
void print_trade_txt(std::ostream & ostr,BookData<n> && book) {
	int buff_len = readable_trade_format_len+1;
	char buffer[buff_len];

	::snprintf(buffer,buff_len,readable_trade_format,
			book.total_traded_quantity.quantity,book.total_traded_quantity.price,
			(char)book.last_trade.side,book.last_trade.quantity,book.last_trade.price);

	ostr << buffer;
}

/**
 * Prints the trade in readable format
 * @param ostr
 * @param book
 */
template <int n=5>
void print_trade_betting(std::ostream & ostr,BookData<n> && book) {
	print_trade_txt(ostr,std::forward<BookData<n>>(book));
}


/**
 * Prints the trade in csv format
 *
 * @param ostr
 * @param book
 */
template <int n=5>
void print_trade_csv(std::ostream & ostr,BookData<n> && book) {
	int buff_len = csv_trade_len+1;
	char buffer[buff_len];

	::snprintf(buffer,buff_len,csv_trade_format,
			(char)book.last_trade.side,book.last_trade.quantity,book.last_trade.price);

	ostr << buffer;
	print_book_csv_base(ostr,std::forward<BookData<n>>(book));
}

/**
 * Print the mid in readable text format
 *
 * @param ostr
 * @param book
 */
template <int n=5>
void print_mid_txt(std::ostream & ostr,BookData<n> && book) {
	int buff_len = readable_mid_format_len+1;
	char buffer[buff_len];

	PriceType mid=0.0;
	if (book.bdprice[0]>0.0 && book.sdprice[0]>0.0) {
		mid=(book.bdprice[0]+book.sdprice[0])/2.0;
	}
	::snprintf(buffer,buff_len,readable_trading_mid_format,
			book.bdprice[0],book.sdprice[0],mid);

	ostr << buffer;
}

/**
 * Print the mid in readable text format
 *
 * @param ostr
 * @param book
 */
template <int n=5>
void print_mid_betting(std::ostream & ostr,BookData<n> && book) {
	int buff_len = readable_betting_mid_format_len+1;
	char buffer[buff_len];

	PriceType mid=0.0;
	if (book.bdprice[0]>0.0 && book.sdprice[0]>0.0) {
		mid=(book.bdprice[0]+book.sdprice[0])/2.0;
	}
	::snprintf(buffer,buff_len,readable_betting_mid_format,
			book.bdprice[0],book.sdprice[0],mid);

	ostr << buffer;
}

/**
 * Do nothing, mid is derived in csv
 *
 * @param ostr
 * @param book
 */
template <int n=5>
void print_mid_csv(std::ostream & ostr,BookData<n> && book) {
}

/**
 * Print function object which holds the signature to the above print functions
 * This can be passed around to select the correct printing method
 */
template<int n> struct PrintFunctionType {
	typedef std::function<void(std::ostream & ostr,BookData<n> && book)> Funcp;
};

#endif
