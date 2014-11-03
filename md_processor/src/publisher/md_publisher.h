#ifndef md_publisher_h
#define md_publisher_h

#include "md_types.h"

template <int N=5>
class md_publisher {
public:
	virtual ~md_publisher() {}
	virtual void stop()=0;
	virtual void offer(BookData<N> && book_data)=0;
};

#endif
