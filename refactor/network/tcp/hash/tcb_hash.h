#ifndef TCB_HASH_H
#define TCB_HASH_H

// Forward declarations (tcp_state.h)
typedef struct TCB TCB;
typedef struct ID ID;

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Initialize the hash table
 */
void tcb_hash_init();

/**
 * @brief Find TCB by connection ID
 * @param id Connection 4-tuple
 * @return TCB pointer or NULL if not found
 */
TCB *tcb_hash_find(ID id);

/**
 * @brief Insert TCB into hash table
 * @param tcb TCB to insert
 * @return true on success, false if already exists
 */
bool tcb_hash_insert(TCB *tcb);

/**
 * @brief Remove TCB from hash table (does not free)
 * @param id Connection identifier
 * @return Removed TCB or NULL if not found
 */
TCB* tcb_hash_remove(ID *id);

/**
 * @brief Destroy hash table and free all TCBs
 */
void tcb_hash_destroy();

#endif