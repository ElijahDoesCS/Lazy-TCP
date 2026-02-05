#ifndef TCP_H
#define TCP_H

typedef enum TCP_State TCP_State; 
typedef enum TCP_Event TCP_Event; 
typedef struct TCP_Connection_ID TCP_Connection_ID;
typedef struct TCP_Send_State TCP_Send_State;
typedef struct TCP_Recieve_State TCP_Recieve_State;
typedef struct TCP_Server_Instance TCP_Server_Instance;
typedef struct TCP_IP_Pseudo_Header TCP_IP_Pseudo_Header;

typedef struct TCB TCB; // Holds per connection state

#include "../http/http.h"
#include "../network.h"
#include "../ip/ip.h"
#include "../../debug/debug.h"
#include <sys/time.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

enum TCP_State {
    TCP_SYN_RECEIVED,  // Got SYN, waiting for final ACK of handshake
    TCP_ESTABLISHED,   // Connected, can exchange data
    TCP_CLOSE_WAIT,    // They closed, we need to close too
    TCP_CLOSING,       // For simultaneous close
    TCP_LAST_ACK,      // We sent FIN, waiting for their ACK
    TCP_TIME_WAIT      // Wait n minutes after they close
};

struct TCP_Connection_ID {
    int src_ip;
    int src_port;
    int dst_ip;
    int dst_port;
};

struct TCP_Send_State {
    int unac;       // Unacknowledge sequence number
    int next;       // Bytes to send next
    int window;     // Maximum byte we can send beyond next
    int iss;        // Initial send sequence number
};

struct TCP_Recieve_State {
    int next;   // Recieve next ack number
    int window; // Recieve next window
    int irs;    // Initial recieve sequence number
};

struct TCB {
    TCP_Connection_ID id; // Four tuple
    TCP_State state;      // Connection state

    // Seqgment space
    TCP_Send_State send;
    TCP_Recieve_State recv;

    // Recieve byte buffer
    uint8_t recv_buffer[8192];
    int recv_size;
    int recv_consumed;

    // struct timeval retransmit_timer;

    struct TCB *next;
};

struct TCP_IP_Pseudo_Header {
    uint32_t src;
    uint32_t dst;
    uint8_t zero;
    uint8_t proto;
    uint16_t tcp_len; // The length of the tcp header + the datagram length
} __attribute__((packed));

struct TCP_Server_Instance {
    TCB *head;
    int size;
};

// Options
# define TCP_OPT_END     0
# define TCP_OPT_NOP     1
# define TCP_OPT_MSS     2

// TCP Flags
# define TCP_FIN  0x01
# define TCP_SYN  0x02
# define TCP_RST  0x04
# define TCP_PSH  0x08
# define TCP_ACK  0x10
# define TCP_URG  0x20
 
// Error codes
# define TCP_FATAL 1    // Fatal error kills server
# define TCP_SILENT 2   // Silent or graceful failure
# define TCP_UNREACHABLE 3 // Port unreachable

// Handle unsigned integer wrapping
# define SEQ_LT(a,b)  ((int32_t)((a)-(b)) < 0)
# define SEQ_LEQ(a,b) ((int32_t)((a)-(b)) <= 0)
# define SEQ_GT(a,b)  ((int32_t)((a)-(b)) > 0)
# define SEQ_GEQ(a,b) ((int32_t)((a)-(b)) >= 0)

/**
 * @brief return the unix time as an unsigned integer
 */
uint32_t tcp_get_ticks();

/**
 * @brief initialize the connection list
 * @param table pointer to the list we init
 */
void tcp_init_server_instance(TCP_Server_Instance *states);

/**
 * @param check if all of the elements of the structure are the same
 * @param id_a the first tcp connection structure
 * @param id_b the second tcp connection structure
 * @return true if they are the same, false if they are not
 */
bool tcp_compare_id(TCP_Connection_ID *id_a, TCP_Connection_ID *id_b);

/**
 * @brief updates the checksum of a tcp header
 * @param tcp_pack the tcp packet we are modifying
 * @param pseudo_ip the pseudo ip header the prepends the tcp header
 * @param returns true if we couldn't allocate memory for the pseudo packet
 */
void tcp_update_checksum(TCP_Header *tcp_pack, int tcp_pack_len, 
                         TCP_IP_Pseudo_Header *pseudo_ip,  int pseudo_len);

/**
 * @brief switch the source and destination ports in a tcp header
 * @param tcp_pack the packet that we are switch ports on
 */
void tcp_switch_port(TCP_Header *tcp_pack);

/**
 * @brief initialize and return a new TCB structure
 * @param id is the four tuple of the new block
 * @return the newly initialized TCB
 */
TCB *tcp_init_tcb(TCP_Connection_ID *id);

/**
 * @brief insert a new TCB into the state list or return if its found
 * @param tcb is the tcb we are inserting
 * @param states is the states list we are linking into
 */
void tcb_insert_tcb(TCB *tcb, TCP_Server_Instance *states);

/**
 * @brief search the linked list for the state block by connection id
 * @param id lets us index the list through source and dest
 * @param states the actual linked list that we are searching
 * @return the state block associated with the passed id, NULL if not found
 */
TCB *tcp_get_state(TCP_Connection_ID *id, TCP_Server_Instance *states);

/**
 * @brief send reset for a nonexistent connection
 * @param ip_pack the ip packet that we are modying
 * @param tcp_pack is the tcp packet content we are modifying
 */
void tcp_null_rst(IPv4_Header *ip_pack, TCP_Header *tcp_pack);

/**
 * @brief send a SYN ACK over a newly created connection
 * @param ip_pack the ip packet we are modifying
 * @param tcp_pack the tcp packet we are modifying
 * @param tcb the state of the connection we are sending a syn through
 */
void tcp_null_syn_ack(IPv4_Header *ip_pack, TCP_Header *tcp_pack, TCB *tcb);

/**
 * @brief remove a connection from the list of TCBs
 * @param states the list of states from which we are removing
 * @param tcb is the tcb we are removing from the list
 */
void tcp_remove_connection(TCP_Server_Instance *states, TCB *tcb);

/**
 * @brief send reset on an existing connection
 * @param tcp_pack the tcp packet we refer to drop the connection
 * @param tcb the state of  the connection that recieved a reset
 */
bool tcp_got_rst(TCP_Header *tcp_pack, TCB *tcb, TCP_Server_Instance *states);

/**
 * @brief send an acknowledgement to the the client's fin message
 * @param ip_pack is the ip packet we are modifying
 * @param tcp_pack the tcp packet header we are modifying
 * @param tcb is the state of the connection we are acting on
 */
void tcp_send_ack(IPv4_Header *ip_pack, TCP_Header *tcp_pack, TCB *tcb, char *body, int body_len);

/**
 * @brief get the data off the connection and advance the recieve state
 * @param ip_pack the ip packet of the received data
 * @param tcp_pack the tcp packet of the recieved data
 * @param tcb is the state of the connection on which we are reading
 */
int tcp_handle_data(IPv4_Header *ip_pack, TCP_Header *tcp_pack, TCB *tcb);

// /**
//  * @brief send a finish sequence immediately after recieving one
//  * @param ip_pack the ip pack we are modifying
//  * @param tcp_pack is the tcp packet we are modifying
//  * @param tcb is the state of the connection we are finishing
//  */
// void tcp_send_fin(IPv4_Header *ip_pack, TCP_Header *tcp_pack, TCB *tcb);

/**
 * @brief Dispatch the TCP packet to its corresponding function
 * @param ip_pack is the IP packet that wraps the TCP packet
 * @param tcp_pack is the tcp packet and all its contents
 * @return true if there is an error
 */
int tcp_dispatch(bool verbose, IPv4_Header *ip_pack, TCP_Header *tcp_pack, TCP_Server_Instance *states);

#endif