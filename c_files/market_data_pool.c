#include <stdlib.h>
#include <string.h>
#include "market_data_pool.h"

MarketDataPool *market_data_pool_init(size_t size) {
    MarketDataPool *pool = (MarketDataPool *)malloc(sizeof(MarketDataPool));
    pool->buffer = (MarketData *)malloc(size * sizeof(MarketData));
    pool->free_slots = (size_t *)malloc(size * sizeof(size_t));
    pool->top = size;
    pool->size = size;

    for (size_t i = 0; i < size; i++) {
        pool->free_slots[i] = i;
    }

    return pool;
}

MarketData *market_data_pool_alloc(MarketDataPool *pool) {
    if (pool->top > 0) {
        size_t index = pool->free_slots[--pool->top];
        return &pool->buffer[index];
    }
    return NULL; // No available slot in the memory pool
}

void market_data_pool_free(MarketDataPool *pool, MarketData *data) {
    size_t index = data - pool->buffer;
    if (index < pool->size) {
        pool->free_slots[pool->top++] = index;
    }
}

void market_data_pool_destroy(MarketDataPool *pool) {
    free(pool->buffer);
    free(pool->free_slots);
    free(pool);
}
