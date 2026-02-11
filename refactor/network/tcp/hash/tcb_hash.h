#ifndef TCB_HASH_H
#define TCB_HASH_H

// Forward declaration
typedef struct TCB TCB;

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

void tcb_hash_init();

TCB* tcb_hash_find(uint32_t src_ip, uint16_t src_port,
                   uint32_t dst_ip, uint16_t dst_port);

bool tcb_hash_insert(TCB *tcb);

TCB* tcp_hash_remove(uint32_t src_ip, uint16_t src_port,
                     uint32_t dst_ip, uint16_t dst_port);

void tcp_hash_destroy(void);

#endif