#ifndef MARKET_DATA_H
#define MARKET_DATA_H

#include <stddef.h>
#include <stdbool.h>


typedef struct MarketData {
    char symbol[10];
    double price;
    int volume;
} MarketData;


MarketData *fetch_market_data();

#endif // MARKET_DATA_H
