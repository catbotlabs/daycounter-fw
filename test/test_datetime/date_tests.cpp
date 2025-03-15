/**
 * @file test/test_datetime/date_tests.cpp
 * @brief Tests for the Date class.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "date_tests.h"
#include <stdint.h>
#include <unity.h>
#include "assertions.h"
#include "datetime.h"

/** == Date-specific tests == */

/** 
 * Tests the static member function Date::is_leap_year,
 * which expects a year value from 0 to 99.
 * 
 * Note: Since the RTC doesn't keep track of centuries,
 * this implementation doesn't account for century leap years.
 * Thus, all century years (XX00) are NOT leap years in this
 * implementation.
 * 
 * (Since 100 is a multiple of 4, the same leap years
 * "repeat" every century. Convenient.)
 */
void test_is_leap_year() {
    // Century leap year is NOT a leap year.
    TEST_ASSERT_FALSE(Date::is_leap_year(0));

    // These years are leap years.
    TEST_ASSERT_TRUE(Date::is_leap_year(24));
    TEST_ASSERT_TRUE(Date::is_leap_year(80));
    TEST_ASSERT_TRUE(Date::is_leap_year(96));

    // These years are not leap years.
    TEST_ASSERT_FALSE(Date::is_leap_year(1));
    TEST_ASSERT_FALSE(Date::is_leap_year(30));
    TEST_ASSERT_FALSE(Date::is_leap_year(74));
    TEST_ASSERT_FALSE(Date::is_leap_year(99));

    // Note that 100 is outside of the function's domain.
}

/**
 * Tests the default value of the Date object.
 */
void test_date_default_value() {
    Date date;  // Default value should be January 1, XX00.
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 0), date);
}

/**
 * Tests the const and non-const Date field accessors,
 * which are the Date::day, Date::month, and Date::year functions.
 */
void test_date_accessors() {
    Date date(4, 8, 10);  // April 8, XX10

    // Test that the const accessors act as expected.
    TEST_ASSERT_EQUAL(4,  date.month());
    TEST_ASSERT_EQUAL(8,  date.day());
    TEST_ASSERT_EQUAL(10, date.year());

    // Get references to the fields using the non-const accessors.
    uint8_t& month_ref = date.month();
    uint8_t& day_ref = date.day();
    uint8_t& year_ref = date.year();

    // Check that the references have the expected values.
    TEST_ASSERT_EQUAL(4,  month_ref);
    TEST_ASSERT_EQUAL(8,  day_ref);
    TEST_ASSERT_EQUAL(10, year_ref);

    // The references should be able to modify the fields of the Date object.
    // Set date to January 21, XX58
    month_ref = 1;
    day_ref = 21;
    year_ref = 58;

    // Check that those values were reflected back into the Date object.
    TEST_ASSERT_EQUAL(1, date.month());
    TEST_ASSERT_EQUAL(21, date.day());
    TEST_ASSERT_EQUAL(58, date.year());
}

/**
 * Tests the Date::is_before function.
 * 
 * TODO: describe further.
 */
void test_date_is_before() {
    Date ref;

    ref = Date(1, 1, 0);  // January 1, XX00

    /** These dates should NOT be counted as before January 1, XX00 */
    TEST_ASSERT_FALSE(Date::is_before(Date(1, 1, 0), ref));   // equal date
    TEST_ASSERT_FALSE(Date::is_before(Date(1, 1, 1), ref));   // year after
    TEST_ASSERT_FALSE(Date::is_before(Date(1, 1, 49), ref));  // 49 years after
    TEST_ASSERT_FALSE(Date::is_before(Date(1, 1, 50), ref));  // 50 years after

    /** These dates SHOULD be counted as before January 1, XX00 */
    TEST_ASSERT_TRUE(Date::is_before(Date(1, 1, 51), ref));  // wrap, 49 before
    TEST_ASSERT_TRUE(Date::is_before(Date(1, 1, 52), ref));  // 48 before
    TEST_ASSERT_TRUE(Date::is_before(Date(1, 1, 95), ref));  // 5 years before
    TEST_ASSERT_TRUE(Date::is_before(Date(1, 1, 99), ref));  // year before


    ref = Date(7, 20, 30);  // July 20, XX30

    // Dates BEFORE ref
    TEST_ASSERT_TRUE(Date::is_before(Date(1, 23, 30), ref));  // earlier month
    TEST_ASSERT_TRUE(Date::is_before(Date(7, 1, 30), ref));   // same month
    TEST_ASSERT_TRUE(Date::is_before(Date(7, 19, 30), ref));  // same month

    // EQUAL with ref
    TEST_ASSERT_FALSE(Date::is_before(Date(7, 20, 30), ref));

    // Dates AFTER ref
    TEST_ASSERT_FALSE(Date::is_before(Date(7, 21, 30), ref));  // same month
    TEST_ASSERT_FALSE(Date::is_before(Date(7, 31, 30), ref));  // same month
    TEST_ASSERT_FALSE(Date::is_before(Date(8, 5, 30), ref));   // next month
    TEST_ASSERT_FALSE(Date::is_before(Date(1, 1, 31), ref));   // next year

    // Year wrap cutoff point: 50 years
    TEST_ASSERT_FALSE(Date::is_before(Date(12, 1, 80), ref));  // 50 years after
    TEST_ASSERT_TRUE(Date::is_before(Date(1, 1, 81), ref));    // 49 years before


    // A date in the middle of the century.
    ref = Date(3, 14, 50);  // March 14, XX50

    // +/- 1 year
    TEST_ASSERT_FALSE(Date::is_before(Date(3, 14, 51), ref));
    TEST_ASSERT_TRUE(Date::is_before(Date(3, 14, 49), ref));

    // +/- 30 years
    TEST_ASSERT_FALSE(Date::is_before(Date(3, 14, 80), ref));
    TEST_ASSERT_TRUE(Date::is_before(Date(3, 14, 20), ref));

    // +/- 49 years
    TEST_ASSERT_FALSE(Date::is_before(Date(3, 14, 99), ref));
    TEST_ASSERT_TRUE(Date::is_before(Date(3, 14, 1), ref));

    // +/- 50 years
    // delta == 50 years: no wrap.
    TEST_ASSERT_TRUE(Date::is_before(Date(3, 14, 0), ref));

    // +/- 55 years -> -/+ 45 years
    // delta > 50 years: yes wrap.
    TEST_ASSERT_FALSE(Date::is_before(Date(3, 14, 95), ref));
    TEST_ASSERT_TRUE(Date::is_before(Date(3, 14, 5), ref));
}


/**
 * Tests the Date::field function.
 * Date::field should return the correct field given
 * an enum, and you should be able to use the reference to
 * modify the field's values.
 */
void test_date_field() {
    Date date(3, 7, 11);  // March 7, XX11

    // The date.field function should return the correct fields.
    TEST_ASSERT_EQUAL(3,  date.field(ADF_MONTH));
    TEST_ASSERT_EQUAL(7,  date.field(ADF_DAY));
    TEST_ASSERT_EQUAL(11, date.field(ADF_YEAR));

    // You should be able to modify the values using date.field.
    // Set the date to November 3, XX07
    date.field(ADF_MONTH) = 11;
    date.field(ADF_DAY) = 3;
    date.field(ADF_YEAR) = 7;

    // Test that the fields were actually set.
    TEST_ASSERT_EQUAL__Date(Date(11, 3, 7), date);
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

/** Tests Date::count_days_until */
void test_date_count_days_until() {
    // test equal date (zero span)
    Date    now(1, 1, 4);   // January 1, XX00
    Date target(1, 1, 4);   // January 1, XX00
    TEST_ASSERT_EQUAL(0, now.count_days_until(target));
    
    // test same month
    target = Date(1, 31, 4);  // January 31, XX04
    TEST_ASSERT_EQUAL(30, now.count_days_until(target));

    // test next month
    target = Date(2, 1, 4);  // February 1, XX04
    TEST_ASSERT_EQUAL(31, now.count_days_until(target));

    // test 2 months, leap year
    target = Date(3, 1, 4);  // March 1, XX04
    TEST_ASSERT_EQUAL(60, now.count_days_until(target));

    // test different year
    now    = Date(7, 15, 55);     // July  15, XX56
    target = Date(4, 15, 56);  // April 15, XX56
    TEST_ASSERT_EQUAL(275, now.count_days_until(target));
    
    // test different year, same month, same day
    now    = Date(12, 31, 60);     // December 31, XX60
    target = Date(12, 31, 63);  // December 31, XX63
    TEST_ASSERT_EQUAL(1095, now.count_days_until(target));

    // test cross-century
    now    = Date(9, 2, 98);     // September 2, XX98
    target = Date(5, 30, 7);  // March 30, XX07 (next century)
    TEST_ASSERT_EQUAL(3191, now.count_days_until(target));

    // test really long spans:
    // Longest possible forward span, I think...
    now    = Date(1, 1, 4);     // January 1, XX04
    target = Date(12, 31, 54);  // December 31, XX54
    TEST_ASSERT_EQUAL(18627, now.count_days_until(target));

    // Long, cross-century span
    now    = Date(7, 7, 77);     // July 7, XX77
    target = Date(2, 22, 22);    // next century, SUPER TWOSDAY
    TEST_ASSERT_EQUAL(16300, now.count_days_until(target));
}
