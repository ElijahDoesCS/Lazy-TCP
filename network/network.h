#ifndef NETWORK_H
#define NETWORK_H

# define IPV4_ICMP 0 
# define IPV4_TCP 1
# define IPV6 2
# define VER_UNDF 3
# define PROTO_UNDF 4

# define IP_TCP_PROTO 6
# define IP_ICMP_PROTO 1

# define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct IPv4_Header IPv4_Header;
typedef struct ICMP_Header ICMP_Header;
typedef struct TCP_Header TCP_Header;
typedef struct Raw_Header Raw_Header;

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct IPv4_Header {
    uint8_t  version_ihl; // Combined Version and IHL
    uint8_t  tos;         // Type of service (Precedence: throughput, reliabality, delay) 
    uint16_t len;         // Header + payload length
    uint16_t id;          // Transmission unit ID for fragmentation
    uint16_t flags_frag;  // Bits 0-3 indicate fragment control, rest indicate fragment offset
    uint8_t  ttl;         // Time to live by hop count
    uint8_t  proto;       // Next layer protocol
    uint16_t checksum;    // 1's complement of all header fields verifies header integrity
    uint32_t src;         // Source IP address
    uint32_t dst;         // Destination IP address
    // Options
} __attribute__((packed));

struct ICMP_Header {
    uint8_t type;         // Request or response
    uint8_t code;         // The type of ICMP message
    uint16_t checksum;    // Checksum of the ICMP message
    uint16_t identifier;  // To aid in matching replies
    uint16_t sequence_no; // To aid in matching replies
    // Options
} __attribute__((packed));

struct TCP_Header {
    uint16_t src_port;     // Source port
    uint16_t dst_port;     // Destination port
    uint32_t seq_num;      // Sequence number to advance state
    uint32_t ack_num;      // Acknowledge number to update state
    uint8_t  data_offset;  // Upper 4 bits, lower 4 reserved
    uint8_t  flags;        // URG|ACK|PSH|RST|SYN|FIN
    uint16_t window;       // The maximum acceptable send beyond next
    uint16_t checksum;     // Verify header integrity, computed w/ ip
    uint16_t urgent_ptr;   // Byte ptr to urgent data in send queue
    // uint16_t options

    // Options
} __attribute__((packed));

struct Raw_Header {
    char *bytes;
    int pack_len;
};

/**
 * @brief deserialize the bytes of the packet
 * @param packet bytes of the ip packet
 * @return the requested operation type
 */
int packet_deserialize(IPv4_Header *packet);

/**
 * @brief compute the header checksum of the entire packet
 * @param pack is the packet we are computing the checksum on
 * @param pack_len is the length of the packet indexed from its base
 * @return the checksum value
 */
uint16_t packet_checksum(void *pack, int pack_len);

#endif
