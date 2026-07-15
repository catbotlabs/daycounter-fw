/**
 * @file test/test_datetime/str_fmt_tests.cpp
 * @brief Tests for the string formatting functions.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, CatBotLabs
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include <unity.h>
#include "str_fmt_tests.h"
#include "datetime.h"

/**
 * Tests the two_digit_itoa function,
 * which sets two characters of a string to represent
 * a two-digit integer.
 */
void test_two_digit_itoa() {
    char str[] = "---56---";   // A string that we'll insert the number into.
    char* substr = str + 3;   // Pointer to where we're putting the number.
    TEST_ASSERT_EQUAL_STRING("---56---", str);

    two_digit_itoa(substr, 0);
    TEST_ASSERT_EQUAL_STRING("---00---", str);

    two_digit_itoa(substr, 99);
    TEST_ASSERT_EQUAL_STRING("---99---", str);

    two_digit_itoa(substr, 1);
    TEST_ASSERT_EQUAL_STRING("---01---", str);

    two_digit_itoa(substr, 10);
    TEST_ASSERT_EQUAL_STRING("---10---", str);

    two_digit_itoa(substr, 9);
    TEST_ASSERT_EQUAL_STRING("---09---", str);

    two_digit_itoa(substr, 80);
    TEST_ASSERT_EQUAL_STRING("---80---", str);

    two_digit_itoa(substr, 89);
    TEST_ASSERT_EQUAL_STRING("---89---", str);

    two_digit_itoa(substr, 90);
    TEST_ASSERT_EQUAL_STRING("---90---", str);

    two_digit_itoa(substr, 55);
    TEST_ASSERT_EQUAL_STRING("---55---", str);

    two_digit_itoa(substr, 49);
    TEST_ASSERT_EQUAL_STRING("---49---", str);

    two_digit_itoa(substr, 11);
    TEST_ASSERT_EQUAL_STRING("---11---", str);

    two_digit_itoa(substr, 91);
    TEST_ASSERT_EQUAL_STRING("---91---", str);

    /** "undefined" behavior, but still test it. */
    two_digit_itoa(substr, 100);
    TEST_ASSERT_EQUAL_STRING("---:0---", str);
}

/**
 * Tests the get_month_name_P function, which returns a
 * (program-space) pointer to a three-character month name
 * string corresponding to a Date::month field.
 * 
 * For instance month=1 -> "Jan"
 *              month=8 -> "Aug"
 */
void test_get_month_name_P() {
    const char* name;

    name = get_month_name_P(1);
    TEST_ASSERT_EQUAL_STRING_LEN("Jan", name, 3);
    name = get_month_name_P(2);
    TEST_ASSERT_EQUAL_STRING_LEN("Feb", name, 3);
    name = get_month_name_P(3);
    TEST_ASSERT_EQUAL_STRING_LEN("Mar", name, 3);
    name = get_month_name_P(4);
    TEST_ASSERT_EQUAL_STRING_LEN("Apr", name, 3);
    name = get_month_name_P(5);
    TEST_ASSERT_EQUAL_STRING_LEN("May", name, 3);
    name = get_month_name_P(6);
    TEST_ASSERT_EQUAL_STRING_LEN("Jun", name, 3);
    name = get_month_name_P(7);
    TEST_ASSERT_EQUAL_STRING_LEN("Jul", name, 3);
    name = get_month_name_P(8);
    TEST_ASSERT_EQUAL_STRING_LEN("Aug", name, 3);
    name = get_month_name_P(9);
    TEST_ASSERT_EQUAL_STRING_LEN("Sep", name, 3);
    name = get_month_name_P(10);
    TEST_ASSERT_EQUAL_STRING_LEN("Oct", name, 3);
    name = get_month_name_P(11);
    TEST_ASSERT_EQUAL_STRING_LEN("Nov", name, 3);
    name = get_month_name_P(12);
    TEST_ASSERT_EQUAL_STRING_LEN("Dec", name, 3);
}

/**
 * Tests the generate_date_string function, which writes
 * a formatted "mmm DD 'YY" string to the 10-character buffer.
 */
void test_generate_date_string() {
    /**
     * String has extra padding on the ends to make sure the function
     * doesn't modify out-of-bounds.
     */
    const char* initial_str = "~~~mmm~DD~^YYz~~~";
    char str[18];
    /** The part in the middle of the string where we'll put the date. */
    char* date_str = str + 3;

    strncpy(str, initial_str, 18);  // reset string
    generate_date_string(date_str, Date(1, 1, 0));
    TEST_ASSERT_EQUAL_STRING_LEN("~~~Jan 01 '00\0~~~", str, 18);

    strncpy(str, initial_str, 18);
    generate_date_string(date_str, Date(12, 25, 99));
    TEST_ASSERT_EQUAL_STRING_LEN("~~~Dec 25 '99\0~~~", str, 18);
}

/**
 * Tests the generate_time_string function, which writes
 * a formatted "HH:MM:SS" string to the 8-character buffer.
 */
void test_generate_time_string() {
    /**
     * String has extra padding on the ends to make sure the function
     * doesn't modify out-of-bounds.
     */
    const char* initial_str = "~~~HH:MM:SSz~~~";
    char str[16];
    /** The part in the middle of the string where we'll put the date. */
    char* time_str = str + 3;

    strncpy(str, initial_str, 16);  // reset string
    generate_time_string(time_str, Time(0, 0, 0));
    TEST_ASSERT_EQUAL_STRING_LEN("~~~00:00:00\0~~~", str, 16);

    strncpy(str, initial_str, 16);
    generate_time_string(time_str, Time(22, 22, 22));
    TEST_ASSERT_EQUAL_STRING_LEN("~~~22:22:22\0~~~", str, 16);

    strncpy(str, initial_str, 16);
    generate_time_string(time_str, Time(59, 59, 59));
    TEST_ASSERT_EQUAL_STRING_LEN("~~~59:59:59\0~~~", str, 16);
}

