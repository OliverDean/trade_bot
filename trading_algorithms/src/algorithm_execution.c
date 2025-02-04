#include "algorithm_execution.h"
#include <math.h>

// Helper function prototypes
static double calculate_dynamic_threshold(const PreProcessedData *data, double base_threshold);
static double calculate_position_size(double price_difference, const RiskManagementParams *params);
static double calculate_standard_deviation(const double *values, size_t count, size_t window_size);
static double trend_strength(const PreProcessedData *data);
static bool is_trade_profitable(double price_difference, double transaction_costs, double latency, const LiquidityInfo *liquidity_info, double liquidity);
static double get_current_liquidity(const PreProcessedData *data);

TradeSignal execute_algorithm(const PreProcessedData *data, TradingAlgorithm algorithm, const RiskManagementSettings *settings) {
    // Execute the specific trading algorithm to generate a trade signal
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
        TradeSignal signal = {BUY, position_size};
        return signal;
    } else if (trade_is_profitable && current_price_difference < -dynamic_threshold && trend_strength(data) < 0) {
        double position_size = calculate_position_size(-current_price_difference, &data->risk_management_params);
        TradeSignal signal = {SELL, position_size};
        return signal;
    }

    TradeSignal signal = {HOLD, 0};
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
    // Placeholder implementation; replace with actual trend calculation
    return data->trend_info.trend_strength;
}

static bool is_trade_profitable(double price_difference, double transaction_costs, double latency, const LiquidityInfo *liquidity_info, double liquidity) {
    return (price_difference - transaction_costs - latency > 0) && (liquidity >= liquidity_info->minimum_liquidity);
}

static double get_current_liquidity(const PreProcessedData *data) {
    // Placeholder implementation; replace with actual liquidity retrieval
    if (data->liquidity_count > 0) {
        return data->liquidity[data->liquidity_count - 1];
    }
    return 0;
}
