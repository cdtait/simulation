
#ifndef md_adapters_h
#define md_adapters_h

#include <string>
#include <tuple>
#include <utility>


#include "md_types.h"
#include "md_stats.h"
#include "md_helper.h"

#include "token_containers.h"

/**
  *  @brief Implementation of the md_adapter
  *
  */
class md_file_adapter {
public:

	md_file_adapter(const char* fn) : file_name(fn) {
	}

    void stop() {
    	stopped=true;
    }

    int get_counter() {
    	return counter;
    }

    template <typename TokenContainer, typename Parser, typename BookContainer>
    void start(md_handler<Parser,BookContainer> & md) {
    	stopped=false;
    	const std::string filename(file_name);
    	std::ifstream infile(filename.c_str(), std::ios::in);
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
    bool stopped=true;
    int counter=1;
    const char* file_name;
};

#endif
