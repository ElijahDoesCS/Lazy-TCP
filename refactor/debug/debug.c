#include "./debug.h"
#include "../tun/tun_device.h"
#include "../network/ip.h"
#include "../network/network.h"
#include "../network/icmp/icmp.h"

void icmp_print(ICMP *icmp) {
    printf("---------------- ICMP Header ----------------\n");

    // Type and Code are single bytes, no ntohs needed
    printf(" Type     : %u ", icmp->type);

    // Provide a little "Translation" for common types
    switch(icmp->type) {
        case 3:  printf("(Destination Unreachable)\n"); break;
        case 8:  printf("(Echo Request)\n"); break;
        case 11: printf("(Time Exceeded)\n"); break;
        default: printf("(Other)\n"); break;
    }

    printf(" Code     : %u\n", icmp->code);
    printf(" Checksum : 0x%04X\n", ntohs(icmp->checksum));

    // These 4 bytes are specific to Echo/Reply (RFC 792)
    printf(" ID       : %u (0x%04X)\n", ntohs(icmp->identifier), ntohs(icmp->identifier));
    printf(" Sequence : %u (0x%04X)\n", ntohs(icmp->sequence_no), ntohs(icmp->sequence_no));
    printf("---------------------------------------------\n"); 
}

char *ip_proto(uint8_t proto) {
    if (proto == PROTO_TCP) return "TCP";
    if (proto == PROTO_ICMP) return "ICMP";
    else return "Not implemented\n";
}

void ip_print(IPv4 *ip) {
    uint8_t version = (ip->version_ihl >> 4);
    uint8_t ihl = (ip->version_ihl & 0x0F);
    uint8_t header_len = ihl * 4;

    uint16_t ff = ntohs(ip->flags_offset);
    uint8_t flags = (ff >> 13);
    uint16_t offset = (ff & 0x1FFF);

    char src_str[INET_ADDRSTRLEN];
    char dst_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ip->src), src_str, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(ip->dst), dst_str, INET_ADDRSTRLEN);

    printf("================ IPv4 Header ================\n");
    printf(" Version: %u | IHL: %u (%u bytes)\n", version, ihl, header_len);
    printf(" ToS    : 0x%02X\n", ip->tos);
    printf(" Length : %u bytes\n", ntohs(ip->len));
    printf(" ID     : 0x%04X (%u)\n", ntohs(ip->id), ntohs(ip->id));
    printf(" Flags  : 0x%X (Res:%d DF:%d MF:%d)\n", 
            flags, (flags >> 2) & 1, (flags >> 1) & 1, flags & 1);
    printf(" Offset : %u\n", offset);
    printf(" TTL    : %u\n", ip->ttl);
    printf(" Proto  : %u (0x%02X) - %s\n", ip->proto, ip->proto, ip_proto(ip->proto));
    printf(" Check  : 0x%04X\n", ntohs(ip->checksum));
    printf(" Source : %s\n", src_str);
    printf(" Dest   : %s\n", dst_str);
    printf("=============================================\n");
}

void tun_print(Tun *tun) {
    printf("TUN device:\n");
    printf("    Device name    : %s\n", tun->name);
    printf("    IP             : %s\n", tun->ip);
    printf("    File desciptor : %d\n", tun->fd);
}

void raw_print(uint8_t *buf, int len) {
    printf("\n=== Packet (%d bytes) ===\n", len);
    
    for (int i = 0; i < len; i += 16) {
        printf("%04x: ", i);
        
        for (int j = 0; j < 16; j++) {
            if (i + j < len) {
                printf("%02x ", (unsigned char)buf[i + j]);
            } else {
                printf("   ");  
            }
            
            if (j == 7) printf(" ");
        }
        
        printf(" |");
        
        for (int j = 0; j < 16 && i + j < len; j++) {
            unsigned char c = buf[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        
        printf("|\n");
    }
    printf("\n");
}