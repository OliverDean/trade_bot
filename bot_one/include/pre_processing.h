#ifndef PRE_PROCESSING_H
#define PRE_PROCESSING_H

#include <stddef.h>
#include "types.h"
#include "market_data.h"
#include "lock_free_queue.h"
#include "config_parser.h"

typedef struct {
    double *prices;
    double *high_prices;
    double *low_prices;
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
    size_t trend_period;
    double *macd;
    double *signal_line;
    size_t macd_count;
    double *atr;
    size_t atr_count;
    double *obv;
    size_t obv_count;
    double *vwap;
    size_t vwap_count;
    double *stochastic_k;
    double *stochastic_d;
    size_t stochastic_count;

} PreProcessedData;


typedef struct {
    LockFreeQueue *input_queue;
    LockFreeQueue *output_queue;
    size_t window_size;
    double ema_alpha;
    int rsi_period;
    double bollinger_multiplier;

} PreProcessingArgs;

typedef struct {
    double *prices;       // Close prices
    double *high_prices;
    double *low_prices;
    double *volumes;
    size_t price_count;
    double *liquidity;
    size_t liquidity_count;
    double transaction_costs;
    double latency;

} RawData;

PreProcessedData *pre_process_data(const RawData *raw_data, const ConfigParams *params);
void free_pre_processed_data(PreProcessedData *data);

#endif // PRE_PROCESSING_H
