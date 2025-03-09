/**
 * @file test/test_datetime/date_tests.h
 * @brief Tests for the Date class.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef TEST_TEST_DATETIME_DATE_TESTS_H_
#define TEST_TEST_DATETIME_DATE_TESTS_H_

/** Date-specific tests */
void test_is_leap_year();
void test_date_default_value();
void test_date_accessors();
void test_date_is_before();
void test_date_field();
void test_date_field_min_value();
void test_date_field_max_value();

#endif  // TEST_TEST_DATETIME_DATE_TESTS_H_
