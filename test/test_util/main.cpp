/**
 * @file test/test_set_glyphs/main.cpp
 * @brief Unit testing for general-purpose utility functions.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */
#include <unity.h>
#include "util.h"

/**
 * Tests the get_least_set_bit function, which returns the
 * the position of the least significant bit that is set to 1.
 */
void test_get_least_set_bit() {
    TEST_ASSERT_EQUAL(-1, get_least_set_bit(0b00000000));
    TEST_ASSERT_EQUAL(0,  get_least_set_bit(0b11111111));
    TEST_ASSERT_EQUAL(0, get_least_set_bit(0b00110101));
    TEST_ASSERT_EQUAL(0, get_least_set_bit(0b01000001));

    TEST_ASSERT_EQUAL(2, get_least_set_bit(0b11111100));

    TEST_ASSERT_EQUAL(5, get_least_set_bit(0b00100000));

    TEST_ASSERT_EQUAL(7, get_least_set_bit(0b10000000));
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_get_least_set_bit);

    UNITY_END();
    return 0;
}
