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

#include "bcd_tests.h"
#include "date_tests.h"
#include "time_tests.h"
#include "str_fmt_tests.h"

/** == Tests with Date and Time together == */

/**
 * Test the datetime_from_registers function, which takes
 * the raw register values read from the MCP7940N chip and
 * loads them into a Date and a Time object.
 *
 * @note: This implementation uses 24-time only, so
 * the 12/24 bit is always 0, and the (AM/PM, HRTEN1) is
 * used for the hours field.
 */
void test_datetime_from_registers() {
    Date date;
    Time time;

    /** Test: low typical values, tens > 0 */
    {
    const Time expected_time(12, 13, 34);  // TIME: 12:23:34
    const Date expected_date(12, 11, 13);  // DATE: December 11, XX13
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
 * Test the update_registers_from_datetime function, which
 * sets the timekeeping bits of the MCP7940N RTC given a
 * Date and a Time object.
*/
void test_update_registers_from_datetime() {
    {
    // Original:  00:00:00 January  1,  00
    //   Set to:  23:59:59 December 31, 99
    DateTimeRegisters registers = {
        .rtcsec  = 0b00000000,  // ST unset, TEN: 0, ONE: 0
        .rtcmin  = 0b00000000,  // TEN: 0, ONE: 0
        .rtchour = 0b00000000,  // TEN: 0, ONE: 0
        .rtcdate = 0b00000001,  // TEN: 0, ONE: 1
        .rtcmth  = 0b00100001,  // LPYR set, TEN: 0, ONE: 1
        .rtcyear = 0b00000000,  // TEN: 0, ONE: 0
    };

    /** Verify that the original register values are as expected */
    Date verify_date;
    Time verify_time;
    datetime_from_registers(&verify_date, &verify_time, registers);
    TEST_ASSERT_EQUAL__Date(Date(1, 1, 0), verify_date);
    TEST_ASSERT_EQUAL__Time(Time(0, 0, 0), verify_time);

    /** Update the registers to the new date and time */
    Time time(23, 59, 59);
    Date date(12, 31, 99);
    update_registers_from_datetime(&registers, date, time);

    DateTimeRegisters expected_result = {
        .rtcsec  = 0b01011001,  // ST unset, TEN: 5, ONE: 9
        .rtcmin  = 0b01011001,  // TEN: 5, ONE: 9
        .rtchour = 0b00100011,  // TEN: 2, ONE: 3
        .rtcdate = 0b00110001,  // TEN: 3, ONE: 1
        .rtcmth  = 0b00110010,  // LPYR set (untouched), TEN: 1, ONE: 2
        .rtcyear = 0b10011001,  // TEN: 9, ONE: 9
    };

    /** Check that the registers were updated to the expected values */
    TEST_ASSERT_EQUAL__DateTimeRegisters(expected_result, registers);
    }

    {
    // Original:  23:47:47 July     17,  97
    //   Set to:  02:03:04 October 29, 04
    DateTimeRegisters registers = {
        .rtcsec  = 0b11000111,  // ST set, TEN: 4, ONE: 7
        .rtcmin  = 0b01000111,  // TEN: 4, ONE: 7
        .rtchour = 0b01100011,  // (12/24) set for fun, TEN: 2, ONE: 3
        .rtcdate = 0b00010111,  // TEN: 1, ONE: 7
        .rtcmth  = 0b00000111,  // LPYR unset, TEN: 0, ONE: 7
        .rtcyear = 0b10010111,  // TEN: 9, ONE: 7
    };

    /** Verify that the original register values are as expected */
    Date verify_date;
    Time verify_time;
    datetime_from_registers(&verify_date, &verify_time, registers);
    TEST_ASSERT_EQUAL__Date(Date(7, 17, 97), verify_date);
    TEST_ASSERT_EQUAL__Time(Time(23, 47, 47), verify_time);

    /** Update the registers to the new date and time */
    Time time(2, 3, 4);
    Date date(10, 29, 4);
    update_registers_from_datetime(&registers, date, time);

    DateTimeRegisters expected_result = {
        .rtcsec  = 0b10000100,  // ST set, TEN: 0, ONE: 4
        .rtcmin  = 0b00000011,  // TEN: 0, ONE: 3
        .rtchour = 0b01000010,  // (12/24) still set, TEN: 0, ONE: 2
        .rtcdate = 0b00101001,  // TEN: 2, ONE: 9
        .rtcmth  = 0b00010000,  // LPYR unset (untouched), TEN: 1, ONE: 0
        .rtcyear = 0b00000100,  // TEN: 0, ONE: 4
    };

    /** Check that the registers were updated to the expected values */
    TEST_ASSERT_EQUAL__DateTimeRegisters(expected_result, registers);


    /** Now let's check that update_registers_from_datetime can
     *  clear all the remaining bits. */
    update_registers_from_datetime(&registers, Date(1, 1, 0), Time(0, 0, 0));

    DateTimeRegisters expected_final_result = {
        .rtcsec  = 0b10000000,  // ST set, TEN: 0, ONE: 0
        .rtcmin  = 0b00000000,  // TEN: 0, ONE: 0
        .rtchour = 0b01000000,  // (12/24) still set, TEN: 0, ONE: 0
        .rtcdate = 0b00000001,  // TEN: 0, ONE: 1
        .rtcmth  = 0b00000001,  // LPYR unset (untouched), TEN: 0, ONE: 1
        .rtcyear = 0b00000000,  // TEN: 0, ONE: 0
    };

    TEST_ASSERT_EQUAL__DateTimeRegisters(expected_final_result, registers);
    }

    // Testing is complete for update_registers_from_datetime!
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
    for (uint64_t i = 0; i < 1000000; i++) {
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
 * century leap year rules; century years are never treated as
 * leap years.
 * 
 */
void test_tick_datetime_brute_four_hundred_million() {
    /** Test: Brute-force 400 million second time delta */
    // 7/4/96 15:01:33 + 400,000,000 seconds = 3/8/09 06:08:13
    Date date = Date(7, 4, 96);   // July 4, XX96
    Time time = Time(15, 1, 33);  // 15:01:33

    // Tick 400,000,000 seconds.
    for (uint64_t i = 0; i < 400000000; i++) {
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
    TEST_ASSERT_EQUAL__Date(Date(3, 9, 9), date);
    // Time should be 06:08:13
    TEST_ASSERT_EQUAL__Time(Time(6, 8, 13), time);
}

int main() {
    UNITY_BEGIN();

    /** General-purpose function tests */
    /** From bcd_tests.cpp */
    RUN_TEST(test_int_from_bcd);
    RUN_TEST(test_bcd_from_int);

    /** Date-specific tests */
    /** From date_tests.cpp */
    RUN_TEST(test_is_leap_year);
    RUN_TEST(test_date_default_value);
    RUN_TEST(test_date_accessors);
    RUN_TEST(test_date_is_before);
    RUN_TEST(test_date_field);
    RUN_TEST(test_date_field_min_value);
    RUN_TEST(test_date_field_max_value);
    RUN_TEST(test_date_count_days_until);

    /** Time-specific tests */
    /** From time_tests.cpp */
    RUN_TEST(test_time_default_value);
    RUN_TEST(test_time_accessors);
    RUN_TEST(test_time_field);
    RUN_TEST(test_time_fields_min_value);
    RUN_TEST(test_time_field_max_value);

    /** String-formatting tests */
    /** From str_fmt_tests.cpp */
    RUN_TEST(test_two_digit_itoa);
    RUN_TEST(test_get_month_name_P);
    RUN_TEST(test_generate_date_string);
    RUN_TEST(test_generate_time_string);

    /** Tests with Date and Time together */
    /** From this source file. */
    RUN_TEST(test_datetime_from_registers);
    RUN_TEST(test_update_registers_from_datetime);
    RUN_TEST(test_tick_datetime_one_second);
    RUN_TEST(test_tick_datetime_brute_million);
#ifdef DC_MAX_TESTING
    RUN_TEST(test_tick_datetime_brute_four_hundred_million);
#endif

    UNITY_END();
    return 0;
}
