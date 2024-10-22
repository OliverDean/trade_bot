// src/data_fetcher.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "config_parser.h"
#include "pre_processing.h"
#include "cJSON.h"

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + real_size + 1);
    if (!ptr) {
        printf("Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, real_size);
    mem->size += real_size;
    mem->memory[mem->size] = '\0';

    return real_size;
}

int fetch_data(ConfigParams *config, RawData *raw_data) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_handle = curl_easy_init();

    if (!curl_handle) {
        printf("Failed to initialize curl\n");
        return -1;
    }

    // Build the URL
    char url[512];
    snprintf(url, sizeof(url), "https://api.binance.com/api/v3/klines?symbol=%s&interval=%s&startTime=%lld&endTime=%lld&limit=1000",
             config->symbol, config->interval, config->start_time_ms, config->end_time_ms);

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

    res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK) {
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl_handle);
        free(chunk.memory);
        curl_global_cleanup();
        return -1;
    }

    // Parse JSON response
    cJSON *json = cJSON_Parse(chunk.memory);
    if (!json) {
        printf("Error parsing JSON response\n");
        curl_easy_cleanup(curl_handle);
        free(chunk.memory);
        curl_global_cleanup();
        return -1;
    }

    size_t data_size = cJSON_GetArraySize(json);
    raw_data->price_count = data_size;
    raw_data->prices = malloc(sizeof(double) * data_size);
    raw_data->high_prices = malloc(sizeof(double) * data_size);
    raw_data->low_prices = malloc(sizeof(double) * data_size);
    raw_data->prices = malloc(sizeof(double) * data_size);
    raw_data->volumes = malloc(sizeof(double) * data_size);

    for (size_t i = 0; i < data_size; i++) {
        cJSON *data_point = cJSON_GetArrayItem(json, i);
        raw_data->prices[i] = atof(cJSON_GetArrayItem(data_point, 4)->valuestring);
        raw_data->high_prices[i] = atof(cJSON_GetArrayItem(data_point, 2)->valuestring);
        raw_data->low_prices[i] = atof(cJSON_GetArrayItem(data_point, 3)->valuestring);
        raw_data->prices[i] = raw_data->prices[i];
        raw_data->volumes[i] = atof(cJSON_GetArrayItem(data_point, 5)->valuestring);
    }

    // Clean up
    cJSON_Delete(json);
    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();

    return 0;
}
