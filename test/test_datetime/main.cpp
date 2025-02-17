/**
 * @file test/test_datetime/main.cpp
 * @brief Unit testing for datetime functions.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */
#include <unity.h>
#include "datetime.h"

/** Date-specific tests */

/** 
 * Tests the static member function Date::is_leap_year,
 * which expects a year value from 0 to 100.
 * 
 * Note that since 100 is a multiple of 4, the same leap years
 * "repeat" every century. Convenient.
 */
void test_is_leap_year() {
    TEST_ASSERT_TRUE(Date::is_leap_year(0));
    TEST_ASSERT_TRUE(Date::is_leap_year(24));
    TEST_ASSERT_TRUE(Date::is_leap_year(80));
    TEST_ASSERT_TRUE(Date::is_leap_year(96));

    TEST_ASSERT_FALSE(Date::is_leap_year(1));
    TEST_ASSERT_FALSE(Date::is_leap_year(30));
    TEST_ASSERT_FALSE(Date::is_leap_year(74));
    TEST_ASSERT_FALSE(Date::is_leap_year(99));
}

/**
 * Tests Date::field_min_value, which returns the
 * minimum allowable value for the given field.
 * 
 * These values do not change with different values
 * of the date fields.
 */
void test_date_field_min_value() {
    // December 4, XX25.
    Date date(12, 4, 25);

    /** Days start counting at 1. (1 == first day of month) */
    TEST_ASSERT_EQUAL(1,
        date.field_min_value(ADF_DAY));

    /** Months start counting at 1. (1 == January) */
    TEST_ASSERT_EQUAL(1,
        date.field_min_value(ADF_MONTH));

    /** Years start counting at 0. (0 == first day of century) */
    TEST_ASSERT_EQUAL(0,
        date.field_min_value(ADF_YEAR));
}

/**
 * Tests Date::field_max_value, which returns the
 * maximum allowable value for the given field.
 * 
 * The values for the Month and Year fields do not change,
 * but the value for Day changes depending on the Month
 * and whether it is a leap Year.
 */
void test_date_field_max_value() {
    Date date;

    /** == Testing maximum allowable value of Day of Month == */

    // Maximum Day in January is 31.
    date = Date(1, 10, 85);   // [January 10, XX85]
    TEST_ASSERT_EQUAL(31, date.field_max_value(ADF_DAY));

    // IF NOT LEAP YEAR: Maximum Day in February is 28.
    date = Date(2, 14, 85);   // [February 14, XX85]
    TEST_ASSERT_EQUAL(28, date.field_max_value(ADF_DAY));

    // Maximum Day in March is 31.
    date = Date(3, 14, 85);   // [March 14, XX85]
    TEST_ASSERT_EQUAL(31, date.field_max_value(ADF_DAY));

    // Maximum Day in April is 30.
    date = Date(4, 7, 85);    // [April 7, XX85]
    TEST_ASSERT_EQUAL(30, date.field_max_value(ADF_DAY));

    // Maximum Day in May is 31.
    date = Date(5, 25, 85);   // [May 25, XX85]
    TEST_ASSERT_EQUAL(31, date.field_max_value(ADF_DAY));

    // Maximum Day in June is 30.
    date = Date(6, 16, 85);   // [June 16, XX85]
    TEST_ASSERT_EQUAL(30, date.field_max_value(ADF_DAY));

    // Maximum Day in July is 31.
    date = Date(7, 4, 85);    // [July 4, XX85]
    TEST_ASSERT_EQUAL(31, date.field_max_value(ADF_DAY));

    // Maximum Day in August is 31.
    date = Date(8, 31, 85);   // [August 31, XX85]
    TEST_ASSERT_EQUAL(31, date.field_max_value(ADF_DAY));

    // Maximum Day in September is 30.
    date = Date(9, 18, 85);   // [September 18, XX85]
    TEST_ASSERT_EQUAL(30, date.field_max_value(ADF_DAY));

    // Maximum Day in October is 31.
    date = Date(10, 7, 85);   // [October 7, XX85]
    TEST_ASSERT_EQUAL(31, date.field_max_value(ADF_DAY));

    // Maximum Day in November is 30.
    date = Date(11, 28, 85);  // [November 28, XX85]
    TEST_ASSERT_EQUAL(30, date.field_max_value(ADF_DAY));

    // Maximum Day in December is 31.
    date = Date(12, 25, 85);  // [December 25, XX85]
    TEST_ASSERT_EQUAL(31, date.field_max_value(ADF_DAY));

    // Now, it IS A LEAP YEAR.
    // Maximum Day in January is still 31.
    date = Date(1, 1, 84);   // [January 1, XX84]
    TEST_ASSERT_EQUAL(31, date.field_max_value(ADF_DAY));

    // IF LEAP YEAR: Maximum Day in February is 29.
    date = Date(2, 14, 84);  // [February 14, XX84]
    TEST_ASSERT_EQUAL(29, date.field_max_value(ADF_DAY));

    // Maximum Day in March is still 31.
    date = Date(3, 14, 84);   // [March 14, XX84]
    TEST_ASSERT_EQUAL(31, date.field_max_value(ADF_DAY));


    /** == Testing maximum allowable value of Month == */

    // Maximum allowable value of Month field is always 12.
    date = Date(1, 1, 1);     // January 1, XX01
    TEST_ASSERT_EQUAL(12, date.field_max_value(ADF_MONTH));


    /** == Testing maximum allowable value of Year == */

    // Maximum value of Year field is always 99.
    date = Date(1, 1, 1);
    TEST_ASSERT_EQUAL(99, date.field_max_value(ADF_YEAR));
}

/** Time-specific tests */

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
void test_time_fields_max_value() {
    Time time;  // Default time: 00:00:00

    // Maximum value of Second is 59.
    TEST_ASSERT_EQUAL(59, time.field_max_value(ATF_SECOND));

    // Maximum value of Minute is 59.
    TEST_ASSERT_EQUAL(59, time.field_max_value(ATF_MINUTE));

    // Maximum value of Hour is 23.
    TEST_ASSERT_EQUAL(23, time.field_max_value(ATF_HOUR));
}

/** Tests with Date and Time together */


int main() {
    UNITY_BEGIN();

    /** Date-specific tests */
    RUN_TEST(test_is_leap_year);
    RUN_TEST(test_date_field_min_value);
    RUN_TEST(test_date_field_max_value);

    /** Time-specific tests */
    RUN_TEST(test_time_fields_min_value);
    RUN_TEST(test_time_fields_max_value);

    UNITY_END();
    return 0;
}
