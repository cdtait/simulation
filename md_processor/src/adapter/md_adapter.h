
#ifndef md_adapter_h
#define md_adapter_h

#include <string>
#include <tuple>
#include <utility>


#include "md_types.h"
#include "md_stats.h"
#include "md_helper.h"

/**
 * Base adapter. Provides ability to set stop, get and inc the counter
 */
struct md_adapter {
    void stop(){
    	stopped=true;
    }
    int get_counter(){
    	return counter;
    }
    void inc_counter(){
    	++counter;
    }
    void set_counter(int c){
    	counter=c;
    }
    bool stopped=true;
    int counter=0;
    int32_t numMessages=0;
};

#endif
