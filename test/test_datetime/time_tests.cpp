/**
 * @file test/test_datetime/time_tests.cpp
 * @brief Tests for the Time class.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "time_tests.h"
#include <stdint.h>
#include <unity.h>
#include "assertions.h"
#include "datetime.h"

/** == Time-specific tests == */

/**
 * Tests the const and non-const Time field accessors,
 * which are the Time::second, Time::minute, and Time::hour functions.
 */
void test_time_accessors() {
    Time time(13, 7, 49);  // 13:07:49

    // Test that the const accessors act as expected.
    TEST_ASSERT_EQUAL(13,  time.hour());
    TEST_ASSERT_EQUAL(7,  time.minute());
    TEST_ASSERT_EQUAL(49, time.second());

    // Get references to the fields using the non-const accessors.
    uint8_t& hour_ref = time.hour();
    uint8_t& minute_ref = time.minute();
    uint8_t& second_ref = time.second();

    // Check that the references have the expected values.
    TEST_ASSERT_EQUAL(13,  hour_ref);
    TEST_ASSERT_EQUAL(7,  minute_ref);
    TEST_ASSERT_EQUAL(49, second_ref);

    // The references should be able to modify the fields of the Time object.
    // Set time to 12:00:01
    hour_ref = 12;
    minute_ref = 0;
    second_ref = 1;

    // Check that those values were reflected back into the Time object.
    TEST_ASSERT_EQUAL(12, time.hour());
    TEST_ASSERT_EQUAL(0, time.minute());
    TEST_ASSERT_EQUAL(1, time.second());
}

/**
 * Tests the Time::field function.
 * Time::field should return the correct field given
 * an enum, and you should be able to use the reference to
 * modify the field's values.
 */
void test_time_field() {
    Time time(15, 32, 59);  // 15:32:59

    // The time.field function should return the correct fields.
    TEST_ASSERT_EQUAL(15,  time.field(ATF_HOUR));
    TEST_ASSERT_EQUAL(32,  time.field(ATF_MINUTE));
    TEST_ASSERT_EQUAL(59, time.field(ATF_SECOND));

    // You should be able to modify the values using time.field.
    // Set the time to 7:42:05
    time.field(ATF_HOUR) = 7;
    time.field(ATF_MINUTE) = 42;
    time.field(ATF_SECOND) = 5;

    // Test that the fields were actually set.
    TEST_ASSERT_EQUAL__Time(Time(7, 42, 5), time);
}

/**
 * Tests Time::fields_min_value, which returns the minimum
 * allowable value for all of the time fields (always 0).
 */
void test_time_fields_min_value() {
    Time time;  // Default time: 00:00:00
    TEST_ASSERT_EQUAL(0, time.fields_min_value());
}

/**
 * Tests Time::field_max_value, which returns the maximum
 * allowable value for the given time field.
 * 
 * These values do not change with different values of the
 * time fields.
 */
void test_time_field_max_value() {
    Time time;  // Default time: 00:00:00

    // Maximum value of Second is 59.
    TEST_ASSERT_EQUAL(59, time.field_max_value(ATF_SECOND));

    // Maximum value of Minute is 59.
    TEST_ASSERT_EQUAL(59, time.field_max_value(ATF_MINUTE));

    // Maximum value of Hour is 23.
    TEST_ASSERT_EQUAL(23, time.field_max_value(ATF_HOUR));
}
