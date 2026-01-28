#include "icmp.h"

void icmp_update_checksum(ICMP_Header *icmp_pack, int pack_len) {
    icmp_pack->checksum = 0;
    icmp_pack->checksum = packet_checksum(icmp_pack, pack_len);
}

void icmp_echo_reply(IPv4_Header *ip_pack, ICMP_Header *icmp_pack) {
    icmp_pack->type = 0; // Echo Reply
    icmp_pack->code = 0;
    
    int ihl_bytes = (ip_pack->version_ihl & 0x0f) * 4;
    int icmp_len = ntohs(ip_pack->len) - ihl_bytes;
    icmp_update_checksum(icmp_pack, icmp_len);

    ip_swap_dst(ip_pack);
    ip_update_checksum(ip_pack, ihl_bytes);
}

void icmp_dispatch(IPv4_Header *ip_pack, ICMP_Header *icmp_pack) {
    if (icmp_pack->type == 8) { // Echo request
        icmp_echo_reply(ip_pack, icmp_pack);
    }
    else if (icmp_pack->type == 3) { // Destination unreachable
        // For furth implementation
    }
}
