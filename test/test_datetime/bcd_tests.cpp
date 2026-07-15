/**
 * @file test/test_datetime/bcd_tests.cpp
 * @brief Tests for the int_from_bcd and bcd_from_int functions.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, CatBotLabs
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "bcd_tests.h"
#include <stdint.h>
#include <unity.h>
#include "datetime.h"

/* BCD-tens-digit-ify */
#define T(d) (d << 4)

/**
 * Pre-baked values for BCD numbers 0 to 99 to verify against.
 */
uint8_t BCD_0_to_99[100] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    T(1)|0, T(1)|1, T(1)|2, T(1)|3, T(1)|4, T(1)|5, T(1)|6, T(1)|7, T(1)|8, T(1)|9,
    T(2)|0, T(2)|1, T(2)|2, T(2)|3, T(2)|4, T(2)|5, T(2)|6, T(2)|7, T(2)|8, T(2)|9,
    T(3)|0, T(3)|1, T(3)|2, T(3)|3, T(3)|4, T(3)|5, T(3)|6, T(3)|7, T(3)|8, T(3)|9,
    T(4)|0, T(4)|1, T(4)|2, T(4)|3, T(4)|4, T(4)|5, T(4)|6, T(4)|7, T(4)|8, T(4)|9,
    T(5)|0, T(5)|1, T(5)|2, T(5)|3, T(5)|4, T(5)|5, T(5)|6, T(5)|7, T(5)|8, T(5)|9,
    T(6)|0, T(6)|1, T(6)|2, T(6)|3, T(6)|4, T(6)|5, T(6)|6, T(6)|7, T(6)|8, T(6)|9,
    T(7)|0, T(7)|1, T(7)|2, T(7)|3, T(7)|4, T(7)|5, T(7)|6, T(7)|7, T(7)|8, T(7)|9,
    T(8)|0, T(8)|1, T(8)|2, T(8)|3, T(8)|4, T(8)|5, T(8)|6, T(8)|7, T(8)|8, T(8)|9,
    T(9)|0, T(9)|1, T(9)|2, T(9)|3, T(9)|4, T(9)|5, T(9)|6, T(9)|7, T(9)|8, T(9)|9,
};

/**
 * Tests the int_from_bcd function,
 * which takes a two-nibble BCD byte with a value between 0 and 99
 * and returns it as an integer.
 * 
 * This function compares the results using each BCD_0_to_99 entry
 * against the array index (the integer).
 */
void test_int_from_bcd() {
    for (int i = 0; i < 100; i++) {
        uint8_t as_integer = int_from_bcd(BCD_0_to_99[i]);
        TEST_ASSERT_EQUAL(i, as_integer);
    }
}

/**
 * Tests the bcd_from_integer function,
 * which returns a two-nibble BCD byte that represents
 * the given an integer ranging from 0 and 99. 
 * 
 * This function compares the results from 0 to 99 against the
 * entries in BCD_0_to_99.
 */
void test_bcd_from_int() {
    uint8_t as_bcd;
    for (int i = 0; i < 100; i++) {
        as_bcd = bcd_from_int(i);
        TEST_ASSERT_EQUAL(BCD_0_to_99[i], as_bcd);
    }
}
