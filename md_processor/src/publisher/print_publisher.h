#ifndef print_publisher_h
#define print_publisher_h

#include "md_publisher.h"
#include "md_types.h"

/**
 * @brief .
 *
 *
 */
template<int N=5>
class print_publisher : public md_publisher<N> {
public:
	print_publisher(PrintType printType)
	{
		if (printType==PrintType::Trading) {
			print_trade_f=print_trade_txt<N>;
		    print_book_f=print_book_txt<N>;
		    print_mid_f=print_mid_txt<N>;
		}
		else if (printType==PrintType::Betting) {
			print_trade_f=print_trade_betting<N>;
			print_book_f=print_book_betting<N>;
			print_mid_f=print_mid_betting<N>;
		}
		else if (printType==PrintType::CSV) {
			print_trade_f=print_trade_csv<N>;
			print_book_f=print_book_csv<N>;
			print_mid_f=print_mid_csv<N>;
		}
	}

	void stop() {
	}

	void offer(BookData<N> && book_data) {
		if (book_data.event==Event::Trade) {
			print_trade_f(std::cout,std::forward<BookData<N>>(book_data));
		}
		else if (book_data.event==Event::Mid) {
			print_mid_f(std::cout,std::forward<BookData<N>>(book_data));
		}
		else {
			print_book_f(std::cout,std::forward<BookData<N>>(book_data));
		}
	}

private:
private:
   // Trade event printing function
	typename PrintFunctionType<N>::Funcp print_trade_f;
	// Book event printing function
	typename PrintFunctionType<N>::Funcp print_book_f;
	// Mid price printing function
	typename PrintFunctionType<N>::Funcp print_mid_f;
};

#endif
