#include "../network.h"
#include "./ip.h"

void ip_swap_dst(IPv4_Header *ip_pack) {
    uint32_t src = ip_pack->src;
    ip_pack->src = ip_pack->dst;
    ip_pack->dst = src;
}

void ip_update_checksum(IPv4_Header *ip_pack, int pack_len) {
    ip_pack->checksum = 0;
    ip_pack->checksum = packet_checksum(ip_pack, pack_len);
}
