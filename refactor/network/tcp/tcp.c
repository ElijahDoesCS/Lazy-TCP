#include "./tcp.h"
#include "./tcb.h"
#include "./hash.h"

#include "../ip.h"
#include "../../log/log.h"

static int tcp_syn_ack(uint8_t *buf, int len, int offset);
static int tcp_rst(uint8_t *buf, int len, int offset);
static int tcp_ack(uint8_t *buf, int len, int offset);
static int tcp_write(uint8_t *buf, int len, int offset);

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

    TCB *con = hash_find(id); 
    if (!con) { 
        if (flags.rst)
            return 0;
        
        if (flags.syn && !flags.ack && id.dst_port == TCP_LISTEN_PORT) { // New connection
            con = tcb_init(id);
            if (!con || !hash_insert(con))
                return 0;

            // Send a SYN ACK
            // Update the state

            // Update the packet

        }
        else { // Send a reset

        }
    
        printf("I think we created and inserted a TCB!\n");
    }
    else { // Handle the active connection

    }

    return 0;
}

static int tcp_syn_ack(uint8_t *buf, int len, int offset) {

}

