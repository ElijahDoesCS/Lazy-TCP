#ifndef IP_H
#define IP_H

#include "../network.h"

/**
 * @brief Swap the destination and source ip insie the ip header
 * @param ip_pack the packet we are swapping the addresses of
 */
void ip_swap_dst(IPv4_Header *ip_pack);

/**
 * @brief Update the checksum value of the ip packet
 * @param ip_pack the packet we are updating the checksum of
 * @param pack_len the length of the packet we are updating
 */
void ip_update_checksum(IPv4_Header *ip_pack, int pack_len);

#endif