#ifndef MARKET_DATA_POOL_H
#define MARKET_DATA_POOL_H

#include <stddef.h>
#include "market_data.h"
#include <pthread.h>

typedef struct {
    MarketData *buffer;
    size_t size;
    size_t next_free;
    pthread_mutex_t mutex;
} MarketDataPool;

MarketDataPool *market_data_pool_init(size_t size);
MarketData *market_data_pool_alloc(MarketDataPool *pool);
void market_data_pool_free(MarketDataPool *pool, MarketData *data);
void market_data_pool_destroy(MarketDataPool *pool);

#endif // MARKET_DATA_POOL_H
