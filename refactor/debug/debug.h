#ifndef DEBUG_H
#define DEBUG_H

typedef struct Tun Tun;
typedef struct IPv4 IPv4;
typedef struct ICMP ICMP;

#include <stdio.h>
#include <stdint.h>

void icmp_print(ICMP *icmp);

char *ip_proto(uint8_t proto);
void ip_print(IPv4 *ip);

void tun_print(Tun *tun);

void raw_print(uint8_t *buf, int len);

#endif