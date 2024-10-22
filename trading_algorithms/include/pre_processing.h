#ifndef PRE_PROCESSING_H
#define PRE_PROCESSING_H

#include <stddef.h>
#include "market_data.h"
#include "lock_free_queue.h"

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
    double *prices;
    size_t price_count;
    double *price_differences;
    size_t price_difference_count;
    double transaction_costs;
    double latency;
    double *rolling_volatility;
    size_t rolling_volatility_count;
    RiskManagementParams risk_management_params;
    LiquidityInfo liquidity_info;
    TrendInfo trend_info;
    double *liquidity;
    size_t liquidity_count;
} PreProcessedData;

typedef struct {
    LockFreeQueue *input_queue;
    LockFreeQueue *output_queue;
} PreProcessingArgs;

PreProcessedData *pre_process_data(const RawData *raw_data, size_t rolling_volatility_window_size);
void free_pre_processed_data(PreProcessedData *data);
void update_pre_processed_data(PreProcessedData *data, const RawData *new_data);

#endif // PRE_PROCESSING_H
