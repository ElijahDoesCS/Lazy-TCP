#ifndef TCP_TCB_H
#define TCP_TCB_H

#include <stdint.h>

typedef enum Con_State {
    TCP_SYN_RECEIVED,  // Received SYN, sent SYN-ACK, awaiting ACK
    TCP_ESTABLISHED,   // Connection open, data transfer enabled
    TCP_CLOSE_WAIT,    // Received FIN, awaiting local close
    TCP_CLOSING,       // Simultaneous close, awaiting final ACK
    TCP_LAST_ACK,      // Sent FIN after receiving FIN, awaiting ACK
    TCP_TIME_WAIT      // Both sides closed, waiting for delayed packets
} Con_State;

typedef struct ID {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t src_ip;
    uint32_t dst_ip;
} ID;

typedef struct Send_State {
    uint32_t unac;
    uint32_t next;
    uint32_t window;
    uint32_t iss;
} Send_State;

typedef struct Receive_State {
    uint32_t next;
    uint32_t window;
    uint32_t irs;
} Receive_State;

typedef struct TCB TCB;

struct TCB {
    ID id;

    Send_State    send;
    Receive_State recv;

    uint8_t  recv_buf[16384];
    uint32_t recv_size;
    uint32_t recv_consumed;

    Con_State state;

    // Going to implement this later
    // struct timeval retransmit_timer;

    TCB *next;
};

#endif