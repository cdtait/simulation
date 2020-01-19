#ifndef ARGUMENTS_H_
#define ARGUMENTS_H_

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

#include <stdarg.h>
#include <iostream>
#include <vector>
#include <map>

static constexpr const bool mandatory_arg = true;
static constexpr const bool optional_arg = false;
static constexpr const bool has_arg = true;
static constexpr const bool no_arg = false;

class arguments {

public:
	arguments(int argc, char *argv[]) {
		for (int i = 1; i < argc; i++) {
			std::string option_val = argv[i];
			std::string option;
			std::string val;

			auto index = option_val.find(">");
			if (option_val.find(">") != std::string::npos and
				option_val.find("\\>") == std::string::npos
			) {
				break;
			}

			index = option_val.find("--");
			if (index == std::string::npos || index > 0) {
				throw std::runtime_error(
						"Option format does not begin with -- " + option);
			}

			index = option_val.find('=');
			if (index != std::string::npos) {
				option = option_val.substr(2, index - 2);
				val = option_val.substr(index + 1, option_val.size());
			} else {
				option = option_val.substr(2, option_val.size());
				val = "1";
			}

			if (_args.find(option) != _args.end()) {
				throw std::runtime_error(
						"Duplicate option name found " + option);
			}

			_args[option] = val;
		}
	}

	template<typename T>
	T get_opt(const std::string &long_name, bool is_mandatory = mandatory_arg,
			bool has_argument = no_arg, const T &default_value = T { }) const {

		auto search = std::find_if(_args.begin(), _args.end(),
				[&long_name](
						const std::pair<std::string, std::string> &s) -> bool {
					return s.first == long_name;
				});

		if (search == _args.end()) {
			if (is_mandatory) {
				throw std::runtime_error(
						"Mandatory option not found:" + long_name);
			} else {
				// No option and no mandatory so only can return default
				return default_value;
			}
		} else {
			// if mandatory it should have a return value
			// and if it is a flag it must be found to get here so it should
			// have a flag value of 1 (true) boolean (set by args)
			return lexical_cast<T>(search->second);
		}

		return default_value;
	}

private:
	std::map<std::string, std::string> _args;
};

#endif /* ARGUMENTS_H_ */

