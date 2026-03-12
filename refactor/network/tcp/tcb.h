#ifndef TCB_H
#define TCB_H

// ═══════════════════════════════════════════════════════════════════════════
// TCP Transport Layer - Per-Connection State
// ═══════════════════════════════════════════════════════════════════════════
//
// Defines the TCB (Transmission Control Block), the core data structure
// tracking send/receive sequence numbers, connection ID, and receive buffer
// for each active TCP connection.
//

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct TCP TCP;

#define TCP_RCVBUF  16384
#define TCP_WIN_DEF 8192

# define SEQ_LT(a,b)  ((int32_t)((a)-(b)) < 0)  // If a is less than b
# define SEQ_LEQ(a,b) ((int32_t)((a)-(b)) <= 0) // If a is let b
# define SEQ_GT(a,b)  ((int32_t)((a)-(b)) > 0)  // If a is greater than b
# define SEQ_GEQ(a,b) ((int32_t)((a)-(b)) >= 0) // If a is get b

typedef enum Edge {
    EVT_SYN,          // Valid new connection request
    EVT_DATA,         // Segment carrying payload data
    EVT_ACK,          // Pure acknowledgment
    EVT_FIN,          // Peer initiated close
    EVT_RST,          // Reset on an active connection
    EVT_DROP,         // Invalid segment, silently drop
    EVT_SEND_RST,     // No matching connection, send reset
} Edge;

typedef enum Con_State {
    CON_CLOSED,       // State block is initialized but connection is not open
    CON_SYN_RECEIVED, // Received SYN, sent SYN-ACK, awaiting ACK
    CON_ESTABLISHED,  // Connection open, data transfer enabled
    CON_CLOSE_WAIT,   // Received FIN, awaiting local close
    CON_CLOSING,      // Simultaneous close, awaiting final ACK
    CON_LAST_ACK,     // Sent FIN after receiving FIN, awaiting ACK
    CON_TIME_WAIT     // Both sides closed, waiting for delayed packets
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
 * @return false if the segment contains an invalid sequence number
 */
bool tcb_state_update(TCP *tcp, TCB *tcb, Edge edge);

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