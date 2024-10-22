#include "pre_processing.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

// Function prototypes
static double *calculate_price_differences(const double *prices, size_t price_count);
static double *calculate_rolling_volatility(const double *price_differences, size_t price_difference_count, size_t window_size);
static void update_price_differences(PreProcessedData *data, const RawData *new_data);
static void update_rolling_volatility(PreProcessedData *data, size_t window_size);

PreProcessedData *pre_process_data(const RawData *raw_data, size_t rolling_volatility_window_size) {
    PreProcessedData *data = (PreProcessedData *)malloc(sizeof(PreProcessedData));
    if (!data) {
        return NULL;
    }

    data->prices = (double *)malloc(sizeof(double) * raw_data->price_count);
    memcpy(data->prices, raw_data->prices, sizeof(double) * raw_data->price_count);
    data->price_count = raw_data->price_count;

    data->price_differences = calculate_price_differences(raw_data->prices, raw_data->price_count);
    data->price_difference_count = raw_data->price_count - 1;

    data->transaction_costs = raw_data->transaction_costs;
    data->latency = raw_data->latency;

    data->rolling_volatility = calculate_rolling_volatility(
        data->price_differences,
        data->price_difference_count,
        rolling_volatility_window_size
    );
    data->rolling_volatility_count = data->price_difference_count - rolling_volatility_window_size + 1;

    data->liquidity = (double *)malloc(sizeof(double) * raw_data->liquidity_count);
    memcpy(data->liquidity, raw_data->liquidity, sizeof(double) * raw_data->liquidity_count);
    data->liquidity_count = raw_data->liquidity_count;

    // Initialize risk management parameters
    data->risk_management_params.risk_multiplier = 1.0;
    data->risk_management_params.max_position_size = 1000.0;

    // Initialize liquidity info
    data->liquidity_info.minimum_liquidity = 10000.0;

    // Initialize trend info
    data->trend_info.trend_strength = 1.0; // Placeholder

    return data;
}

void free_pre_processed_data(PreProcessedData *data) {
    if (data) {
        free(data->prices);
        free(data->price_differences);
        free(data->rolling_volatility);
        free(data->liquidity);
        free(data);
    }
}

void update_pre_processed_data(PreProcessedData *data, const RawData *new_data) {
    // Update prices
    size_t new_total_prices = data->price_count + new_data->price_count;
    data->prices = (double *)realloc(data->prices, sizeof(double) * new_total_prices);
    memcpy(data->prices + data->price_count, new_data->prices, sizeof(double) * new_data->price_count);
    data->price_count = new_total_prices;

    // Update price differences
    update_price_differences(data, new_data);

    // Update rolling volatility
    update_rolling_volatility(data, 30); // Using a window size of 30

    // Update liquidity
    size_t new_total_liquidity = data->liquidity_count + new_data->liquidity_count;
    data->liquidity = (double *)realloc(data->liquidity, sizeof(double) * new_total_liquidity);
    memcpy(data->liquidity + data->liquidity_count, new_data->liquidity, sizeof(double) * new_data->liquidity_count);
    data->liquidity_count = new_total_liquidity;

    // Update other fields if necessary
}

// Helper function implementations
static double *calculate_price_differences(const double *prices, size_t price_count) {
    if (price_count < 2) {
        return NULL;
    }

    double *price_differences = (double *)malloc(sizeof(double) * (price_count - 1));
    for (size_t i = 1; i < price_count; i++) {
        price_differences[i - 1] = prices[i] - prices[i - 1];
    }
    return price_differences;
}

static double *calculate_rolling_volatility(const double *price_differences, size_t price_difference_count, size_t window_size) {
    if (price_difference_count < window_size) {
        return NULL;
    }

    size_t volatility_count = price_difference_count - window_size + 1;
    double *rolling_volatility = (double *)malloc(sizeof(double) * volatility_count);

    double sum = 0;
    double sum_sq = 0;
    for (size_t i = 0; i < window_size; i++) {
        sum += price_differences[i];
        sum_sq += price_differences[i] * price_differences[i];
    }

    for (size_t i = 0; i < volatility_count; i++) {
        if (i > 0) {
            sum -= price_differences[i - 1];
            sum += price_differences[i + window_size - 1];
            sum_sq -= price_differences[i - 1] * price_differences[i - 1];
            sum_sq += price_differences[i + window_size - 1] * price_differences[i + window_size - 1];
        }

        double mean = sum / window_size;
        double variance = (sum_sq / window_size) - (mean * mean);
        rolling_volatility[i] = sqrt(variance);
    }

    return rolling_volatility;
}

static void update_price_differences(PreProcessedData *data, const RawData *new_data) {
    size_t new_price_difference_count = new_data->price_count - 1;
    size_t total_price_difference_count = data->price_difference_count + new_price_difference_count + 1; // +1 for the difference between last old price and first new price

    data->price_differences = (double *)realloc(data->price_differences, sizeof(double) * total_price_difference_count);

    // Compute the difference between last old price and first new price
    data->price_differences[data->price_difference_count] = new_data->prices[0] - data->prices[data->price_count - new_data->price_count - 1];

    // Compute new price differences
    for (size_t i = 1; i < new_data->price_count; i++) {
        data->price_differences[data->price_difference_count + i] = new_data->prices[i] - new_data->prices[i - 1];
    }

    data->price_difference_count = total_price_difference_count;
}

static void update_rolling_volatility(PreProcessedData *data, size_t window_size) {
    if (data->price_difference_count < window_size) {
        return;
    }

    size_t new_volatility_count = data->price_difference_count - window_size + 1;
    data->rolling_volatility = (double *)realloc(data->rolling_volatility, sizeof(double) * new_volatility_count);

    double sum = 0;
    double sum_sq = 0;
    size_t start_idx = data->price_difference_count - new_volatility_count - window_size + 1;

    for (size_t i = start_idx; i < start_idx + window_size; i++) {
        sum += data->price_differences[i];
        sum_sq += data->price_differences[i] * data->price_differences[i];
    }

    for (size_t i = 0; i < new_volatility_count; i++) {
        if (i > 0) {
            sum -= data->price_differences[start_idx + i - 1];
            sum += data->price_differences[start_idx + i + window_size - 1];
            sum_sq -= data->price_differences[start_idx + i - 1] * data->price_differences[start_idx + i - 1];
            sum_sq += data->price_differences[start_idx + i + window_size - 1] * data->price_differences[start_idx + i + window_size - 1];
        }

        double mean = sum / window_size;
        double variance = (sum_sq / window_size) - (mean * mean);
        data->rolling_volatility[data->rolling_volatility_count + i] = sqrt(variance);
    }

    data->rolling_volatility_count += new_volatility_count;
}
