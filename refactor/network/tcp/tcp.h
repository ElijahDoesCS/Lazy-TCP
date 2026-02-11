#ifndef TCP_H
#define TCP_H

#include <stdbool.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <arpa/inet.h>

typedef struct {
    uint16_t src_port;     // Source port
    uint16_t dst_port;     // Destination port
    uint32_t seq_num;      // Sequence number
    uint32_t ack_num;      // Acknowledgment number
    uint8_t  data_offset;  // Data offset (header length in 32-bit words) << 4
    uint8_t  flags;        // Control flags: URG|ACK|PSH|RST|SYN|FIN
    uint16_t window;       // Receive window size
    uint16_t checksum;     // Header + pseudo-header checksum
    uint16_t urgent_ptr;   // Urgent pointer (if URG flag set)
} __attribute__((packed)) TCP;

// # define SEQ_LT(a,b)  ((int32_t)((a)-(b)) < 0)
// # define SEQ_LEQ(a,b) ((int32_t)((a)-(b)) <= 0)
// # define SEQ_GT(a,b)  ((int32_t)((a)-(b)) > 0)
// # define SEQ_GEQ(a,b) ((int32_t)((a)-(b)) >= 0)

# define FIN  0x01
# define SYN  0x02
# define RST  0x04
# define PSH  0x08
# define ACK  0x10

typedef struct {
    bool fin;
    bool syn;
    bool rst;
    bool psh;
    bool ack;
} Flags;

typedef struct {
    uint32_t src;
    uint32_t dst;
    uint8_t zero;
    uint8_t proto;
    uint16_t tcp_len;
} __attribute__((packed)) Pseudo;

/**
 * @brief Calculate the TCP checksum
 * @param buf    Opaque pointer to the org packet
 * @param len    Full length of the packet
 * @param offset Offset into the tcp header
 * @return The TCP checksum value
 */
uint16_t tcp_checksum(uint8_t *buf, int len, int offset);

/**
 * @brief Dispatch the TCP packet to its corresponding function
 * @param fd     The tun file descriptor for multi-write actions
 * @param buf    An opaque pointer to the packet contents
 * @param len    The length of the entire packet
 * @param offset The offset into the TCP segment
 * @return The number of bytes still writeable to the device 
 */
int tcp_dispatch(int fd, uint8_t *buf, int len, int offset);

#endif