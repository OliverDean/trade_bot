// pre_processing.h
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <stddef.h>
#include <float.h>
// pre_processing.c
#include "pre_processing.h"
#include <stdlib.h>
#include <math.h>
#include "market_data.h"

#define DEFAULT_CALCULATION_INTERVAL 1000
#define MAX_RECORDS_TO_PROCESS 10000


PreProcessedData *pre_process_data(const RawData *raw_data, size_t rolling_volatility_window_size, size_t custom_window_size) {
    PreProcessedData *data = (PreProcessedData *)malloc(sizeof(PreProcessedData));
    data->price_differences = calculate_price_differences(raw_data->prices, raw_data->price_count);
    data->price_difference_count = raw_data->price_count - 1;
    data->transaction_costs = raw_data->transaction_costs;
    data->latency = raw_data->latency;
    data->rolling_volatilities = calculate_rolling_volatilities(data->price_differences, data->price_difference_count, custom_window_size ? custom_window_size : rolling_volatility_window_size);
    data->rolling_volatility_count = raw_data->price_count - rolling_volatility_window_size;
    // Additional pre-processing steps, such as calculating support and resistance levels, can be added here
    return data;
}

void free_pre_processed_data(PreProcessedData *data) {
    free(data->price_differences);
    free(data->rolling_volatilities);
    // Free any additional allocated memory for other data fields
    free(data);
}

void update_pre_processed_data(PreProcessedData *data, const RawData *new_data) {
    // Update the price differences with the new price data
    update_price_differences(data, new_data);
    
    // Update the rolling volatilities with the new price data
    update_rolling_volatilities(data, new_data, data->rolling_volatility_count);
    
    // Update any additional data fields based on the new raw data
}

// Additional helper functions for the pre-processing module
double *calculate_price_differences(const double *prices, size_t price_count);
double *calculate_rolling_volatility(const double *price_differences, size_t price_difference_count, size_t window_size);

double *calculate_price_differences(const double *prices, size_t price_count) {
    double *price_differences = (double *)malloc(sizeof(double) * (price_count - 1));

    for (size_t i = 0; i < price_count - 1; i++) {
        price_differences[i] = prices[i + 1] - prices[i];
    }

    return price_differences;
}

double *calculate_rolling_volatility(const double *price_differences, size_t price_difference_count, size_t window_size) {
    double *rolling_volatilities = (double *)malloc(sizeof(double) * (price_difference_count - window_size + 1));
    double sum = 0, mean = 0, variance = 0;

    // Initialize the first window
    for (size_t i = 0; i < window_size; i++) {
        sum += price_differences[i];
    }
    mean = sum / window_size;

    for (size_t i = 0; i < window_size; i++) {
        variance += pow(price_differences[i] - mean, 2);
    }
    variance /= window_size;

    rolling_volatilities[0] = sqrt(variance);

    // Calculate the rolling volatilities for the rest of the dataset
    for (size_t i = 1; i < price_difference_count - window_size + 1; i++) {
        sum = sum - price_differences[i - 1] + price_differences[i + window_size - 1];
        mean = sum / window_size;

        variance = 0;
        for (size_t j = i; j < i + window_size; j++) {
            variance += pow(price_differences[j] - mean, 2);
        }
        variance /= window_size;

        rolling_volatilities[i] = sqrt(variance);
    }

    return rolling_volatilities;
}


// Helper function to find pivot points
double calculate_pivot_point(const double *prices, size_t price_count);

// Additional functions for the pre-processing module to interact with support and resistance levels
double calculate_support_level(const double *prices, size_t price_count);
double calculate_resistance_level(const double *prices, size_t price_count);

double calculate_pivot_point(const double *prices, size_t price_count) {
    double high = -DBL_MAX;
    double low = DBL_MAX;
    double close = prices[price_count - 1];

    for (size_t i = 0; i < price_count; i++) {
        if (prices[i] > high) {
            high = prices[i];
        }
        if (prices[i] < low) {
            low = prices[i];
        }
    }

    return (high + low + close) / 3.0;
}

double calculate_support_level(const double *prices, size_t price_count) {
    double pivot_point = calculate_pivot_point(prices, price_count);
    double high = -DBL_MAX;

    for (size_t i = 0; i < price_count; i++) {
        if (prices[i] > high) {
            high = prices[i];
        }
    }

    return 2 * pivot_point - high;
}

double calculate_resistance_level(const double *prices, size_t price_count) {
    double pivot_point = calculate_pivot_point(prices, price_count);
    double low = DBL_MAX;

    for (size_t i = 0; i < price_count; i++) {
        if (prices[i] < low) {
            low = prices[i];
        }
    }

    return 2 * pivot_point - low;
}

void update_price_differences(PreProcessedData *data, const RawData *new_data) {
    size_t new_price_difference_count = new_data->price_count - 1;
    double *new_price_differences = calculate_price_differences(new_data->prices, new_data->price_count);

    double *updated_price_differences = (double *)realloc(data->price_differences, sizeof(double) * (data->price_difference_count + new_price_difference_count));
    if (updated_price_differences == NULL) {
        // Handle the error, e.g., log the error and exit the function
        return;
    }
    data->price_differences = updated_price_differences;

    for (size_t i = 0; i < new_price_difference_count; i++) {
        data->price_differences[data->price_difference_count + i] = new_price_differences[i];
    }
    data->price_difference_count += new_price_difference_count;

    free(new_price_differences);
}

void update_rolling_volatilities(PreProcessedData *data, const RawData *new_data, size_t window_size) {
    size_t new_price_difference_count = new_data->price_count - 1;
    double *new_price_differences = calculate_price_differences(new_data->prices, new_data->price_count);

    double *extended_price_differences = (double *)realloc(data->price_differences, sizeof(double) * (data->price_difference_count + new_price_difference_count));
    if (extended_price_differences == NULL) {
        // Handle the error, e.g., log the error and exit the function
        return;
    }
    data->price_differences = extended_price_differences;

    for (size_t i = 0; i < new_price_difference_count; i++) {
        data->price_differences[data->price_difference_count + i] = new_price_differences[i];
    }
    data->price_difference_count += new_price_difference_count;

    double *new_rolling_volatilities = calculate_rolling_volatility(data->price_differences, data->price_difference_count, window_size);
    size_t new_rolling_volatility_count = data->price_difference_count - window_size + 1;

    double *updated_rolling_volatilities = (double *)realloc(data->rolling_volatilities, sizeof(double) * new_rolling_volatility_count);
    if (updated_rolling_volatilities == NULL) {
        // Handle the error, e.g., log the error and exit the function
        return;
    }
    data->rolling_volatilities = updated_rolling_volatilities;

    for (size_t i = 0; i < new_rolling_volatility_count; i++) {
        data->rolling_volatilities[i] = new_rolling_volatilities[i];
    }
    data->rolling_volatility_count = new_rolling_volatility_count;

    free(new_price_differences);
    free(new_rolling_volatilities);
}

