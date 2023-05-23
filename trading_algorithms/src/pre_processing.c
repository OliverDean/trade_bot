// pre_processing.h
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <stddef.h>
#include <float.h>
#include <pthread.h>
// pre_processing.c
#include "pre_processing.h"
#include "config_parser.h" 
#include <stdlib.h>
#include <math.h>
#include "market_data.h"
#include "market_data_pool.h"
#include "lock_free_queue.h"

#define DEFAULT_CALCULATION_INTERVAL 1000
#define MAX_RECORDS_TO_PROCESS 10000
#define WINDOW_SIZE 10
#define EMA_ALPHA 0.1
#define RSI_PERIOD 14
#define BOLLINGER_MULTIPLIER 2



PreProcessedData *pre_process_data(const RawData *raw_data, size_t rolling_volatility_window_size, size_t custom_window_size)
{
    PreProcessedData *data = (PreProcessedData *)malloc(sizeof(PreProcessedData));
    data->price_differences = calculate_price_differences(raw_data->prices, raw_data->price_count);
    data->price_difference_count = raw_data->price_count - 1;
    data->transaction_costs = raw_data->transaction_costs;
    data->latency = raw_data->latency;
    data->rolling_volatilities = calculate_rolling_volatilities(data->price_differences, data->price_difference_count, custom_window_size ? custom_window_size : rolling_volatility_window_size);
    data->rolling_volatility_count = raw_data->price_count - rolling_volatility_window_size;
    
    // Calculate and store resistance and support levels
    data->resistance_level = calculate_resistance_level(raw_data->prices, raw_data->price_count);
    data->support_level = calculate_support_level(raw_data->prices, raw_data->price_count);

    // Calculate price levels
    PriceLevels price_levels = calculate_price_levels(raw_data->prices, raw_data->price_count);
    data->lower_price_level = price_levels.lower;
    data->upper_price_level = price_levels.upper;
    
    return data;
}

void free_pre_processed_data(PreProcessedData *data)
{
    free(data->price_differences);
    free(data->rolling_volatilities);
    // Free any additional allocated memory for other data fields
    free(data);
}

void update_pre_processed_data(PreProcessedData *data, const RawData *new_data)
{
    // Update the price differences with the new price data
    update_price_differences(data, new_data);

    // Update the rolling volatilities with the new price data
    update_rolling_volatilities(data, new_data, data->rolling_volatility_count);

    // Update any additional data fields based on the new raw data
}

static double calculate_avg(const double *array, size_t size)
{
    double sum = 0;
    for (size_t i = 0; i < size; ++i)
    {
        sum += array[i];
    }
    return sum / size;
}

static double calculate_std_dev(const double *array, size_t size)
{
    double mean = calculate_avg(array, size);
    double temp = 0;
    for (size_t i = 0; i < size; ++i)
    {
        temp += (array[i] - mean) * (array[i] - mean);
    }
    return sqrt(temp / size);
}

void calculate_moving_average(MarketData *data, double *window, size_t window_size)
{
    data->moving_average = calculate_average(window, window_size);
}

void calculate_exponential_moving_average(MarketData *data, double *prev_ema)
{
    if (*prev_ema == 0)
    {
        *prev_ema = data->price;
    }
    data->ema = (1 - EMA_ALPHA) * (*prev_ema) + EMA_ALPHA * data->price;
    *prev_ema = data->ema;
}

void calculate_bollinger_bands(MarketData *data, double *window, size_t window_size)
{
    double mean = calculate_avg(window, window_size);
    double stddev = calculate_std_dev(window, window_size);
    data->bollinger_upper = mean + BOLLINGER_MULTIPLIER * stddev;
    data->bollinger_lower = mean - BOLLINGER_MULTIPLIER * stddev;
}

void calculate_relative_strength_index(MarketData *data, double *price_changes, size_t period, size_t *index)
{
    double gain_sum = 0, loss_sum = 0, rs;
    for (size_t i = 0; i < period; i++)
    {
        if (price_changes[i] >= 0)
        {
            gain_sum += price_changes[i];
        }
        else
        {
            loss_sum += fabs(price_changes[i]);
        }
    }
    rs = gain_sum / loss_sum;
    data->rsi = 100 - (100 / (1 + rs));
    *index = (*index + 1) % period;
}

void calculate_rate_of_change(MarketData *data, double *window, size_t window_size)
{
    data->roc = (data->price - window[0]) / window[0] * 100;
}

double *calculate_price_differences(const double *prices, size_t price_count)
{
    double *price_differences = (double *)malloc(sizeof(double) * (price_count - 1));

    for (size_t i = 0; i < price_count - 1; i++)
    {
        price_differences[i] = prices[i + 1] - prices[i];
    }

    return price_differences;
}

double *calculate_rolling_volatility(const double *price_differences, size_t price_difference_count, size_t window_size)
{
    double *rolling_volatilities = (double *)malloc(sizeof(double) * (price_difference_count - window_size + 1));
    double sum = 0, mean = 0, variance = 0;

    // Initialize the first window
    for (size_t i = 0; i < window_size; i++)
    {
        sum += price_differences[i];
    }
    mean = sum / window_size;

    for (size_t i = 0; i < window_size; i++)
    {
        variance += pow(price_differences[i] - mean, 2);
    }
    variance /= window_size;

    rolling_volatilities[0] = sqrt(variance);

    // Calculate the rolling volatilities for the rest of the dataset
    for (size_t i = 1; i < price_difference_count - window_size + 1; i++)
    {
        sum = sum - price_differences[i - 1] + price_differences[i + window_size - 1];
        mean = sum / window_size;

        variance = 0;
        for (size_t j = i; j < i + window_size; j++)
        {
            variance += pow(price_differences[j] - mean, 2);
        }
        variance /= window_size;

        rolling_volatilities[i] = sqrt(variance);
    }

    return rolling_volatilities;
}

PriceLevels calculate_price_levels(const double *prices, size_t price_count)
{
    PriceLevels levels = {.upper = -DBL_MAX, .lower = DBL_MAX};
    levels.pivot_point = prices[price_count - 1];

    for (size_t i = 0; i < price_count; i++)
    {
        if (prices[i] > levels.upper)
        {
            levels.upper = prices[i];
        }
        if (prices[i] < levels.lower)
        {
            levels.lower = prices[i];
        }
    }

    levels.pivot_point = (levels.upper + levels.lower + levels.pivot_point) / 3.0;

    return levels;
}

double calculate_support_level(const double *prices, size_t price_count)
{
    PriceLevels levels = calculate_price_levels(prices, price_count);
    return 2 * levels.pivot_point - levels.upper;
}

double calculate_resistance_level(const double *prices, size_t price_count)
{
    PriceLevels levels = calculate_price_levels(prices, price_count);
    return 2 * levels.pivot_point - levels.lower;
}

void update_price_differences(PreProcessedData *data, const RawData *new_data)
{
    size_t new_price_difference_count = new_data->price_count - 1;
    double *new_price_differences = calculate_price_differences(new_data->prices, new_data->price_count);

    double *updated_price_differences = (double *)realloc(data->price_differences, sizeof(double) * (data->price_difference_count + new_price_difference_count));
    if (updated_price_differences == NULL)
    {
        // Handle the error, e.g., log the error and exit the function
        return;
    }
    data->price_differences = updated_price_differences;

    for (size_t i = 0; i < new_price_difference_count; i++)
    {
        data->price_differences[data->price_difference_count + i] = new_price_differences[i];
    }
    data->price_difference_count += new_price_difference_count;

    free(new_price_differences);
}

void update_rolling_volatilities(PreProcessedData *data, const RawData *new_data, size_t window_size)
{
    size_t new_price_difference_count = new_data->price_count - 1;
    double *new_price_differences = calculate_price_differences(new_data->prices, new_data->price_count);

    double *extended_price_differences = (double *)realloc(data->price_differences, sizeof(double) * (data->price_difference_count + new_price_difference_count));
    if (extended_price_differences == NULL)
    {
        // Handle the error, e.g., log the error and exit the function
        return;
    }
    data->price_differences = extended_price_differences;

    for (size_t i = 0; i < new_price_difference_count; i++)
    {
        data->price_differences[data->price_difference_count + i] = new_price_differences[i];
    }
    data->price_difference_count += new_price_difference_count;

    double *new_rolling_volatilities = calculate_rolling_volatility(data->price_differences, data->price_difference_count, window_size);
    size_t new_rolling_volatility_count = data->price_difference_count - window_size + 1;

    double *updated_rolling_volatilities = (double *)realloc(data->rolling_volatilities, sizeof(double) * new_rolling_volatility_count);
    if (updated_rolling_volatilities == NULL)
    {
        // Handle the error, e.g., log the error and exit the function
        return;
    }
    data->rolling_volatilities = updated_rolling_volatilities;

    for (size_t i = 0; i < new_rolling_volatility_count; i++)
    {
        data->rolling_volatilities[i] = new_rolling_volatilities[i];
    }
    data->rolling_volatility_count = new_rolling_volatility_count;

    free(new_price_differences);
    free(new_rolling_volatilities);
}

void *pre_processing_thread(void *args)
{
    PreProcessingArgs *pre_processing_args = (PreProcessingArgs *)args;

    size_t records_processed = 0;
    double window_price[WINDOW_SIZE] = {0};
    double window_price_changes[RSI_PERIOD] = {0};
    size_t index_price = 0;
    size_t index_price_changes = 0;
    double prev_ema = 0;
    size_t calculation_interval = DEFAULT_CALCULATION_INTERVAL;

    while (records_processed < MAX_RECORDS_TO_PROCESS)
    {
        MarketData *raw_data = dequeue(pre_processing_args->input_queue);
        if (raw_data == NULL)
        {
            usleep(calculation_interval);
            continue;
        }

        MarketData data;
        calculate_market_indicators(raw_data, &data, window_price, window_price_changes, &prev_ema, index_price, index_price_changes);

        // Update rolling volatilities
        update_rolling_volatilities(&data, raw_data, WINDOW_SIZE);

        // Update price differences
        update_price_differences(&data, raw_data);

        // Calculate and store resistance and support levels
        data.resistance_level = calculate_resistance_level(window_price, WINDOW_SIZE);
        data.support_level = calculate_support_level(window_price, WINDOW_SIZE);

        // Calculate price levels
        PriceLevels price_levels = calculate_price_levels(window_price, WINDOW_SIZE);
        data.lower_price_level = price_levels.lower;
        data.upper_price_level = price_levels.upper;

        enqueue(pre_processing_args->output_queue, &data);

        // Update index values
        index_price = (index_price + 1) % WINDOW_SIZE;
        index_price_changes = (index_price_changes + 1) % RSI_PERIOD;

        records_processed++;

        // Perform calculations at certain intervals, depending on the trade volume and ROC
        if (records_processed % calculation_interval == 0)
        {
            calculation_interval = calculate_adaptive_interval(data.volume, data.roc);
        }
    }

    return NULL;
}

/**
 * main function - Entry point of the program.
 *
 * This function sets up the input and output data queues, 
 * initializes the pre-processing thread, 
 * and monitors it until MAX_RECORDS_TO_PROCESS records have been processed or a stopping condition is met. 
 * Once done, the thread is joined back to the main thread and resources are cleaned up.
 * 
 * Input:
 *   Takes command-line arguments, including the configuration file path as the first argument.
 * 
 * Output:
 *   Returns 0 on successful execution, non-zero error code otherwise.
 *
 * Usage:
 *   ./program config.ini
 */
int main(int argc, char* argv[]) {
    // Check if config file path is provided
    if (argc < 2) {
        printf("Usage: ./program config.ini\n");
        return 1;
    }

    // Load the constants from the config file
    char* config_file_path = argv[1];
    ConfigParams params;
    if (load_config(config_file_path, &params) != EXIT_SUCCESS) { // load_config now takes a pointer to ConfigParams structure
        printf("Failed to load the config file\n");
        return 1;
    }

    // Prepare queues
    LockFreeQueue *input_queue = lock_free_queue_init();
    LockFreeQueue *output_queue = lock_free_queue_init();

    // Prepare arguments
    PreProcessingArgs args;
    args.input_queue = &input_queue;
    args.output_queue = &output_queue;

    // Create thread
    pthread_t pre_processing_thread_id;
    int err = pthread_create(&pre_processing_thread_id, NULL, pre_processing_thread, &args);
    if (err != 0) {
        printf("Error: unable to create thread, %d\n", err);
        return err;
    }

    // Main processing logic, like enqueueing RawData into input_queue
    // RawData *raw_data;
    // while (/* condition to continue fetching data */) {
        // raw_data = /* Fetch or create your RawData here */
        
        // Create MarketData from RawData
        // MarketData *market_data = (MarketData*)malloc(sizeof(MarketData));
        // Convert RawData to MarketData (this depends on your implementation)
        // For example:
        // market_data->price = raw_data->price;
        // market_data->volume = raw_data->volume;
        // ...

        // Enqueue the MarketData
        //lock_free_queue_enqueue(&input_queue, market_data);
    // }
    // When stopping condition met, enqueue stop signal
    MarketData *stop_signal = NULL; // Assuming NULL is the stop signal
    enqueue(&input_queue, stop_signal);

    // Wait for pre-processing thread to exit
    pthread_join(pre_processing_thread_id, NULL);

    // Clean up queues
    lock_free_queue_destroy(input_queue);
    lock_free_queue_destroy(output_queue);

    return 0;
}
