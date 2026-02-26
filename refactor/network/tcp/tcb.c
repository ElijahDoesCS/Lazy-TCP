#include "./tcb.h"
#include "./tcp.h"
#include "../ip.h"

#include <stdio.h>
#include <stdlib.h>

void tcb_state_update(TCP *tcp, TCB *tcb, Edge edge) {
    switch (edge) {
        case TCP_EVT_SYN:
            tcb->recv.irs = ntohl(tcp->seq_num);
            tcb->recv.next = tcb->recv.irs + 1; // Expect ISN + 1
            tcb->send.window = ntohs(tcp->window);
            tcb->send.iss = tcp_gen_iss();
            tcb->send.next = tcb->send.iss + 1; // Next seq no we'll send
            tcb->send.unac = tcb->send.iss; // Nothing acknoledged yet
            break;
        case TCP_EVT_DATA:
            break;
        case TCP_EVT_ACK:
            break;
        case TCP_EVT_FIN:
            break;
        case TCP_EVT_RST:
            break;
        case TCP_EVT_DROP:
            break;
        case TCP_EVT_SEND_RST:
            break;
        default:
            break;
    }
}

bool tcb_id_match(ID *a, ID *b) {
    return a->src_ip == b->src_ip &&
           a->dst_ip == b->dst_ip &&
           a->src_port == b->src_port &&
           a->dst_port == b->dst_port;
}

TCB *tcb_init(ID id) {
    TCB *tcb = malloc(sizeof(TCB));
    if (!tcb)
        return NULL;

    memset(tcb, 0, sizeof(TCB));
    tcb->id.src_port = id.src_port;
    tcb->id.dst_port = id.dst_port;
    tcb->id.src_ip   = id.src_ip;
    tcb->id.dst_ip   = id.dst_ip;

    tcb->state = TCP_SYN_RECEIVED;

    tcb->send.unac   = 0;
    tcb->send.next   = 0;
    tcb->send.window = 0;
    tcb->send.iss    = 0;

    tcb->recv.next = 0;
    tcb->recv.window = TCP_WIN_DEF;

    tcb->recv_size = 0;
    tcb->recv_consumed = 0;

    return tcb;
}