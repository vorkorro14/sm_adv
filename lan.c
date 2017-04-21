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
#include "lan.h"

uint8_t mac_addr[6] = MAC_ADDR;

uint32_t ip_addr = IP_ADDR;
uint32_t ip_mask = IP_SUBNET_MASK;
uint32_t ip_gateway = IP_DEFAULT_GATEWAY;

uint8_t net_buf[ENC28J60_MAXFRAME];

uint8_t arp_cache_wr;
arp_cache_entry_t arp_cache[ARP_CACHE_SIZE];

void eth_send(eth_frame_t *frame, uint16_t len);
void eth_reply(eth_frame_t *frame, uint16_t len);
uint8_t *arp_resolve(uint32_t node_ip_addr);
uint8_t ip_send(eth_frame_t *frame, uint16_t len);
void ip_reply(eth_frame_t *frame, uint16_t len);
uint16_t ip_cksum(uint32_t sum, uint8_t *buf, uint16_t len);

void send_packet(eth_frame_t *frame, uint16_t len);


/*
 * IP
 */

// calculate IP checksum
uint16_t ip_cksum(uint32_t sum, uint8_t *buf, size_t len)
{
	while(len >= 2)
	{
		sum += ((uint16_t)*buf << 8) | *(buf+1);
		buf += 2;
		len -= 2;
	}

	if(len)
		sum += (uint16_t)*buf << 8;

	while(sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~htons((uint16_t)sum);
}

// send IP packet
// fields must be set:
//	- ip.dst
//	- ip.proto
// len is IP packet payload length
uint8_t ip_send(eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *ip = (void*)(frame->data);
	uint32_t route_ip;
	uint8_t *mac_addr_to;

	// apply route
	if( ((ip->to_addr ^ ip_addr) & ip_mask) == 0 )
		route_ip = ip->to_addr;
	else
		route_ip = ip_gateway;

	// resolve mac address
	if(!(mac_addr_to = arp_resolve(route_ip)))
		return 0;

	// send packet
	len += sizeof(ip_packet_t);

	memcpy(frame->to_addr, mac_addr_to, 6);
	frame->type = ETH_TYPE_IP;

	ip->ver_head_len = 0x45;
	ip->tos = 0;
	ip->total_len = htons(len);
	ip->fragment_id = 0;
	ip->flags_framgent_offset = 0;
	ip->ttl = IP_PACKET_TTL;
	ip->cksum = 0;
	ip->from_addr = ip_addr;
	ip->cksum = ip_cksum(0, (void*)ip, sizeof(ip_packet_t));

	eth_send(frame, len);
	return 1;
}

// send IP packet back
// len is IP packet payload length
void ip_reply(eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *packet = (void*)(frame->data);

	len += sizeof(ip_packet_t);

	packet->total_len = htons(len);
	packet->fragment_id = 0;
	packet->flags_framgent_offset = 0;
	packet->ttl = IP_PACKET_TTL;
	packet->cksum = 0;
	packet->to_addr = packet->from_addr;
	packet->from_addr = ip_addr;
	packet->cksum = ip_cksum(0, (void*)packet, sizeof(ip_packet_t));

	eth_reply((void*)frame, len);
}

// process IP packet
void ip_filter(eth_frame_t *frame, uint16_t len)
{
	ip_packet_t *packet = (void*)(frame->data);
	
	//if(len >= sizeof(ip_packet_t))
	//{
		if( (packet->ver_head_len == 0x45) &&
			(packet->to_addr == ip_addr) )
		{
			len = ntohs(packet->total_len) - 
				sizeof(ip_packet_t);

		}
	//}
}


/*
 * ARP
 */

// search ARP cache
uint8_t *arp_search_cache(uint32_t node_ip_addr)
{
	uint8_t i;
	for(i = 0; i < ARP_CACHE_SIZE; ++i)
	{
		if(arp_cache[i].ip_addr == node_ip_addr)
			return arp_cache[i].mac_addr;
	}
	return 0;
}

// resolve MAC address
// returns 0 if still resolving
// (invalidates net_buffer if not resolved)
uint8_t *arp_resolve(uint32_t node_ip_addr)
{
	eth_frame_t *frame = (void*)net_buf;
	arp_message_t *msg = (void*)(frame->data);
	uint8_t *mac;

	// search arp cache
	if((mac = arp_search_cache(node_ip_addr)))
		return mac;

	// send request
	memset(frame->to_addr, 0xff, 6);
	frame->type = ETH_TYPE_ARP;

	msg->hw_type = ARP_HW_TYPE_ETH;
	msg->proto_type = ARP_PROTO_TYPE_IP;
	msg->hw_addr_len = 6;
	msg->proto_addr_len = 4;
	msg->type = ARP_TYPE_REQUEST;
	memcpy(msg->mac_addr_from, mac_addr, 6);
	msg->ip_addr_from = ip_addr;
	memset(msg->mac_addr_to, 0x00, 6);
	msg->ip_addr_to = node_ip_addr;

	eth_send(frame, sizeof(arp_message_t));
	return 0;
}

// process arp packet
void arp_filter(eth_frame_t *frame, uint16_t len)
{
	arp_message_t *msg = (void*)(frame->data);

	if(len >= sizeof(arp_message_t))
	{
		if( (msg->hw_type == ARP_HW_TYPE_ETH) &&
			(msg->proto_type == ARP_PROTO_TYPE_IP) &&
			(msg->ip_addr_to == ip_addr) )
		{
			switch(msg->type)
			{
			case ARP_TYPE_REQUEST:
				msg->type = ARP_TYPE_RESPONSE;
				memcpy(msg->mac_addr_to, msg->mac_addr_from, 6);
				memcpy(msg->mac_addr_from, mac_addr, 6);
				msg->ip_addr_to = msg->ip_addr_from;
				msg->ip_addr_from = ip_addr;
				eth_reply(frame, sizeof(arp_message_t));
				break;
			case ARP_TYPE_RESPONSE:
				if(!arp_search_cache(msg->ip_addr_from))
				{
					arp_cache[arp_cache_wr].ip_addr = msg->ip_addr_from;
					memcpy(arp_cache[arp_cache_wr].mac_addr, msg->mac_addr_from, 6);
					arp_cache_wr++;
					if(arp_cache_wr == ARP_CACHE_SIZE)
						arp_cache_wr = 0;
				}
				break;
			}
		}
	}
}


/*
 * Ethernet
 */

// send Ethernet frame
// fields must be set:
//	- frame.to_addr
//	- frame.type
void eth_send(eth_frame_t *frame, uint16_t len)
{
	memcpy(frame->from_addr, mac_addr, 6);
	send_packet((void*)frame, len +
		sizeof(eth_frame_t));
}

// send Ethernet frame back
void eth_reply(eth_frame_t *frame, uint16_t len)
{
	memcpy(frame->to_addr, frame->from_addr, 6);
	memcpy(frame->from_addr, mac_addr, 6);
	send_packet((void*)frame, len + 
		sizeof(eth_frame_t));
}

void eth_filter(eth_frame_t *frame, uint16_t len)
{
	if(len >= sizeof(eth_frame_t))
	{
		switch(frame->type)
		{
		case ETH_TYPE_ARP:
			arp_filter(frame, len - sizeof(eth_frame_t));
			break;
		case ETH_TYPE_IP:
			ip_filter(frame, len - sizeof(eth_frame_t));
			break;
		}
	}
}
