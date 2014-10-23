
#ifndef token_char_types_h
#define token_char_types_h

#include "token_containers.h"

/**
 * Handle floating types
 * Using strtk included and all copyright honoured
 * Handles all types of integers included signed
 *
 * @param itr
 * @param end
 * @param result
 * @param
 * @return
 */
template <typename T>
inline bool make_type(char_token_vector::iterator itr, const char_token_vector::iterator end, T &result,
		typename std::enable_if<std::is_floating_point<T>::value >::type* = 0 ) {
	if (end == itr) return false;
	return strtk::string_to_type_converter(std::string(*itr),result);
}

/**
 * Handle a integer type
 * Using strtk included and all copyright honoured
 * Handles all types of integers included signed
 *
 * @param itr
 * @param end
 * @param result
 * @param
 * @return True if its an integer OK
 */
template <typename T>
inline bool make_type(char_token_vector::iterator itr, const char_token_vector::iterator end, T &result,
		typename std::enable_if<std::is_integral<T>::value >::type* = 0) {
	if (end == itr) return false;
	return strtk::string_to_type_converter(std::string(*itr),result);
}

/**
 * Handle char* strings
 * @param itr
 * @param end
 * @param t
 * @return True if have a string OK
 */
inline bool make_type(char_token_vector::iterator itr, const char_token_vector::iterator end, char* t) {
	if (end == itr) return false;
	char_token_vector::reference i = *itr;
	while (end != itr && *t) {
		*(t++)=*(i++);
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
inline bool make_type(char_token_vector::iterator itr, const char_token_vector::iterator end, char& t) {
	if (end == itr) return false;
	char_token_vector::reference i = *itr;
	t=i[0];
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
inline bool make_type(char_token_vector::iterator itr, const char_token_vector::iterator end, Event & event) {
	if (end == itr) return false;
	char_token_vector::reference i = *itr;
	if (i[0]=='A')
		event=Event::Add;
	else if (i[0]=='M')
		event=Event::Modify;
	else if (i[0]=='X')
		event=Event::Cancel;
	else if (i[0]=='T')
		event=Event::Trade;
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
inline bool make_type(char_token_vector::iterator itr, const char_token_vector::iterator end, Side & side) {
	if (end == itr) return false;
	char_token_vector::reference i = *itr;
	if (i[0]=='B')
		side=Side::Bid;
	else if (i[0]=='S')
		side=Side::Ask;
	else {
		side=Side::Unknown;
		return false;
	}

	return true;
}

#endif
