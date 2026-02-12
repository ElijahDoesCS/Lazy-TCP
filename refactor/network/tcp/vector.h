#ifndef VECTOR_H
#define VECTOR_H

// ═══════════════════════════════════════════════════════════════════════════
// TCB Vector - Dynamic Array
// ═══════════════════════════════════════════════════════════════════════════
//
// Simple dynamic array for storing TCB pointers. Used as the underlying
// storage for hash table buckets. Grows by doubling capacity when full.
// Removal uses swap-and-pop for O(1) deletion (order not preserved).
//

typedef struct TCB TCB;
typedef struct ID ID;

#define DEF_CAP 16

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct Vector {
    TCB **data;
    uint32_t capacity;
    uint32_t size;
} Vector;

/**
 * @brief Initialize an empty vector with default capacity.
 * @param vec Pointer to the vector to initialize
 * @return    true on success, false on allocation failure
 */
bool vec_init(Vector *vec);

/**
 * @brief Append a TCB to the end of the vector.
 * @param vec Pointer to the vector
 * @param tcb TCB pointer to append
 * @return    true on success, false on allocation failure
 */
bool vec_push(Vector *vec, TCB *tcb);

/**
 * @brief Find a TCB inside the vector
 * @param vec Pointer to the vector
 * @param id  Pointer to the ID for which we search
 * @return    TCB on success, NULL on failure
 */
TCB *vec_find(Vector *vec, ID *id);

/**
 * @brief Remove and return the last TCB from the vector.
 * @param vec Pointer to the vector
 * @return    TCB pointer, or NULL if vector is empty
 */
TCB *vec_pop(Vector *vec);

/**
 * @brief Remove a specific TCB from the vector by ID match.
 * @param vec Pointer to the vector
 * @param tcb TCB to remove (matched by ID, then freed)
 */
void vec_delete(Vector *vec, TCB *tcb);

/**
 * @brief Free all TCBs in the vector and destroy the vector itself.
 * @param vec Pointer to the vector
 */
void vec_destroy(Vector *vec);

#endif