/**
 * @file test/test_datetime/assertions.h
 * @brief Custom test assertions for my Date and Time objects.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef TEST_TEST_DATETIME_ASSERTIONS_H_
#define TEST_TEST_DATETIME_ASSERTIONS_H_

/**
 * @brief Asserts that two of my Date objects are equal.
 * 
 * Note that the functions behind this assertion are essentially
 * tested by the test_date_accessors test.
 */
#define TEST_ASSERT_EQUAL__Date(expected, actual) \
    {const Date _Expected_Date_ = expected; \
    const Date _Actual_Date_ = actual; \
    TEST_ASSERT_EQUAL_MESSAGE(_Expected_Date_.month(), \
        _Actual_Date_.month(), "Months don't match."); \
    TEST_ASSERT_EQUAL_MESSAGE(_Expected_Date_.day(), \
        _Actual_Date_.day(), "Days don't match."); \
    TEST_ASSERT_EQUAL_MESSAGE(_Expected_Date_.year(), \
        _Actual_Date_.year(), "Years don't match.");}

/**
 * @brief Asserts that two of my Time objects are equal.
 * 
 * Note that the functions behind this assertion are essentially
 * tested by the test_time_accessors test.
 */
#define TEST_ASSERT_EQUAL__Time(expected, actual) \
    {const Time _Expected_Time_ = expected; \
    const Time _Actual_Time_ = actual; \
    TEST_ASSERT_EQUAL_MESSAGE(_Expected_Time_.hour(), \
        _Actual_Time_.hour(), "Hours don't match."); \
    TEST_ASSERT_EQUAL_MESSAGE(_Expected_Time_.minute(), \
        _Actual_Time_.minute(), "Minutes don't match."); \
    TEST_ASSERT_EQUAL_MESSAGE(_Expected_Time_.second(), \
        _Actual_Time_.second(), "Seconds don't match.");}

/**
 * @brief Asserts that two of my DateTimeRegisters objects are equal.
 */
#define TEST_ASSERT_EQUAL__DateTimeRegisters(expected, actual) \
    {const DateTimeRegisters _Expected_Regs_ = expected; \
    const DateTimeRegisters _Actual_Regs_ = actual; \
    TEST_ASSERT_EQUAL_MESSAGE(_Expected_Regs_.rtcsec, \
        _Actual_Regs_.rtcsec, "RTCSEC mismatch."); \
    TEST_ASSERT_EQUAL_MESSAGE(_Expected_Regs_.rtcmin, \
        _Actual_Regs_.rtcmin, "RTCMIN mismatch."); \
    TEST_ASSERT_EQUAL_MESSAGE(_Expected_Regs_.rtchour, \
        _Actual_Regs_.rtchour, "RTCHOUR mismatch."); \
    TEST_ASSERT_EQUAL_MESSAGE(_Expected_Regs_.rtcdate, \
        _Actual_Regs_.rtcdate, "RTCDATE mismatch."); \
    TEST_ASSERT_EQUAL_MESSAGE(_Expected_Regs_.rtcmth, \
        _Actual_Regs_.rtcmth, "RTCMTH mismatch."); \
    TEST_ASSERT_EQUAL_MESSAGE(_Expected_Regs_.rtcyear, \
        _Actual_Regs_.rtcyear, "RTCYEAR mismatch.");}


#endif  // TEST_TEST_DATETIME_ASSERTIONS_H_
