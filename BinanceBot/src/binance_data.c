#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "market_data_array.h"

// Function to resize a MarketDataArray
void market_data_array_resize(MarketDataArray *array) {
    array->capacity *= 2;
    array->data = realloc(array->data, array->capacity * sizeof(MarketData));
}

// Function to free a MarketDataArray
void market_data_array_free(MarketDataArray *array) {
    free(array->data);
    free(array);
}

// Function to read and parse a CSV file
void read_csv_file(const char *filename, MarketDataArray *array) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Could not open file: %s\n", filename);
        return;
    }

    // Skip the header line
    char line[200];
    fgets(line, sizeof(line), file);

    while (fgets(line, sizeof(line), file)) {
        // Resize the array if it's full
        if (array->length == array->capacity) {
            market_data_array_resize(array);
        }

        MarketData *data = &array->data[array->length++];

        // Parse the line into the MarketData structure
        sscanf(line, "%19[^,],%lf,%lf,%lf,%lf,%lf,%19[^,],%lf,%d,%lf,%lf,%d",
            data->timestamp,
            &data->open,
            &data->high,
            &data->low,
            &data->close,
            &data->volume,
            data->close_time,
            &data->quote_av,
            &data->trades,
            &data->tb_base_av,
            &data->tb_quote_av,
            &data->ignore
        );
    }

    fclose(file);
}