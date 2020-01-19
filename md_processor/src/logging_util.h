#ifndef LOGGINGI_H_
#define LOGGINGI_H_

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/ndc.h>

#include <boost/version.hpp>

#if BOOST_VERSION >= 104400
#include <boost/exception/all.hpp>
#else
#include <boost/exception.hpp>
#endif

using namespace std;
using namespace log4cxx;

#ifdef __LOG_TRACE__
    #define LOG_TRACE(logger,value) \
            LOG4CXX_TRACE(logger, value);
#else
    #define LOG_TRACE(logger,value)
#endif

#define LOG_DEBUG(debug,logger,value) \
    if (debug && LOG4CXX_UNLIKELY(logger->isDebugEnabled())) { \
        LOG4CXX_DEBUG(logger, value); \
    }

#define LOG_WARN(logger,value) \
    LOG4CXX_WARN(logger, value); \

#define LOG_INFO(logger,value) \
    LOG4CXX_INFO(logger, value); \

#define LOG_ERROR(logger,value) \
    LOG4CXX_ERROR(logger, value); \

#define LOG_FATAL(logger,value) \
    LOG4CXX_FATAL(logger, value); \

/**
 * @brief Generic exception for catching boost and stl types of exception.
 */
class base_exception : public boost::exception, public std::exception {
public:
	explicit base_exception(const char * msg) : _msg(msg) { }

	explicit base_exception(const std::string& msg) : _msg(msg) { }

	virtual const char* 
    what() const throw () {
		return _msg.c_str();
	}

	virtual ~base_exception() throw () { }

protected:
	const std::string _msg;
};

#endif /* LOGGINGI_H_ */
