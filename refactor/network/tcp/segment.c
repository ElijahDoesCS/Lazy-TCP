#include <stdlib.h>
#include <stdio.h>

#include "./segment.h"

#include "../ip.h"
#include "./tcp.h"
#include "./tcb.h"
#include "./hash.h"

static void tcp_swap_port(TCP *tcp) {
    uint16_t src_port = tcp->src_port;
    tcp->src_port = tcp->dst_port;
    tcp->dst_port = src_port;
}

int tcp_syn_ack(uint8_t *state, uint8_t *buf, int len, int offset) {
    IPv4 *ip = (IPv4 *) buf;
    TCP *tcp = (TCP *) (buf + offset);
    TCB *tcb = (TCB *) state;

    // Swap destinations
    ip_swap_dst(ip);
    tcp_swap_port(tcp);

    // Set TCP header fields
    int tcp_hlen = (tcp->data_offset >> 4) * 4;

    tcp->seq_num = htonl(tcb->send.iss);
    tcp->ack_num = htonl(tcb->recv.next);
    tcp->data_offset = (tcp_hlen / 4) << 4;
    tcp->flags = SYN | ACK;
    tcp->window = htons(tcb->recv.window);
    tcp->urgent_ptr = 0;

    // Update the IP checksum
    int ip_hlen = (ip->version_ihl & 0xf) * 4;  
    ip->len = htons(ip_hlen + tcp_hlen);
    ip_checksum(ip);

    tcp->checksum = tcp_checksum(buf, len, offset);

    return ip_hlen + tcp_hlen;
}

int tcp_rst(uint8_t *state, uint8_t *buf, int len, int offset) {
    IPv4 *ip = (IPv4 *) buf;
    TCP *tcp = (TCP *) (buf + offset);
    TCB *tcb = (TCB *) state;

    int tcp_hlen, ip_hlen, tot_len, seg_len;
    int seq_no, ack_no, pl_flags;

    Flags flags;
    flags.ack = tcp->flags & ACK; 
    flags.fin = tcp->flags & FIN;
    flags.syn = tcp->flags & SYN;
    
    tcp_hlen = (tcp->data_offset >> 4) * 4; 
    ip_hlen  = (ip->version_ihl & 0xf) * 4;
    tot_len  = ntohs(ip->len);

    // Get the length of the TCP payload length
    seg_len  = tot_len - ip_hlen - tcp_hlen;

    if (flags.ack) {
        seq_no = htons(tcp->ack_num);
        pl_flags = RST;
    }
    else {
        seg_len += (flags.fin + flags.syn);
        ack_no = ntohl(tcp->seq_num) + seg_len; 
    }

    ip_swap_dst(ip);
    tcp_swap_port(tcp);

    ip->len = htons(ip_hlen + tcp_hlen);

    tcp->seq_num = htonl(seq_no);
    tcp->ack_num = htonl(ack_no);
    tcp->data_offset = (tcp_hlen / 4) << 4;
    tcp->flags = pl_flags;
    tcp->window = 0;
    tcp->urgent_ptr = 0;

    hash_delete(tcb);

    return ip_hlen + tcp_hlen;
}





