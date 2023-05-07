#ifndef MARKET_DATA_POOL_H
#define MARKET_DATA_POOL_H

#include <stdbool.h>
#include "market_data.h"

typedef struct MarketDataPool {
    MarketData *buffer;
    bool *is_allocated;
    size_t size;
} MarketDataPool;

MarketDataPool *market_data_pool_init(size_t size);
MarketData *market_data_pool_alloc(MarketDataPool *pool);
void market_data_pool_free(MarketDataPool *pool, MarketData *data);
void market_data_pool_destroy(MarketDataPool *pool);

#endif // MARKET_DATA_POOL_H
