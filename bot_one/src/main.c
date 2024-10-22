#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "market_data.h"
#include "market_data_pool.h"
#include "lock_free_queue.h"
#include "pre_processing.h"
#include "config_parser.h"
#include "algorithm_execution.h"
#include "risk_management.h"
#include "data_fetcher.h"
#include "types.h"

// Global variables for synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

#define MAX_WINDOW_SIZE 1000

int running = 1;

// Placeholder function to fetch market data
MarketData *fetch_market_data() {
    // Implement actual market data fetching logic
    static double price = 100.0;
    MarketData *data = (MarketData *)malloc(sizeof(MarketData));
    data->price = price;
    data->volume = rand() % 1000 + 1;
    price += (rand() % 100 - 50) * 0.01;
    return data;
}

void *data_ingestion_thread(void *args) {
    LockFreeQueue *queue = (LockFreeQueue *)args;
    while (running) {
        MarketData *data = fetch_market_data();
        pthread_mutex_lock(&mutex);
        lock_free_queue_enqueue(queue, data);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        sleep(10); // Sleep for 100 milliseconds
    }
    return NULL;
}

void *pre_processing_thread(void *args) {
    PreProcessingArgs *pre_processing_args = (PreProcessingArgs *)args;
    size_t window_size = pre_processing_args->window_size;
    size_t rsi_period = pre_processing_args->rsi_period;
    double ema_alpha = pre_processing_args->ema_alpha;
    double bollinger_multiplier = pre_processing_args->bollinger_multiplier;

    double prices[MAX_WINDOW_SIZE] = {0};
    double gains[MAX_WINDOW_SIZE] = {0};
    double losses[MAX_WINDOW_SIZE] = {0};

    size_t price_index = 0;
    size_t gain_loss_index = 0;
    size_t n = 0;

    double window_price_sum = 0.0;
    double prev_ema = 0.0;
    double mean = 0.0;
    double variance = 0.0;
    double avg_gain = 0.0, avg_loss = 0.0;
    double prev_price = 0.0;

    while (running) {
        pthread_mutex_lock(&mutex);
        while (queue_is_empty(pre_processing_args->input_queue)) {
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        MarketData *data = (MarketData *)lock_free_queue_dequeue(pre_processing_args->input_queue);
        if (data == NULL) continue;

        // Update price buffer
        double old_price = prices[price_index];
        prices[price_index] = data->price;

        // Update moving average
        if (n < window_size) {
            window_price_sum += data->price;
            n++;
            data->moving_average = window_price_sum / n;
        } else {
            window_price_sum = window_price_sum - old_price + data->price;
            data->moving_average = window_price_sum / window_size;
        }

        // Update EMA
        if (prev_ema == 0.0) {
            prev_ema = data->price;
        }
        data->ema = ema_alpha * data->price + (1 - ema_alpha) * prev_ema;
        prev_ema = data->ema;

        // Update Bollinger Bands
        if (n >= window_size) {
            // Calculate mean and standard deviation over the window
            double sum = 0.0, sum_sq = 0.0;
            for (size_t i = 0; i < window_size; i++) {
                sum += prices[i];
                sum_sq += prices[i] * prices[i];
            }
            mean = sum / window_size;
            variance = (sum_sq / window_size) - (mean * mean);
            double stddev = sqrt(variance);
            data->bollinger_upper = mean + bollinger_multiplier * stddev;
            data->bollinger_lower = mean - bollinger_multiplier * stddev;
        }

        // Update RSI
        if (prev_price != 0.0) {
            double change = data->price - prev_price;
            double gain = (change > 0) ? change : 0.0;
            double loss = (change < 0) ? -change : 0.0;

            gains[gain_loss_index % rsi_period] = gain;
            losses[gain_loss_index % rsi_period] = loss;

            if (n >= rsi_period) {
                double total_gain = 0.0, total_loss = 0.0;
                for (size_t i = 0; i < rsi_period; i++) {
                    total_gain += gains[i];
                    total_loss += losses[i];
                }
                avg_gain = total_gain / rsi_period;
                avg_loss = total_loss / rsi_period;
            } else {
                avg_gain = ((avg_gain * (n - 1)) + gain) / n;
                avg_loss = ((avg_loss * (n - 1)) + loss) / n;
            }

            if (avg_loss == 0) {
                data->rsi = 100.0;
            } else {
                double rs = avg_gain / avg_loss;
                data->rsi = 100 - (100 / (1 + rs));
            }

            gain_loss_index++;
        }

        prev_price = data->price;
        price_index = (price_index + 1) % window_size;

        // Enqueue processed data
        lock_free_queue_enqueue(pre_processing_args->output_queue, data);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    const char *config_file_path = "../config/config.ini";
    ConfigParams params;

    // Check if the file exists and can be read
    if (access(config_file_path, F_OK) != 0) {
        fprintf(stderr, "Configuration file '%s' does not exist.\n", config_file_path);
        return 1;
    }
    if (access(config_file_path, R_OK) != 0) {
        fprintf(stderr, "Configuration file '%s' cannot be read (permission denied).\n", config_file_path);
        return 1;
    }
    // Attempt to load the configuration
    if (load_config(config_file_path, &params) != 0) {
        fprintf(stderr, "Failed to load configuration from '%s'.\n", config_file_path);
        return 1;
    }

    // Initialize queues
    LockFreeQueue *input_queue = lock_free_queue_init();
    LockFreeQueue *output_queue = lock_free_queue_init();

    // Start data ingestion thread
    pthread_t data_thread, pre_process_thread;
    pthread_create(&data_thread, NULL, data_ingestion_thread, input_queue);

    // Start pre-processing thread
    PreProcessingArgs pre_processing_args = {
        .input_queue = input_queue,
        .output_queue = output_queue,
        .window_size = params.window_size,
        .ema_alpha = params.ema_alpha,
        .rsi_period = params.rsi_period,
        .bollinger_multiplier = params.bollinger_multiplier
    };
    pthread_create(&pre_process_thread, NULL, pre_processing_thread, &pre_processing_args);

    // Initialize risk management settings
    RiskManagementSettings risk_settings = {
        .calculate_dynamic_stop_loss = NULL,  // Implement as needed
        .calculate_position_limit = NULL,     // Implement as needed
        .monitor_market_behavior = NULL       // Implement as needed
    };

    // Main loop
    size_t records_processed = 0; // Use size_t
    while (records_processed < params.max_records_to_process) {
        pthread_mutex_lock(&mutex);
        while (queue_is_empty(output_queue)) {
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        MarketData *data = (MarketData *)lock_free_queue_dequeue(output_queue);
        if (data) {
            // Prepare PreProcessedData for the algorithm
            PreProcessedData pre_processed_data = {0};
            pre_processed_data.prices = &data->price;
            pre_processed_data.price_count = 1;
            pre_processed_data.price_differences = NULL; // Assuming single price point
            pre_processed_data.price_difference_count = 0;
            pre_processed_data.transaction_costs = params.transaction_costs;
            pre_processed_data.latency = params.latency;
            pre_processed_data.liquidity = &data->volume; // Using volume as liquidity
            pre_processed_data.liquidity_count = 1;
            pre_processed_data.risk_management_params = params.risk_management_params;
            pre_processed_data.liquidity_info = params.liquidity_info;
            pre_processed_data.trend_info = params.trend_info;
            pre_processed_data.trend_period = params.trend_period; // Ensure this field exists
            // Add other necessary fields

            // Execute trading algorithm
            TradeSignal signal = execute_algorithm(&pre_processed_data, arbitrage_trading_strategy, &risk_settings);

            // Output trade signal
            printf("Trade Action: %d, Position Size: %.2f, Entry Price: %.2f\n",
                   signal.action, signal.position_size, signal.entry_price);

            free(data);
            records_processed++;
        }
    }

    running = 0;
    pthread_join(data_thread, NULL);
    pthread_join(pre_process_thread, NULL);

    lock_free_queue_destroy(input_queue);
    lock_free_queue_destroy(output_queue);

    return 0;
}