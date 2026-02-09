#ifndef IP_H
#define IP_H

// ═══════════════════════════════════════════════════════════════════════════
// IPv4 Header Structure and Validation
// ═══════════════════════════════════════════════════════════════════════════
//
// Defines the IPv4 packet header structure and provides validation and
// manipulation functions. No IP options parsing or fragmentation support.
// Validation checks version, IHL bounds, length consistency, and header
// checksum using ones-complement algorithm.
//

typedef struct IPv4 IPv4;

#include <stdint.h>
#include <arpa/inet.h>

struct IPv4 {
    uint8_t  version_ihl;  // Upper 4: version, Lower 4: header length in 32-bit words
    uint8_t  tos;          // Type of service
    uint16_t len;          // Total length (header + payload)
    uint16_t id;           // Identification
    uint16_t flags_offset; // Upper 3: flags, Lower 13: fragment offset
    uint8_t  ttl;          // Time to live
    uint8_t  proto;        // Protocol (6=TCP, 1=ICMP)
    uint16_t checksum;     // Header checksum
    uint32_t src;          // Source IP
    uint32_t dst;          // Destination IP
} __attribute__((packed));

/**
 * @brief Validate IPv4 header integrity.
 * @param ip   Generic pointer to the IPv4 header
 * @param len  Total packet length in bytes
 * @return     1 if valid, 0 if malformed (caller must silently drop)
 */
int ip_validate(const IPv4 *ip, int len);

void ip_swap_dst(IPv4 *ip);
void ip_checksum(IPv4 *ip);

#endif
