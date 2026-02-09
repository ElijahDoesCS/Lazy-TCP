#ifndef ICMP_H
#define ICMP_H

#include "../network.h"
#include "../ip/ip.h"
#include <string.h>

/**
 * @brief Update the checksum of the ICMP header
 * @param icmp_pack the packet header we are updating
 * @param pack_len the length of the packet header
 */
void icmp_update_checksum(ICMP_Header *icmp_pack, int pack_len);

/**
 * @brief Dispatch the ICMP packet to its implementation function
 * @param ip_pack is the IP packet that wraps the ICMP packet
 * @param icmp_pack is the ICMP packet
 * @return the bytes of the crafted response
 */
void icmp_echo_reply(IPv4_Header *ip_pack, ICMP_Header *icmp_pack);

/**
 * @brief Parse the ICMP packet by type and code, send a response as necessary
 * @param ip_pack is the IP packet that wraps the ICMP packet
 * @param icmp_pack is the ICMP packet
 */
void icmp_dispatch(IPv4_Header *ip_pack, ICMP_Header *icmp_pack);

#endif