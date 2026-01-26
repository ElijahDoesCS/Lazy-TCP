#include "../network.h"
#include "./ip.h"

void ip_print(IPv4_Header *packet) {
    // 1. Extract Version and IHL
    uint8_t version = (packet->version_ihl >> 4);
    uint8_t ihl = (packet->version_ihl & 0x0F);
    uint8_t header_len = ihl * 4;

    // 2. Extract Flags and Offset
    uint16_t ff = ntohs(packet->flags_frag);
    uint8_t flags = (ff >> 13);
    uint16_t offset = (ff & 0x1FFF);

    // 3. Convert IPs to human-readable strings
    char src_str[INET_ADDRSTRLEN];
    char dst_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(packet->src), src_str, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(packet->dst), dst_str, INET_ADDRSTRLEN);

    printf("================ IPv4 Header ================\n");
    printf(" Version: %u | IHL: %u (%u bytes)\n", version, ihl, header_len);
    printf(" ToS    : 0x%02X\n", packet->tos);
    printf(" Length : %u bytes\n", ntohs(packet->len));
    printf(" ID     : 0x%04X (%u)\n", ntohs(packet->id), ntohs(packet->id));
    printf(" Flags  : 0x%X (Res:%d DF:%d MF:%d)\n", 
            flags, (flags >> 2) & 1, (flags >> 1) & 1, flags & 1);
    printf(" Offset : %u\n", offset);
    printf(" TTL    : %u\n", packet->ttl);
    printf(" Proto  : %u (0x%02X)\n", packet->proto, packet->proto);
    printf(" Check  : 0x%04X\n", ntohs(packet->checksum));
    printf(" Source : %s\n", src_str);
    printf(" Dest   : %s\n", dst_str);
    printf("=============================================\n");
}
