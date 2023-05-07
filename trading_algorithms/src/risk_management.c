// risk_management.h
#include "pre_processing.h"
#include "algorithm_execution.h"
#include <stdbool.h>
#include <stddef.h>
#include <math.h>

typedef struct {
    double (*calculate_dynamic_stop_loss)(const PreProcessedData *, TradeSignal);
    double (*calculate_position_limit)(const PreProcessedData *, double position_size);
    void (*monitor_market_behavior)(const PreProcessedData *);
} RiskManagementSettings;

bool should_stop_loss(const PreProcessedData *data, const double current_price, const double entry_price, TradeSignal trade_signal, const RiskManagementSettings *settings);
bool is_position_within_limit(const PreProcessedData *data, double position_size, const RiskManagementSettings *settings);

// risk_management.c
#include "risk_management.h"

bool should_stop_loss(const PreProcessedData *data, const double current_price, const double entry_price, TradeSignal trade_signal, const RiskManagementSettings *settings) {
    double dynamic_stop_loss = settings->calculate_dynamic_stop_loss(data, trade_signal);
    double price_change_percentage = 100 * (current_price - entry_price) / entry_price;

    if (trade_signal.action == BUY && -price_change_percentage >= dynamic_stop_loss) {
        return true;
    } else if (trade_signal.action == SELL && price_change_percentage >= dynamic_stop_loss) {
        return true;
    }

    return false;
}

bool is_position_within_limit(const PreProcessedData *data, double position_size, const RiskManagementSettings *settings) {
    double position_limit = settings->calculate_position_limit(data, position_size);

    if (position_size <= position_limit) {
        return true;
    }

    return false;
}

void integrate_risk_management(const PreProcessedData *data, TradeSignal *trade_signal, const RiskManagementSettings *settings) {
    double current_price = get_current_price(data); // Implement this function
    double entry_price = get_entry_price(data); // Implement this function

    if (should_stop_loss(data, current_price, entry_price, *trade_signal, settings)) {
        trade_signal->action = HOLD;
    }

    if (!is_position_within_limit(data, trade_signal->position_size, settings)) {
        trade_signal->position_size = settings->calculate_position_limit(data, trade_signal->position_size);
    }

    settings->monitor_market_behavior(data);
}


double calculate_historical_volatility(const PreProcessedData *data, size_t window_size) {
    if (!data || data->rolling_volatility_count == 0 || window_size > data->rolling_volatility_count) {
        return -1.0; // Return an error code if the input data is invalid or the window size is too large
    }

    double sum = 0.0;

    for (size_t i = data->rolling_volatility_count - window_size; i < data->rolling_volatility_count; i++) {
        sum += data->rolling_volatility[i];
    }

    return sum / window_size;
}

double calculate_support_level(const PreProcessedData *data) {
    if (!data) {
        return -1.0; // Return an error code if the input data is invalid
    }
    // Assuming the support level calculation has been added to the PreProcessedData structure
    return data->support_level;
}

double calculate_resistance_level(const PreProcessedData *data) {
    if (!data) {
        return -1.0; // Return an error code if the input data is invalid
    }
    // Assuming the resistance level calculation has been added to the PreProcessedData structure
    return data->resistance_level;
}

bool detect_flash_crash(const PreProcessedData *data, double threshold) {
    if (!data || data->rolling_volatility_count == 0) {
        return false; // Return false if the input data is invalid
    }

    for (size_t i = 1; i < data->rolling_volatility_count; i++) {
        double price_change = (data->prices[i] - data->prices[i - 1]) / data->prices[i - 1];
        if (price_change < -threshold) {
            return true;
        }
    }

    return false;
}

bool detect_high_volatility(const PreProcessedData *data, double threshold) {
    if (!data || data->rolling_volatility_count == 0) {
        return false; // Return false if the input data is invalid
    }

    for (size_t i = 0; i < data->rolling_volatility_count; i++) {
        if (data->rolling_volatility[i] > threshold) {
            return true;
        }
    }

    return false;
}

bool detect_sudden_liquidity_drop(const PreProcessedData *data, double threshold) {
    if (!data || data->liquidity_count == 0) {
        return false; // Return false if the input data is invalid
    }

    for (size_t i = 1; i < data->liquidity_count; i++) {
        double liquidity_change = (data->liquidity[i] - data->liquidity[i - 1]) / data->liquidity[i - 1];
        if (liquidity_change < -threshold) {
            return true;
        }
    }

    return false;
}
