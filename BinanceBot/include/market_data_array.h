#ifndef MARKET_DATA_ARRAY_H
#define MARKET_DATA_ARRAY_H

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

// Define a struct to hold a dynamic array of market data
typedef struct {
    MarketData *data;
    size_t length;
    size_t capacity;
} MarketDataArray;

// Function prototypes
MarketDataArray *market_data_array_init(size_t initial_capacity);
void market_data_array_resize(MarketDataArray *array);
void market_data_array_free(MarketDataArray *array);
void read_csv_file(const char *filename, MarketDataArray *array);

#endif // MARKET_DATA_ARRAY_H
