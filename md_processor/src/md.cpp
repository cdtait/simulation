
#include <fstream>
#include <iostream>
#include <string>

#include "md_handler.h"
#include "md_adapters.h"

#include <queue>

/** @brief
 *
 * Allows a selection of many combinations of coding data structures for market data processing.
 * The result is the impact factor in the integrity and performance of the code.
 *
 */

/**
 * Convert selected print type to enum
 *
 * @param print_type
 * @return the enum print type
 */
PrintType char2printtype(char print_type) {
	if (print_type == 'T') {
		return PrintType::Trading;
	} else if (print_type == 'C') {
		return PrintType::CSV;
	} else {
		return PrintType::Unknown;
	}
}

/**
 * Run a test with the file base input adapter
 *
 * @param file_name
 * @param md the md handlert
 * @param printType
 * @return
 */
template <typename Adapter,typename MD>
int run_test(MD &md, Adapter &adapter, PrintType printType,const arguments& args)
{
	struct timeval start_time_;
	struct timeval stop_time_;

	md.start(printType,args);
	adapter.wait();
	::gettimeofday(&start_time_, NULL);
	adapter.start(md);
	::gettimeofday(&stop_time_, NULL);
	adapter.stop();
	md.stop();

	double total_micros = ((1e6 * (stop_time_.tv_sec  - start_time_.tv_sec )) + (stop_time_.tv_usec - start_time_.tv_usec));
	std::cerr << "Time to process " << adapter.get_counter() << " messages => " << total_micros << " micros\n";
	std::cerr << "Time for each => " << (total_micros/adapter.get_counter())*1000 << " nanos\n";
	std::cerr << std::flush;
	md.printStats(std::cerr);

  return 0;
}

#include <getopt.h>

void print_usage() {
	std::string message =
{R"(Usage: md_processor -a F -f <file name>|-a [P|ZR|ZP|ZS] [-p T|C] [-d M|H|V] [-x L] [-t A|S|C]  [-s P|D|N ]                    
       -f is name of file to stream the input
         The file name can be relative or absolute
       -p is for the type of print out put you wish to see
         T is a text book and C is a csv format output
       -d selects the underlying book structure to test
         M is a map, H is a hash and V is vector base data structures
       -x select the type of parser model to test
         L is the simple token list parse for csv text or json line formats
       -t select the type of token container use in test
         A is a json_spirit based array, S is a strtk string vector and C is a custom char* vector
         Important - When using json_spirit it will expect a json file where S and C expect csv.
       -a select the adapter to source the market data
         F is a file based input
         Zx uses a zeromq broken into different models aimed at demonstrating messaging
			ZR - Reliable-request-reply
			ZS - Subscriber
			ZP - Pull
         P is a pcap device either file or ethernet alias
       -s select publisher to handle the output we want
         P is print based publisher
         D is the disruptor
         N is none action publisher
)"};

    printf(message.c_str());
}

/**
 *
 * @param file_name
 * @param md
 * @param printType
 * @param adapter
 */
template <typename TokenContainer, typename Parser, typename BookContainer, typename Publisher>
void select_adapter_and_run_test(const std::string& file_name,
		md_handler<Parser,BookContainer,Publisher> &md,
		const std::string& adapter,
		PrintType printType,
		const arguments& args) {
	typedef md_handler<Parser,BookContainer,Publisher> MD;

	if (adapter == "F") {
		file_adapter<TokenContainer,MD> adapter(file_name.c_str());
		run_test(md,adapter,printType,args);
	} else if (adapter == "ZR") {
		md_zmq_rrr_adapter<TokenContainer,MD> adapter;
		run_test(md,adapter,printType,args);
	} else if (adapter == "ZP") {
		md_zmq_pull_adapter<TokenContainer,MD> adapter;
		run_test(md,adapter,printType,args);
	} else if (adapter == "ZS") {
		md_zmq_sub_adapter<TokenContainer,MD> adapter;
		run_test(md,adapter,printType,args);
	} else if (adapter == "P") {
		md_pcap_adapter<TokenContainer,MD> adapter(file_name.c_str());
		run_test(md,adapter,printType,args);
	} else {
		print_usage();
	}
}

/**
 * Select the data structure we want to use for underlying book.
 *
 * @param file_name File/device we use for parsing
 * @param data_struct Type of data structure used for book
 * @param adapter The adapter for the source of data i.e file or pcap etc
 * @param print_type type print format type
 */
template <typename TokenContainer, typename Publisher>
void select_data_structure_and_run_test(const std::string& file_name,
		const std::string & adapter,
		const std::string & data_struct,
		PrintType print_type,
		const arguments& args) {
	if (data_struct == "M") {
		md_handler<list_parser, BookMap, Publisher> md_handler;
		select_adapter_and_run_test<TokenContainer>(file_name, md_handler, adapter, print_type,args);
	} else if (data_struct == "H") {
		md_handler<list_parser, BookHash, Publisher> md_handler;
		select_adapter_and_run_test<TokenContainer>(file_name, md_handler, adapter, print_type,args);
	} else if (data_struct == "V") {
		md_handler<list_parser, BookVector, Publisher> md_handler;
		select_adapter_and_run_test<TokenContainer>(file_name, md_handler, adapter, print_type,args);
	} else {
		print_usage();
	}
}

/**
 * Select the parser model we want, L is the basic simple list.
 * Betfair or other trading exchanges may have much more complex models
 *
 * @param file_name File/device we use for parsing
 * @param adapter The adapter for the source of data i.e file or pcap etc
 * @param data_struct Type of data structure used for book
 * @param parser Type of parser we use depending on data stream model
 * @param print_type type print format type
 */
template <typename TokenContainer,typename Publisher>
void select_parser_and_run_test(const std::string& file_name,
		const std::string adapter,
		const std::string data_struct,
		const std::string parser,
		PrintType print_type,
		const arguments& args) {
	if (parser == "L") {
		select_data_structure_and_run_test<TokenContainer,Publisher>(file_name,adapter,data_struct,print_type,args);
	}
	// Other parser specific to the shape of the data
	else {
		print_usage();
	}
}

/**
 * Select the tokenizer and then select the correct parser
 *
 * @param file_name File/device we use for parsing
 * @param adapter The adapter for the source of data i.e file or pcap etc
 * @param data_struct Type of data structure used for book
 * @param parser Type of parser we use depending on data stream model
 * @param tokenizer the token structure we use for decoding the stream
 * @param print_type type print format type
 */
template <typename Publisher>
void select_token_method_and_run_test(const std::string& file_name,
		const std::string& adapter,
		const std::string&  data_struct,
		const std::string&  parser,
		const std::string&  tokenizer,
		PrintType print_type,
		const arguments& args) {
	// Allows a comparison between different tokenization techniques
	if (tokenizer=="A") {
		// Tokenized simple csv json array
		select_parser_and_run_test<json_token_array,Publisher>(file_name,adapter,data_struct,parser,print_type,args);
	}
	else if(tokenizer=="S") {
		// Tokenized csv std::string vector
		select_parser_and_run_test<string_token_vector,Publisher>(file_name,adapter,data_struct,parser,print_type,args);
	}
	else if(tokenizer=="C") {
		// Tokenized csv char * vector
		select_parser_and_run_test<char_token_vector,Publisher>(file_name,adapter,data_struct,parser,print_type,args);
	}
	else {
		print_usage();
		exit(EXIT_FAILURE);
	}
}

/**
 * Select the correct publisher and then select the correct tokenizer
 *
 * @param file_name File/device we use for parsing
 * @param adapter The adapter for the source of data i.e file or pcap etc
 * @param data_struct Type of data structure used for book
 * @param parser Type of parser we use depending on data stream model
 * @param tokenizer the token structure we use for decoding the stream
 * @param publisher the pusher type
 * @param print_type type print format type
 */
void select_publisher_and_run_test(const std::string& file_name,
		const std::string& adapter,
		const std::string&  data_struct,
		const std::string&  parser,
		const std::string&  tokenizer,
		const std::string& publisher,
		PrintType print_type,
		const arguments& args) {
	// Allows a comparison between different tokenization techniques
	if (publisher=="D") {
		// Tokenized simple csv json array
		select_token_method_and_run_test<disruptor_publisher<>>(file_name,adapter,data_struct,parser,tokenizer,print_type,args);
	}
	else if(publisher=="U") {
		// Tokenized csv char * vector
		select_token_method_and_run_test<udp_publisher<>>(file_name,adapter,data_struct,parser,tokenizer,print_type,args);
	}
	else if(publisher=="P") {
		// Tokenized csv std::string vector
		select_token_method_and_run_test<print_publisher<>>(file_name,adapter,data_struct,parser,tokenizer,print_type,args);
	}
	else if(publisher=="N") {
		// Tokenized csv char * vector
		select_token_method_and_run_test<null_publisher>(file_name,adapter,data_struct,parser,tokenizer,print_type,args);
	}
	else {
		print_usage();
		exit(EXIT_FAILURE);
	}
}

/**
 * Select and run a test with different data structures, tokenizers, adapters and parser model in order
 * to asses performance, regression and process features.
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char **argv) {
	/*
	int option = 0;

	while ((option = getopt(argc, argv, "h?f:p:d:x:t:a:s:")) != -1) {
		switch (option) {
		case '?':
		case 'h':
			print_usage();
			exit(EXIT_FAILURE);
		case 'f':
			file_name = optarg;
			break;
		case 'p':
			print_type = char2printtype(optarg[0]);
			break;
		case 'd':
			data_struct = optarg;
			break;
		case 'x':
			parser = optarg;
			break;
		case 't':
			tokenizer = optarg;
			break;
		case 'a':
			adapter = optarg;
			break;
		case 's':
			publisher = optarg;
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
		}
	}
	*/

    arguments args( argc, argv);

    std::string file_name = args.get_opt<std::string>("f", optional_arg, has_arg);
    PrintType print_type = char2printtype(args.get_opt<char>("p", optional_arg, has_arg, (char)PrintType::Trading));
	std::string data_struct = args.get_opt<std::string>("d", optional_arg, has_arg, "M");
	std::string parser = args.get_opt<std::string>("x", optional_arg, has_arg, "L");
	std::string tokenizer = args.get_opt<std::string>("t", optional_arg, has_arg, "A");
	std::string adapter = args.get_opt<std::string>("a", optional_arg, has_arg, "F");
	std::string publisher = args.get_opt<std::string>("s", optional_arg, has_arg, "P");

	// Check if we need a file name
	if (file_name.empty()) {
		// If we are using zero mq we don't need a filename or device name
		if (adapter[0] != 'Z') {
			printf("No file input\n");
			print_usage();
			exit(EXIT_FAILURE);
		}
	}
	else {
		// If we are using csv then either 'C' character or string 'S' would be valid
		// With json only json array 'A'
		std::string f(file_name);
		if (
			(f.substr(f.length()-4,f.length())==".csv" && tokenizer=="A") ||
			(f.substr(f.length()-5,f.length())==".json" && (tokenizer=="C" || tokenizer=="S"))
		)
		{
			printf("Bad combination of tokenizer and file type\n");
			print_usage();
			exit(EXIT_FAILURE);
		}
	}

	// Is it a vallid print type
	if (print_type==PrintType::Unknown) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	// Start the test by configuring the options then passing it on to run_test
	select_publisher_and_run_test(file_name,adapter,data_struct,parser,tokenizer,publisher,print_type,args);

	return 0;
}
