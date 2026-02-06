#ifndef NETWORK_H
#define NETWORK_H

// ═══════════════════════════════════════════════════════════════════════════
// Network Layer - Protocol Dispatch
// ═══════════════════════════════════════════════════════════════════════════
//
// Top-level demultiplexer that validates incoming IPv4 packets and routes
// them to protocol handlers (TCP, ICMP). Also provides the checksum function
// used across IP and transport layers.
//

# define PROTO_TCP  6
# define PROTO_ICMP 1

# define MTU_SIZE 1500

#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>

uint16_t checksum(uint8_t *buf, int len);

/**
 * @brief Demultiplex an incoming IP packet to the appropriate protocol handler.
 * @param fd      File descriptor of the TUN device (for direct writes)
 * @param buf     Packet buffer, modified in place if a response is built
 * @param len     Length of the incoming packet in bytes
 * @param verbose Debug print option
 * @return        Size of response packet in buf, or 0 if no response / already written
 */
int net_demux(int fd, uint8_t *buf, int len);

#endif