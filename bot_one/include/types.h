// include/types.h
#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

typedef enum {
    BUY,
    SELL,
    HOLD
} TradeAction;

typedef struct {
    TradeAction action;
    double position_size;
    double entry_price;
} TradeSignal;

typedef struct {
    double risk_multiplier;
    double max_position_size;
} RiskManagementParams;

typedef struct {
    double minimum_liquidity;
    double slippage_factor; // New field to represent slippage per unit time
} LiquidityInfo;

typedef struct {
    double trend_strength;
} TrendInfo;

#endif // TYPES_H
