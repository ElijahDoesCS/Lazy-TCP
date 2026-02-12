#include "./vector.h"
#include "./tcb.h"

bool vec_init(Vector *vec) {
    TCB **data = malloc(DEF_CAP * sizeof(TCB *));
    if (data == NULL)
        return false;
    vec->data = data;
    vec->capacity = DEF_CAP;
    vec->size = 0;
    return true;
}

bool vec_push(Vector *vec, TCB *tcb) {
    if (vec->size == vec->capacity) {
        uint32_t cap_new = vec->capacity * 2;
        TCB **data = realloc(vec->data, cap_new * sizeof(TCB *));
        if (data == NULL)
            return false;    
        vec->data = data;
        vec->capacity = cap_new;
    }
    vec->data[vec->size++] = tcb;
    return true;
}

TCB *vec_find(Vector *vec, ID *id) {
    for (size_t i = 0; i < vec->size; i++) {
        if (tcb_id_match(&(vec->data[i]->id), id))
            return vec->data[i];
    }

    return NULL;
}

TCB *vec_pop(Vector *vec) {
    if (vec->size == 0)
        return NULL;
    return vec->data[--vec->size];
}

void vec_delete(Vector *vec, TCB *tcb) {
    for (size_t i = 0; i < vec->size; i++) {
        if (tcb_id_match(&(vec->data[i]->id), &(tcb->id))) {
            free(vec->data[i]);
            vec->data[i] = vec->data[vec->size - 1];
            vec->size--;
            return;
        }
    }
}

void vec_destroy(Vector *vec) {
    for (size_t i = 0; i < vec->size; i++) {
        free(vec->data[i]);
    }

    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}