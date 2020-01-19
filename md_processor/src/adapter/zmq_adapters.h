
#ifndef md_zmq_adapter_h
#define md_zmq_adapter_h


#include "adapter/md_adapter.h"
#include <zmq.hpp>

/**
  *  @brief Implementation of the zeromq simple response server
  *  We feed this from some zeromq client (zmqrrr.py) 'Lazy Pirate'
  *  which obeys a simple protocol to tell us how many message
  *  we will be sending and uses a handshake of acknowledgements
  *  to reliably receive data
  *  We get an initial message to know the number that will be sent over
  *  and allows us to synchronise timing on the start in the run_test
  *
  */
template <typename TokenContainer,typename MD>
class md_zmq_rrr_adapter : public md_adapter {
public:
	/**
	 * Create the reliable request reply server
	 *
	 * @param port we will bind/listen to
	 * @param topic
	 */
	md_zmq_rrr_adapter(const std::string port="5555",const std::string topic="") :
		m_context(1),
		m_subscriber(m_context, ZMQ_REP)
	{
		std::string address = "tcp://*:"+port;
		m_subscriber.bind (address.c_str());
	}

	/**
	 * Wait for the first message to tell us how many message we will receive
	 * This actually tells us to be ready to receive, otherwise we don't know
	 * when to accurately start
	 */
	void wait() {
		// Receive the first message
        zmq::message_t message;
        zmq::detail::recv_result_t r = m_subscriber.recv(message);
    	numMessages = 0;
        if (r) {
			numMessages = std::stoi(std::string(static_cast<char*>(message.data()), message.size()),nullptr);
			// Ack the first message
			std::string nm=std::to_string(numMessages);
			//zmq::message_t nmreply(nm.size());
			//::memcpy (nmreply.data(),nm.data(),nm.size());
			//m_subscriber.send (nmreply);
			m_subscriber.send (zmq::const_buffer(nm.data(),nm.size()));
        }

	}

	/**
	 * Start the adapter processing.
	 * @param md The market data handler we will send the data to
	 */
    void start(MD & md) {
    	zmq::message_t update;
       	std::string line;
       	std::string countStr;

       	stopped=false;
    	while(!stopped && counter<numMessages) {
		try
    		{
			// Receive a message and process
		        zmq::detail::recv_result_t r = m_subscriber.recv(update);
    			// TODO this shouldn't be a string, but a byte stream (nasty cast here!!)
 			if (r) {
    				line=std::string(static_cast<char*>(update.data()), update.size());
    				md.process_message(get_tokens<TokenContainer>(line));
			}
    		}
    		catch(std::exception const& e)
    		{
    		    std::cerr << "Caught exception: " << e.what() << " for event(" << line << ") line number:" << counter+1 << "\n";
    		}
    		counter++;
    		// Ack this message
    		countStr=std::to_string(counter);
    		//zmq::message_t reply (countStr.size());
    		//::memcpy(reply.data(),countStr.data(),countStr.size());
    		m_subscriber.send (zmq::const_buffer(countStr.data(),countStr.size()));
    	}
    }

private:
    // zeromq context
    zmq::context_t m_context;
    // zeromq socket
    zmq::socket_t m_subscriber;

};

/**
  *  @brief Implementation of the zeromq subscriber
  *  We feed this from zeromq client (zmqpub.py)
  *  No handshake is done here so, some message loss is possible
  *  Again we get an initial message to know the number that will be sent over
  *  and allows us to synchronise timing on the start in the run_test
  */
template <typename TokenContainer,typename MD>
class md_zmq_sub_adapter : public md_adapter {
public:
	/**
	 * Create the zeromq subscriber
	 *
	 * @param port
	 * @param topic
	 */
	md_zmq_sub_adapter(const std::string port="5556",const std::string topic="") :
		m_context(1),
		m_subscriber(m_context, ZMQ_SUB)
	{
		std::string address = "tcp://*:"+port;
		m_subscriber.setsockopt( ZMQ_SUBSCRIBE, topic.c_str(), topic.length());
		m_subscriber.bind (address.c_str());
		// Set the high water mark a little higeher to minimise message loss
		// TODO - This is hard coded so may want configurable
		uint32_t receivehwm=10000;
		m_subscriber.setsockopt(ZMQ_RCVHWM,&receivehwm,sizeof(uint32_t));
	}

	/**
	 * Wait for the first message to tell us how many message we will receive
	 * This actually tells us to be ready to receive, otherwise we don't know
	 * when to accurately start
	 */
	void wait() {
        zmq::message_t message;
        zmq::detail::recv_result_t r = m_subscriber.recv(message);
        numMessages = 0;
        if (r)
        	numMessages = std::stoi(std::string(static_cast<char*>(message.data()), message.size()),nullptr);
	}

	/**
	 * Start the adapter processing.
	 * @param md The market data handler we will send the data to
	 */
    void start(MD & md) {
       	zmq::message_t update;
        std::string line;

    	stopped=false;
    	while(!stopped && counter<numMessages) {
    		try
    		{
    			// TODO this shouldn't be a string, but a byte stream (nasty cast here!!)
    			zmq::detail::recv_result_t r = m_subscriber.recv(update);
    			if (r) {
    				line=std::string(static_cast<char*>(update.data()), update.size());
    				md.process_message(get_tokens<TokenContainer>(line));
    			}
    		}
    		catch(std::exception const& e)
    		{
    		    std::cerr << "Caught exception: " << e.what() << " for event(" << line << ") line number:" << counter+1 << "\n";
    		}
    		counter++;

    	}
    }

private:
    // zeromq context
    zmq::context_t m_context;
    // zeromq subscriber
    zmq::socket_t m_subscriber;

};

/**
  *  @brief Implementation of the md_adapter
  *
  */
template <typename TokenContainer,typename MD>
class md_zmq_pull_adapter : public md_adapter {
public:
	/**
	 * Create a pull base server
	 * @param port
	 * @param topic
	 */
	md_zmq_pull_adapter(const std::string port="5557",const std::string topic="") :
		m_context(1),
		m_subscriber(m_context, ZMQ_PULL)
	{
		std::string address = "tcp://*:"+port;
		m_subscriber.bind (address.c_str());
		uint32_t receivehwm=10000;
		m_subscriber.setsockopt(ZMQ_RCVHWM,&receivehwm,sizeof(uint32_t));
	}

	/**
	 * Wait for the first message to tell us how many message we will receive
	 * This actually tells us to be ready to receive, otherwise we don't know
	 * when to accurately start
	 */
	void wait() {
        zmq::message_t message;
        zmq::detail::recv_result_t r = m_subscriber.recv(message);
        numMessages = 0;
        if (r) {
        	numMessages = std::stoi(std::string(static_cast<char*>(message.data()), message.size()),nullptr);
        }
	}

	/**
	 * Start the adapter processing.
	 *
	 * @param md The market data handler we will send the data to
	 */
    void start(MD & md) {
    	zmq::message_t update;
       	std::string line;

    	stopped=false;
    	while(!stopped && counter<numMessages) {
    		try
    		{
    			// Receive and process data message
    	        zmq::detail::recv_result_t r = m_subscriber.recv(update);
    	        if (r) {
    	        	// TODO this shouldn't be a string, but a byte stream (nasty cast here!!)
    	        	line=std::string(static_cast<char*>(update.data()), update.size());
    	        	md.process_message(get_tokens<TokenContainer>(line));
    	        }
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
