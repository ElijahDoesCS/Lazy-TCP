#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "./thread_pool/thread_pool.h"
#include "./v_device/v_device.h"
#include "./network/network.h"
#include "./network/icmp/icmp.h"
#include "./network/ip/ip.h"
#include "./network/tcp/tcp.h"
#include "./debug/debug.h"

# define THREAD_COUNT 4

int parse_packet(bool print, 
        Raw_Header *raw_packet, Virtual_Device *vd,
        TCP_Server_Instance *states) 
    {

    char *packet = raw_packet->bytes;
    int pack_len = raw_packet->pack_len;

    IPv4_Header *ip_pack = (IPv4_Header *) packet;
    int op = packet_deserialize(ip_pack);
    int ihl = ip_pack->version_ihl & 0xf, err;
    // if (print) raw_packet_print(packet, pack_len);
    
    switch (op) {   
        case IPV4_ICMP: // ICMP over IPv4
            ICMP_Header *icmp_pack = (ICMP_Header *) ((char *) packet + (4 * ihl));
            icmp_dispatch(ip_pack, icmp_pack); // Dispactch to handle the corresponding type/code

            if (print) {
                ip_print(ip_pack);    
                icmp_print(icmp_pack);
            }

            // Write back to the file descriptor
            write(vd->fd, ip_pack, ntohs(ip_pack->len));

            return 0;
        
        case IPV4_TCP: // TCP over IPv4
            TCP_Header *tcp_pack = (TCP_Header *) ((char *) packet + (4 * ihl));
            if (print) {
                printf("--- INBOUND PACKET ---\n");
                ip_print(ip_pack);
                tcp_print(tcp_pack);
            }

            err = tcp_dispatch(print, ip_pack, tcp_pack, states);

            if (err == TCP_FATAL) return 1;
            if (err == TCP_SILENT) return 0;
            
            // If we aren'treturning before sending
            if (print) {
                printf("RESPONSE PACKET\n");
                ip_print(ip_pack);
                tcp_print(tcp_pack);
            }

            // if (err == TCP_UNREACHABLE) {
            //     // Call ICMP Destination unreachable
            // }
           
            // Write to the file descriptor
            write(vd->fd, ip_pack, ntohs(ip_pack->len));

            return 0;

        case PROTO_UNDF: // Undefined protocol over IPv4
            return 0;
        case IPV6: 
            return 0;   
        case VER_UNDF:
            return 0;
        default: // Some unknown error occured
            return 1;
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "[USAGE ERROR]: ip, dev");
        exit(EXIT_FAILURE);
    }

    // Initialize the tun name and ip address
    char *ip_addr = (char *) calloc(IFNAMSIZ + 1, sizeof(char));
    if (ip_addr == NULL) {
        fprintf(stderr, "[ERROR]: Could not initialize ip address\n");
        exit(EXIT_FAILURE);
    }

    memcpy(ip_addr, argv[1], IFNAMSIZ);

    char *tun_name = (char *) calloc(IFNAMSIZ + 1, sizeof(char));
    if (tun_name == NULL) {
        fprintf(stderr, "[ERROR]: Could not initialize device memory\n");
        free(ip_addr);
        exit(EXIT_FAILURE);
    }

    if (argc >= 3) memcpy(tun_name, argv[2], IFNAMSIZ);

    // Enable debug logs
    bool verbose = false;
    if (argc >= 4) {
        printf("Debug ~\n");
        verbose = true;
    }

    // Wrap the device in a structure
    Virtual_Device vd = {
        .dev = tun_name,
        .ip = ip_addr,
        .flags = IFF_TUN | IFF_NO_PI,
        .fd = -1
    };

    // Allocate a virtual device
    if ((vd.fd = tun_alloc(vd.dev, vd.flags)) < 0) {
        fprintf(stderr, "[ERROR]: Could not initialize virtual device\n");
        free(ip_addr);
        free(tun_name);
        exit(EXIT_FAILURE);
    }   

    if (verbose) virtual_device_print(&vd);

    // Initialize the device to an IP
    if (tun_init(verbose, &vd)) {
        fprintf(stderr, "[ERROR]: Could not bind virtual device to IP %s", vd.ip);
        virtual_device_destroy(vd);
        exit(EXIT_FAILURE);
    }

    // Read on a fixed buffer size transmission unit
    char *tu = (char *) calloc(MTU_SIZE + 1, sizeof(char));
    if (tu == NULL) {
        fprintf(stderr, "[ERROR]: Could not initialize transmission unit\n");
        virtual_device_destroy(vd);
        exit(EXIT_FAILURE);
    }

    TCP_Server_Instance states;
    tcp_init_server_instance(&states);

    int nread;
    while ((nread = read(vd.fd, tu, MTU_SIZE)) > 0) {
        Raw_Header packet;
        packet.bytes = tu;
        packet.pack_len = nread;

        if (parse_packet(verbose, &packet, &vd, &states)) {
            virtual_device_destroy(vd);
            free(tu);
            exit(EXIT_FAILURE);
        }
    }

    virtual_device_destroy(vd);
    free(tu);
    exit(EXIT_SUCCESS);
}
