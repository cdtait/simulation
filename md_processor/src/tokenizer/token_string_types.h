
#ifndef token_string_types_h
#define token_string_types_h

#include "token_containers.h"

/**
 * Handle a integer type
 * Using atoi
 * Handles all types of integers included signed
 *
 * @param itr
 * @param end
 * @param result
 * @param
 * @return True if its an integer OK
 */

template <typename T>
inline bool make_type(string_token_vector::iterator & itr,
		const string_token_vector::iterator & end, T &result,
		typename std::enable_if<std::is_integral<T>::value >::type* = 0) {
	if (end == itr) return false;
	result=::atoi(itr->c_str());
	return true;
}


/**
 * Handle floating types
 * Using atof
 * Handles all types of floating
 *
 * @param itr
 * @param end
 * @param result
 * @param
 * @return
 */
template <typename T>
inline bool make_type(string_token_vector::iterator & itr,
		const string_token_vector::iterator & end, T &result,
		typename std::enable_if<std::is_floating_point<T>::value >::type* = 0 ) {
	if (end == itr) return false;
	result=::atof(itr->c_str());
	return true;
}


/**
 * Handle char* strings
 *
 * @param itr
 * @param end
 * @param t
 * @return True if have a string OK
 */
inline bool make_type(string_token_vector::iterator & itr,
		const string_token_vector::iterator & end, char* t) {
	if (end == itr) return false;
	std::string s = *itr;
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
inline bool make_type(string_token_vector::iterator & itr,
		const std::vector<std::string>::iterator & end, char& t) {
	if (end == itr) return false;
	t=(*itr)[0];
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
inline bool make_type(string_token_vector::iterator & itr,
		const string_token_vector::iterator & end,
		Event & event) {
	if (end == itr) return false;
	if (*itr=="A")
		event=Event::Add;
	else if (*itr=="M")
		event=Event::Modify;
	else if (*itr=="X")
		event=Event::Cancel;
	else if (*itr=="T")
		event=Event::Trade;
	else if (*itr=="S")
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
inline bool make_type(string_token_vector::iterator & itr,
		const string_token_vector::iterator & end, Side & side) {
	if (end == itr) return false;
	if (*itr=="B")
		side=Side::Bid;
	else if (*itr=="S")
		side=Side::Ask;
	else {
		side=Side::Unknown;
		return false;
	}

	return true;
}

#endif
