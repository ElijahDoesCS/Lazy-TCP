#include "tcp.h"

uint32_t tcp_get_ticks() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000 + tv.tv_usec) + rand();

}

void tcp_init_server_instance(TCP_Server_Instance *states) {
    states->size = 0;
    states->head = NULL;
}

bool tcp_compare_id(TCP_Connection_ID *id_a, TCP_Connection_ID *id_b) {
    if (id_a->src_ip != id_b->src_ip) return false;
    if (id_a->src_port != id_b->src_port) return false;
    if (id_a->dst_ip != id_b->dst_ip) return false;
    if (id_a->dst_port != id_b->dst_port) return false;
    return true;
}

void tcp_update_checksum(TCP_Header *tcp_pack, int tcp_pack_len, 
                        TCP_IP_Pseudo_Header *pseudo_ip, int pseudo_len) 
    {

    tcp_pack->checksum = 0;
    char buffer[tcp_pack_len + pseudo_len + 1];
    memcpy(buffer, pseudo_ip, pseudo_len);
    memcpy(buffer + pseudo_len, tcp_pack, tcp_pack_len);
    
    uint16_t checksum = packet_checksum((void *) buffer, tcp_pack_len + pseudo_len);
    tcp_pack->checksum = checksum;
}

void tcp_switch_port(TCP_Header *tcp_pack) {
    uint16_t dst = tcp_pack->dst_port;
    tcp_pack->dst_port = tcp_pack->src_port;
    tcp_pack->src_port = dst;
}

TCB *tcp_init_tcb(TCP_Connection_ID *id) {
    TCB *new = (TCB *) malloc(sizeof(TCB));
    if (new == NULL) return NULL;

    memset(new, 0, sizeof(TCB));
    new->id.src_ip = id->src_ip;
    new->id.src_port = id->src_port;
    new->id.dst_ip = id->dst_ip;
    new->id.dst_port = id->dst_port;

    new->state = TCP_SYN_RECEIVED;

    // Send state will be initialized on SYN-ACK
    new->send.unac = 0;      // Nothing sent yet
    new->send.next = 0;      // Will set to ISS when we send SYN-ACK
    new->send.window = 0;    // Will be set from client's window
    new->send.iss = 0;       // Will generate when sending SYN-ACK
    
    new->recv.next = 0;      // Will set to client's ISN + 1
    new->recv.window = 8192; // Our receive window (8KB)
    new->recv.irs = 0;       // Will set to client's ISN
    
    // Receive buffer
    new->recv_size = 0;
    new->recv_consumed = 0;
    
    // Linked list
    new->next = NULL;

    return new;
}

void tcp_insert_tcb(TCB *tcb, TCP_Server_Instance *states) {
    TCB *search = states->head, *tail;
    while (search) {
        if (tcp_compare_id(&(search->id), &(tcb->id))) return;
        tail = search;
        search = search->next;
    }
    if (states->head == NULL) {
        states->head = tcb;
        states->size++;
    }
    else {
        tail->next = tcb;
        states->size++;
    }
}

TCB *tcp_get_state(TCP_Connection_ID *id, TCP_Server_Instance *states) {
    TCB *search = states->head, *found = NULL;
    while (search) {
        if (tcp_compare_id(&(search->id), id)) {
            found = search;
            break;
        }
        search = search->next;
    }

    return found;
}

void tcp_null_rst(IPv4_Header *ip_pack, TCP_Header *tcp_pack) {
    int tcp_header_len, ip_header_len;
    int total_len, seg_len;

    uint32_t seq_num, ack_num;
    uint8_t flags;

    bool syn, ack, fin;
    flags = tcp_pack->flags;
    syn = flags & TCP_SYN;
    ack = flags & TCP_ACK;
    fin = flags & TCP_FIN;

    tcp_header_len = (tcp_pack->data_offset >> 4) * 4;
    ip_header_len = (ip_pack->version_ihl & 0xf) * 4;
    total_len = ntohs(ip_pack->len);
    seg_len = total_len - ip_header_len - tcp_header_len;
    seq_num = 0, ack_num = 0;

    if (ack) {
        seq_num = ntohl(tcp_pack->ack_num);
        flags = TCP_RST;
    }
    else { // RST+ACK, SEQ=0, ACK=incoming_SEQ + length 
        seg_len += (fin + syn);
        ack_num = ntohl(tcp_pack->seq_num) + seg_len;
        flags = TCP_RST | TCP_ACK;
    }
    
    // Swap addresses
    ip_swap_dst(ip_pack);
    tcp_switch_port(tcp_pack);

    // Set ip header fields
    ip_pack->len = htons(ip_header_len + tcp_header_len);
    
    // Set header fields
    tcp_pack->seq_num = htonl(seq_num);
    tcp_pack->ack_num = htonl(ack_num);
    tcp_pack->data_offset = (tcp_header_len / 4) << 4;
    tcp_pack->flags = flags;
    tcp_pack->window = 0;
    tcp_pack->urgent_ptr = 0;

    // Update the ip checksum
    ip_update_checksum((void *) ip_pack, ip_header_len);

    // Update the tcp checksum
    TCP_IP_Pseudo_Header pseudo_ip;
    pseudo_ip.src = ip_pack->src;
    pseudo_ip.dst = ip_pack->dst;
    pseudo_ip.zero = 0;
    pseudo_ip.proto = IP_TCP_PROTO;
    pseudo_ip.tcp_len = htons(tcp_header_len);

    tcp_update_checksum(tcp_pack, tcp_header_len, &pseudo_ip, sizeof(TCP_IP_Pseudo_Header));
}

void tcp_null_syn_ack(IPv4_Header *ip_pack, TCP_Header *tcp_pack, TCB *tcb) {
    // Our recv sequence is their initial sequence num
    tcb->recv.irs = ntohl(tcp_pack->seq_num);
    tcb->recv.next = tcb->recv.irs + 1; // Expect ISN + 1
    tcb->send.window = ntohs(tcp_pack->window);
    tcb->send.iss = tcp_get_ticks();

    // Next sequence number we'll send
    tcb->send.next = tcb->send.iss + 1;

    // Nothing acknowledged yet
    tcb->send.unac = tcb->send.iss;

    // Swap destinations
    ip_swap_dst(ip_pack);
    tcp_switch_port(tcp_pack);

    // Set TCP header fields
    int tcp_header_len = (tcp_pack->data_offset >> 4) * 4;
    tcp_pack->seq_num = htonl(tcb->send.iss);
    tcp_pack->ack_num = htonl(tcb->recv.next);
    tcp_pack->data_offset = (tcp_header_len / 4) << 4;
    tcp_pack->flags = TCP_SYN | TCP_ACK;
    tcp_pack->window = htons(tcb->recv.window);
    tcp_pack->urgent_ptr = 0;

    // Update the IP checksum
    int ip_header_len = (ip_pack->version_ihl & 0xf) * 4;  
    ip_pack->len = htons(ip_header_len + tcp_header_len);
    ip_update_checksum(ip_pack, ip_header_len);

    TCP_IP_Pseudo_Header pseudo;
    pseudo.src = ip_pack->src;
    pseudo.dst = ip_pack->dst;
    pseudo.zero = 0;
    pseudo.proto = IP_TCP_PROTO;
    pseudo.tcp_len = htons(tcp_header_len);

    tcp_update_checksum(tcp_pack, tcp_header_len, &pseudo, sizeof(TCP_IP_Pseudo_Header));
}

void tcp_remove_connection(TCP_Server_Instance *states, TCB *tcb) {
    if (states->head == tcb) {
        states->head = states->head->next;
    }
    else {
        TCB *prev = states->head;
        while (prev && prev->next != tcb) {
            prev = prev->next;
        }
        if (prev) {
            prev->next = tcb->next;
        }
    }

    free(tcb);
    states->size--;
}

bool tcp_got_rst(TCP_Header *tcp_pack, TCB *tcb, TCP_Server_Instance *states) {
    uint32_t rst_seq = tcp_pack->seq_num;

    // User requested to remove the connection
    if (SEQ_GEQ(rst_seq, tcb->recv.next) && 
        SEQ_LT(rst_seq, tcb->recv.next + tcb->recv.window)) {
        tcp_remove_connection(states, tcb);
    }
    else return true;

    return false;
}

void tcp_send_ack(IPv4_Header *ip_pack, TCP_Header *tcp_pack, TCB *tcb, char *body, int body_len) {
    // Modify the ip packet contents
    ip_swap_dst(ip_pack);
    tcp_switch_port(tcp_pack);

    int ip_hl = (ip_pack->version_ihl & 0xf) * 4;
    int tcp_hl = (tcp_pack->data_offset >> 4) * 4;

    // Build the ACK
    tcp_pack->seq_num = htonl(tcb->send.next);
    tcp_pack->ack_num = htonl(tcb->recv.next);
    tcp_pack->data_offset = (tcp_hl / 4) << 4;
    tcp_pack->window = htons(tcb->recv.window);
    tcp_pack->urgent_ptr = 0;

    if (body != NULL && body_len > 0) {
        tcp_pack->flags = TCP_PSH | TCP_ACK;
        uint8_t *payload = ((uint8_t *) tcp_pack) + tcp_hl;
        memcpy(payload, body, body_len); // Payload is up to 1500 bytes - ip & tcp hl (bof vuln)
        ip_pack->len = htons(ip_hl + tcp_hl + body_len);
        tcb->send.next += body_len;
    }
    else {
        tcp_pack->flags = TCP_ACK;
        ip_pack->len = htons(ip_hl + tcp_hl);
    }

    ip_update_checksum(ip_pack, ip_hl);

    TCP_IP_Pseudo_Header pseudo;
    pseudo.src = ip_pack->src;
    pseudo.dst = ip_pack->dst;
    pseudo.zero = 0;
    pseudo.proto = IP_TCP_PROTO;
    pseudo.tcp_len = htons(tcp_hl + body_len);
    tcp_update_checksum(tcp_pack, tcp_hl + body_len, &pseudo, sizeof(pseudo));
}

int tcp_handle_data(IPv4_Header *ip_pack, TCP_Header *tcp_pack, TCB *tcb) {
    int ip_hl = (ip_pack->version_ihl & 0xf) * 4;
    int tcp_hl = (tcp_pack->data_offset >> 4) * 4;
    int data_len = ntohs(ip_pack->len) - ip_hl - tcp_hl;
    if (data_len <= 0) return TCP_SILENT;

    char *http_response = NULL;
    int message_len = 0;

    uint8_t *data = ((uint8_t *) ip_pack) + ip_hl + tcp_hl;
    http_response = http_handle_request((char *) data, data_len, &message_len);
    tcb->recv.next += data_len;
    tcp_send_ack(ip_pack, tcp_pack, tcb, http_response, message_len);
    free(http_response);
    
    return 0;
}

// void tcp_send_fin(IPv4_Header *ip_pack, TCP_Header *tcp_pack, TCB *tcb) {
//     printf("We aren't doing anything to finish right now buddy!\n");
//     return;
// }

int tcp_dispatch(bool verbose, 
                 IPv4_Header *ip_pack, TCP_Header *tcp_pack, 
                 TCP_Server_Instance *states) 
    {   
    
    TCP_Connection_ID id = { 
        .src_ip = ip_pack->src, 
        .dst_ip = ip_pack->dst,
        .src_port = tcp_pack->src_port,
        .dst_port = tcp_pack->dst_port
    };

    uint8_t flags = tcp_pack->flags;
    bool syn = flags & TCP_SYN;
    bool ack = flags & TCP_ACK;
    bool fin = flags & TCP_FIN; // Going to be handled when we have established
    bool rst = flags & TCP_RST;

    TCB *con_state = tcp_get_state(&id, states);
    if (con_state == NULL) {
        if (rst) {
            return TCP_SILENT;
        }
        if (syn && !ack) { 
            TCB *tcb = tcp_init_tcb(&id);
            tcp_insert_tcb(tcb, states);
            tcp_null_syn_ack(ip_pack, tcp_pack, tcb);

            if (tcb == NULL) 
                return TCP_SILENT;
            
            if (verbose) {
                printf("--- CURRENT STATE --- \n");
                print_TCP_Server_Instance(states);
            }
        }
        else {
            tcp_null_rst(ip_pack, tcp_pack);
        }

        return 0;
    }

    if (rst) {
        if (!tcp_got_rst(tcp_pack, con_state, states))
            printf("[ERROR]: Recieved invalid reset request, may or may not be malicioius\n");
        
        return TCP_SILENT;
    }
    
    switch (con_state->state) {
        case TCP_SYN_RECEIVED:
            if (syn && !ack) {
                tcp_null_syn_ack(ip_pack, tcp_pack, con_state);
            }
            else if (ack && !syn) { // Final ack of handshake
                if (verbose) {
                    printf("SESSION INITIALIZED FOR CONNECTION:\n   ");
                    print_TCP_Connection_ID(&(con_state->id));
                }

                uint32_t ack_num = ntohl(tcp_pack->ack_num);
                
                // If it's a valid ACK advance the state
                if (ack_num == (uint32_t) con_state->send.next) {
                    con_state->state = TCP_ESTABLISHED;
                    con_state->send.unac = ack_num;
                }
                else {
                    printf("[ERROR]: Recieved invalid connection acknowledgement. Failed handshake\n");
                }
            }
            else {
                printf("[DEBUG]: Unexpected flag in SYN_RECIEVED state\n");
            }

            return TCP_SILENT;

        case TCP_ESTABLISHED:
            if (fin) { // The client is closing their side of the connection
                con_state->recv.next = ntohl(tcp_pack->seq_num) + 1;
                tcp_send_ack(ip_pack, tcp_pack, con_state, NULL, 0);
                con_state->state = TCP_CLOSE_WAIT;

                // Send our fin when we are done sending our data
                // tcp_send_fin(ip_pack, tcp_pack, con_state);
                return 0;
            }
            else {
                return tcp_handle_data(ip_pack, tcp_pack, con_state);
            }
        case TCP_CLOSE_WAIT:
            // We have acknowledged their fin, sent our fin, and are waiting for their ack
            return TCP_SILENT;
        case TCP_CLOSING:
            return TCP_SILENT;
        case TCP_LAST_ACK:
            return TCP_SILENT;
        case TCP_TIME_WAIT:
            return TCP_SILENT;
        default:
            return TCP_SILENT;
    }

    return 0;
}
