// algorithm_execution.h

#ifndef ALGORITHM_EXECUTION_H
#define ALGORITHM_EXECUTION_H

#include "pre_processing.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include "arbitrage_trading_strategy.h"

typedef enum
{
    BUY,
    SELL,
    HOLD
} TradeAction;

typedef struct {
    TradeAction action;
    double position_size;
} TradeSignal;

typedef TradeSignal (*TradingAlgorithm)(const PreProcessedData *);

// Function to check the profitability of a trade considering transaction costs, latency, and liquidity
bool is_trade_profitable(double price_difference, double transaction_costs, double latency, const LiquidityInfo *liquidity_info, double liquidity);

// Implement the arbitrage trading strategy
TradeSignal arbitrage_trading_strategy(const PreProcessedData *data);

// Calculate the trend strength based on the TrendInfo data
double trend_strength(const PreProcessedData *data);

// Calculate the dynamic threshold based on the standard deviation of the historical price differences
double calculate_dynamic_threshold(const PreProcessedData *data, double base_threshold);

// Calculate the position size based on the price difference and the risk management parameters
double calculate_position_size(double price_difference, const RiskManagementParams *params);

// Calculate the standard deviation of the given values
double calculate_standard_deviation(const double *values, size_t count, size_t window_size);

#endif

// algorithm_execution.c

#include "algorithm_execution.h"

bool is_trade_profitable(double price_difference, double transaction_costs, double latency, const LiquidityInfo *liquidity_info, double liquidity)
{
    // Check if the potential profit after considering transaction costs, latency, and liquidity is greater than zero
    if (price_difference - transaction_costs - latency > 0 && liquidity >= liquidity_info->minimum_liquidity)
    {
        return true;
    }

    return false;
}

// Define the base_threshold
const double base_threshold = 0.01;

double get_current_liquidity(const PreProcessedData *data)
{
    // Implement this function to return the current liquidity based on the PreProcessedData structure
    // This is a placeholder implementation; replace with your own method of obtaining liquidity
    (void)data;
    return 100000;
}

TradeSignal arbitrage_trading_strategy(const PreProcessedData *data)
{
    double dynamic_threshold = calculate_dynamic_threshold(data, base_threshold);
    double current_price_difference = data->price_differences[data->price_difference_count - 1];
    double liquidity = get_current_liquidity(data);
    bool trade_is_profitable = is_trade_profitable(current_price_difference, data->transaction_costs, data->latency, &data->liquidity_info, liquidity);

    if (trade_is_profitable && current_price_difference > dynamic_threshold && trend_strength(data) > 0)
    {
        double position_size = calculate_position_size(current_price_difference, &data->risk_management_params);
        TradeSignal signal;
        signal.action = BUY;
        signal.position_size = position_size;
        return signal;
    }
    else if (trade_is_profitable && current_price_difference < -dynamic_threshold && trend_strength(data) < 0)
    {
        double position_size = calculate_position_size(-current_price_difference, &data->risk_management_params);
        TradeSignal signal;
        signal.action = SELL;
        signal.position_size = position_size;
        return signal;
    }

    TradeSignal signal;
    signal.action = HOLD;
    signal.position_size = 0;
    return signal;
}

double trend_strength(const PreProcessedData *data)
{
    // Implement this function to calculate the trend strength based on the TrendInfo data
    return data->trend_info.trend_strength;
}

double calculate_dynamic_threshold(const PreProcessedData *data, double base_threshold)
{
    // Define a window size for the rolling standard deviation
    size_t window_size = 30;
    // Calculate the standard deviation of the historical price differences
    double standard_deviation = calculate_standard_deviation(data->price_differences, data->price_difference_count, window_size);

    // Scale the base threshold by the standard deviation to create a dynamic threshold
    return base_threshold * standard_deviation;
}

double calculate_position_size(double price_difference, const RiskManagementParams *params)
{
    // Calculate the position size based on the price difference and the risk management parameters
    double position_size = price_difference * params->risk_multiplier;

    // Limit the position size to the maximum allowed by the risk management parameters
    if (position_size > params->max_position_size)
    {
        position_size = params->max_position_size;
    }

    return position_size;
}

double calculate_standard_deviation(const double *values, size_t count, size_t window_size)
{
    if (count == 0)
    {
        return 0;
    }

    // Calculate the rolling standard deviation
    double sum = 0;
    for (size_t i = count - window_size; i < count; i++)
    {
        sum += values[i];
    }

    double mean = sum / window_size;

    double variance_sum = 0;
    for (size_t i = count - window_size; i < count; i++)
    {
        variance_sum += pow(values[i] - mean, 2);
    }

    double variance = variance_sum / window_size;

    return sqrt(variance);
}
