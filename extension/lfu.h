#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct lfu_item_ {
    struct lfu_item_ *next;
    struct lfu_item_ *prev;
    struct lfu_item_ *h_next;
    int nbytes;
    uint8_t nkey;
    char data[];
} lfu_item;

typedef struct lfu_stat {
    uint64_t total_items;
    uint64_t curr_items;
    uint64_t total_bytes;
    uint64_t malloc;
    uint64_t malloc_failed;
    // finish this
} lfu_stat;

typedef struct lfu {
    unsigned int hashpower;
    uint64_t max_bytes;
    lfu_item **table;
    lfu_item *head;
    lfu_item *tail;
    lfu_stat stat;
} lfu;

lfu* lfu_init(const uint64_t maxbytes, const unsigned int hashpower);
