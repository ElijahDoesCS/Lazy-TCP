#include "./ip.h"
#include "./network.h"

int ip_validate(const IPv4 *ip, int len) {
    if (len < (int) sizeof(IPv4))
        return 0;

    int ihl = (ip->version_ihl & 0xf) * 4;

    if (ihl < 20 || ihl > len)
        return 0;
    if ((ip->version_ihl >> 4) != 4)
        return 0;
    if (checksum((uint8_t *) ip, ihl) != 0) {
        return 0;
    }

    return 1;
}

void ip_swap_dst(IPv4 *ip) {
    int src = ip->src;
    ip->src = ip->dst;
    ip->dst = src;
}

void ip_checksum(IPv4 *ip) {
    uint8_t *buf = (uint8_t *) ip;
    int ihl = (ip->version_ihl & 0xf) * 4;
    
    ip->checksum = 0;
    ip->checksum = checksum(buf, ihl);
}