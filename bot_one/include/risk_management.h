#ifndef RISK_MANAGEMENT_H
#define RISK_MANAGEMENT_H

#include "pre_processing.h"
#include "types.h"
#include <stdbool.h>

typedef struct {
    double (*calculate_dynamic_stop_loss)(const PreProcessedData *, TradeSignal);
    double (*calculate_position_limit)(const PreProcessedData *, double position_size);
    void (*monitor_market_behavior)(const PreProcessedData *);
} RiskManagementSettings;

void integrate_risk_management(const PreProcessedData *data, TradeSignal *trade_signal, const RiskManagementSettings *settings);

#endif // RISK_MANAGEMENT_H
