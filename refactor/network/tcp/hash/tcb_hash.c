#include "./tcb_hash.h"
#include "../tcp_state.h"

# define HT_SIZE 1024

typedef struct {
    TCB *buckets[HT_SIZE];
    size_t size;
} State_Table;

// Locally global to obfuscate from external functionality
static State_Table g_state_table = {0};
static bool g_state_init = false;

static uint32_t tcb_id_hash(ID id) {
    uint32_t hash = id.src_ip ^ id.dst_ip;
    hash ^= ((uint32_t) id.src_port << 16) | id.dst_port;
    
    // Jenkins one-at-a-time hash for better distribution
    hash += (hash << 10);
    hash ^= (hash >> 6);
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    
    return hash % HT_SIZE;
}

static bool tcb_id_match(ID *a, ID *b) {
    return a->src_ip == b->src_ip &&
           a->dst_ip == b->dst_ip &&
           a->src_port == b->src_port &&
           a->dst_port == b->dst_port;
}

void tcb_hash_init() {
    for (int i = 0; i < HT_SIZE; i++) {
        g_state_table.buckets[i] = NULL;
    }

    g_state_table.size = 0;
    g_state_init = true;
}

TCB *tcb_hash_find(ID id) {
    if (!g_state_init) {
        tcb_hash_init();
        return NULL;
    }

    uint32_t idx = tcb_id_hash(id);
    TCB *cmp = g_state_table.buckets[idx];
    while (cmp) {
        if (tcb_id_match(&(cmp->id), &id))
            return cmp;

        cmp = cmp->next;
    }

    return NULL;
}

