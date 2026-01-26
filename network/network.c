#include "./network.h"

uint16_t packet_checksum(void *icmp_pack, int pack_len) {
    uint32_t sum = 0;
    uint16_t *ptr = (uint16_t *) icmp_pack;

    // Sum up shorts
    while (pack_len > 1) {
        sum += *ptr++;
        pack_len -= 2;
    }

    // Add leftover byte
    if (pack_len > 0) {
        sum += *(uint8_t *)ptr;
    }

    // Add the end around carry
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)~sum;
}

int packet_deserialize(IPv4_Header *packet) {
    int ip_version = packet->version_ihl >> 4;
    int ihl = packet->version_ihl & 0xf;

    bool v4 = false;
    if (ip_version == 4) v4 = true;
    else if (ip_version == 6 && !v4) return IPV6;
    else if (!v4) return VER_UNDF;

    // Header length must be at least five bytes (RFC)
    if (ihl < 5) return PROTO_UNDF;

    // Have to figure out whether or not this is ICMP or HTTP
    if (packet->proto == 1) return IPV4_ICMP;
    else if (packet->proto == 6) return IPV4_TCP;
    else return PROTO_UNDF;
}

void raw_packet_print(char *packet, int pack_len) {
    printf("\n=== Packet (%d bytes) ===\n", pack_len);
    
    for (int i = 0; i < pack_len; i += 16) {
        // Print offset
        printf("%04x: ", i);
        
        // Print hex bytes (16 per line)
        for (int j = 0; j < 16; j++) {
            if (i + j < pack_len) {
                printf("%02x ", (unsigned char)packet[i + j]);
            } else {
                printf("   ");  // Padding for incomplete lines
            }
            
            // Extra space between 8-byte groups
            if (j == 7) printf(" ");
        }
        
        printf(" |");
        
        // Print ASCII representation
        for (int j = 0; j < 16 && i + j < pack_len; j++) {
            unsigned char c = packet[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        
        printf("|\n");
    }
    printf("\n");
}
