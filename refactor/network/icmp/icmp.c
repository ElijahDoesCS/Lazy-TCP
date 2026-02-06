#include "./icmp.h"
#include "../ip.h"
#include "../network.h"

void icmp_checksum(ICMP *icmp, int len) {
    uint8_t *buf = (uint8_t *) icmp;
    icmp->checksum = 0;

    icmp->checksum = checksum(buf, len);
}

int icmp_validate(const ICMP *icmp, int len) {
    if (len < 8) 
        return 0;

    uint8_t *buf = (uint8_t *) icmp;
    if (checksum((uint8_t *) buf, len) != 0) 
        return 0;

    return 1;
}

int icmp_dest_unreachable(uint8_t *buf, int len, int offset) {
    IPv4 *ip = (IPv4 *) buf;
    uint8_t *dgram = buf + offset;
    ICMP *icmp = (ICMP *) dgram;

    int dgram_len = (len - offset) < 8 ? (len - offset) : 8;
    int icmp_pl_len = offset + dgram_len;
    int ip_len = offset + sizeof(ICMP) + icmp_pl_len;
    
    if (ip_len > MTU_SIZE)
        return 0;

    uint8_t icmp_pl[68];     
    memcpy(icmp_pl, ip, offset);
    memcpy(icmp_pl + offset, dgram, dgram_len);

    // Modify the IP header
    ip->len = htons(ip_len);
    ip->id = 0;
    ip->proto = PROTO_ICMP;
    ip_swap_dst(ip);
    ip_checksum(ip);

    // Create the ICMP header
    icmp->type = ICMP_DST_UNREACHABLE;
    icmp->code = ICMP_PROTO_UNREACHABLE;
    icmp->identifier = 0;
    icmp->sequence_no = 0;

    // Append the payload
    memcpy(dgram + sizeof(ICMP), icmp_pl, icmp_pl_len);

    // Update the checksum
    icmp_checksum(icmp, sizeof(ICMP) + icmp_pl_len);

    return ip_len;
}

int icmp_echo_reply(uint8_t *buf, int len, int offset) {
    IPv4 *ip = (IPv4 *) buf;
    ICMP *icmp = (ICMP *) (buf + offset);
    
    icmp->type = 0;
    icmp->code = 0;

    // Update icmp checksum
    icmp_checksum(icmp, len - offset);
    
    // Update the ip fields
    ip_swap_dst(ip);
    ip_checksum(ip);

    return len;
}

int icmp_dispatch(uint8_t *buf, int len, int offset) {
    ICMP *icmp = (ICMP *) (buf + offset);

    if (!icmp_validate(icmp, len - offset))
        return 0;

    if (icmp->type == ICMP_ECHO)
        return icmp_echo_reply(buf, len, offset);
    
    return 0;
}
