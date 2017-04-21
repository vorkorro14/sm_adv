#pragma once

#include <string.h>


/*
 * Config
 */

#define WITH_ICMP

#define ARP_CACHE_SIZE		3

#define MAC_ADDR			{0x00,0x13,0x37,0x01,0x23,0x45}

#define IP_ADDR				inet_addr(192,168,0,222)
#define IP_SUBNET_MASK		inet_addr(255,255,255,0)
#define IP_DEFAULT_GATEWAY	inet_addr(192,168,0,1)

#define IP_PACKET_TTL		64


/*
 * BE conversion
 */

#define htons(a)			((((a)>>8)&0xff)|(((a)<<8)&0xff00))
#define ntohs(a)			htons(a)

#define htonl(a)			( (((a)>>24)&0xff) | (((a)>>8)&0xff00) |\
								(((a)<<8)&0xff0000) | (((a)<<24)&0xff000000) )
#define ntohl(a)			htonl(a)

#define inet_addr(a,b,c,d)	( ((uint32_t)a) | ((uint32_t)b << 8) |\
								((uint32_t)c << 16) | ((uint32_t)d << 24) )


/*
 * Ethernet
 */
 
#define ETH_TYPE_ARP		htons(0x0806)
#define ETH_TYPE_IP			htons(0x0800)

typedef struct eth_frame {
	uint8_t to_addr[6];
	uint8_t from_addr[6];
	uint16_t type;
	uint8_t data[];
} eth_frame_t;

/*
 * ARP
 */

#define ARP_HW_TYPE_ETH		htons(0x0001)
#define ARP_PROTO_TYPE_IP	htons(0x0800)

#define ARP_TYPE_REQUEST	htons(1)
#define ARP_TYPE_RESPONSE	htons(2)

typedef struct arp_message {
	uint16_t hw_type;
	uint16_t proto_type;
	uint8_t hw_addr_len;
	uint8_t proto_addr_len;
	uint16_t type;
	uint8_t mac_addr_from[6];
	uint32_t ip_addr_from;
	uint8_t mac_addr_to[6];
	uint32_t ip_addr_to;
} arp_message_t;

typedef struct arp_cache_entry {
	uint32_t ip_addr;
	uint8_t mac_addr[6];
} arp_cache_entry_t;

/*
 * IP
 */

#define IP_PROTOCOL_ICMP	1
#define IP_PROTOCOL_TCP		6
#define IP_PROTOCOL_UDP		17

typedef struct ip_packet {
	uint8_t ver_head_len;
	uint8_t tos;
	uint16_t total_len;
	uint16_t fragment_id;
	uint16_t flags_framgent_offset;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t cksum;
	uint32_t from_addr;
	uint32_t to_addr;
	uint8_t data[];
} ip_packet_t;
