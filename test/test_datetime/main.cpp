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
#include "assertions.h"
#include "datetime.h"

/** == Date-specific tests == */

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

/** == Time-specific tests == */

/**
 * Tests the default value of the Time object.
 */
void test_time_default_value() {
    Time time;  // Default value should be 00:00:00
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 0), time);
}

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

/** == Tests with Date and Time together == */

/**
 * Test the datetime_from_registers function, which takes
 * the raw register values read from the MCP7940N chip and
 * loads them into a Date and a Time object.
 *
 * @note: This implmentation uses 24-time only, so
 * the 12/24 bit is always 0, and the (AM/PM, HRTEN1) is
 * used for the hours field.
 */
void test_datetime_from_registers() {
    Date date;
    Time time;

    /** Test: low typical values, tens > 0 */
    {
    const Time expected_time(12, 13, 34); // TIME: 12:23:34
    const Date expected_date(12, 11, 13); // DATE: December 11, XX13
    DateTimeRegisters rg = {
        .rtcsec  = 0b00110100,   // ST: true, TEN: 3, ONE: 4
        .rtcmin  = 0b00010011,   // TEN: 2, ONE: 3
        .rtchour = 0b00010010,   // TEN: 1, ONE: 2
        .rtcdate = 0b00010001,   // TEN: 1, ONE: 1
        .rtcmth  = 0b00010010,   // LPYR: false, TEN: 1, ONE: 2
        .rtcyear = 0b00010011    // TEN: 1, ONE: 3
    };
    // Load the values from the registers into the Date and Time objects.
    datetime_from_registers(&date, &time, rg);
    TEST_ASSERT_EQUAL__Time(expected_time, time);
    TEST_ASSERT_EQUAL__Date(expected_date, date);
    }

    /** Test: Max values in fields, tens > 0 */
    {
    const Time expected_time(23, 59, 59);  // TIME: 23:59:59
    const Date expected_date(12, 31, 99);  // DATE: December 31, XX99
    DateTimeRegisters rg = {
        .rtcsec  = 0b01011001,   // ST: false, TEN: 5, ONE: 9
        .rtcmin  = 0b01011001,   // TEN: 5, ONE: 9
        .rtchour = 0b00100011,   // TEN: 2, ONE: 3
        .rtcdate = 0b00110001,   // TEN: 3, ONE: 1
        .rtcmth  = 0b00010010,   // LPYR: false, TEN: 1, ONE: 2
        .rtcyear = 0b10011001,   // TEN: 9, ONE: 9
    };
    // Load the values from the registers into the Date and Time objects.
    datetime_from_registers(&date, &time, rg);
    TEST_ASSERT_EQUAL__Time(expected_time, time);
    TEST_ASSERT_EQUAL__Date(expected_date, date);
    }

    /** Test: LYPR bit high, tens = 0. */
    {
    const Time expected_time(0, 0, 0);  // TIME: 00:00:00
    const Date expected_date(7, 7, 0);  // DATE: July 7, XX00
    DateTimeRegisters rg = {
        .rtcsec  = 0b10000000,  // ST: true, TEN: 0, ONE: 0
        .rtcmin  = 0b00000000,  // TEN: 0, ONE: 0
        .rtchour = 0b00000000,  // TEN: 0, ONE: 0
        .rtcdate = 0b00000111,  // TEN: 0, ONE: 7
        .rtcmth  = 0b00100111,  // LPYR: true, TEN: 0, ONE: 7
        .rtcyear = 0b00000000,  // TEN: 0, ONE: 0
    };
    // Load the values from the registers into the Date and Time objects.
    datetime_from_registers(&date, &time, rg);
    TEST_ASSERT_EQUAL__Time(expected_time, time);
    TEST_ASSERT_EQUAL__Date(expected_date, date);
    }
}

/**
 * Test the tick_datetime_one_second function.
 * The function should increment the seconds by one
 * while properly performing all carries to the higher
 * fields in the time and the date.
 * 
 * This function tests each carry and checks the return values.
 */
void test_tick_datetime_one_second() {
    Date date;  // Default: January 1, XX00
    Time time;  // Default: 00:00:00
    uint8_t ret = 0;  // Return value of tick_datetime_one_second.
    const char RVM[] = "Return value fail.";  // Return value message

    /** Super basic behavior */
    {
    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 0), date);  // Date unchanged.
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 1), time);  // Time is now 00:00:01.
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, RVM);  // Return value: 0 - seconds increment

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 0), date);  // Date unchanged.
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 2), time);  // Time is now 00:00:02.
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, RVM);  // Return value: 0 - seconds increment
    }

    /** Test: Seconds carry */
    {
    time = Time(0, 0, 58);  // 00:00:58

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 0), date);  // Date unchanged.
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 59), time);  // Time is now 00:00:59.
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, RVM);  // Return value: 0 - seconds increment

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 0), date);  // Date unchanged.
    TEST_ASSERT_EQUAL__Time(Time(0, 1, 0), time);  // Time is now 00:01:00.
    TEST_ASSERT_EQUAL_MESSAGE(1, ret, RVM);  // Return value: 1 - minutes increment

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 0), date);  // Date unchanged.
    TEST_ASSERT_EQUAL__Time(Time(0, 1, 1), time);  // Time is now 00:01:01.
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, RVM);  // Return value: 0 - seconds increment
    }

    /** Test: Minutes carry */
    {
    time = Time(0, 59, 59);  // 00:59:59

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 0), date);  // Date unchanged.
    TEST_ASSERT_EQUAL__Time(Time(1, 0, 0), time);  // Time is now 01:00:00.
    TEST_ASSERT_EQUAL_MESSAGE(2, ret, RVM);  // Return value: 2 - hours increment

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 0), date);  // Date unchanged.
    TEST_ASSERT_EQUAL__Time(Time(1, 0, 1), time);  // Time is now 01:00:01.
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, RVM);  // Return value: 0 - seconds increment
    }

    /** Test: Hours carry */
    {
    time = Time(23, 59, 59);  // 23:59:59

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 2, 0), date);  // Date is now Jan 2, XX00!
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 0), time);  // Time is now 00:00:00.
    TEST_ASSERT_EQUAL_MESSAGE(3, ret, RVM);  // Return value: 3 - days increment

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 2, 0), date);  // Date unchanged.
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 1), time);  // Time is now 00:00:01.
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, RVM);  // Return value: 0 - seconds increment
    }

    /** Test: Days carry */
    {
    // There are always 31 days in March.
    date = Date(3, 31, 0);    // March 31, XX00.
    time = Time(23, 59, 59);  // 23:59:59

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(4, 1, 0), date);  // Date is now April 1, XX00!
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 0), time);  // Time is now 00:00:00.
    TEST_ASSERT_EQUAL_MESSAGE(4, ret, RVM);  // Return value: 4 - months increment

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(4, 1, 0), date);  // Date unchanged.
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 1), time);  // Time is now 00:00:01.
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, RVM);  // Return value: 0 - seconds increment
    }

    /** Test: Months carry */
    {
    // Happy New Year! "~~bvweeeeepp!"
    date = Date(12, 31, 0);    // December 31, XX00.
    time = Time(23, 59, 59);  // 23:59:59

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 1), date);  // Date is now Jan 1, XX01!
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 0), time);  // Time is now 00:00:00.
    TEST_ASSERT_EQUAL_MESSAGE(5, ret, RVM);  // Return value: 5 - years increment

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 1), date);  // Date unchanged.
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 1), time);  // Time is now 00:00:01.
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, RVM);  // Return value: 0 - seconds increment
    }

    /** Test: Years century "carry" */
    {
    date = Date(12, 31, 99);    // December 31, XX99.
    time = Time(23, 59, 59);  // 23:59:59

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 0), date);  // Date is now Jan 1, XX00!
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 0), time);  // Time is now 00:00:00.
    TEST_ASSERT_EQUAL_MESSAGE(6, ret, RVM);  // Return value: 6 - years wrapped

    ret = tick_datetime_one_second(&date, &time);
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 0), date);  // Date unchanged.
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 1), time);  // Time is now 00:00:01.
    TEST_ASSERT_EQUAL_MESSAGE(0, ret, RVM);  // Return value: 0 - seconds increment
    }
}

/**
 * Test the tick_datetime_one_second function
 * by calling the function 1 million times and checking
 * if we end up on the correct date and time.
 * After each call, the return value is also checked.
 * 
 * The time delta is about 11 days, and it crosses over a century.
 * 
 */
void test_tick_datetime_brute_million() {
    /** Test: Brute-force 1 million second time delta */
    // 12/25/99 04:30:01 + 1,000,000 seconds = 1/5/00 18:16:41
    Date date = Date(12, 25, 99);  // Dec 25, XX99
    Time time = Time(4, 30, 1);  // 04:30:01

    // Tick 1,000,000 seconds.
    for (uint64_t i=0; i<1'000'000; i++) {
        Date prev_date = date;
        Time prev_time = time;
        uint8_t ret = tick_datetime_one_second(&date, &time);

        // Test return value:
        if (prev_date.year() != date.year()) {
            if (date.year() == 0) {
                TEST_ASSERT_EQUAL(6, ret);  // Century "carry"
            } else {
                TEST_ASSERT_EQUAL(5, ret);  // Year increment
            }
        } else if (prev_date.month() != date.month()) {
            TEST_ASSERT_EQUAL(4, ret);  // Month increment
        } else if (prev_date.day() != date.day()) {
            TEST_ASSERT_EQUAL(3, ret);  // Day increment
        } else if (prev_time.hour() != time.hour()) {
            TEST_ASSERT_EQUAL(2, ret);  // Hour increment
        } else if (prev_time.minute() != time.minute()) {
            TEST_ASSERT_EQUAL(1, ret);  // Minute increment
        } else {
            TEST_ASSERT_EQUAL(0, ret);  // Must be seconds increment.
        }
    }

    // Date should be Jan 5, XX00
    TEST_ASSERT_EQUAL__Date(Date(1, 5, 0), date);
    // Time should be 18:16:41
    TEST_ASSERT_EQUAL__Time(Time(18, 16, 41), time);
}

/**
 * Test the tick_datetime_one_second function by calling
 * it 400 million times and checking the final date and time.
 * After each call, the return value is also checked.
 * 
 * The time delta is about 13 years, and it crosses over a century and
 * tests leap years.
 * 
 * NOTE that the tick_datetime_one_second function does not follow
 * century leap year rules; instead all century years are treated as
 * leap years.
 * 
 */
void test_tick_datetime_brute_four_hundred_million() {
    /** Test: Brute-force 400 million second time delta */
    // 7/4/96 15:01:33 + 400,000,000 seconds = 3/8/09 06:08:13
    Date date = Date(7, 4, 96);   // July 4, XX96
    Time time = Time(15, 1, 33);  // 15:01:33

    // Tick 400,000,000 seconds.
    for (uint64_t i=0; i<400'000'000; i++) {
        Date prev_date = date;
        Time prev_time = time;
        uint8_t ret = tick_datetime_one_second(&date, &time);

        // Test return value:
        if (prev_date.year() != date.year()) {
            if (date.year() == 0) {
                TEST_ASSERT_EQUAL(6, ret);  // Century "carry"
            } else {
                TEST_ASSERT_EQUAL(5, ret);  // Year increment
            }
        } else if (prev_date.month() != date.month()) {
            TEST_ASSERT_EQUAL(4, ret);  // Month increment
        } else if (prev_date.day() != date.day()) {
            TEST_ASSERT_EQUAL(3, ret);  // Day increment
        } else if (prev_time.hour() != time.hour()) {
            TEST_ASSERT_EQUAL(2, ret);  // Hour increment
        } else if (prev_time.minute() != time.minute()) {
            TEST_ASSERT_EQUAL(1, ret);  // Minute increment
        } else {
            TEST_ASSERT_EQUAL(0, ret);  // Must be seconds increment.
        }
    }

    // Date should be 3/9/XX09
    TEST_ASSERT_EQUAL__Date(Date(3, 8, 9), date);
    // Time should be 06:08:13
    TEST_ASSERT_EQUAL__Time(Time(6, 8, 13), time);
}

int main() {
    UNITY_BEGIN();

    /** Date-specific tests */
    RUN_TEST(test_is_leap_year);
    RUN_TEST(test_date_default_value);
    RUN_TEST(test_date_accessors);
    RUN_TEST(test_date_field);
    RUN_TEST(test_date_field_min_value);
    RUN_TEST(test_date_field_max_value);

    /** Time-specific tests */
    RUN_TEST(test_time_default_value);
    RUN_TEST(test_time_accessors);
    RUN_TEST(test_time_field);
    RUN_TEST(test_time_fields_min_value);
    RUN_TEST(test_time_field_max_value);

    /** Tests with Date and Time together */
    RUN_TEST(test_datetime_from_registers);
    RUN_TEST(test_tick_datetime_one_second);
    RUN_TEST(test_tick_datetime_brute_million);
#ifdef DC_MAX_TESTING
    RUN_TEST(test_tick_datetime_brute_four_hundred_million);
#endif

    UNITY_END();
    return 0;
}
