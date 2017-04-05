/*
 * mc_net.c
 *
 * Project: Smart_advertising
 *
 * Created: 05.04.2017 15:45:46
 * Author : Vladimir Litvinenko
 * 
 * Code for ATmega8535, realizing work of hardware stand
 * with the Internet and transmitting data to server for following processing
 *
 *
 */


#include <avr/io.h>
#include <string.h>

// Recoding word type from little-endian to big-endian 
#define htons(a)            ((((a)>>8)&0xff)|(((a)<<8)&0xff00))
#define ntohs(a)            htons(a)

// Recoding dword type from little-endian to big-endian
#define htonl(a)            ( (((a)>>24)&0xff) | (((a)>>8)&0xff00) |\
(((a)<<8)&0xff0000) | (((a)<<24)&0xff000000) )
#define ntohl(a)            htonl(a)

// Macros for IP-address
#define inet_addr(a,b,c,d)    ( ((uint32_t)a) | ((uint32_t)b << 8) |\
                                ((uint32_t)c << 16) | ((uint32_t)d << 24) )

// MAC-address
uint8_t mac_addr[6] = {0x00,0x13,0x37,0x01,0x23,0x45};

// IP-address
uint32_t ip_addr = inet_addr(192,168,0,222);



//PACKETS
//

// Ethernet-frame
typedef struct eth_frame {
	uint8_t to_addr[6]; // destination address
	uint8_t from_addr[6]; //  source address
	uint16_t type; // protocol
	uint8_t data[];
} eth_frame_t;

// ARP-packet
typedef struct arp_message {
	uint16_t hw_type; // Link layer protocol (Ethernet)
	uint16_t proto_type; // Internet layer protocol (IP)
	uint8_t hw_addr_len; // MAC-address length =6
	uint8_t proto_addr_len; // IP-address length =4
	uint16_t type; // Message type (request/response)
	uint8_t mac_addr_from[6]; // Source MAC-address
	uint32_t ip_addr_from; // Source IP-address
	uint8_t mac_addr_to[6]; // Destination MAC-address, zeros if unknown
	uint32_t ip_addr_to; // Destination IP-address
} arp_message_t;

//
//END OF PACKETS



void eth_reply(eth_frame_t *frame, uint16_t len);
void eth_filter(eth_frame_t *frame, uint16_t len);
void arp_filter(eth_frame_t *frame, uint16_t len);
void send_packet(eth_frame_t *frame, uint16_t len);
void ip_filter();



//ETHERNET
//

#define ETH_TYPE_ARP        htons(0x0806)
#define ETH_TYPE_IP            htons(0x0800)

// Sending answer to Ethernet-frame
void eth_reply(eth_frame_t *frame, uint16_t len)
{
	memcpy(frame->to_addr, frame->from_addr, 6);
	memcpy(frame->from_addr, mac_addr, 6);
	send_packet((void*)frame, len + sizeof(eth_frame_t));
}

// Handler of receiving Ethernet-frames
void eth_filter(eth_frame_t *frame, uint16_t len)
{
	// Checking size of frame
	if(len >= sizeof(eth_frame_t))
	{
		switch(frame->type)
		{
			// ARP-packet received, executing ARP-packet handler
			case ETH_TYPE_ARP:
			arp_filter(frame, len - sizeof(eth_frame_t));
			break;
			
			// IP packet received, executing IP-packet handler
			case ETH_TYPE_IP:
			ip_filter(frame, len - sizeof(eth_frame_t));
			break;
		}
	}
}

//
//END OF ETHERNET



//ARP
//

#define ARP_HW_TYPE_ETH        htons(0x0001)
#define ARP_PROTO_TYPE_IP    htons(0x0800)

#define ARP_TYPE_REQUEST    htons(1)
#define ARP_TYPE_RESPONSE    htons(2)

// ARP-packet handler
void arp_filter(eth_frame_t *frame, uint16_t len)
{
	arp_message_t *msg = (void*)(frame->data);

	// Checking size of frame
	if(len >= sizeof(arp_message_t))
	{
		// Checking protocol type
		if( (msg->hw_type == ARP_HW_TYPE_ETH) &&
		(msg->proto_type == ARP_PROTO_TYPE_IP) )
		{
			// ARP-request and self IP-address?
			if( (msg->type == ARP_TYPE_REQUEST) &&
			(msg->ip_addr_to == ip_addr) )
			{
				// Sending an answer
				msg->type = ARP_TYPE_RESPONSE;
				memcpy(msg->mac_addr_to, msg->mac_addr_from, 6);
				memcpy(msg->mac_addr_from, mac_addr, 6);
				msg->ip_addr_to = msg->ip_addr_from;
				msg->ip_addr_from = ip_addr;

				eth_reply(frame, sizeof(arp_message_t));
			}
		}
	}
}

//
//END OF ARP


int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
    }
}

