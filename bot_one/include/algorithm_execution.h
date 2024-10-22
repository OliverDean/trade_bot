#ifndef ALGORITHM_EXECUTION_H
#define ALGORITHM_EXECUTION_H

#include "pre_processing.h"
#include "types.h"
#include "risk_management.h"


typedef TradeSignal (*TradingAlgorithm)(const PreProcessedData *);

TradeSignal arbitrage_trading_strategy(const PreProcessedData *data);
TradeSignal execute_algorithm(const PreProcessedData *data, TradingAlgorithm algorithm, const RiskManagementSettings *settings);

#endif // ALGORITHM_EXECUTION_H
