#include "./tcp.h"
#include "./tcb.h"
#include "./hash.h"
#include "./segment.h"

#include "../ip.h"
#include "../../log/log.h"

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

    // Get the state transition and TCB
    Edge edge = tcp_parse_edge(&flags, &id, &tcb);
    tcb_state_update(tcp, tcb, edge);

    switch (edge) {
        case TCP_EVT_SYN:
            return tcp_syn_ack((uint8_t *) tcb, buf, len, offset);
        default: 
            break;
    }

    // tcb_state_update(tcp, tcb, edge);

    // // Switch on event types
    // switch (edge) {
    //     case TCP_EVT_SYN:
    //         int size = tcp_syn_ack((uint8_t *) tcb, buf, len, offset);
    //         printf("The size of the new packet is: %d\n", size);
    //         return size;
    //     case TCP_EVT_SEND_RST:
    //         break;
    //     case TCP_EVT_DROP:
    //         return 0;
    //     default:
    //         break;
    // }

    return 0;
}

// Classify based on flags, connection ID, and current TCB state
static Edge tcp_parse_edge(Flags *flags, ID *id, TCB **tcb) {
    TCB *con = hash_find(*id);

    if (!con) {
        if (flags->rst)
            return TCP_EVT_DROP;

        if (flags->syn && !flags->ack && id->dst_port == TCP_LISTEN_PORT) {
            con = tcb_init(*id);
            if (!con || !hash_insert(con))
                return TCP_EVT_DROP;

            printf("Just created a new tcb\n");

            *tcb = con;

            return TCP_EVT_SYN;
        }

        return TCP_EVT_SEND_RST;
    }

    if (flags->rst)
        return TCP_EVT_RST;

    switch (con->state) {
        // case TCP_SYN_RECEIVED:
        //     break;
        // case TCP_ESTABLISHED:
        //     break;
        // case TCP_CLOSE_WAIT:
        //     break;
        // case TCP_CLOSING:
        //     break;
        // case TCP_LAST_ACK:
        //     break;
        // case TCP_TIME_WAIT:
        //     break;
        default:
            return TCP_EVT_DROP;
    }
}

