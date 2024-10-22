#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <stddef.h>
#include "types.h"

typedef struct {
    // Trading parameters
    int default_calculation_interval;
    size_t max_records_to_process;
    size_t window_size;
    double ema_alpha;
    int rsi_period;
    double bollinger_multiplier;

    // API and symbol configuration
    char api_key[128];
    char api_secret[128];
    char symbol[16];
    char interval[16];
    char start_date[32];
    char end_date[32];
    
    // Add these fields for millisecond timestamps
    long long start_time_ms;  // Start time in milliseconds
    long long end_time_ms;    // End time in milliseconds

    // Risk management parameters
    double risk_multiplier;
    double max_position_size;
    double minimum_liquidity;

    // Advanced parameters
    int rolling_volatility_window_size;

    // Statistical analysis parameters
    int macd_short_period;
    int macd_long_period;
    int macd_signal_period;
    int atr_period;
    int stochastic_period;

    double transaction_costs;
    double latency;
    RiskManagementParams risk_management_params;
    LiquidityInfo liquidity_info;
    TrendInfo trend_info;
    size_t trend_period;
    // Add other necessary fields
} ConfigParams;

// Function to load config
int load_config(const char *config_file_path, ConfigParams *params);

#endif // CONFIG_PARSER_H