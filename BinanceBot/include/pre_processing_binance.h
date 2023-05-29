// pre_processing.h

#ifndef PRE_PROCESSING_BINANCE_H
#define PRE_PROCESSING_BINANCE_H

#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "market_data_array.h"
#include "lock_free_queue.h"
#include "proces_queue.h"

typedef struct {
    double *prices;
    size_t price_count;
    double *liquidity;
    size_t liquidity_count;
    double transaction_costs;
    double latency;
} RawData;

typedef struct {
    double risk_multiplier;
    double max_position_size;
} RiskManagementParams;

typedef struct {
    double minimum_liquidity;
} LiquidityInfo;

typedef struct {
    double trend_strength;
} TrendInfo;

typedef struct {
    double *price_differences;
    size_t price_difference_count;
    double transaction_costs;
    double latency;
    double *rolling_volatilities;
    size_t rolling_volatility_count;
    RiskManagementParams risk_management_params;
    LiquidityInfo liquidity_info;
    TrendInfo trend_info;
    // Add any additional fields required for other strategies and pre-processing steps
    double lower_price_level;
    double upper_price_level;
    double support_level;
    double resistance_level;
} PreProcessedData;

typedef struct PreProcessingArgs {
    LockFreeQueue *input_queue;
    ProcesLockFreeNode *output_queue;  // Changed from LockFreeQueue *output_queue
    // Other members of the struct...
} PreProcessingArgs;

typedef struct PriceLevels
{
    double upper;
    double lower;
    double pivot_point;
} PriceLevels;

#define DEFAULT_CALCULATION_INTERVAL 1000
#define MAX_RECORDS_TO_PROCESS 10000
#define WINDOW_SIZE 100
#define EMA_ALPHA 0.1
#define RSI_PERIOD 14
#define BOLLINGER_MULTIPLIER 2


PreProcessedData *pre_process_data(const MarketData *market_data, size_t data_count, size_t rolling_volatility_window_size, size_t custom_window_size);
double *calculate_price_differences(const MarketData *market_data, size_t data_count);
double *calculate_rolling_volatilities(const double *price_differences, size_t price_difference_count, size_t window_size);
PriceLevels calculate_price_levels(const MarketData *market_data, size_t data_count);
double calculate_support_level(const MarketData *market_data, size_t data_count);
double calculate_resistance_level(const MarketData *market_data, size_t data_count);
void update_price_differences(PreProcessedData *data, const MarketData *new_data, size_t new_data_count);
void update_rolling_volatilities(PreProcessedData *data, const MarketData *new_data, size_t new_data_count, size_t window_size);
void *pre_processing_thread(void *args);

#endif // PRE_PROCESSING_BINANCE_H
