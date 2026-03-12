#include "./tcp.h"
#include "./tcb.h"
#include "./hash.h"
#include "./segment.h"

#include "../ip.h"
#include "../../log/log.h"

#include <stdbool.h>

static Edge tcp_parse_edge(Flags *flags, ID *id, TCB **tcb);

uint32_t tcp_gen_iss() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec) + rand();
}

uint16_t tcp_checksum(uint8_t *buf, int len, int offset) {
    IPv4 *ip = (IPv4 *) buf;
    TCP *tcp = (TCP *) (buf + offset);

    uint16_t org_cs   = tcp->checksum;
    int seg_len = len - offset;

    Pseudo pseudo = {
        .src     = ip->src,
        .dst     = ip->dst,
        .zero    = 0,
        .proto   = ip->proto,
        .tcp_len = htons(seg_len)
    };

    tcp->checksum = 0;

    uint8_t ghost_seg[sizeof(Pseudo) + seg_len];
    memcpy(ghost_seg, &pseudo, sizeof(Pseudo));
    memcpy(ghost_seg + sizeof(Pseudo), tcp, seg_len);

    uint16_t cs = checksum(ghost_seg, sizeof(Pseudo) + seg_len);

    tcp->checksum = org_cs;

    return cs;
}

int tcp_dispatch(int fd, uint8_t *buf, int len, int offset) {
    IPv4 *ip = (IPv4 *) buf;
    TCP *tcp = (TCP *) (buf + offset);
    TCB *tcb = NULL;

    uint16_t valid = tcp_checksum(buf, len, offset);
    if (tcp->checksum != valid)
        return 0;

    ID id = {
        .src_port = ntohs(tcp->src_port),
        .dst_port = ntohs(tcp->dst_port),
        .src_ip   = ntohl(ip->src),
        .dst_ip   = ntohl(ip->dst)
    };

    Flags flags = {
        .fin = tcp->flags & FIN,
        .syn = tcp->flags & SYN,
        .rst = tcp->flags & RST,
        .psh = tcp->flags & PSH,
        .ack = tcp->flags & ACK
    };

    Edge edge = tcp_parse_edge(&flags, &id, &tcb);

    if (!tcb_state_update(tcp, tcb, edge)) 
        edge = EVT_DROP;

    switch (edge) {
        case EVT_SYN:
            return tcp_syn_ack((uint8_t *) tcb, buf, len, offset);
        case EVT_SEND_RST:
            return tcp_rst((uint8_t *) tcb, buf, len, offset);
        default: 
            break;
    }

    return 0;
}

static Edge tcp_parse_edge(Flags *flags, ID *id, TCB **tcb) {
    TCB *con = hash_find(*id);
    Edge edge = EVT_DROP;

    if (!con) {
        con = tcb_init(*id);
        if (!con) goto pedge_out;
        if (!hash_insert(con)) {
            free(con);
            con = NULL;
            goto pedge_out;
        }
    }

    if (id->dst_port != TCP_LISTEN_PORT) {
        edge = EVT_SEND_RST;
        goto pedge_out;
    }

    if (flags->rst) {
        edge = EVT_RST;
        goto pedge_out;
    }

    // What if we get a reset in an attempt to establish a connection
    // Do we send a reset if we get anything else on a closed connection

    if (con->state == CON_CLOSED) {
        if (flags->rst) goto pedge_out;
        if (flags->syn && !flags->ack) {
            edge = EVT_SYN;
            goto pedge_out;
        }
        
        edge = EVT_SEND_RST;
        goto pedge_out;
    }

    switch (con->state) {
        case CON_SYN_RECEIVED:
            if (flags->ack && !flags->syn)
                edge = EVT_ACK;
            else
                edge = EVT_SEND_RST;
     
            break;

        case CON_ESTABLISHED:
            if (flags->rst) // Dead code?
                edge = EVT_RST;
            else if (flags->syn)
                edge = EVT_SEND_RST;
            else
                edge = EVT_DATA;  

            break;
        // case CON_CLOSE_WAIT:
        //     break;
        // case CON_CLOSING:
        //     break;
        // case CON_LAST_ACK:
        //     break;
        // case CON_TIME_WAIT:
        //     break;
        default:
            break;
    }

pedge_out:
    if (con && edge != EVT_DROP)
        *tcb = con;
    return edge;
}

