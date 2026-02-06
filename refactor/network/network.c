#include "./network.h"
#include "./icmp/icmp.h"
#include "./tcp/tcp.h"
#include "./ip.h"

uint16_t checksum(uint8_t *buf, int len) {
    uint32_t sum = 0;
    uint16_t *ptr = (uint16_t *) buf;

    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }

    if (len > 0) sum += *(uint8_t *)ptr;
    
    // Add the end around carry
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)~sum;
}

int net_demux(int fd, uint8_t *buf, int len) {
    IPv4 *ip = (IPv4 *) buf;

    if (!ip_validate(ip, len))
        return 0;

    int proto = ip->proto, ihl = (ip->version_ihl & 0xf) * 4;

    switch (proto) {
        case PROTO_ICMP:
            return icmp_dispatch(buf, len, ihl);
        case PROTO_TCP:
            // return tcp_dispatch(buf, len, ihl);
            return 0;
        default:
            // Echo back or send proto unreachable
            return icmp_dest_unreachable(buf, len, ihl);
    }
}