#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

typedef struct {
    int default_calculation_interval;
    int max_records_to_process;
    int window_size;
    double ema_alpha;
    int rsi_period;
    double bollinger_multiplier;
} ConfigParams;

int load_config(const char *config_file_path, ConfigParams *params);

#endif // CONFIG_PARSER_H
