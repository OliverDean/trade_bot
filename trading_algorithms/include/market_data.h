
#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include <stddef.h>
#include <stdbool.h>
#include "lock_free_queue.h"

typedef struct MarketData {
    char symbol[10];
    double price;
    int volume;
    double moving_average;
    double ema;
    double bollinger_upper;
    double bollinger_lower;
    double rsi;
    double roc;
    double resistance_level;
    double support_level;
    double upper_price_level;
    double lower_price_level;
} MarketData;

MarketData *dequeue(LockFreeQueue *queue);
void enqueue(LockFreeQueue *queue, MarketData *data);
MarketData *fetch_market_data();

#endif // MARKET_DATA_H
