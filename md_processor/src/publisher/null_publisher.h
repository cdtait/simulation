#ifndef null_publisher_h
#define null_publisher_h

#include "md_publisher.h"

/**
 * @brief publisher which does nothing, useful for raw timing
 */
class null_publisher : public md_publisher<> {
public:
	null_publisher(PrintType printType) {}
	~null_publisher() {}
	void stop() {}
	void offer(BookData<> && book_data){}
};

#endif
