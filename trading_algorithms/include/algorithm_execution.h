// algorithm_execution.h

#ifndef ALGORITHM_EXECUTION_H
#define ALGORITHM_EXECUTION_H

#include "pre_processing.h"
#include <stddef.h>

typedef enum { BUY, SELL, HOLD } TradeSignal;
typedef TradeSignal (*TradingAlgorithm)(const PreProcessedData *);

TradeSignal moving_average_crossover_strategy(const PreProcessedData *data);
TradeSignal mean_reversion_strategy(const PreProcessedData *data);
TradeSignal momentum_trading_strategy(const PreProcessedData *data);
TradeSignal pairs_trading_strategy(const PreProcessedData *data1, const PreProcessedData *data2, size_t spread_window, double dynamic_threshold_factor);
TradeSignal arbitrage_trading_strategy(const PreProcessedData *data);

#endif
