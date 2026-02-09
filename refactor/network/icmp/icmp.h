#ifndef ICMP_H
#define ICMP_H

// ═══════════════════════════════════════════════════════════════════════════
// ICMP Protocol Handler
// ═══════════════════════════════════════════════════════════════════════════
//
// Handles ICMP echo requests (ping) and generates protocol unreachable
// responses for unhandled IP protocols. All operations modify packet buffers
// in place, working on both IP and ICMP headers as needed.
//
// Supported ICMP types:
//   - Type 8 (Echo Request)  → Type 0 (Echo Reply)
//   - Type 3 (Dest Unreach)  → Generated for unhandled protocols
//

typedef struct ICMP ICMP;

// Type fields
# define ICMP_ECHO              8
# define ICMP_DST_UNREACHABLE   3

// Code fields
# define ICMP_PROTO_UNREACHABLE 2

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

struct ICMP {
    uint8_t  type;         // Request or response
    uint8_t  code;         // The type of ICMP message
    uint16_t checksum;     // Checksum of the ICMP message
    uint16_t identifier;   // Identify conversation
    uint16_t sequence_no;  // Note the packet index per convo
} __attribute__((packed));


void icmp_checksum(ICMP *icmp, int len);

/**
 * @brief Validate ICMP message integrity.
 * @param icmp Pointer to the ICMP header
 * @param len  Length of the ICMP message (header + payload) in bytes
 * @return     1 if valid, 0 if malformed (caller must silently drop)
 */
int icmp_validate(const ICMP *icmp, int len);

/**
 * @brief Build a destination unreachable ICMP response.
 * @param buf    Packet buffer containing original packet, overwritten with response
 * @param len    Length of original packet
 * @param offset IP header length (where ICMP data would start)
 * @return       Total length of response packet, or 0 on failure
 */
int icmp_dest_unreachable(uint8_t *buf, int len, int offset);

/**
 * @brief Convert echo request to echo reply in place.
 * @param buf    Packet buffer containing echo request
 * @param len    Total packet length
 * @param offset IP header length
 * @return       Total length of response packet (unchanged from len)
 */
int icmp_echo_reply(uint8_t *buf, int len, int offset);

/**
 * @brief Top-level ICMP dispatcher.
 * @param buf    Packet buffer starting at IP header
 * @param len    Total packet length
 * @param offset IP header length
 * @return       Size of response to send, or 0 if no response
 */
int icmp_dispatch(uint8_t *buf, int len, int offset);

#endif