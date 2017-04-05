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

// Recoding word type from big-endian to little-endian 
#define htons(a)            ((((a)>>8)&0xff)|(((a)<<8)&0xff00))
#define ntohs(a)            htons(a)

// Recoding dword type from big-endian to little-endian
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

#define ETH_TYPE_ARP        htons(0x0806)
#define ETH_TYPE_IP            htons(0x0800)


// Ethernet-frame
typedef struct eth_frame {
	uint8_t to_addr[6]; // destination address
	uint8_t from_addr[6]; //  source address
	uint16_t type; // protocol
	uint8_t data[];
} eth_frame_t;

void send_packet(eth_frame_t *frame, uint16_t len);
void arp_filter();
void ip_filter();

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



int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
    }
}

