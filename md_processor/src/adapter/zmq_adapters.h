
#ifndef md_zmq_adapter_h
#define md_zmq_adapter_h


#include "adapter/md_adapter.h"
#include <zmq.hpp>

/**
  *  @brief Implementation of the md_adapter
  *
  */
template <typename TokenContainer,typename MD>
class md_zmq_rrr_adapter : public md_adapter {
public:
	md_zmq_rrr_adapter(const std::string port="5555",const std::string topic="") :
		m_context(1),
		m_subscriber(m_context, ZMQ_REP)
	{
		std::string address = "tcp://*:"+port;
		m_subscriber.bind (address.c_str());
	}

	void wait() {
        zmq::message_t message;
        m_subscriber.recv(&message);
    	numMessages = std::stoi(std::string(static_cast<char*>(message.data()), message.size()),nullptr);

    	std::string nm=std::to_string(numMessages);
	    zmq::message_t nmreply(nm.size());
	    ::memcpy (nmreply.data(),nm.data(),nm.size());
	    m_subscriber.send (nmreply);
	}

    void start(MD & md) {
    	zmq::message_t update;
       	std::string line;
       	std::string countStr;

       	stopped=false;
    	while(!stopped && counter<numMessages) {
    		try
    		{
    			m_subscriber.recv(&update);
    			line=std::string(static_cast<char*>(update.data()), update.size());
    			md.process_message(get_tokens<TokenContainer>(line));
    		}
    		catch(std::exception const& e)
    		{
    		    std::cerr << "Caught exception: " << e.what() << " for event(" << line << ") line number:" << counter+1 << "\n";
    		}
    		counter++;
    		countStr=std::to_string(counter);
    		zmq::message_t reply (countStr.size());
    		::memcpy(reply.data(),countStr.data(),countStr.size());
    	    m_subscriber.send (reply);
    	}
    }

private:
    zmq::context_t m_context;
    zmq::socket_t m_subscriber;

};

/**
  *  @brief Implementation of the md_adapter
  *
  */
template <typename TokenContainer,typename MD>
class md_zmq_sub_adapter : public md_adapter {
public:
	md_zmq_sub_adapter(const std::string port="5556",const std::string topic="") :
		m_context(1),
		m_subscriber(m_context, ZMQ_SUB)
	{
		std::string address = "tcp://*:"+port;
		m_subscriber.setsockopt( ZMQ_SUBSCRIBE, topic.c_str(), topic.length());
		m_subscriber.bind (address.c_str());
		uint32_t receivehwm=10000;
		m_subscriber.setsockopt(ZMQ_RCVHWM,&receivehwm,sizeof(uint32_t));
	}

	void wait() {
        zmq::message_t message;
        m_subscriber.recv(&message);
    	numMessages = std::stoi(std::string(static_cast<char*>(message.data()), message.size()),nullptr);
	}

    void start(MD & md) {
       	zmq::message_t update;
        std::string line;

    	stopped=false;
    	while(!stopped && counter<numMessages) {
    		try
    		{
    			m_subscriber.recv(&update);
    			line=std::string(static_cast<char*>(update.data()), update.size());
    			md.process_message(get_tokens<TokenContainer>(line));
    		}
    		catch(std::exception const& e)
    		{
    		    std::cerr << "Caught exception: " << e.what() << " for event(" << line << ") line number:" << counter+1 << "\n";
    		}
    		counter++;

    	}
    }

private:
    zmq::context_t m_context;
    zmq::socket_t m_subscriber;

};

/**
  *  @brief Implementation of the md_adapter
  *
  */
template <typename TokenContainer,typename MD>
class md_zmq_pull_adapter : public md_adapter {
public:
	md_zmq_pull_adapter(const std::string port="5557",const std::string topic="") :
		m_context(1),
		m_subscriber(m_context, ZMQ_PULL)
	{
		std::string address = "tcp://*:"+port;
		m_subscriber.bind (address.c_str());
		uint32_t receivehwm=10000;
		m_subscriber.setsockopt(ZMQ_RCVHWM,&receivehwm,sizeof(uint32_t));
	}

	void wait() {
        zmq::message_t message;
        m_subscriber.recv(&message);
    	numMessages = std::stoi(std::string(static_cast<char*>(message.data()), message.size()),nullptr);
	}

    void start(MD & md) {
    	zmq::message_t update;
       	std::string line;

    	stopped=false;
    	while(!stopped && counter<numMessages) {
    		try
    		{
    			m_subscriber.recv(&update);
    			line=std::string(static_cast<char*>(update.data()), update.size());
    			md.process_message(get_tokens<TokenContainer>(line));
    		}
    		catch(std::exception const& e)
    		{
    		    std::cerr << "Caught exception: " << e.what() << " for event(" << line << ") line number:" << counter+1 << "\n";
    		}
    		counter++;
    	}

    }

private:
    zmq::context_t m_context;
    zmq::socket_t m_subscriber;

};

#endif
