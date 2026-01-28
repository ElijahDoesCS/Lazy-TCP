#include "./debug.h"

#include <stdio.h>
#include <stdint.h>

// Print the TCP state enum
const char* TCP_State_to_string(TCP_State state) {
    switch (state) {
        case TCP_SYN_RECEIVED: return "SYN_RECEIVED";
        case TCP_ESTABLISHED:  return "ESTABLISHED";
        case TCP_CLOSE_WAIT:   return "CLOSE_WAIT";
        default: return "UNKNOWN_STATE";
    }
}

// Print a TCP connection ID
void print_TCP_Connection_ID(TCP_Connection_ID *id) {
    printf("Connection: %d:%d -> %d:%d\n",
           id->src_ip, id->src_port, id->dst_ip, id->dst_port);
}

// Print the send state
void print_TCP_Send_State(TCP_Send_State *send) {
    printf("Send State:\n");
    printf("  unac: %d\n", send->unac);
    printf("  next: %d\n", send->next);
    printf("  window: %d\n", send->window);
    printf("  iss: %d\n", send->iss);
}

// Print the receive state
void print_TCP_Recieve_State(TCP_Recieve_State *recv) {
    printf("Receive State:\n");
    printf("  next: %d\n", recv->next);
    printf("  window: %d\n", recv->window);
    printf("  irs: %d\n", recv->irs);
}

// Print a single TCB
void print_TCB(TCB *tcb) {
    if (!tcb) return;

    printf("=== TCB ===\n");
    print_TCP_Connection_ID(&tcb->id);
    printf("State: %s\n", TCP_State_to_string(tcb->state));
    print_TCP_Send_State(&tcb->send);
    print_TCP_Recieve_State(&tcb->recv);

    printf("recv_size: %d\n", tcb->recv_size);
    printf("recv_buffer (first 32 bytes): ");
    for (int i = 0; i < tcb->recv_size && i < 32; i++) {
        printf("%02x ", tcb->recv_buffer[i]);
    }
    if (tcb->recv_size > 32) printf("...");
    printf("\n");
}

// Print the entire server instance
void print_TCP_Server_Instance(TCP_Server_Instance *server) {
    if (!server) return;

    printf("=== TCP Server Instance ===\n");
    printf("Number of TCBs: %d\n", server->size);

    TCB *current = server->head;
    int idx = 0;
    while (current) {
        printf("\n[TCB %d]\n", idx++);
        print_TCB(current);
        current = current->next;
    }
}

void virtual_device_print(Virtual_Device *vd) {
  printf("Virtual device:\n");
  printf("    IP             : %s\n", vd->ip);
  printf("    Device name    : %s\n", vd->dev);
  printf("    File desciptor : %d\n", vd->fd);
}

void raw_packet_print(char *packet, int pack_len) {
    printf("\n=== Packet (%d bytes) ===\n", pack_len);
    
    for (int i = 0; i < pack_len; i += 16) {
        // Print offset
        printf("%04x: ", i);
        
        // Print hex bytes (16 per line)
        for (int j = 0; j < 16; j++) {
            if (i + j < pack_len) {
                printf("%02x ", (unsigned char)packet[i + j]);
            } else {
                printf("   ");  // Padding for incomplete lines
            }
            
            // Extra space between 8-byte groups
            if (j == 7) printf(" ");
        }
        
        printf(" |");
        
        // Print ASCII representation
        for (int j = 0; j < 16 && i + j < pack_len; j++) {
            unsigned char c = packet[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        
        printf("|\n");
    }
    printf("\n");
}

void tcp_print(TCP_Header *tcp) {
    printf("--- TCP Header ---\n");
    // Convert Network Byte Order to Host Byte Order
    printf("Source Port:      %u\n", ntohs(tcp->src_port));
    printf("Dest Port:        %u\n", ntohs(tcp->dst_port));
    printf("Seq Number:       %u\n", ntohl(tcp->seq_num));
    printf("Ack Number:       %u\n", ntohl(tcp->ack_num));
    
    // Data Offset is the upper 4 bits of that byte
    // It represents the number of 32-bit words (4 bytes) in the header
    int header_len = (tcp->data_offset >> 4) * 4;
    printf("Header Length:    %d bytes\n", header_len);
    
    // Parsing Flags
    printf("Flags:            0x%02x (", tcp->flags);
    if (tcp->flags & 0x20) printf("URG ");
    if (tcp->flags & 0x10) printf("ACK ");
    if (tcp->flags & 0x08) printf("PSH ");
    if (tcp->flags & 0x04) printf("RST ");
    if (tcp->flags & 0x02) printf("SYN ");
    if (tcp->flags & 0x01) printf("FIN ");
    printf(")\n");

    printf("Window Size:      %u\n", ntohs(tcp->window));
    printf("Checksum:         0x%04x\n", ntohs(tcp->checksum));
    printf("Urgent Pointer:   %u\n", ntohs(tcp->urgent_ptr));
    printf("------------------\n");
}

char *ip_proto_string(uint8_t proto) {
    if (proto == IP_TCP_PROTO) return "TCP";
    if (proto == IP_ICMP_PROTO) return "ICMP";
    else return "Not implemented\n";
}

void ip_print(IPv4_Header *ip_pack) {
    // 1. Extract Version and IHL
    uint8_t version = (ip_pack->version_ihl >> 4);
    uint8_t ihl = (ip_pack->version_ihl & 0x0F);
    uint8_t header_len = ihl * 4;

    // 2. Extract Flags and Offset
    uint16_t ff = ntohs(ip_pack->flags_frag);
    uint8_t flags = (ff >> 13);
    uint16_t offset = (ff & 0x1FFF);

    // 3. Convert IPs to human-readable strings
    char src_str[INET_ADDRSTRLEN];
    char dst_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(ip_pack->src), src_str, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(ip_pack->dst), dst_str, INET_ADDRSTRLEN);

    printf("================ IPv4 Header ================\n");
    printf(" Version: %u | IHL: %u (%u bytes)\n", version, ihl, header_len);
    printf(" ToS    : 0x%02X\n", ip_pack->tos);
    printf(" Length : %u bytes\n", ntohs(ip_pack->len));
    printf(" ID     : 0x%04X (%u)\n", ntohs(ip_pack->id), ntohs(ip_pack->id));
    printf(" Flags  : 0x%X (Res:%d DF:%d MF:%d)\n", 
            flags, (flags >> 2) & 1, (flags >> 1) & 1, flags & 1);
    printf(" Offset : %u\n", offset);
    printf(" TTL    : %u\n", ip_pack->ttl);
    printf(" Proto  : %u (0x%02X) - %s\n", ip_pack->proto, ip_pack->proto, ip_proto_string(ip_pack->proto));
    printf(" Check  : 0x%04X\n", ntohs(ip_pack->checksum));
    printf(" Source : %s\n", src_str);
    printf(" Dest   : %s\n", dst_str);
    printf("=============================================\n");
}

void icmp_print(ICMP_Header *packet) {
    printf("---------------- ICMP Header ----------------\n");
    
    // Type and Code are single bytes, no ntohs needed
    printf(" Type     : %u ", packet->type);
    
    // Provide a little "Translation" for common types
    switch(packet->type) {
        case 0:  printf("(Echo Reply)\n"); break;
        case 3:  printf("(Destination Unreachable)\n"); break;
        case 8:  printf("(Echo Request)\n"); break;
        case 11: printf("(Time Exceeded)\n"); break;
        default: printf("(Other)\n"); break;
    }

    printf(" Code     : %u\n", packet->code);
    printf(" Checksum : 0x%04X\n", ntohs(packet->checksum));
    
    // These 4 bytes are specific to Echo/Reply (RFC 792)
    printf(" ID       : %u (0x%04X)\n", ntohs(packet->identifier), ntohs(packet->identifier));
    printf(" Sequence : %u (0x%04X)\n", ntohs(packet->sequence_no), ntohs(packet->sequence_no));
    printf("---------------------------------------------\n");
}


