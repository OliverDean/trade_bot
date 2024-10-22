#ifndef DATA_FETCHER_H
#define DATA_FETCHER_H

#include "config_parser.h"
#include "pre_processing.h"

// Structure to hold memory for libcurl response
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Function to fetch data from Binance API
int fetch_data(const ConfigParams *config, RawData *raw_data);

#endif // DATA_FETCHER_H
