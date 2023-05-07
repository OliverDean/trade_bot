// algorithm_execution.h
#include "pre_processing.h"

typedef enum { BUY, SELL, HOLD } TradeSignal;
typedef TradeSignal (*TradingAlgorithm)(const PreProcessedData *);

TradeSignal moving_average_crossover_strategy(const PreProcessedData *data);
TradeSignal mean_reversion_strategy(const PreProcessedData *data);
TradeSignal momentum_trading_strategy(const PreProcessedData *data);
TradeSignal pairs_trading_strategy(const PreProcessedData *data);
TradeSignal arbitrage_trading_strategy(const PreProcessedData *data);

// algorithm_execution.c
#include "algorithm_execution.h"

TradeSignal moving_average_crossover_strategy(const PreProcessedData *data) {
    // Check if there's enough data to calculate the moving averages
    if (data->short_term_mavg_count < 2 || data->long_term_mavg_count < 2) {
        return HOLD;
    }

    // Check for buy and sell signals based on moving average crossover
    if (data->short_term_mavg[data->short_term_mavg_count - 1] > data->long_term_mavg[data->long_term_mavg_count - 1] &&
        data->short_term_mavg[data->short_term_mavg_count - 2] <= data->long_term_mavg[data->long_term_mavg_count - 2]) {
        return BUY;
    } else if (data->short_term_mavg[data->short_term_mavg_count - 1] < data->long_term_mavg[data->long_term_mavg_count - 1] &&
               data->short_term_mavg[data->short_term_mavg_count - 2] >= data->long_term_mavg[data->long_term_mavg_count - 2]) {
        return SELL;
    } else {
        return HOLD;
    }
}

TradeSignal mean_reversion_strategy(const PreProcessedData *data, double stddev_multiplier) {
    // Check if there's enough data to calculate the moving average and standard deviation
    if (data->mavg_count < 2 || data->std_dev_count < 2 || data->rsi_count < 2) {
        return HOLD;
    }

    // Calculate the upper and lower Bollinger Bands
    double upper_band = data->mavg[data->mavg_count - 1] + stddev_multiplier * data->std_dev[data->std_dev_count - 1];
    double lower_band = data->mavg[data->mavg_count - 1] - stddev_multiplier * data->std_dev[data->std_dev_count - 1];

    // Check for buy and sell signals based on Bollinger Bands and RSI
    if (data->price[data->price_count - 1] < lower_band && data->rsi[data->rsi_count - 1] < 30) {
        return BUY;
    } else if (data->price[data->price_count - 1] > upper_band && data->rsi[data->rsi_count - 1] > 70) {
        return SELL;
    } else {
        return HOLD;
    }
}

TradeSignal momentum_trading_strategy(const PreProcessedData *data, double roc_threshold) {
    // Check if there's enough data to calculate the ROC and moving averages
    if (data->roc_count < 2 || data->mavg_count < 2 || data->rsi_count < 2) {
        return HOLD;
    }

    // Check for buy and sell signals based on ROC, RSI, and moving averages
    if (data->roc[data->roc_count - 1] > roc_threshold && data->roc[data->roc_count - 2] <= roc_threshold &&
        data->rsi[data->rsi_count - 1] < 70 && data->price[data->price_count - 1] > data->mavg[data->mavg_count - 1]) {
        return BUY;
    } else if (data->roc[data->roc_count - 1] < -roc_threshold && data->roc[data->roc_count - 2] >= -roc_threshold &&
               data->rsi[data->rsi_count - 1] > 30 && data->price[data->price_count - 1] < data->mavg[data->mavg_count - 1]) {
        return SELL;
    } else {
        return HOLD;
    }
}

TradeSignal pairs_trading_strategy(const PreProcessedData *data1, const PreProcessedData *data2, size_t spread_window, double dynamic_threshold_factor) {
    // Check if there's enough data to calculate the spreads and moving averages
    if (data1->price_count < spread_window || data2->price_count < spread_window) {
        return HOLD;
    }

    // Calculate the moving average of the spread and its historical volatility
    double spread_moving_average = 0;
    double spread_volatility = 0;
    for (size_t i = data1->price_count - spread_window; i < data1->price_count; i++) {
        double spread = data1->price[i] - data2->price[i];
        spread_moving_average += spread;
        spread_volatility += spread * spread;
    }
    spread_moving_average /= spread_window;
    spread_volatility = sqrt(spread_volatility / spread_window - spread_moving_average * spread_moving_average);

    // Calculate the spread between the two stocks
    double spread = data1->price[data1->price_count - 1] - data2->price[data2->price_count - 1];

    // Calculate the dynamic threshold based on historical volatility
    double threshold = dynamic_threshold_factor * spread_volatility;

    // Generate buy and sell signals based on the deviation from the moving average of the spread
    if (spread > spread_moving_average + threshold) {
        return SELL;
    } else if (spread < spread_moving_average - threshold) {
        return BUY;
    } else {
        return HOLD;
    }
}
