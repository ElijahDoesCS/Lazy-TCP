#include "./hash.h"
#include "./tcb.h"
#include "./vector.h"

typedef struct Hash_Table {
    Vector buckets[HT_SIZE];
    size_t size;
} Hash_Table;

static Hash_Table g_con_table = {0};
static bool       g_tbl_init = false;

static inline uint32_t hash_id(ID id) {
    uint32_t hash = id.src_ip ^ id.dst_ip;
    hash ^= ((uint32_t) id.src_port << 16) | id.dst_port;
    
    hash += (hash << 10);
    hash ^= (hash >> 6);
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    
    return hash % HT_SIZE;
}

bool hash_init() {
    for (size_t i = 0; i < HT_SIZE; i++) {
        if (!vec_init(&(g_con_table.buckets[i])))
            return false;
    }

    g_con_table.size = 0;
    g_tbl_init = true;

    return true;
}

TCB *hash_find(ID id) {
    if (!g_tbl_init)    
        return NULL;

    uint32_t idx = hash_id(id);
    Vector *cons = &(g_con_table.buckets[idx]);
    
    return vec_find(cons, &id);
}

bool hash_insert(TCB *tcb) {
    if (!g_tbl_init && !hash_init())
        return false;
    
    uint32_t idx = hash_id(tcb->id);
    Vector *cons = (&g_con_table.buckets[idx]);

    if (!vec_push(cons, tcb))
        return false;
    
    g_con_table.size++;
    return true;
}

void hash_delete(TCB *tcb) {
    if (!g_tbl_init)
        return;

    uint32_t idx = hash_id(tcb->id);
    Vector *cons = &(g_con_table.buckets[idx]);

    vec_delete(cons, tcb);
    g_con_table.size--;
}

void hash_destroy() {
    if (!g_tbl_init)
        return;

    for (size_t i = 0; i < HT_SIZE; i++) {
        vec_destroy(&(g_con_table.buckets[i]));
    }

    g_con_table.size = 0;
    g_tbl_init = false;
}








