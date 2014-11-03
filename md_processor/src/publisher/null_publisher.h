#ifndef null_publisher_h
#define null_publisher_h

#include "md_publisher.h"

class null_publisher : public md_publisher<> {
public:
	null_publisher(PrintType printType) {}
	~null_publisher() {}
	void stop() {}
	void offer(BookData<> && book_data){}
};

#endif
