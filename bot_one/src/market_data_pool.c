#include "market_data_pool.h"
#include <stdlib.h>
#include <pthread.h>

MarketDataPool *market_data_pool_init(size_t size) {
    MarketDataPool *pool = (MarketDataPool *)malloc(sizeof(MarketDataPool));
    pool->buffer = (MarketData *)malloc(sizeof(MarketData) * size);
    pool->size = size;
    pool->next_free = 0;
    pthread_mutex_init(&pool->mutex, NULL);
    return pool;
}

MarketData *market_data_pool_alloc(MarketDataPool *pool) {
    pthread_mutex_lock(&pool->mutex);
    if (pool->next_free < pool->size) {
        MarketData *data = &pool->buffer[pool->next_free++];
        pthread_mutex_unlock(&pool->mutex);
        return data;
    } else {
        pthread_mutex_unlock(&pool->mutex);
        return NULL;
    }
}

// void market_data_pool_free(MarketDataPool *pool, MarketData *data) {
//     // Implement if necessary
// }

void market_data_pool_destroy(MarketDataPool *pool) {
    pthread_mutex_destroy(&pool->mutex);
    free(pool->buffer);
    free(pool);
}
