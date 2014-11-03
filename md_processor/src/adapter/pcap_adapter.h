#ifndef md_pcap_adapter_h
#define md_pcap_adapter_h

#include <pcap.h>
#include <pcap-bpf.h>
#include <pcap/sll.h>
#include <signal.h>
#include <exception>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <array>
#include <exception>

#include <string>

template<typename TokenContainer, typename MD>
class md_pcap_adapter: public md_adapter {
public:
	const static int DEFAULT_SNAPLEN = 65535;

	md_pcap_adapter(const std::string & dev_name,
			const std::string & filter="",
			int snapLength = DEFAULT_SNAPLEN,
			bool promisc = true,
			int timeOut = 500) {

		char error_buff[256];

		pcap_hdl = pcap_open_offline(dev_name.c_str(), error_buff);

		if (!pcap_hdl) {
			pcap_hdl = pcap_open_live(dev_name.c_str(), snapLength, promisc,timeOut, error_buff);
		}

		if (!pcap_hdl) {
			throw std::runtime_error(std::string("Could not find pcap device or file:") + error_buff);
		}

		std::cerr << "[name:" + std::string(dev_name) + "]\n";

		int link_layer_type = pcap_datalink(pcap_hdl);

		if (link_layer_type == DLT_LINUX_SLL) {
			link_layer_type = SLL_HDR_LEN;
			std::cerr << "Changed _linkLayerLength to cooked :" << link_layer_length
			<< " "
			<< pcap_datalink_val_to_name(link_layer_type)
			<< "\n";
		}

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

	void wait() {

	}

	void start(MD & md) {
		struct pcap_pkthdr header;
		stopped=false;
		while (!stopped) {
			const unsigned char *packet = (u_char *) pcap_next(pcap_hdl,&header);

			if (packet) {
				//struct timeval ts = (header.ts);

				const struct ip *ip = (struct ip *) (packet + link_layer_length);
				int size_ip = (ip->ip_hl & 0x0f) * 4;

				int protocolHeader;
				unsigned char * value = nullptr;
				uint32_t length = 0;

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
					stopped = true;
				}

			} else {
				stopped = true;
			}
		}
	}

private:
	pcap_t *pcap_hdl { };
	int link_layer_length { ETH_HLEN };

};

#endif
