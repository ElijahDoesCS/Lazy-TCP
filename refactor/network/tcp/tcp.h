#ifndef TCP_H
#define TCP_H

typedef enum Con_State Con_State;

typedef struct TCP TCP;
typedef struct ID ID;
typedef struct Flags Flags;
typedef struct Send_State Send_State;
typedef struct Recieve_State Recieve_State;
typedef struct TCB TCB;

#include <stdbool.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/time.h>

enum Con_State {
    TCP_SYN_RECEIVED,  // Received SYN, sent SYN-ACK, awaiting ACK
    TCP_ESTABLISHED,   // Connection open, data transfer enabled
    TCP_CLOSE_WAIT,    // Received FIN, awaiting local close
    TCP_CLOSING,       // Simultaneous close, awaiting final ACK
    TCP_LAST_ACK,      // Sent FIN after receiving FIN, awaiting ACK
    TCP_TIME_WAIT      // Both sides closed, waiting for delayed packets
};

struct TCP {
    uint16_t src_port;     // Source port
    uint16_t dst_port;     // Destination port
    uint32_t seq_num;      // Sequence number
    uint32_t ack_num;      // Acknowledgment number
    uint8_t  data_offset;  // Data offset (header length in 32-bit words) << 4
    uint8_t  flags;        // Control flags: URG|ACK|PSH|RST|SYN|FIN
    uint16_t window;       // Receive window size
    uint16_t checksum;     // Header + pseudo-header checksum
    uint16_t urgent_ptr;   // Urgent pointer (if URG flag set)

} __attribute__((packed));

struct ID {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t src_ip;
    uint32_t dst_ip;
};

struct Flags {
    bool fin;
    bool syn;
    bool rst;
    bool psh;
    bool ack;
};

struct Send_State {
    uint32_t unac;
    uint32_t next;
    uint32_t window;
    uint32_t iss;
};

struct Recieve_State {
    uint32_t next;
    uint32_t window;
    uint32_t irs;
};

struct TCB {
    ID id;

    Send_State    send;
    Recieve_State recv;

    uint8_t  recv_buf[16384];
    uint32_t recv_size;
    uint32_t recv_consumed;

    // Going to implement this later
    // struct timeval retransmit_timer;

    TCB *next;
};

# define FIN  0x01
# define SYN  0x02
# define RST  0x04
# define PSH  0x08
# define ACK  0x10

int tcp_dispatch(uint8_t *buf, int len, int offset, int fd);

#endif