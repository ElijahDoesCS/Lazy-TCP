#ifndef LOG_H
#define LOG_H

// ═══════════════════════════════════════════════════════════════════════════
// Debug Logging System
// ═══════════════════════════════════════════════════════════════════════════
//
// Provides packet and internal state logging to an external Python viewer
// via named FIFO. Events capture raw packet bytes and TCP control block
// snapshots for real-time protocol debugging.
//
// Architecture:
//   - Named FIFO at ./tcp_less_log.fifo
//   - Non-blocking writes (drops events if Python can't keep up)
//   - Python viewer runs independently, parses and prettifies output
//

typedef struct Event Event;
extern int g_log_fd;

#define VERBOSE(log) ((log) > -1)

// Log event if the global log fd is > -1
# define LOG_EVENT(tu, len, dir) \
    do { if (VERBOSE(g_log_fd)) log_event((tu), (len), (dir), g_log_fd); } while(0)

# define LOG_READ     1
# define LOG_WRITE    0
# define LOG_SHUTDOWN 2

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

// User libraries
#include "../network/network.h"
#include "../network/tcp/tcp.h"

struct Event {
    uint8_t  dir;            // Direction: 1 = recv from network, 0 = send to network
    uint64_t timestamp_ns;   // Event timestamp (nanoseconds since epoch)
    uint16_t tu_len;         // Length of the packet
    uint8_t  tu[MTU_SIZE];   // Raw packet bytes (IP header + payload)
    // TCB      state;          // TCP control block snapshot (if TCP packet)
} __attribute__((packed));

uint64_t log_time();

/**
 * @brief Logs a network event from the TUN interface
 * @param tu  Pointer to the TUN packet data buffer
 * @param len Length of the packet in bytes
 * @param dir Direction of the packet (e.g., 1 = inbound, 0 = outbound)
 * @param fd  The file descriptor of the debug interface pipe
 */
void log_event(uint8_t *tu, uint16_t len, uint8_t dir, int fd); 

/**
 * Initialize debug logging via named FIFO.
 * Creates ./tcp_less_log.fifo and opens it for writing (non-blocking).
 * Python debug viewer must be running and reading from FIFO before calling.
 * 
 * @return File descriptor for writing events on success, -1 if no logger attached
 */
int log_init();

#endif