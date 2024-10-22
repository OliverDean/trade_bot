#include "algorithm_execution.h"
#include "risk_management.h"
#include "types.h"
#include <stdbool.h>
#include <math.h>
#include "pre_processing.h"


// Helper function prototypes
static double calculate_dynamic_threshold(const PreProcessedData *data, double base_threshold);
static double calculate_position_size(double price_difference, const RiskManagementParams *params);
static double calculate_standard_deviation(const double *values, size_t count, size_t window_size);
static double trend_strength(const PreProcessedData *data);
static bool is_trade_profitable(double price_difference, double transaction_costs, double latency, const LiquidityInfo *liquidity_info, double liquidity);
static double get_current_liquidity(const PreProcessedData *data);

TradeSignal execute_algorithm(const PreProcessedData *data, TradingAlgorithm algorithm, const RiskManagementSettings *settings) {    // Execute the specific trading algorithm to generate a trade signal
    TradeSignal trade_signal = algorithm(data);

    // Integrate risk management with the trade signal
    integrate_risk_management(data, &trade_signal, settings);

    return trade_signal;
}

TradeSignal arbitrage_trading_strategy(const PreProcessedData *data) {
    const double base_threshold = 0.01;
    double dynamic_threshold = calculate_dynamic_threshold(data, base_threshold);
    double current_price_difference = data->price_differences[data->price_difference_count - 1];
    double liquidity = get_current_liquidity(data);
    bool trade_is_profitable = is_trade_profitable(
        current_price_difference,
        data->transaction_costs,
        data->latency,
        &data->liquidity_info,
        liquidity
    );

    if (trade_is_profitable && current_price_difference > dynamic_threshold && trend_strength(data) > 0) {
        double position_size = calculate_position_size(current_price_difference, &data->risk_management_params);
        TradeSignal signal = {.action = BUY, .position_size = position_size, .entry_price = data->prices[data->price_count - 1]};
        return signal;
    } else if (trade_is_profitable && current_price_difference < -dynamic_threshold && trend_strength(data) < 0) {
        double position_size = calculate_position_size(-current_price_difference, &data->risk_management_params);
        TradeSignal signal = {.action = SELL, .position_size = position_size, .entry_price = data->prices[data->price_count - 1]};
        return signal;
    }

    TradeSignal signal = {.action = HOLD, .position_size = 0.0, .entry_price = 0.0};
    return signal;
}

// Helper function implementations
static double calculate_dynamic_threshold(const PreProcessedData *data, double base_threshold) {
    size_t window_size = 30;
    double standard_deviation = calculate_standard_deviation(
        data->price_differences,
        data->price_difference_count,
        window_size
    );

    return base_threshold * standard_deviation;
}

static double calculate_standard_deviation(const double *values, size_t count, size_t window_size) {
    if (count < window_size) {
        return 0;
    }

    double sum = 0;
    for (size_t i = count - window_size; i < count; i++) {
        sum += values[i];
    }

    double mean = sum / window_size;

    double variance_sum = 0;
    for (size_t i = count - window_size; i < count; i++) {
        variance_sum += pow(values[i] - mean, 2);
    }

    double variance = variance_sum / window_size;
    return sqrt(variance);
}

static double calculate_position_size(double price_difference, const RiskManagementParams *params) {
    double position_size = price_difference * params->risk_multiplier;
    if (position_size > params->max_position_size) {
        position_size = params->max_position_size;
    }
    return position_size;
}

static double trend_strength(const PreProcessedData *data) {
    /**
     * @brief Calculates the strength of the current market trend.
     *
     * @param data Pointer to pre-processed data.
     * @return A double representing the trend strength (-1 to 1).
     */
    size_t n = data->price_count;
    if (n < data->trend_period) return 0.0;

    // Calculate linear regression slope over the trend period
    double sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_xx = 0.0;
    size_t start = n - data->trend_period;

    for (size_t i = 0; i < data->trend_period; i++) {
        double x = i;
        double y = data->prices[start + i];
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_xx += x * x;
    }

    double slope = (data->trend_period * sum_xy - sum_x * sum_y) / (data->trend_period * sum_xx - sum_x * sum_x);

    // Normalize the slope to a value between -1 and 1
    double max_slope = (data->prices[n - 1] - data->prices[start]) / data->trend_period;
    if (max_slope == 0) return 0.0;
    double trend_strength = slope / max_slope;

    return trend_strength;
}


static bool is_trade_profitable(double price_difference, double transaction_costs, double latency, const LiquidityInfo *liquidity_info, double liquidity) {
    // Calculate net expected profit
    double net_profit = price_difference - transaction_costs - latency * liquidity_info->slippage_factor;

    // Check liquidity constraints
    bool sufficient_liquidity = liquidity >= liquidity_info->minimum_liquidity;

    return (net_profit > 0) && sufficient_liquidity;
}

static double get_current_liquidity(const PreProcessedData *data) {

    if (data->liquidity_count > 0) {
        // Use an average over the last few data points for stability
        size_t window_size = 5;
        size_t count = data->liquidity_count < window_size ? data->liquidity_count : window_size;
        double sum = 0.0;
        for (size_t i = data->liquidity_count - count; i < data->liquidity_count; i++) {
            sum += data->liquidity[i];
        }
        return sum / count;
    }
    return 0.0;
}