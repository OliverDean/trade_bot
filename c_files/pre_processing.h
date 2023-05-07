// pre_processing.h

#ifndef PRE_PROCESSING_H
#define PRE_PROCESSING_H

#include <stddef.h>
#include <stdlib.h>
#include <math.h>

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
    double *transaction_costs;
    double latency;
    double *rolling_volatilities;
    size_t rolling_volatility_count;
    RiskManagementParams risk_management_params;
    LiquidityInfo liquidity_info;
    TrendInfo trend_info;
    // Add any additional fields required for other strategies and pre-processing steps
} PreProcessedData;


PreProcessedData *pre_process_data(const RawData *raw_data, size_t rolling_volatility_window_size, size_t custom_window_size);
void free_pre_processed_data(PreProcessedData *data);
void update_pre_processed_data(PreProcessedData *data, const RawData *new_data);

// Additional helper functions for the pre-processing module
double *calculate_price_differences(const double *prices, size_t price_count);
double *calculate_rolling_volatility(const double *price_differences, size_t price_difference_count, size_t window_size);

// Helper function to find pivot points
double calculate_pivot_point(const double *prices, size_t price_count);

// Additional functions for the pre-processing module to interact with support and resistance levels
double calculate_support_level(const double *prices, size_t price_count);
double calculate_resistance_level(const double *prices, size_t price_count);

double *calculate_rolling_volatilities(const double *price_differences, size_t price_difference_count, size_t window_size);
void update_price_differences(PreProcessedData *data, const RawData *new_data);
void update_rolling_volatilities(PreProcessedData *data, const RawData *new_data, size_t rolling_volatility_count);


#endif // PRE_PROCESSING_H
