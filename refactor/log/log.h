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
//   - Named FIFO at /tmp/tcp_less_log.fifo
//   - Non-blocking writes (drops events if Python can't keep up)
//   - Python viewer runs independently, parses and prettifies output
//

typedef struct Event Event;

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

// User libraries
#include "../network/network.h"
#include "../network/tcp/tcp.h"

struct Event {
    uint8_t  dir;            // Direction: 1 = recv from network, 0 = send to network
    uint64_t timestamp_ns;   // Event timestamp (nanoseconds since epoch)
    uint8_t  tu[MTU_SIZE];   // Raw packet bytes (IP header + payload)
    TCB      internal_state; // TCP control block snapshot (if TCP packet)
};


uint64_t log_time();

/**
 * Initialize debug logging via named FIFO.
 * Creates /tmp/tcp_less_log.fifo and opens it for writing (non-blocking).
 * Python debug viewer must be running and reading from FIFO before calling.
 * 
 * @return File descriptor for writing events on success, -1 if no logger attached
 */
int log_init();

#endif