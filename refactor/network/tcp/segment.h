#ifndef SEGMENT_H
#define SEGMENT_H

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

int tcp_rst(uint8_t *state, uint8_t *buf, int len, int offset);
int tcp_ack(uint8_t *state, uint8_t *buf, int len, int offset);
int tcp_write(uint8_t *state, uint8_t *buf, int len, int offset);

#endif