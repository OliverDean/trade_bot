#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "market_data.h"
#include "lock_free_queue.h"

#define WINDOW_SIZE 10
#define NUM_RECORDS_TO_PROCESS 1000
#define EMA_ALPHA 0.1
#define RSI_PERIOD 14

typedef struct PreProcessingArgs {
    LockFreeQueue *input_queue;
    LockFreeQueue *output_queue;
} PreProcessingArgs;

void calculate_moving_average(MarketData *data, double *window, size_t window_size) {
    double sum = 0;
    for (size_t i = 0; i < window_size; i++) {
        sum += window[i];
    }
    data->moving_average = sum / window_size;
}

void calculate_exponential_moving_average(MarketData *data, double *prev_ema) {
    if (*prev_ema == 0) {
        *prev_ema = data->price;
    }
    data->ema = (1 - EMA_ALPHA) * (*prev_ema) + EMA_ALPHA * data->price;
    *prev_ema = data->ema;
}

void calculate_bollinger_bands(MarketData *data, double *window, size_t window_size) {
    double sum = 0, mean = 0, variance = 0, stddev;

    for (size_t i = 0; i < window_size; i++) {
        sum += window[i];
    }
    mean = sum / window_size;

    for (size_t i = 0; i < window_size; i++) {
        variance += pow(window[i] - mean, 2);
    }
    variance /= window_size;
    stddev = sqrt(variance);

    data->bollinger_upper = mean + 2 * stddev;
    data->bollinger_lower = mean - 2 * stddev;
}

void calculate_relative_strength_index(MarketData *data, double *price_changes, size_t period, size_t *index) {
    double gain_sum = 0, loss_sum = 0, rs;

    for (size_t i = 0; i < period; i++) {
        if (price_changes[i] >= 0) {
            gain_sum += price_changes[i];
        } else {
            loss_sum += fabs(price_changes[i]);
        }
    }

    rs = gain_sum / loss_sum;
    data->rsi = 100 - (100 / (1 + rs));
    *index = (*index + 1) % period;
}

void calculate_rate_of_change(MarketData *data, double *window, size_t window_size) {
    data->roc = (data->price - window[0]) / window[0] * 100;
}

void *pre_processing_thread(void *args) {
    PreProcessingArgs *pre_processing_args = (PreProcessingArgs *)args;

    size_t records_processed = 0;
    double window_price[WINDOW_SIZE] = {0};
    double window_price_changes[RSI_PERIOD] = {0};
    size_t index_price = 0;
    size_t index_price_changes = 0;
    double prev_ema = 0;
    size_t calculation_interval = DEFAULT_CALCULATION_INTERVAL;

    while (records_processed < MAX_RECORDS_TO_PROCESS) {
        MarketData *data = dequeue(pre_processing_args->input_queue);
        if (data == NULL) {
            continue; // No data available in the input queue, try again
        }

        if (data->volume <= 0 || data->price <= 0) {
            // Invalid data, skip processing
            continue;
        }

        // Store the price in the circular buffer
        window_price[index_price] = data->price;
        index_price = (index_price + 1) % WINDOW_SIZE;

        // Calculate the moving average and store it in the MarketData struct
        calculate_moving_average(data, window_price, WINDOW_SIZE);

        // Calculate the Exponential Moving Average (EMA)
        calculate_exponential_moving_average(data, &prev_ema);

        // Calculate Bollinger Bands
        if (records_processed >= WINDOW_SIZE - 1) {
            calculate_bollinger_bands(data, window_price, WINDOW_SIZE);
        }

        // Calculate the Relative Strength Index (RSI)
        if (records_processed >= RSI_PERIOD) {
            calculate_relative_strength_index(data, window_price_changes, RSI_PERIOD, &index_price_changes);
        }

        // Calculate the Rate of Change (ROC)
        if (records_processed >= WINDOW_SIZE) {
            calculate_rate_of_change(data, window_price, WINDOW_SIZE);
        }

        // Enqueue the pre-processed data to the output queue
        enqueue(pre_processing_args->output_queue, data);

        records_processed++;

        // Perform calculations at certain intervals, depending on the trade volume and ROC
        if (records_processed % calculation_interval == 0) {
            // Calculate EMA, Bollinger Bands, RSI, and ROC
            calculation_interval = calculate_adaptive_interval(data->volume, data->roc);
        }
    }

    return NULL;
}


int main() {
    // Initialize the input and output lock-free queues
    LockFreeQueue *input_queue = lock_free_queue_init();
    LockFreeQueue *output_queue = lock_free_queue_init();

    // Create and start pre-processing thread
    pthread_t pre_processing_thread_id;
    PreProcessingArgs pre_processing_args = {input_queue, output_queue};
    pthread_create(&pre_processing_thread_id, NULL, pre_processing_thread, (void *)&pre_processing_args);

    // Wait for the thread to finish
    pthread_join(pre_processing_thread_id, NULL);

    // Clean up resources
    lock_free_queue_destroy(input_queue);
    lock_free_queue_destroy(output_queue);

    return 0;
}