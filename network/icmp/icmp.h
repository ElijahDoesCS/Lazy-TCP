#ifndef ICMP_H
#define ICMP_H

#include "../network.h"
#include "../ip/ip.h"
#include <string.h>

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

void icmp_print(ICMP_Header *packet);

#endif