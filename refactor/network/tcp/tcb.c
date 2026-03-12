#include "./tcb.h"
#include "./tcp.h"
#include "../ip.h"
#include "./hash.h"

#include <stdio.h>
#include <stdlib.h>

bool tcb_state_update(TCP *tcp, TCB *tcb, Edge edge) {
    switch (edge) {
        case EVT_SYN:
            tcb->recv.irs = ntohl(tcp->seq_num);
            tcb->recv.next = tcb->recv.irs + 1; // Expect ISN + 1
            tcb->send.window = ntohs(tcp->window);

            if (tcb->state == CON_CLOSED) {
                tcb->send.iss = tcp_gen_iss();
                tcb->send.next = tcb->send.iss + 1; // Next seq no we'll send
                tcb->send.unac = tcb->send.iss;     // Nothing acknoledged yet
            }

            tcb->state = CON_SYN_RECEIVED;

            return true;

        case EVT_ACK:    
            uint32_t ack_no = ntohl(tcp->ack_num);

            if (ack_no == (uint32_t) tcb->send.next) {
                tcb->state = CON_ESTABLISHED;
                tcb->send.unac = ack_no;
            
                return true;
            }

            return false;

        case EVT_RST:
            uint32_t seq_no = ntohl(tcp->seq_num);
 
            if (SEQ_GEQ(seq_no, tcb->recv.next) && 
                SEQ_LT(seq_no, tcb->recv.next + tcb->recv.window)) 
            {
                hash_delete(tcb);
                return true;
            }
            
            return false;
        // case EVT_DATA:
        //     return true;
        // case EVT_FIN:
        //     return true;
        // case EVT_DROP:
        //     return true;
        default:
            return true;
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

    tcb->state = CON_CLOSED;

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