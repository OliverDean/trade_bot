// risk_management.h

#ifndef RISK_MANAGEMENT_H
#define RISK_MANAGEMENT_H

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

double calculate_historical_volatility(const PreProcessedData *data, size_t window_size);
bool detect_flash_crash(const PreProcessedData *data, double threshold);
bool detect_high_volatility(const PreProcessedData *data, double threshold);
bool detect_sudden_liquidity_drop(const PreProcessedData *data, double threshold);

void integrate_risk_management(const PreProcessedData *data, TradeSignal *trade_signal, const RiskManagementSettings *settings);

#endif // RISK_MANAGEMENT_H
