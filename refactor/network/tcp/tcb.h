#ifndef TCB_H
#define TCB_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct TCP TCP;

#define TCP_RCVBUF  16384
#define TCP_WIN_DEF 8192

typedef enum Edge {
    TCP_EVT_SYN,          // Valid new connection request
    TCP_EVT_DATA,         // Segment carrying payload data
    TCP_EVT_ACK,          // Pure acknowledgment
    TCP_EVT_FIN,          // Peer initiated close
    TCP_EVT_RST,          // Reset on an active connection
    TCP_EVT_DROP,         // Invalid segment, silently drop
    TCP_EVT_SEND_RST,     // No matching connection, send reset
} Edge;

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

typedef struct TCB {
    ID id;

    Send_State    send;
    Receive_State recv;

    uint8_t  recv_buf[16384];
    uint32_t recv_size;
    uint32_t recv_consumed;

    Con_State state;

    // Going to implement this later
    // struct timeval retransmit_timer;
} TCB;

/**
 * @brief  Update the state of a valid TCB 
 */
void tcb_state_update(TCP *tcp, TCB *tcb, Edge edge);

/**
 * @brief  Check if TCB IDs are matching
 * @return True if they match
 */
bool tcb_id_match(ID *a, ID *b);

/**
 * @brief  Initialize a new TCB
 * @return Pointer to TCB if successful, 
 *         NULL on failure
 */
TCB *tcb_init(ID id);

#endif