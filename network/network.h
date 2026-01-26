#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>

# define IPV4_ICMP 0 
# define IPV4_TCP 1
# define IPV6 2
# define VER_UNDF 3
# define PROTO_UNDF 4

typedef struct IPv4_Header IPv4_Header;
typedef struct ICMP_Header ICMP_Header;
typedef struct TCP_Header TCP_Header;
typedef struct Raw_Header Raw_Header;

struct IPv4_Header {
    uint8_t  version_ihl; // Combined Version and IHL
    uint8_t  tos; // Type of service (Precedence: throughput, reliabality, delay) 
    uint16_t len; // Total header + payload length
    uint16_t id; // Transmission unit ID for fragmentation
    uint16_t flags_frag; // Bits 0-3 indicate the fragment control, and the rest indicate the fragment offset
    uint8_t  ttl; // Time to live in the internet system
    uint8_t  proto; // Protocol implemented by the next layer
    uint16_t checksum; // 1's complement of all header fields verifies header integrity
    uint32_t src; // Source IP address
    uint32_t dst; // Destination IP address
} __attribute__((packed)); // Prevents the compiler from adding padding

struct ICMP_Header {
    uint8_t type; // Request or response
    uint8_t code; // The type of ICMP message
    uint16_t checksum; // Checksum of the ICMP message
    uint16_t identifier; // To aid in matching replies
    uint16_t sequence_no; // To aid in matching replies
} __attribute__((packed));

struct TCP_Header {
    
} __attribute__((packed));

struct Raw_Header {
    char *bytes;
    int pack_len;
};

/**
 * @brief deserialize the bytes of the packet
 * @param packet bytes of the ip packet
 * @return the requested operation
 */
int packet_deserialize(IPv4_Header *packet);

/**
 * @brief compute the header checksum of the entire packet
 * @param pack is the packet we are computing the checksum on
 * @param pack_len is the length of the packet indexed from its base
 * @return the checksum value
 */
uint16_t packet_checksum(void *pack, int pack_len);

void raw_packet_print(char *packet, int pack_len);

#endif
