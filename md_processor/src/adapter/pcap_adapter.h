#ifndef md_pcap_adapter_h
#define md_pcap_adapter_h

#include <linux/if_ether.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <pcap/bpf.h>
#include <pcap/pcap.h>
#include <pcap/sll.h>

#include <sys/types.h>

#include <cstdbool>
#include <cstdint>

#include <iostream>
#include <stdexcept>
#include <string>

template<typename TokenContainer, typename MD>
class md_pcap_adapter: public md_adapter {
public:
	const static int DEFAULT_SNAPLEN = 65535;

	/**
	 * Create the pcap adapter to allow file or device packet sensing
	 *
	 * @param dev_name the device i.e eth0 or file name data/md-test-2.pcap
	 * @param filter used for pcap capture filter i.e certain sub address or ports
	 * @param snapLength length of the data segment, default allows the biggest
	 * @param promisc allows us to listen to traffic
	 * @param timeOut
	 */
	md_pcap_adapter(const std::string & dev_name,
			const std::string & filter="",
			int snapLength = DEFAULT_SNAPLEN,
			bool promisc = true,
			int timeOut = 500) {

		char error_buff[256];

		// See if it is a file type
		pcap_hdl = pcap_open_offline(dev_name.c_str(), error_buff);

		// If not open and interface i.e eth0
		if (!pcap_hdl) {
			pcap_hdl = pcap_open_live(dev_name.c_str(), snapLength, promisc,timeOut, error_buff);
			isdev=true;
		}

		if (!pcap_hdl) {
			throw std::runtime_error(std::string("Could not find pcap device or file:") + error_buff);
		}

		std::cerr << "[name:" + std::string(dev_name) + "]\n";

		// what kind of link layer is it, just to waht out for the differnce in cooked versus
		// ethernet
		int link_layer_type = pcap_datalink(pcap_hdl);

		if (link_layer_type == DLT_LINUX_SLL) {
			link_layer_type = SLL_HDR_LEN;
			std::cerr << "Changed _linkLayerLength to cooked :" << link_layer_length
			<< " "
			<< pcap_datalink_val_to_name(link_layer_type)
			<< "\n";
		}

		// Apply the filter @see pcap manual for details, same as tcpdump
		bpf_program fcode;
		int optimize = 1;
		if (pcap_compile(pcap_hdl, &fcode, filter.c_str(), optimize, 0)
				< 0) {
			throw std::runtime_error(std::string("Falied to compile pcap:")+ pcap_geterr(pcap_hdl));
		}
		if (pcap_setfilter(pcap_hdl, &fcode) < 0) {
			throw std::runtime_error(std::string("Falied to set filter for pcap:")+ pcap_geterr(pcap_hdl));
		}

		pcap_freecode(&fcode);
	}

    /**
       * Destroy and cleanup
       *
       */
     ~md_pcap_adapter() {
		  if (pcap_hdl) {
			pcap_close(pcap_hdl);
		  }
      }

     /**
      * Don't need to know how many messages.
      * When there is no more messages, packet is nullptr
      */
	void wait() {
	}

	/**
	 * Start the adapter processing.
	 *
	 * @param md The market data handler we will send the data to.
	 */
	void start(MD & md) {
		struct pcap_pkthdr header;
		// If this is used for a device listener we would have to override
		// the stopped and let the loop spin for timeout with isdev
		stopped=false;
		while (!stopped) {
			// Get the next packet from the device
			const unsigned char *packet = (u_char *) pcap_next(pcap_hdl,&header);

			if (packet) {
				//struct timeval ts = (header.ts);

				const struct ip *ip = (struct ip *) (packet + link_layer_length);
				int size_ip = (ip->ip_hl & 0x0f) * 4;

				int protocolHeader;
				unsigned char * value = nullptr;
				uint32_t length = 0;

				// Is it a TCP or UDP packet
				// Work out the length and extraction point for the byte stream
				if (ip->ip_p == IPPROTO_TCP) {
					const struct tcphdr *tcp = (struct tcphdr *) (packet
							+ link_layer_length + size_ip);
					protocolHeader = (tcp->doff & 0x0f) * 4;
					value = (unsigned char *) (packet + link_layer_length
							+ size_ip + protocolHeader);
					length = (ntohs(ip->ip_len) - (size_ip + protocolHeader));

				} else if (ip->ip_p == IPPROTO_UDP) {
					udphdr *uh = (udphdr *) ((unsigned char *) ip + size_ip);
					protocolHeader = sizeof(*uh);
					value = (unsigned char *) (packet + link_layer_length
							+ size_ip + protocolHeader);
					length = (ntohs(ip->ip_len) - (size_ip + protocolHeader));
				}

				if (length>0) {
					// TODO this shouldn't be a string, but a byte stream (even nastier cast here!!)
					std::string line(reinterpret_cast<const char*>(value), length);

					try {
						md.process_message(get_tokens<TokenContainer>(line));
					} catch (std::exception const& e) {
						std::cerr << "Caught exception: " << e.what()
								<< " for event(" << line << ") line number:"
								<< counter << "\n";
					}
					counter++;
				}
				else {
					// If it is file we must be finished
					if (!isdev) stop();
				}

			} else {
				// If it is file we must be finished
				if (!isdev) stop();
			}
		}
	}

private:
	// Handle to device
	pcap_t *pcap_hdl { };
	// Default to ethernet
	int link_layer_length { ETH_HLEN };
	// Allow to listen to device and file
	bool isdev{false};

};

#endif
