#include "risk_management.h"
#include "pre_processing.h"
#include "types.h"

// Placeholder functions for price retrieval
static double get_current_price(const PreProcessedData *data);
static double get_entry_price(const TradeSignal *trade_signal);

// Helper function prototypes
static bool should_stop_loss(const PreProcessedData *data, double current_price, double entry_price, TradeSignal trade_signal, const RiskManagementSettings *settings);
static bool is_position_within_limit(const PreProcessedData *data, double position_size, const RiskManagementSettings *settings);

void integrate_risk_management(const PreProcessedData *data, TradeSignal *trade_signal, const RiskManagementSettings *settings) {
    double current_price = get_current_price(data);
    double entry_price = get_entry_price(trade_signal);

    if (should_stop_loss(data, current_price, entry_price, *trade_signal, settings)) {
        trade_signal->action = HOLD;
    }

    if (!is_position_within_limit(data, trade_signal->position_size, settings)) {
        trade_signal->position_size = settings->calculate_position_limit(data, trade_signal->position_size);
    }

    settings->monitor_market_behavior(data);
}

// Implement placeholder functions
static double get_current_price(const PreProcessedData *data) {
    if (data->price_count > 0) {
        return data->prices[data->price_count - 1];
    }
    return 0;
}

// Implement get_entry_price
static double get_entry_price(const TradeSignal *trade_signal) {
    // Assuming the entry price is stored in the trade signal
    return trade_signal->entry_price;  // Use trade_signal, not data
}

// Helper function implementations
static bool should_stop_loss(const PreProcessedData *data, double current_price, double entry_price, TradeSignal trade_signal, const RiskManagementSettings *settings) {
    double dynamic_stop_loss = settings->calculate_dynamic_stop_loss(data, trade_signal);
    double price_change_percentage = 100 * (current_price - entry_price) / entry_price;

    if (trade_signal.action == BUY && -price_change_percentage >= dynamic_stop_loss) {
        return true;
    } else if (trade_signal.action == SELL && price_change_percentage >= dynamic_stop_loss) {
        return true;
    }

    return false;
}

static bool is_position_within_limit(const PreProcessedData *data, double position_size, const RiskManagementSettings *settings) {
    double position_limit = settings->calculate_position_limit(data, position_size);

    return position_size <= position_limit;
}
