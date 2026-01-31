#ifndef DEBUG_H
#define DEBUG_H

#include "../network/network.h"
#include "../network/ip/ip.h"
#include "../network/icmp/icmp.h"
#include "../network/tcp/tcp.h"
#include "../v_device/v_device.h"

#include <stdio.h>
#include <stdlib.h>

const char* TCP_State_to_string(TCP_State state);
void print_TCP_Connection_ID(TCP_Connection_ID *id);
void print_TCP_Send_State(TCP_Send_State *send);
void print_TCP_Recieve_State(TCP_Recieve_State *recv);
void print_TCB(TCB *tcb);
void print_TCP_Server_Instance(TCP_Server_Instance *server);
void virtual_device_print(Virtual_Device *vd);
void raw_packet_print(char *packet, int pack_len);
void tcp_print(TCP_Header *tcp_pack);
char *ip_proto_string(uint8_t proto);
void ip_print(IPv4_Header *ip_pack);
void icmp_print(ICMP_Header *packet);
void tcp_print_buf(char *buffer, int buf_len);

#endif