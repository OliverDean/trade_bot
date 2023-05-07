#include <stdlib.h>
#include <string.h>
#include "market_data_pool.h"

MarketDataPool *market_data_pool_init(size_t size) {
    MarketDataPool *pool = (MarketDataPool *)malloc(sizeof(MarketDataPool));
    pool->buffer = (MarketData *)malloc(size * sizeof(MarketData));
    pool->is_allocated = (bool *)malloc(size * sizeof(bool));
    pool->size = size;

    for (size_t i = 0; i < size; i++) {
        pool->is_allocated[i] = false;
    }

    return pool;
}


MarketData *market_data_pool_alloc(MarketDataPool *pool) {
    for (size_t i = 0; i < pool->size; i++) {
        if (!pool->is_allocated[i]) {
            pool->is_allocated[i] = true;
            return &pool->buffer[i];
        }
    }

    return NULL; // No available slot in the memory pool
}

void market_data_pool_free(MarketDataPool *pool, MarketData *data) {
    size_t index = data - pool->buffer;
    if (index < pool->size) {
        pool->is_allocated[index] = false;
    }
}

void market_data_pool_destroy(MarketDataPool *pool) {
    free(pool->buffer);
    free(pool->is_allocated);
    free(pool);
}
