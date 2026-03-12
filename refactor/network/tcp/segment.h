#ifndef SEGMENT_H
#define SEGMENT_H

// ═══════════════════════════════════════════════════════════════════════════
// TCP Transport Layer - Segment Construction
// ═══════════════════════════════════════════════════════════════════════════
//
// Builds and transmits outgoing TCP segments. Each function modifies the
// provided packet buffer in-place. Some functions handle their own writes
// internally and return 0 to indicate no further send is needed by the caller.
//

#include <stdint.h>

/**
 * @brief Generate and transmit a SYN-ACK in response to a valid SYN.
 *
 * @param state   Pointer to the connection's TCB.
 * @param buf     Pointer to the start of the received packet buffer.
 * @param len     Total length of the received packet.
 * @param offset  Byte offset from the IP header to the TCP header.
 *
 * @return 0 on success, negative value on error.
 */
int tcp_syn_ack(uint8_t *state, uint8_t *buf, int len, int offset);

/**
 * @brief Transmit a RST in response to an invalid segment, deleting the corresponding TCB.
 * 
 * @param state   Pointer to the connection's TCB.
 * @param buf     Pointer to the start of the received packet buffer.
 * @param len     Total length of the received packet.
 * @param offset  Byte offset from the IP header to the TCP header.
 * 
 * @return 0 on success, negative value on error.
 */
int tcp_rst(uint8_t *state, uint8_t *buf, int len, int offset);

int tcp_ack(uint8_t *state, uint8_t *buf, int len, int offset);
int tcp_write(uint8_t *state, uint8_t *buf, int len, int offset);

#endif