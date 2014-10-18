
#include <fstream>
#include <iostream>
#include <string>

#include "md_handler.h"

#include <queue>

template <typename Parser, typename Container>
int test(const char * file_name,	md_handler<Parser,Container> &md, PrintType printType)
{
	const std::string filename(file_name);
	std::ifstream infile(filename.c_str(), std::ios::in);
	int counter=0;
	std::string line;

	md.start(printType);

	struct timeval start_time_;
	struct timeval stop_time_;
	::gettimeofday(&start_time_, NULL);
	while(std::getline(infile, line)) {
		try
		{
			// TODO make option here to allow any kind of type
			// from betfair or any trade format
			json_spirit::Value value;
			json_spirit::read_string(line, value);
			assert(value.type()==json_spirit::array_type);
			json_spirit::Array& trade_event_array = value.get_array();
			json_spirit::Array::iterator begin = trade_event_array.begin();
			json_spirit::Array::iterator end = trade_event_array.end();

			md.process_message(begin,end);
		}
		catch(std::exception const& e)
		{
		    std::cerr << "Caught exception: " << e.what() << " for event(" << line << ") line number:" << counter << "\n";
		}
		counter++;
	}

	md.stop();
	::gettimeofday(&stop_time_, NULL);
	double total_micros = ((1e6 * (stop_time_.tv_sec  - start_time_.tv_sec )) + (stop_time_.tv_usec - start_time_.tv_usec));
	std::cerr << "Time to process " << counter << " messages => " << total_micros << " micros" << std::endl;
	std::cerr << "Time for each => " << (total_micros/counter)*1000 << " nanos" << std::endl;

	md.printStats(std::cerr);

  return 0;
}

#include <getopt.h>

void print_usage() {
    printf("Usage: fead_handler [-p T|P] [-d M|H|V]\n");
}

int main(int argc, char **argv) {
	int option = 0;
	char print_type = 'T';
	char data_struct = 'M';
	char * file_name=nullptr;

	while ((option = getopt(argc, argv, "f:p:d:")) != -1) {
		switch (option) {
		std::cout << option << std::endl;
		case 'f':
			file_name = optarg;
			break;
		case 'p':
			print_type = optarg[0];
			break;
		case 'd':
			data_struct = optarg[0];
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
		}
	}

	if (!file_name) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	if (print_type == 'T') {
		if (data_struct == 'M') {
			md_handler<json_parser, BookMap> map_md;
			test(file_name, map_md, PrintType::Trading);
		} else if (data_struct == 'H') {
			md_handler<json_parser, BookHash> map_md;
			test(file_name, map_md, PrintType::Trading);
		} else if (data_struct == 'V') {
			md_handler<json_parser, BookVector> map_md;
			test(file_name, map_md, PrintType::Trading);
		}
	} else if (print_type == 'C') {
		if (data_struct == 'M') {
			md_handler<json_parser, BookMap> map_md;
			test(file_name, map_md, PrintType::CSV);
		} else if (data_struct == 'H') {
			md_handler<json_parser, BookHash> map_md;
			test(file_name, map_md, PrintType::CSV);
		} else if (data_struct == 'V') {
			md_handler<json_parser, BookVector> map_md;
			test(file_name, map_md, PrintType::CSV);
		}
	} else {
		print_usage();
	}

	return 0;
}
