#include "./tcb_hash.h"
#include "../tcp_state.h"

# define HT_SIZE 1024

typedef struct State_Table State_Table;

struct State_Table {
    TCB *buckets[HT_SIZE];
    size_t size;
};

// Global variables for the fucking thing
static State_Table g_state = {0};
static bool g_state_up = false;


