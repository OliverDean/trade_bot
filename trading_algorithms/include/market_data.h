
#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include <stddef.h>
#include <stdbool.h>

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

#endif // MARKET_DATA_H
