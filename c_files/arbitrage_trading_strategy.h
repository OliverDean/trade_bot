// arbitrage_trading_strategy.h

#ifndef ARBITRAGE_TRADING_STRATEGY_H
#define ARBITRAGE_TRADING_STRATEGY_H

#include "pre_processing.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include "algorithm_execution.h"


// Function to check the profitability of a trade considering transaction costs, latency, and liquidity
bool is_trade_profitable(double price_difference, double transaction_costs, double latency, const LiquidityInfo *liquidity_info, double liquidity);

// Calculate the trend strength based on the TrendInfo data
double trend_strength(const PreProcessedData *data);

// Calculate the dynamic threshold based on the standard deviation of the historical price differences
double calculate_dynamic_threshold(const PreProcessedData *data, double base_threshold);

// Calculate the position size based on the price difference and the risk management parameters
double calculate_position_size(double price_difference, const RiskManagementParams *params);

// Calculate the standard deviation of the given values
double calculate_standard_deviation(const double *values, size_t count, size_t window_size);
#endif
