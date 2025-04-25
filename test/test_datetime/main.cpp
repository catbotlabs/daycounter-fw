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

/** == General-purpose function tests == */

/**
 * Test the to_ascending_df function, which
 * remaps a StdDateField enum to a AscDateField enum.
 */
void test_to_ascending_df() {
    /** to_ascending_df simply remaps these three enums: */
    TEST_ASSERT_EQUAL(AscDateField::Day,
                      to_ascending_df(StdDateField::Day));

    TEST_ASSERT_EQUAL(AscDateField::Month,
                      to_ascending_df(StdDateField::Month));

    TEST_ASSERT_EQUAL(AscDateField::Year,
                      to_ascending_df(StdDateField::Year));
}

/**
 * Test the to_ascending_tf function, which
 * remaps a StdTimeField enum to a AscTimeField enum.
 */
void test_to_ascending_tf() {
    /** to_ascending_tf simply remaps these three enums: */
    TEST_ASSERT_EQUAL(AscTimeField::Second,
                      to_ascending_tf(StdTimeField::Second));

    TEST_ASSERT_EQUAL(AscTimeField::Minute,
                      to_ascending_tf(StdTimeField::Minute));

    TEST_ASSERT_EQUAL(AscTimeField::Hour,
                      to_ascending_tf(StdTimeField::Hour));
}


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
    Date date(1, 1, 0);  // Default: January 1, XX00
    Time time(0, 0, 0);  // Default: 00:00:00
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

/**
 * Tests the count_time_units_until function,
 * which is the general-purpose date/time counting
 * function.
 * It can count forwards/backwards, in days/weeks/hours,
 * and it does bounds checking for if the counting
 * is backwards / exceeds 999.
 */
void test_count_time_units_until() {
    TimeCountResult result;
    uint16_t count;

    /** Test case: AHEAD/BEHIND OK result, D/W */
    // Days, AHEAD
    count = 11;
    result = count_time_units_until(
        Date(8, 26, 25),  // target:   Aug 26, '25
        Date(4, 4, 25),   // now date: Apr 04, '25
        Time(16, 0, 0),   // now time: 16:00:00
        TimeCountUnit::Days,
        TimeCountDirection::Ahead,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);  // OK result
    TEST_ASSERT_EQUAL(144, count);  // 144 day delta

    // Days, BEHIND
    count = 22;
    result = count_time_units_until(
        Date(4, 4, 25),    // target:   Apr 04, '25
        Date(8, 26, 25),   // now date: Aug 26, '25
        Time(16, 0, 0),    // now time: 16:00:00
        TimeCountUnit::Days,
        TimeCountDirection::Behind,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);  // OK result
    TEST_ASSERT_EQUAL(144, count);  // 144 day delta
                                
    // Weeks, AHEAD
    count = 33;
    result = count_time_units_until(
        Date(8, 26, 25),  // target:   Aug 26, '25
        Date(4, 4, 25),   // now date: Apr 04, '25
        Time(16, 0, 0),   // now time: 16:00:00
        TimeCountUnit::Weeks,
        TimeCountDirection::Ahead,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);  // OK result
    TEST_ASSERT_EQUAL(20, count);  // 20 week delta

    // Weeks, BEHIND
    count = 44;
    result = count_time_units_until(
        Date(4, 4, 25),    // target:   Apr 04, '25
        Date(8, 26, 25),   // now date: Aug 26, '25
        Time(16, 0, 0),    // now time: 16:00:00
        TimeCountUnit::Weeks,
        TimeCountDirection::Behind,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);  // OK result
    TEST_ASSERT_EQUAL(20, count);  // 20 week delta

    /** Test case: The Time component, AHEAD/BEHIND, Hours */
    count = 101;
    result = count_time_units_until(
        Date(6, 5, 30),    // target:   Jun 05, '30
        Date(6, 1, 30),    // now date: Jun 01, '30 
        Time(15, 30, 0),   // now time: 15:30:00
        TimeCountUnit::Hours,
        TimeCountDirection::Ahead,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);  // OK result
    TEST_ASSERT_EQUAL(81, count);  // 81 hour delta

    count = 102;
    result = count_time_units_until(
        Date(6, 1, 30),    // target:   Jun 01, '30
        Date(6, 5, 30),    // now date: Jun 05, '30 
        Time(15, 30, 0),   // now time: 15:30:00
        TimeCountUnit::Hours,
        TimeCountDirection::Behind,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);  // OK result
    TEST_ASSERT_EQUAL(111, count);  // 111 hour delta

    /** Test case: AHEAD/BEHIND OK result, count=0, D/H/W */
    {
    Date same_date(4, 24, 25);

    count = 111;
    // Days: Ahead
    result = count_time_units_until(same_date, same_date,
        Time(18, 0, 0),
        TimeCountUnit::Days, TimeCountDirection::Ahead, &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);
    TEST_ASSERT_EQUAL(0, count);

    count = 112;
    // Hours: Ahead
    result = count_time_units_until(same_date, same_date,
        Time(0, 59, 59),
        TimeCountUnit::Hours, TimeCountDirection::Ahead, &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);
    TEST_ASSERT_EQUAL(0, count);

    count = 113;
    // Weeks: Ahead
    result = count_time_units_until(same_date, same_date,
        Time(18, 0, 0),
        TimeCountUnit::Weeks, TimeCountDirection::Ahead, &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);
    TEST_ASSERT_EQUAL(0, count);

    count = 114;
    // Days: Behind
    result = count_time_units_until(same_date, same_date,
        Time(18, 0, 0),
        TimeCountUnit::Days, TimeCountDirection::Behind, &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);
    TEST_ASSERT_EQUAL(0, count);

    count = 115;
    // Hours: Behind
    result = count_time_units_until(same_date, same_date,
        Time(0, 59, 59),
        TimeCountUnit::Hours, TimeCountDirection::Behind, &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);
    TEST_ASSERT_EQUAL(0, count);

    count = 116;
    // Weeks: Behind
    result = count_time_units_until(same_date, same_date,
        Time(18, 0, 0),
        TimeCountUnit::Weeks, TimeCountDirection::Behind, &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);
    TEST_ASSERT_EQUAL(0, count);
    }

    /** Test case: AHEAD/BEHIND NEGATIVE result */
    // AHEAD
    count = 33;      // reset count
    result = count_time_units_until(
        Date(4, 4, 25),    // target:   Apr 04, '25
        Date(8, 26, 25),   // now date: Aug 26, '25
        Time(16, 0, 0),    // now time: 16:00:00
        TimeCountUnit::Days,
        TimeCountDirection::Ahead,
        &count);


    TEST_ASSERT_EQUAL(TimeCountResult::Negative, result);  // NEGATIVE result
    TEST_ASSERT_EQUAL(33, count);  // count should be unmodified.

    // BEHIND
    count = 44;
    result = count_time_units_until(
        Date(8, 26, 25),  // target:   Aug 26, '25
        Date(4, 4, 25),   // now date: Apr 04, '25
        Time(16, 0, 0),   // now time: 16:00:00
        TimeCountUnit::Days,
        TimeCountDirection::Behind,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::Negative, result);  // NEGATIVE result
    TEST_ASSERT_EQUAL(44, count);  // count should be unmodified.

    /** Test case: OVERFLOW result, D/H/W */
    {
    // Days, count = 999: OK
    count = 55;
    result = count_time_units_until(
        Date(12, 29, 27),   // target:   Dec 29, '27
        Date(4, 4, 25),     // now date: Apr 04, '25
        Time(16, 0, 0),     // now time: 16:00:00
        TimeCountUnit::Days,
        TimeCountDirection::Ahead,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);  // OK result
    TEST_ASSERT_EQUAL(999, count);  // 999 day delta

    // Days, day delta = 1000: OVERFLOW!
    count = 55;
    result = count_time_units_until(
        Date(12, 30, 27),   // target:   Dec 30, '25
        Date(4, 4, 25),     // now date: Apr 04, '25
        Time(16, 0, 0),     // now time: 16:00:00
        TimeCountUnit::Days,
        TimeCountDirection::Ahead,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::Overflow, result);  // OVERFLOW result
    TEST_ASSERT_EQUAL(55, count);  // count should be unmodified

    // Hours, count = 999: OK
    count = 66;
    result = count_time_units_until(
        Date(5, 16, 25),    // target:   May 16, '25
        Date(4, 4, 25),     // now date: Apr 04, '25
        Time(9, 0, 0),     // now time: 09:00:00
        TimeCountUnit::Hours,
        TimeCountDirection::Ahead,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);  // OK result
    TEST_ASSERT_EQUAL(999, count);  // 999 hour delta

    // Hours, hour delta = 1000: OVERFLOW!
    count = 66;
    result = count_time_units_until(
        Date(5, 16, 25),    // target:   May 16, '25
        Date(4, 4, 25),     // now date: Apr 04, '25
        Time(8, 0, 0),     // now time: 08:00:00
        TimeCountUnit::Hours,
        TimeCountDirection::Ahead,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::Overflow, result);  // OVERFLOW result
    TEST_ASSERT_EQUAL(66, count);  // count should be unmodified.

    // Weeks, count = 999: OK
    count = 77;
    result = count_time_units_until(
        Date(6, 2, 44),    // target:   Jun 02, '44
        Date(4, 4, 25),    // now date: Apr 04, '25
        Time(16, 0, 0),    // now time: 16:00:00
        TimeCountUnit::Weeks,
        TimeCountDirection::Ahead,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);  // OK result
    TEST_ASSERT_EQUAL(999, count);  // 999 week delta

    // Weeks, week delta = 1000: OVERFLOW!
    count = 77;
    result = count_time_units_until(
        Date(6, 3, 44),    // target:   Jun 03, '44
        Date(4, 4, 25),     // now date: Apr 04, '25
        Time(16, 0, 0),     // now time: 16:00:00
        TimeCountUnit::Weeks,
        TimeCountDirection::Ahead,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::Overflow, result);  // OVERFLOW result
    TEST_ASSERT_EQUAL(77, count);  // count should be unmodified.
    }

    /** Test case: 50 year max time delta overflow, D/H/W */
    {
    Date then(12, 31, 54);  // Dec 31, '54
    Date now_d(1, 1, 4);    // Jan 01, '04
    Time now_t(23, 59, 59);

    count = 131;
    // Days
    result = count_time_units_until(then, now_d, now_t,
        TimeCountUnit::Days, TimeCountDirection::Ahead, &count);

    TEST_ASSERT_EQUAL(TimeCountResult::Overflow, result);  // Overflow
    TEST_ASSERT_EQUAL(131, count);  // unmodified
                                 
    count = 132;
    // Hours
    result = count_time_units_until(then, now_d, now_t,
        TimeCountUnit::Hours, TimeCountDirection::Ahead, &count);

    TEST_ASSERT_EQUAL(TimeCountResult::Overflow, result);  // Overflow
    TEST_ASSERT_EQUAL(132, count);  // unmodified
                                 
    count = 133;
    // Weeks
    result = count_time_units_until(then, now_d, now_t,
        TimeCountUnit::Weeks, TimeCountDirection::Ahead, &count);

    TEST_ASSERT_EQUAL(TimeCountResult::Overflow, result);  // Overflow
    TEST_ASSERT_EQUAL(133, count);  // unmodified
    }

    /** Test case: "hour_count" 16-bit unsigned int overflow protection */
    // day delta: ceil((2**16)/24) = 2731
    // A day delta of 2731 days would set
    // hour_count to 2731*24, which equals 65544, but this
    // exceeds uint16_t bounds, so hour_count would wrap to around 8.
    count = 88;
    result = count_time_units_until(
        Date(9, 25, 32),   // target:   Sep 25, '32
        Date(4, 4, 25),    // now date: Apr 04, '25
        Time(16, 0, 0),    // now time: 16:00:00
        TimeCountUnit::Hours,
        TimeCountDirection::Ahead,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::Overflow, result);  // OVERFLOW result
    TEST_ASSERT_EQUAL(88, count);  // count should be unmodified.

    /** Test case: The situation where day_count==0 and hours_delta<0. */
    count = 99;
    result = count_time_units_until(
        Date(4, 4, 25),   // target:   Apr 04, '25
        Date(4, 4, 25),   // now date: Apr 04, '25
        Time(1, 0, 0),    // now time: 01:00:00
        TimeCountUnit::Hours,
        TimeCountDirection::Ahead,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::Negative, result);  // NEGATIVE result
    TEST_ASSERT_EQUAL(99, count);  // count should be unmodified.

    // but if hours_delta == 0, then result is OK and count is zero.
    result = count_time_units_until(
        Date(4, 4, 25),   // target:   Apr 04, '25
        Date(4, 4, 25),   // now date: Apr 04, '25
        Time(0, 59, 59),  // now time: 00:59:59
        TimeCountUnit::Hours,
        TimeCountDirection::Ahead,
        &count);

    TEST_ASSERT_EQUAL(TimeCountResult::OK, result);  // OK result
    TEST_ASSERT_EQUAL(0, count);  // count is zero hours.
}

int main() {
    UNITY_BEGIN();

    /** General-purpose function tests */
    RUN_TEST(test_to_ascending_df);
    RUN_TEST(test_to_ascending_tf);

    /** From bcd_tests.cpp */
    RUN_TEST(test_int_from_bcd);
    RUN_TEST(test_bcd_from_int);

    /** Date-specific tests */
    /** From date_tests.cpp */
    RUN_TEST(test_is_leap_year);
    RUN_TEST(test_date_accessors);
    RUN_TEST(test_date_is_before);
    RUN_TEST(test_date_field);
    RUN_TEST(test_date_field_min_value);
    RUN_TEST(test_date_field_max_value);
    RUN_TEST(test_date_count_days_until);

    /** Time-specific tests */
    /** From time_tests.cpp */
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
    RUN_TEST(test_count_time_units_until);

    UNITY_END();
    return 0;
}
