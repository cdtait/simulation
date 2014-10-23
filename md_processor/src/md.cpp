
#include <fstream>
#include <iostream>
#include <string>

#include "md_handler.h"
#include "md_file_adapter.h"

#include <queue>

/** @brief
 *
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
 * Run a file based test with the file base input adapter
 *
 * @param file_name
 * @param md the md handlert
 * @param printType
 * @return
 */
template <typename TokenContainer, typename Parser, typename BookContainer>
int run_file_test(const char * file_name, md_handler<Parser,BookContainer> &md, PrintType printType)
{
	md.start(printType);
	md_file_adapter file_adapter(file_name);

	struct timeval start_time_;
	struct timeval stop_time_;
	::gettimeofday(&start_time_, NULL);

	file_adapter.start<TokenContainer>(md);
	file_adapter.stop();

	::gettimeofday(&stop_time_, NULL);

	double total_micros = ((1e6 * (stop_time_.tv_sec  - start_time_.tv_sec )) + (stop_time_.tv_usec - start_time_.tv_usec));
	std::cerr << "Time to process " << file_adapter.get_counter() << " messages => " << total_micros << " micros" << std::endl;
	std::cerr << "Time for each => " << (total_micros/file_adapter.get_counter())*1000 << " nanos" << std::endl;

	md.printStats(std::cerr);
	md.stop();

  return 0;
}

#include <getopt.h>

void print_usage() {
	std::string message =
{R"(Usage: feed_handler -f <file name> [-p T|C] [-d M|H|V] [-x L] [-t A|S|C]                          
       -f is name of file to stream the input
         The file name can be realtive or absolute
       -p is for the type of print out put you wish to see
         T is a text book and C is a csv format output
       -d selects the underlying book structure to test
         M is a map, H is a hash and V is vector base data structures
       -x select the type of parser model to test
         L is the simple token list parsee for csv text or json line formats
       -t select the type of token container use in test
         A is a json_spirit based array, S is a strtk string vector and C is a custom char* vector
         Important - When using json_spirit it will expect a json file where S and C expect csv.
)"};

    printf(message.c_str());
}

template <typename TokenContainer>
void select_data_structure_and_run_test(char data_struct,
		PrintType print_type,
		char* file_name) {
	if (data_struct == 'M') {
		md_handler<list_parser, BookMap> md_handler;
		run_file_test<TokenContainer>(file_name, md_handler, print_type);
	} else if (data_struct == 'H') {
		md_handler<list_parser, BookHash> md_handler;
		run_file_test<TokenContainer>(file_name, md_handler, print_type);
	} else if (data_struct == 'V') {
		md_handler<list_parser, BookVector> md_handler;
		run_file_test<TokenContainer>(file_name, md_handler, print_type);
	} else {
		print_usage();
	}
}

template <typename TokenContainer>
void select_parser_and_run_test(char data_struct,
		PrintType print_type,
		char* file_name,
		char parser) {
	if (parser == 'L') {
		select_data_structure_and_run_test<TokenContainer>(data_struct,print_type,file_name);
	}
	// Other parser specific to the shape of the data
	else {
		print_usage();
	}
}

void select_token_method_and_run_test(char data_struct,
		PrintType print_type,
		char* file_name,
		char parser,
		char tokenizer) {
	// Allows a comparison between different tokenization techniques
	if (tokenizer=='A') {
		// Tokenized simple csv json array
		select_parser_and_run_test<json_token_array>(data_struct, print_type, file_name, parser);
	}
	else if(tokenizer=='S') {
		// Tokenized csv std::string vector
		select_parser_and_run_test<string_token_vector>(data_struct, print_type, file_name, parser);
	}
	else if(tokenizer=='C') {
		// Tokenized csv char * vector
		select_parser_and_run_test<char_token_vector>(data_struct, print_type, file_name, parser);
	}
	else {
		print_usage();
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv) {
	int option = 0;
	PrintType print_type = PrintType::Trading;
	char data_struct = 'M';
	char parser = 'L';
	char tokenizer = 'A';

	char * file_name=nullptr;

	while ((option = getopt(argc, argv, "h?f:p:d:x:t:")) != -1) {
		switch (option) {
		std::cout << option << std::endl;
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
			data_struct = optarg[0];
			break;
		case 'x':
			parser = optarg[0];
			break;
		case 't':
			tokenizer = optarg[0];
			break;
		default:
			print_usage();
			exit(EXIT_FAILURE);
		}
	}

	std::string f(file_name);

	if (!file_name ) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	if (
		(f.substr(f.length()-4,f.length())==".csv" && tokenizer=='A') ||
		(f.substr(f.length()-5,f.length())==".json" && (tokenizer=='C' || tokenizer=='S'))
	)
	{
		printf("Bad combination of tokenizer and file type\n");
		print_usage();
		exit(EXIT_FAILURE);
	}


	if (print_type==PrintType::Unknown) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	select_token_method_and_run_test(data_struct,print_type,file_name,parser,tokenizer);

	return 0;
}
