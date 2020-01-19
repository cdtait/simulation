#ifndef udp_publisher_h
#define udp_publisher_h

#include <boost/asio.hpp>

#include "arguments.h"
#include "logging_util.h"
#include "md_publisher.h"
#include "md_types.h"

using boost::asio::ip::udp;

template<int N=5>
class udp_publisher : public md_publisher<N> {
public:
    udp_publisher() :
     _logger(Logger::getLogger("udp_publisher")),
     publisher(_io_service)
    {         
    }
     
	udp_publisher(PrintType printType,const arguments& args) : udp_publisher(args) {
	}

    udp_publisher(const udp::endpoint & endpoint,bool multicast) :
     _logger(Logger::getLogger("udp_publisher")),
     publisher(_io_service)
    {         
        init(multicast);
    }
     
    udp_publisher(std::string address,std::string port,bool multicast) :
     _logger(Logger::getLogger("udp_publisher")),
     publisher(_io_service),
     _endpoint(boost::asio::ip::address::from_string(address), std::stoi(port))
    {         
        init(multicast);
    }

    udp_publisher(const arguments & args) :
    _logger(Logger::getLogger("udp_publisher")),
    publisher(_io_service)
    {
        const std::string sink_address = args.get_opt<std::string>("publish_address",mandatory_arg,has_arg,"");
 
        auto sep_index = sink_address.find(':');

        if (sep_index != std::string::npos) {
            std::string ip = sink_address.substr(0, sep_index);
            int port = ::atoi(sink_address.substr(sep_index + 1, sink_address.size()).c_str());
            LOG_INFO(_logger, "Adding publisher:" << sink_address);
            _endpoint.address(boost::asio::ip::address::from_string(ip));
            _endpoint.port(port);
        }
        
        auto multicast = args.get_opt("multicast",optional_arg,no_arg,false);
        
        init(multicast);
    }

    void init(bool multicast)
    {         
        boost::system::error_code errorCode;

        publisher.open(_endpoint.protocol(), errorCode);

        if (errorCode) {
                LOG_FATAL(_logger, "Failed to open socket - error code [" << errorCode.value() << "]");
                exit(errorCode.value());
        } else {
                LOG_INFO(_logger, "Opened socket");
        }

        if (!multicast) publisher.set_option(boost::asio::ip::udp::socket::broadcast(true));
    }
       
    void stop() {
            publisher.close();
    }
    
	void offer(BookData<N> && book_data) {
        size_t len = sizeof(BookData<N>);
        publisher.async_send_to(boost::asio::buffer((void*)&book_data, len), _endpoint,
        [this](boost::system::error_code, std::size_t) {});
	}
          
    const udp::endpoint & endpoint() const noexcept {
        return _endpoint;
    }
    
    void initialize(std::string address,std::string port,bool multicast)   {
        _endpoint.address(boost::asio::ip::address::from_string(address));
        _endpoint.port(::atoi(port.c_str()));
        init(multicast);
    }
    
    bool open() {
        return publisher.is_open();
    }
private:
    LoggerPtr _logger;
    constexpr static const int buffer_size = 2048;
    std::array<unsigned char, buffer_size> pCmp;

    boost::asio::io_service _io_service;
    udp::socket publisher;
    udp::endpoint _endpoint;
};

#endif
