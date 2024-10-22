#include "pre_processing.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include "config_parser.h"

static double *calculate_price_differences(const double *prices, size_t price_count);
static double *calculate_rolling_volatility(const double *price_differences, size_t price_difference_count, size_t window_size);
static void update_price_differences(PreProcessedData *data, const RawData *new_data) __attribute__((unused));
static void update_rolling_volatility(PreProcessedData *data, size_t window_size) __attribute__((unused));

// New statistical functions
static void calculate_MACD(PreProcessedData *data, int short_period, int long_period, int signal_period);
static void calculate_ATR(PreProcessedData *data, int period);
static void calculate_OBV(PreProcessedData *data, const double *volumes);
static void calculate_VWAP(PreProcessedData *data, const double *volumes);
static void calculate_Stochastic_Oscillator(PreProcessedData *data, int period);

PreProcessedData *pre_process_data(const RawData *raw_data, const ConfigParams *params) {
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
        params->rolling_volatility_window_size
    );
    data->rolling_volatility_count = data->price_difference_count - params->rolling_volatility_window_size + 1;

    data->liquidity = (double *)malloc(sizeof(double) * raw_data->liquidity_count);
    memcpy(data->liquidity, raw_data->liquidity, sizeof(double) * raw_data->liquidity_count);
    data->liquidity_count = raw_data->liquidity_count;

    // Initialize risk management parameters
    data->risk_management_params.risk_multiplier = params->risk_multiplier;
    data->risk_management_params.max_position_size = params->max_position_size;

    // Initialize liquidity info
    data->liquidity_info.minimum_liquidity = params->minimum_liquidity;

    // Initialize trend info
    data->trend_info.trend_strength = 1.0; // Placeholder

    // Calculate additional indicators
    calculate_MACD(data, params->macd_short_period, params->macd_long_period, params->macd_signal_period);
    calculate_ATR(data, params->atr_period);
    calculate_OBV(data, raw_data->volumes);
    calculate_VWAP(data, raw_data->volumes);
    calculate_Stochastic_Oscillator(data, params->stochastic_period);

    return data;
}

void free_pre_processed_data(PreProcessedData *data) {
    if (data) {
        free(data->prices);
        free(data->price_differences);
        free(data->rolling_volatility);
        free(data->liquidity);
        free(data->macd);
        free(data->signal_line);
        free(data->atr);
        free(data->obv);
        free(data->vwap);
        free(data->stochastic_k);
        free(data->stochastic_d);
        free(data);
    }
}

static double *calculate_price_differences(const double *prices, size_t price_count) {
    if (price_count < 2) return NULL;
    double *differences = (double *)malloc(sizeof(double) * (price_count - 1));
    if (!differences) return NULL;

    for (size_t i = 1; i < price_count; i++) {
        differences[i - 1] = prices[i] - prices[i - 1];
    }
    return differences;
}


static double *calculate_rolling_volatility(const double *price_differences, size_t price_difference_count, size_t window_size) {
    if (price_difference_count < window_size) return NULL;
    double *volatility = (double *)malloc(sizeof(double) * (price_difference_count - window_size + 1));
    if (!volatility) return NULL;

    for (size_t i = 0; i <= price_difference_count - window_size; i++) {
        double sum = 0;
        for (size_t j = 0; j < window_size; j++) {
            sum += price_differences[i + j] * price_differences[i + j];
        }
        volatility[i] = sqrt(sum / window_size);  // Standard deviation
    }
    return volatility;
}

static void update_price_differences(PreProcessedData *data, const RawData *new_data) {
    
    (void)data;      // Suppress unused parameter warning
    (void)new_data;
    // if (new_data->price_count < 2) return;

    // free(data->price_differences);
    // data->price_differences = calculate_price_differences(new_data->prices, new_data->price_count);
    // data->price_difference_count = new_data->price_count - 1;

}

static void update_rolling_volatility(PreProcessedData *data, size_t window_size) {
    
    (void)data;         // Suppress unused parameter warning
    (void)window_size;
    // free(data->rolling_volatility);
    // data->rolling_volatility = calculate_rolling_volatility(
    //     data->price_differences,
    //     data->price_difference_count,
    //     window_size
    // );
    // data->rolling_volatility_count = data->price_difference_count - window_size + 1;

}


// Implement the statistical functions
static void calculate_MACD(PreProcessedData *data, int short_period, int long_period, int signal_period) {
    size_t count = data->price_count;
    if (count < (size_t)long_period) return;

    data->macd = (double *)malloc(sizeof(double) * count);
    data->signal_line = (double *)malloc(sizeof(double) * count);

    double *ema_short = (double *)malloc(sizeof(double) * count);
    double *ema_long = (double *)malloc(sizeof(double) * count);

    double alpha_short = 2.0 / (short_period + 1);
    double alpha_long = 2.0 / (long_period + 1);
    double alpha_signal = 2.0 / (signal_period + 1);

    // Initialize EMAs
    ema_short[0] = data->prices[0];
    ema_long[0] = data->prices[0];

    // Calculate EMAs
    for (size_t i = 1; i < count; i++) {
        ema_short[i] = alpha_short * data->prices[i] + (1 - alpha_short) * ema_short[i - 1];
        ema_long[i] = alpha_long * data->prices[i] + (1 - alpha_long) * ema_long[i - 1];
    }

    // Calculate MACD line
    for (size_t i = 0; i < count; i++) {
        data->macd[i] = ema_short[i] - ema_long[i];
    }

    // Calculate Signal line
    data->signal_line[0] = data->macd[0];
    for (size_t i = 1; i < count; i++) {
        data->signal_line[i] = alpha_signal * data->macd[i] + (1 - alpha_signal) * data->signal_line[i - 1];
    }

    data->macd_count = count;

    free(ema_short);
    free(ema_long);
}

void calculate_ATR(PreProcessedData *data, int period) {
    if (data->price_count < (size_t)period) {
        return;
    }

    data->atr = (double *)malloc(sizeof(double) * data->price_count);
    data->atr_count = data->price_count;

    double sum_tr = 0.0;

    for (size_t i = 0; i < data->price_count; i++) {
        double tr;
        if (i == 0) {
            tr = data->high_prices[i] - data->low_prices[i];
        } else {
            double hl = data->high_prices[i] - data->low_prices[i];
            double hc = fabs(data->high_prices[i] - data->prices[i - 1]);
            double lc = fabs(data->low_prices[i] - data->prices[i - 1]);
            tr = fmax(hl, fmax(hc, lc));
        }
        if (i < (size_t)period) {
            sum_tr += tr;
            data->atr[i] = 0.0; // Not enough data to compute ATR
        } else if (i == (size_t)period) {
            sum_tr += tr;
            data->atr[i] = sum_tr / period;
        } else {
            data->atr[i] = ((data->atr[i - 1] * (period - 1)) + tr) / period;
        }
    }
}

static void calculate_OBV(PreProcessedData *data, const double *volumes) {
    size_t count = data->price_count;
    data->obv = (double *)malloc(sizeof(double) * count);
    data->obv[0] = volumes[0];
    for (size_t i = 1; i < count; i++) {
        if (data->prices[i] > data->prices[i - 1]) {
            data->obv[i] = data->obv[i - 1] + volumes[i];
        } else if (data->prices[i] < data->prices[i - 1]) {
            data->obv[i] = data->obv[i - 1] - volumes[i];
        } else {
            data->obv[i] = data->obv[i - 1];
        }
    }
    data->obv_count = count;
}

static void calculate_VWAP(PreProcessedData *data, const double *volumes) {
    size_t count = data->price_count;
    data->vwap = (double *)malloc(sizeof(double) * count);
    double cumulative_price_volume = 0;
    double cumulative_volume = 0;
    for (size_t i = 0; i < count; i++) {
        cumulative_price_volume += data->prices[i] * volumes[i];
        cumulative_volume += volumes[i];
        data->vwap[i] = cumulative_price_volume / cumulative_volume;
    }
    data->vwap_count = count;
}

static void calculate_Stochastic_Oscillator(PreProcessedData *data, int period) {
    size_t count = data->price_count;
    if (count < (size_t)period) return;

    data->stochastic_k = (double *)malloc(sizeof(double) * count);
    data->stochastic_d = (double *)malloc(sizeof(double) * count);

    for (size_t i = (size_t)period - 1; i < count; i++) {
        double highest_high = data->prices[i];
        double lowest_low = data->prices[i];
        for (int j = 0; j < period; j++) {
            if (data->prices[i - j] > highest_high)
                highest_high = data->prices[i - j];
            if (data->prices[i - j] < lowest_low)
                lowest_low = data->prices[i - j];
        }
        data->stochastic_k[i] = 100 * ((data->prices[i] - lowest_low) / (highest_high - lowest_low));
        if (i >= (size_t)period + 2) {
            data->stochastic_d[i] = (data->stochastic_k[i] + data->stochastic_k[i - 1] + data->stochastic_k[i - 2]) / 3;
        } else {
            data->stochastic_d[i] = data->stochastic_k[i];
        }
    }
    data->stochastic_count = count;
}
