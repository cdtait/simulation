
#ifndef memorymapped_file_adapter_h
#define memorymapped_file_adapter_h

#include <fcntl.h>
#include "mio/mio.hpp"
#include "md_adapter.h"

/**
  *  @brief Implementation of the file_adapter
  *  Simply loops round a csv or json or text file and sends the
  *  message to the processor.
  *
  */
template <
	typename TokenContainer,
	typename MD
>
class memorymapped_file_adapter : public md_adapter {
private:
	using mm = mio::mmap_source;
	bool _mmgetline(mm::const_iterator & cit, mm::const_iterator & eit, std::string & line) {
		mm::const_iterator lit = cit;
		while (*lit++ != '\n' and lit != eit);
		line.assign(cit,lit-1);
		if (lit != eit) {
			cit = lit;
			return true;
		}
		return false;
	}
public:

	memorymapped_file_adapter(const char* fn) {
		std::error_code error;
		infile.map(fn, error);

		if (error) {
			const auto& errmsg = error.message();
			std::ostringstream oss;
			oss << "error mapping file: " << errmsg.c_str() << "exiting...\n";
			throw std::runtime_error(oss.str());
		}
	}

	/**
	 * No need to know the number of messages
	 */
	void wait() {
	}

	/**
	 * Start the adapter processing.
	 *
	 * @param md The market data handler we will send the data to
	 */
    void start(MD & md) {
    	stopped=false;

    	std::string line;
    	auto it = infile.begin();
    	auto eit = infile.end();

    	while(not stopped && _mmgetline(it, eit, line)) {
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
    mio::mmap_source  infile;
};

#endif
