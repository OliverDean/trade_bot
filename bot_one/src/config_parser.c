#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config_parser.h"
#include <libconfig.h>

int load_config(const char *config_file_path, ConfigParams *params) {
    config_t cfg;
    config_setting_t *setting;
    const char *str;
    config_init(&cfg);

    if (!config_read_file(&cfg, config_file_path)) {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return(EXIT_FAILURE);
    }

    // Load DEFAULTS
    if ((setting = config_lookup(&cfg, "DEFAULTS")) != NULL) {
        config_setting_lookup_int(setting, "DEFAULT_CALCULATION_INTERVAL", &params->default_calculation_interval);
        
        // Temporary int variables for size_t fields
        int max_records_to_process_tmp;
        config_setting_lookup_int(setting, "MAX_RECORDS_TO_PROCESS", &max_records_to_process_tmp);
        params->max_records_to_process = (size_t)max_records_to_process_tmp;
        
        int window_size_tmp;
        config_setting_lookup_int(setting, "WINDOW_SIZE", &window_size_tmp);
        params->window_size = (size_t)window_size_tmp;
        
        config_setting_lookup_float(setting, "EMA_ALPHA", &params->ema_alpha);
        config_setting_lookup_int(setting, "RSI_PERIOD", &params->rsi_period);
        config_setting_lookup_float(setting, "BOLLINGER_MULTIPLIER", &params->bollinger_multiplier);
    }

    // Load API
    if ((setting = config_lookup(&cfg, "API")) != NULL) {
        if (config_setting_lookup_string(setting, "API_KEY", &str))
            snprintf(params->api_key, sizeof(params->api_key), "%s", str);
        if (config_setting_lookup_string(setting, "API_SECRET", &str))
            snprintf(params->api_secret, sizeof(params->api_secret), "%s", str);
        if (config_setting_lookup_string(setting, "SYMBOL", &str))
            snprintf(params->symbol, sizeof(params->symbol), "%s", str);
        if (config_setting_lookup_string(setting, "INTERVAL", &str))
            snprintf(params->interval, sizeof(params->interval), "%s", str);
        if (config_setting_lookup_string(setting, "START_DATE", &str))
            snprintf(params->start_date, sizeof(params->start_date), "%s", str);
        if (config_setting_lookup_string(setting, "END_DATE", &str))
            snprintf(params->end_date, sizeof(params->end_date), "%s", str);
    }

    // Load RISK_MANAGEMENT
    if ((setting = config_lookup(&cfg, "RISK_MANAGEMENT")) != NULL) {
        config_setting_lookup_float(setting, "RISK_MULTIPLIER", &params->risk_multiplier);
        config_setting_lookup_float(setting, "MAX_POSITION_SIZE", &params->max_position_size);
        config_setting_lookup_float(setting, "MINIMUM_LIQUIDITY", &params->minimum_liquidity);
    }

    // Load ADVANCED
    if ((setting = config_lookup(&cfg, "ADVANCED")) != NULL) {
        config_setting_lookup_int(setting, "ROLLING_VOLATILITY_WINDOW_SIZE", &params->rolling_volatility_window_size);
    }

    // Load STATISTICAL_ANALYSIS
    if ((setting = config_lookup(&cfg, "STATISTICAL_ANALYSIS")) != NULL) {
        config_setting_lookup_int(setting, "MACD_SHORT_PERIOD", &params->macd_short_period);
        config_setting_lookup_int(setting, "MACD_LONG_PERIOD", &params->macd_long_period);
        config_setting_lookup_int(setting, "MACD_SIGNAL_PERIOD", &params->macd_signal_period);
        config_setting_lookup_int(setting, "ATR_PERIOD", &params->atr_period);
        config_setting_lookup_int(setting, "STOCHASTIC_PERIOD", &params->stochastic_period);
    }

    config_destroy(&cfg);
    return 0;
}
