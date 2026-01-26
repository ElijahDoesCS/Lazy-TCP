#include "icmp.h"

void icmp_echo_reply(IPv4_Header *ip_pack, ICMP_Header *icmp_pack) {
    icmp_pack->type = 0; // Echo Reply
    icmp_pack->code = 0;
    
    // Calculate checksum for ICMP
    int ihl_bytes = (ip_pack->version_ihl & 0x0f) * 4;
    int icmp_len = ntohs(ip_pack->len) - ihl_bytes;
    
    icmp_pack->checksum = 0;
    icmp_pack->checksum = packet_checksum(icmp_pack, icmp_len);

    // Swap src and dst ip
    uint32_t src = ip_pack->src;
    ip_pack->src = ip_pack->dst;
    ip_pack->dst = src;

    // Update the IP checksum
    ip_pack->checksum = 0;
    ip_pack->checksum = packet_checksum(ip_pack, ihl_bytes);
}

void icmp_dispatch(IPv4_Header *ip_pack, ICMP_Header *icmp_pack) {
    if (icmp_pack->type == 8) { // Echo request
        icmp_echo_reply(ip_pack, icmp_pack);
    }
    else if (icmp_pack->type == 3) { // Destination unreachable
        // Implement the destination unreachable function
    }

    // Space for other specifications
}

void icmp_print(ICMP_Header *packet) {
    printf("---------------- ICMP Header ----------------\n");
    
    // Type and Code are single bytes, no ntohs needed
    printf(" Type     : %u ", packet->type);
    
    // Provide a little "Translation" for common types
    switch(packet->type) {
        case 0:  printf("(Echo Reply)\n"); break;
        case 3:  printf("(Destination Unreachable)\n"); break;
        case 8:  printf("(Echo Request)\n"); break;
        case 11: printf("(Time Exceeded)\n"); break;
        default: printf("(Other)\n"); break;
    }

    printf(" Code     : %u\n", packet->code);
    printf(" Checksum : 0x%04X\n", ntohs(packet->checksum));
    
    // These 4 bytes are specific to Echo/Reply (RFC 792)
    printf(" ID       : %u (0x%04X)\n", ntohs(packet->identifier), ntohs(packet->identifier));
    printf(" Sequence : %u (0x%04X)\n", ntohs(packet->sequence_no), ntohs(packet->sequence_no));
    printf("---------------------------------------------\n");
}

