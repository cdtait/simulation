
#ifndef token_json_types_h
#define token_json_types_h

#include "token_containers.h"

/**
 * Handle floating types
 * Using json implementation conversion included and all copyright honoured
 * Handles all types of integers included signed
 *
 * @param itr
 * @param end
 * @param result
 * @param
 * @return
 */
template <typename T>
inline bool make_type(json_token_array::iterator & itr, const json_token_array::iterator & end, T &result,
		typename std::enable_if<std::is_floating_point<T>::value >::type* = 0 ) {
	result=itr->get_real();
	return true;
}

/**
 * Handle a integer type
 * Using json implementation conversion included and all copyright honoured
 * Handles all types of integers included signed
 *
 * @param itr
 * @param end
 * @param result
 * @param
 * @return True if its an integer OK
 */
template <typename T>
inline bool make_type(json_token_array::iterator & itr, const json_token_array::iterator & end, T &result,
		typename std::enable_if<std::is_integral<T>::value >::type* = 0) {
	result=itr->get_int();
	return true;
}

/**
 * Handle char* strings
 * Using json implementation conversion included and all copyright honoured
 *
 * @param itr
 * @param end
 * @param t
 * @return True if have a string OK
 */
inline bool make_type(json_token_array::iterator & itr, const json_token_array::iterator & end, char* t) {
	if (end == itr) return false;
	std::string s = itr->get_str();
	std::string::iterator sit = s.begin();
	std::string::iterator eit = s.end();
	while (eit != sit && *t) {
		*(t++)=*(sit++);
	}
	*t='\0';
	return true;
}

/**
 * Handle single char
 *
 * @param itr
 * @param end
 * @param t
 * @return True if one char OK
 */

inline bool make_type(json_token_array::iterator & itr, const json_token_array::iterator & end, char& t) {
	if (end == itr) return false;
	t=itr->get_str()[0];
	return true;
}

/**
 * Make type implementation for Event enums
 * need specific handling
 *
 * @param itr
 * @param end
 * @param side
 * @return True if syntax OK
 */
inline bool make_type(json_token_array::iterator & itr, const json_token_array::iterator & end, Event & event) {
	if (end == itr) return false;
	if (itr->get_str()=="A")
		event=Event::Add;
	else if (itr->get_str()=="M")
		event=Event::Modify;
	else if (itr->get_str()=="X")
		event=Event::Cancel;
	else if (itr->get_str()=="T")
		event=Event::Trade;
	else if (itr->get_str()=="S")
		event=Event::Snapshot;
	else {
		event=Event::Unknown;
		return false;
	}

	return true;
}

/**
 * Make type implementation Side enums
 * need specific handling
 *
 * @param itr
 * @param end
 * @param side
 * @return True if syntax OK
 */
inline bool make_type(json_token_array::iterator & itr, const json_token_array::iterator & end, Side & side) {
	if (end == itr) return false;
	if (itr->get_str()=="B")
		side=Side::Bid;
	else if (itr->get_str()=="S")
		side=Side::Ask;
	else {
		side=Side::Unknown;
		return false;
	}

	return true;
}

#endif
