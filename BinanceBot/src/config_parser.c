#include "config_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <libconfig.h>

int load_config(const char *config_file_path, ConfigParams *params)
{
    config_t cfg;
    config_init(&cfg);

    if (!config_read_file(&cfg, config_file_path))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return(EXIT_FAILURE);
    }

    if (config_lookup_int(&cfg, "DEFAULTS.DEFAULT_CALCULATION_INTERVAL", &(params->default_calculation_interval)) &&
        config_lookup_int(&cfg, "DEFAULTS.MAX_RECORDS_TO_PROCESS", &(params->max_records_to_process)) &&
        config_lookup_int(&cfg, "DEFAULTS.WINDOW_SIZE", &(params->window_size)) &&
        config_lookup_float(&cfg, "DEFAULTS.EMA_ALPHA", &(params->ema_alpha)) &&
        config_lookup_int(&cfg, "DEFAULTS.RSI_PERIOD", &(params->rsi_period)) &&
        config_lookup_float(&cfg, "DEFAULTS.BOLLINGER_MULTIPLIER", &(params->bollinger_multiplier)))
    {
        config_destroy(&cfg);
        return(EXIT_SUCCESS);
    }
    else
    {
        fprintf(stderr, "Missing default settings in configuration file.\n");
        config_destroy(&cfg);
        return(EXIT_FAILURE);
    }
}
