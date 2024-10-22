#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "market_data.h"
#include "market_data_pool.h"
#include "lock_free_queue.h"
#include "pre_processing.h"
#include "config_parser.h"
#include "algorithm_execution.h"
#include "risk_management.h"

// Global variables for synchronization
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
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
        usleep(100000); // Simulate data fetching interval
    }
    return NULL;
}

void *pre_processing_thread(void *args) {
    PreProcessingArgs *pre_processing_args = (PreProcessingArgs *)args;
    size_t records_processed = 0;
    double window_price_sum = 0;
    double prev_ema = 0;
    double avg_gain = 0, avg_loss = 0;
    double M2 = 0, mean = 0;
    size_t n = 0;
    double previous_price = 0;

    while (running) {
        pthread_mutex_lock(&mutex);
        while (queue_is_empty(pre_processing_args->input_queue)) {
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        MarketData *data = (MarketData *)lock_free_queue_dequeue(pre_processing_args->input_queue);
        if (data == NULL) continue;

        n++;
        // Update moving average
        window_price_sum += data->price;
        if (n > pre_processing_args->window_size) {
            window_price_sum -= data->price; // Placeholder for old price
            data->moving_average = window_price_sum / pre_processing_args->window_size;
        } else {
            data->moving_average = window_price_sum / n;
        }

        // Update EMA
        if (prev_ema == 0) {
            prev_ema = data->price;
        }
        data->ema = (1 - pre_processing_args->ema_alpha) * prev_ema + pre_processing_args->ema_alpha * data->price;
        prev_ema = data->ema;

        // Update Bollinger Bands
        // Implement Welford's method
        double delta = data->price - mean;
        mean += delta / n;
        double delta2 = data->price - mean;
        M2 += delta * delta2;
        if (n >= pre_processing_args->window_size) {
            double variance = M2 / (n - 1);
            double stddev = sqrt(variance);
            data->bollinger_upper = mean + pre_processing_args->bollinger_multiplier * stddev;
            data->bollinger_lower = mean - pre_processing_args->bollinger_multiplier * stddev;
        }

        // Update RSI
        if (n > 1) {
            double change = data->price - previous_price;
            if (change > 0) {
                avg_gain = (avg_gain * (pre_processing_args->rsi_period - 1) + change) / pre_processing_args->rsi_period;
                avg_loss = (avg_loss * (pre_processing_args->rsi_period - 1)) / pre_processing_args->rsi_period;
            } else {
                avg_gain = (avg_gain * (pre_processing_args->rsi_period - 1)) / pre_processing_args->rsi_period;
                avg_loss = (avg_loss * (pre_processing_args->rsi_period - 1) - change) / pre_processing_args->rsi_period;
            }
            double rs = avg_gain / avg_loss;
            data->rsi = 100 - (100 / (1 + rs));
        }
        previous_price = data->price;

        // Enqueue processed data
        lock_free_queue_enqueue(pre_processing_args->output_queue, data);
        records_processed++;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    ConfigParams params;
    if (load_config("config/config.ini", &params) != 0) {
        fprintf(stderr, "Failed to load configuration.\n");
        return 1;
    }

    LockFreeQueue *input_queue = lock_free_queue_init();
    LockFreeQueue *output_queue = lock_free_queue_init();

    pthread_t data_thread, pre_process_thread;

    pthread_create(&data_thread, NULL, data_ingestion_thread, input_queue);

    PreProcessingArgs pre_processing_args = {
        .input_queue = input_queue,
        .output_queue = output_queue,
        .window_size = params.window_size,
        .ema_alpha = params.ema_alpha,
        .rsi_period = params.rsi_period,
        .bollinger_multiplier = params.bollinger_multiplier
    };
    pthread_create(&pre_process_thread, NULL, pre_processing_thread, &pre_processing_args);

    // Main loop
    size_t records_processed = 0;
    while (records_processed < params.max_records_to_process) {
        pthread_mutex_lock(&mutex);
        while (queue_is_empty(output_queue)) {
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        MarketData *data = (MarketData *)lock_free_queue_dequeue(output_queue);
        if (data) {
            // Implement trading algorithm execution and risk management
            // Placeholder: Print processed data
            printf("Price: %.2f, EMA: %.2f, RSI: %.2f\n", data->price, data->ema, data->rsi);
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
