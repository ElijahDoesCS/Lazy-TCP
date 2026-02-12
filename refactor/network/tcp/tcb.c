#include "./tcb.h"

void tcb_state_update(TCP *tcp, TCB *tcb) {
    
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