#include "./tcp.h"
#include "../ip.h"

int tcp_dispatch(uint8_t *buf, int len, int offset, int fd) {
    IPv4 *ip = (IPv4 *) buf;
    TCP *tcp = (TCP *) (buf + offset);

    ID id = {
        .src_port = ntohs(tcp->src_port),
        .dst_port = noths(tcp->dst_port),
        .src_ip   = nothl(ip->src),
        .dst_ip   = ntohl(ip->dst) 
    };

    Flags flags = {
        .fin = (tcp->flags) & FIN,
        .syn = (tcp->flags) & SYN,
        .rst = (tcp->flags) & RST,
        .psh = (tcp->flags) & PSH,
        .ack = (tcp->flags) & ACK
    };

    // Get the tcb if it exists
    // TCB *con = tcp_tcb_get(&id, );    

    // Validate the TCP packet


    // Get the  flags
    // Get the connection ID and see if we have a TCB for it
    // If the connection is null
        // If we get a rst, drop it
        // If we get just a SYN we create a new connection
        // Otherwise we send a reset ourselves
    // If the connection is open
        
        // If we got a reset, remove if seq no is valid
        // If we got a syn, resend our syn/ack
        // If it's an ack, move to established if ack is valid
        // 
}