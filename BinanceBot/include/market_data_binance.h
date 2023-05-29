#ifndef MARKET_DATA_BINANCE_H
#define MARKET_DATA_BINANCE_H

#include <stddef.h>
#include <stdbool.h>

// Define a struct to hold the market data
typedef struct {
    char timestamp[20]; 
    double open;
    double high;
    double low;
    double close;
    double volume;
    char close_time[20];
    double quote_av;
    int trades;
    double tb_base_av;
    double tb_quote_av;
    int ignore;
} MarketData;

#endif // MARKET_DATA_BINANCE_H