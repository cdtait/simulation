#ifndef token_containers_h
#define token_containers_h

#include "json_spirit/json_spirit_reader_template.h"
#include "strtk/strtk.hpp"
#include "token_vector.h"

typedef json_spirit::Array json_token_array;
typedef std::vector<std::string> string_token_vector;
typedef token_vector<char> char_token_vector;


template <typename T>
inline typename std::enable_if<std::is_same<T, json_token_array>::value,json_token_array>::type
get_tokens(const std::string& line) {
	json_spirit::Value value;
	json_spirit::read_string(line, value);
	assert(value.type()==json_spirit::array_type);
	return value.get_array();
}


template <typename T>
inline typename std::enable_if<std::is_same<T, string_token_vector>::value,string_token_vector>::type
get_tokens(const std::string& line) {
	string_token_vector token_list;
	strtk::parse(line,",",token_list);
	return token_list;
}

template <typename T>
inline typename std::enable_if<std::is_same<T, char_token_vector>::value,char_token_vector>::type
get_tokens(std::string& line) {
	char_token_vector token_list(line);
	return token_list;
}

#endif
