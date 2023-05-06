#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include "pre_processing.h"

// Define sample data and expected results for test cases
double test_prices[] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0};
size_t num_prices = sizeof(test_prices) / sizeof(test_prices[0]);

// Moving average test
START_TEST(test_moving_average)
{
    double expected_moving_average = 5.5;
    double moving_average = calculate_moving_average(test_prices, num_prices);
    ck_assert_msg(moving_average == expected_moving_average, "Moving average calculation failed");
}
END_TEST

// Exponential moving average test
START_TEST(test_exponential_moving_average)
{
    double expected_ema = 5.564;
    double ema = calculate_exponential_moving_average(test_prices, num_prices);
    ck_assert_msg(fabs(ema - expected_ema) < 0.001, "Exponential moving average calculation failed");
}
END_TEST

// Bollinger Bands test
START_TEST(test_bollinger_bands)
{
    double expected_upper_band = 10.971;
    double expected_lower_band = 0.029;
    double upper_band, lower_band;
    calculate_bollinger_bands(test_prices, num_prices, &upper_band, &lower_band);
    ck_assert_msg(fabs(upper_band - expected_upper_band) < 0.001, "Upper Bollinger Band calculation failed");
    ck_assert_msg(fabs(lower_band - expected_lower_band) < 0.001, "Lower Bollinger Band calculation failed");
}
END_TEST

// Relative Strength Index test
START_TEST(test_relative_strength_index)
{
    double expected_rsi = 89.583;
    double rsi = calculate_relative_strength_index(test_prices, num_prices);
    ck_assert_msg(fabs(rsi - expected_rsi) < 0.001, "Relative Strength Index calculation failed");
}
END_TEST

// Rate of Change test
START_TEST(test_rate_of_change)
{
    double expected_roc = 0.8;
    double roc = calculate_rate_of_change(test_prices, num_prices);
    ck_assert_msg(fabs(roc - expected_roc) < 0.001, "Rate of Change calculation failed");
}
END_TEST

// Test case for invalid data
START_TEST(test_invalid_data)
{
    double invalid_prices[] = {1.0, 2.0, -3.0, 4.0, 5.0};
    size_t num_invalid_prices = sizeof(invalid_prices) / sizeof(invalid_prices[0]);

    double invalid_result = calculate_moving_average(invalid_prices, num_invalid_prices);
    ck_assert_msg(invalid_result == 0.0, "Invalid data handling failed");
}
END_TEST

// Test suite setup
Suite *pre_processing_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Pre-processing");
    tc_core = tcase_create("Core");

    // Add test cases to the suite
    tcase_add_test(tc_core, test_moving_average);
    tcase_add_test(tc_core, test_exponential_moving_average);
    tcase_add_test(tc_core, test_bollinger_bands);
    tcase_add_test(tc_core, test_relative_strength_index);
    tcase_add_test(tc_core, test_rate_of_change);
    tcase_add_test(tc_core, test_invalid_data);
    suite_add_tcase(s, tc_core);
    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;
    s = pre_processing_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
