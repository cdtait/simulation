
#ifndef file_adapter_h
#define file_adapter_h

#include "md_adapter.h"

/**
  *  @brief Implementation of the md_adapter
  *
  */
template <
	typename TokenContainer,
	typename MD
>
class file_adapter : public md_adapter {
public:

	file_adapter(const char* fn) :
		infile(fn, std::ios::in) {
	}

	void wait() {
	}

    void start(MD & md) {
    	stopped=false;

    	std::string line;

    	while(!stopped && std::getline(infile, line)) {
    		try
    		{
    			md.process_message(get_tokens<TokenContainer>(line));
    		}
    		catch(std::exception const& e)
    		{
    		    std::cerr << "Caught exception: " << e.what() << " for event(" << line << ") line number:" << counter << "\n";
    		}
    		counter++;
    	}
    }

private:
    std::ifstream infile;
};

#endif
