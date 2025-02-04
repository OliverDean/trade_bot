#ifndef ALGORITHM_EXECUTION_H
#define ALGORITHM_EXECUTION_H

#include "pre_processing.h"
#include "risk_management.h"

typedef enum {
    BUY,
    SELL,
    HOLD
} TradeAction;

typedef struct {
    TradeAction action;
    double position_size;
} TradeSignal;

typedef TradeSignal (*TradingAlgorithm)(const PreProcessedData *);

TradeSignal arbitrage_trading_strategy(const PreProcessedData *data);
TradeSignal execute_algorithm(const PreProcessedData *data, TradingAlgorithm algorithm, const RiskManagementSettings *settings);

#endif // ALGORITHM_EXECUTION_H
