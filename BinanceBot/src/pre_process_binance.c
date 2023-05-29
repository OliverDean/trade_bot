// pre_processing.h
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <stddef.h>
#include <float.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
// pre_processing.c
#include "pre_processing_binance.h"
#include "config_parser.h"
#include "market_data_array.h"
#include "lock_free_queue.h"
#include "proces_queue.h"


typedef struct BinanaceData {
    char symbol[10];
    double price;
    int volume;
    double moving_average;
    double ema;
    double bollinger_upper;
    double bollinger_lower;
    double rsi;
    double roc;
    double resistance_level;
    double support_level;
    double upper_price_level;
    double lower_price_level;
} BinanceData;


PreProcessedData *pre_process_data(const MarketData *market_data, size_t data_count, size_t rolling_volatility_window_size, size_t custom_window_size)
{
    PreProcessedData *data = (PreProcessedData *)malloc(sizeof(PreProcessedData));
    data->price_differences = calculate_price_differences(market_data, data_count);
    data->price_difference_count = data_count - 1;
    data->rolling_volatilities = calculate_rolling_volatilities(data->price_differences, data->price_difference_count, custom_window_size ? custom_window_size : rolling_volatility_window_size);
    data->rolling_volatility_count = data_count - rolling_volatility_window_size;

    // Calculate and store resistance and support levels
    data->resistance_level = calculate_resistance_level(market_data, data_count);
    data->support_level = calculate_support_level(market_data, data_count);

    // Calculate price levels
    PriceLevels price_levels = calculate_price_levels(market_data, data_count);
    data->lower_price_level = price_levels.lower;
    data->upper_price_level = price_levels.upper;
    
    return data;
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

void calculate_moving_average(BinanceData *data, double *window, size_t window_size)
{
    data->moving_average = calculate_avg(window, window_size);
}

void calculate_exponential_moving_average(BinanceData *data, double *prev_ema)
{
    if (*prev_ema == 0)
    {
        *prev_ema = data->price;
    }
    data->ema = (1 - EMA_ALPHA) * (*prev_ema) + EMA_ALPHA * data->price;
    *prev_ema = data->ema;
}

void calculate_bollinger_bands(BinanceData *data, double *window, size_t window_size)
{
    double mean = calculate_avg(window, window_size);
    double stddev = calculate_std_dev(window, window_size);
    data->bollinger_upper = mean + BOLLINGER_MULTIPLIER * stddev;
    data->bollinger_lower = mean - BOLLINGER_MULTIPLIER * stddev;
}

void calculate_relative_strength_index(BinanceData *data, double *price_changes, size_t period, size_t *index)
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

void calculate_rate_of_change(BinanceData *data, double *window, size_t window_size)
{
    if (window_size > 0) {
        data->roc = ((data->price - window[window_size - 1]) / window[window_size - 1]) * 100;
    } else {
        // Handle the case where window_size is 0
    }
}

double *calculate_price_differences(const MarketData *market_data, size_t data_count)
{
    double *price_differences = (double *)malloc(sizeof(double) * (data_count - 1));

    for (size_t i = 0; i < data_count - 1; i++)
    {
        price_differences[i] = market_data[i + 1].close - market_data[i].close;
    }

    return price_differences;
}

double *calculate_rolling_volatilities(const double *price_differences, size_t price_difference_count, size_t window_size)
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
PriceLevels calculate_price_levels(const MarketData *market_data, size_t data_count)
{
    // Initialize the highest value as the smallest possible double
    // and the lowest value as the largest possible double
    PriceLevels levels = {.upper = -DBL_MAX, .lower = DBL_MAX};

    // Check for valid data
    if (data_count < 1 || !market_data) {
        fprintf(stderr, "No market data available.\n");
        // Here you may return a PriceLevels with default values, or handle it in other ways
        return levels; 
    }

    // Use the most recent closing price as the initial pivot point
    levels.pivot_point = market_data[data_count - 1].close;

    // Iterate through all market data
    for (size_t i = 0; i < data_count; i++)
    {
        // If the high of the current data point is greater than the current upper level, update it
        if (market_data[i].high > levels.upper)
        {
            levels.upper = market_data[i].high;
        }
        
        // If the low of the current data point is lower than the current lower level, update it
        if (market_data[i].low < levels.lower)
        {
            levels.lower = market_data[i].low;
        }
    }

    // Calculate the pivot point as the average of the upper level, lower level, and most recent close
    levels.pivot_point = (levels.upper + levels.lower + levels.pivot_point) / 3.0;

    return levels;
}

double calculate_support_level(const MarketData *market_data, size_t data_count)
{
    PriceLevels levels = calculate_price_levels(market_data, data_count);
    return 2 * levels.pivot_point - levels.upper;
}

double calculate_resistance_level(const MarketData *market_data, size_t data_count)
{
    PriceLevels levels = calculate_price_levels(market_data, data_count);
    return 2 * levels.pivot_point - levels.lower;
}

void update_price_differences(PreProcessedData *data, const MarketData *new_data, size_t new_data_count)
{
    size_t new_price_difference_count = new_data_count - 1;
    double *new_price_differences = calculate_price_differences(new_data, new_data_count);

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

void update_rolling_volatilities(PreProcessedData *data, const MarketData *new_data, size_t new_data_count, size_t window_size)
{
    size_t new_price_difference_count = new_data_count - 1;
    double *new_price_differences = calculate_price_differences(new_data, new_data_count);

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

    double *new_rolling_volatilities = calculate_rolling_volatilities(data->price_differences, data->price_difference_count, window_size);
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
    MarketData window_data[WINDOW_SIZE] = {0};
    size_t index_data = 0;
    size_t calculation_interval = DEFAULT_CALCULATION_INTERVAL;
    PreProcessedData *preProcessedData = (PreProcessedData *)malloc(sizeof(PreProcessedData));

    while (records_processed < MAX_RECORDS_TO_PROCESS)
    {
        MarketData *new_data = lock_free_queue_dequeue(pre_processing_args->input_queue);
        if (new_data == NULL)
        {
            usleep(calculation_interval);
            continue;
        }

        // Update the window data
        window_data[index_data] = *new_data;

        // Update rolling volatilities
        update_rolling_volatilities(preProcessedData, window_data, WINDOW_SIZE, WINDOW_SIZE);

        // Update price differences
        update_price_differences(preProcessedData, window_data, WINDOW_SIZE);

        // Calculate and store resistance and support levels
        preProcessedData->resistance_level = calculate_resistance_level(window_data, WINDOW_SIZE);
        preProcessedData->support_level = calculate_support_level(window_data, WINDOW_SIZE);

        // Calculate price levels
        PriceLevels price_levels = calculate_price_levels(window_data, WINDOW_SIZE);
        preProcessedData->lower_price_level = price_levels.lower;
        preProcessedData->upper_price_level = price_levels.upper;

        proces_enqueue((ProcesLockFreeNode*)pre_processing_args->output_queue, preProcessedData);


        // Update index values
        index_data = (index_data + 1) % WINDOW_SIZE;

        records_processed++;
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    // Check if config file path is provided
    if (argc < 3) {
        printf("Usage: ./program config.ini data.csv\n");
        return 1;
    }

    // Load the constants from the config file
    char* config_file_path = argv[1];
    
    ConfigParams params;
    if (load_config(config_file_path, &params) != EXIT_SUCCESS) { // load_config now takes a pointer to ConfigParams structure
        printf("Failed to load the config file\n");
        return 1;
    }
    char* csv_file_name = argv[2];

    // Prepare queues
    LockFreeQueue *input_queue = lock_free_queue_init();
    ProcesLockFreeNode *output_queue = proces_queue_init();

    // Prepare arguments
    PreProcessingArgs args;
    args.input_queue = input_queue;
    args.output_queue = output_queue;

    // Create thread
    pthread_t pre_processing_thread_id;
    int err = pthread_create(&pre_processing_thread_id, NULL, pre_processing_thread, &args);
    if (err != 0) {
        printf("Error: unable to create thread, %d\n", err);
        return err;
    }

    // Initialize the dynamic array
    MarketDataArray *array = market_data_array_init(MAX_RECORDS_TO_PROCESS);

    // Read the data from the CSV file
    read_csv_file(csv_file_name, array);

    // Main processing logic, enqueueing MarketData into input_queue
    for (size_t i = 0; i < array->length; i++) {
        // Enqueue the MarketData
        MarketData *market_data = (MarketData*)malloc(sizeof(MarketData));
        *market_data = array->data[i];
        lock_free_queue_enqueue(input_queue, market_data);

        // Check if we have reached the maximum number of records to process
        if(i == MAX_RECORDS_TO_PROCESS - 1) {
            break;
        }
    }
    // When stopping condition met, enqueue stop signal
    MarketData *stop_signal = NULL; // Assuming NULL is the stop signal
    lock_free_queue_enqueue(input_queue, stop_signal);

    // Clean up resources
    market_data_array_free(array);

    // Wait for pre-processing thread to exit
    pthread_join(pre_processing_thread_id, NULL);

    // Clean up queues
    lock_free_queue_destroy(input_queue);
    proces_queue_destroy(output_queue);
    
    return 0;
}
