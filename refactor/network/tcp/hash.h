#ifndef HASH_H
#define HASH_H

// ═══════════════════════════════════════════════════════════════════════════
// TCP Connection Hash Table
// ═══════════════════════════════════════════════════════════════════════════
//
// Global hash table for active TCP connections. Maps connection IDs (4-tuple)
// to TCBs. Uses chaining with vectors for collision resolution. Auto-initializes
// on first insert. Freeing TCBs happens in vec_delete/vec_destroy.
//

typedef struct TCB TCB;
typedef struct ID ID;

#define HT_SIZE 256

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Initialize the hash table (optional, auto-inits on first insert).
 * @return true on success, false on allocation failure
 */
bool hash_init();

/**
 * @brief Find a TCB by connection ID.
 * @param id Connection ID to search for
 * @return   Matching TCB or NULL if not found
 */
TCB *hash_find(ID id);

/**
 * @brief Insert a TCB into the hash table.
 * @param tcb TCB to insert (hashed by ID)
 * @return    true on success, false on allocation failure
 */
bool hash_insert(TCB *tcb);

/**
 * @brief Remove and free a TCB from the hash table. Sets TCB to NULL.
 * @param tcb TCB to remove (matched by ID, then freed)
 */
void hash_delete(TCB *tcb);

/**
 * @brief Destroy the hash table and free all TCBs.
 */
void hash_destroy();

#endif