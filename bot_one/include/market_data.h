// include/market_data.h
#ifndef MARKET_DATA_H
#define MARKET_DATA_H

typedef struct {
    double price;
    double volume;
    double moving_average;
    double ema;
    double rsi;
    double bollinger_upper;
    double bollinger_lower;
    // Add other fields as needed
} MarketData;

#endif // MARKET_DATA_H
