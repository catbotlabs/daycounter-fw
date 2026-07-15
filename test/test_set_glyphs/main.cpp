/**
 * @file test/test_set_glyphs/main.cpp
 * @brief Unit testing for the set_glyphs_0_to_999 function.
 * 
 * This set_glyphs_0_to_999 function converts a three-digit integer
 * to the three bytes of segment bits to be sent to the shift register.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, CatBotLabs
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */
#include <unity.h>
#include "display.h"
#include "glyphdefs.h"

/**
 * These are the seven-segment glyph definitions
 * for integers 0-9, mapped properly in an array.
 * 
 * (Such that DIGIT_GLYPHS[3] is the glyph for the number '3'.)
 */
#define DG DIGIT_GLYPHS

/**
 * Sets all three glyphs to \c 0xaa, (0b10101010, which
 * doesn't match any of our glyph values.
 */
void reset_gl(DisplayGlyphs* gl) {
    (*gl)[2] = (*gl)[1] = (*gl)[0] = 0xaa;
}

/**
 * Tests if three array elements equal their expected values.
 * @param arr:  The array with the actual values.
 * @param val2: Expected value of element 2.
 * @param val1: Expected value of element 1.
 * @param val0: Expected value of element 0.
 */
#define TEST_ARR3_EQUAL(arr, val2, val1, val0) \
  TEST_ASSERT_EQUAL_HEX8(val2, (arr[2])); \
  TEST_ASSERT_EQUAL_HEX8(val1, (arr[1])); \
  TEST_ASSERT_EQUAL_HEX8(val0, (arr[0]));

/**
 * Unit test for set_glyphs_0_to_999.
 * 
 * Ensure that:
 *  - All three glyphs are set on every function call.
 *  - If given number is negative, glyphs equal "  -"
 *  - If given number exceeds 999, glyphs equal "---"
 *  - The hundreds and tens digits are properly blanked instead of leading zeros.
 *  - The correct numbers are shown.
 */
void test_set_glyphs_0_to_999() {
    DisplayGlyphs gl;

    // Note that we always OR the tens place glyph against 0b1,
    // because the tens place glyph must always have the DG1 bit set.

    /** Typical values */

    // 9
    set_glyphs_0_to_999(9, gl);
    TEST_ARR3_EQUAL(gl, GLYPH_NONE, GLYPH_NONE | 1, DG[9]);
    reset_gl(&gl);

    // 23
    set_glyphs_0_to_999(23, gl);
    TEST_ARR3_EQUAL(gl, GLYPH_NONE, DG[2] | 1, DG[3]);
    reset_gl(&gl);

    // 88
    set_glyphs_0_to_999(88, gl);
    TEST_ARR3_EQUAL(gl, GLYPH_NONE, DG[8] | 1, DG[8]);
    reset_gl(&gl);

    // 119
    set_glyphs_0_to_999(119, gl);
    TEST_ARR3_EQUAL(gl, DG[1], DG[1] | 1, DG[9]);
    reset_gl(&gl);

    // 799
    set_glyphs_0_to_999(799, gl);
    TEST_ARR3_EQUAL(gl, DG[7], DG[9] | 1, DG[9]);
    reset_gl(&gl);


    /** Typical values with 1s and 0s */

    // 0
    set_glyphs_0_to_999(0, gl);
    TEST_ARR3_EQUAL(gl, GLYPH_NONE, GLYPH_NONE | 1, DG[0]);
    reset_gl(&gl);

    // 1
    set_glyphs_0_to_999(1, gl);
    TEST_ARR3_EQUAL(gl, GLYPH_NONE, GLYPH_NONE | 1, DG[1]);
    reset_gl(&gl);

    // 10
    set_glyphs_0_to_999(10, gl);
    TEST_ARR3_EQUAL(gl, GLYPH_NONE, DG[1] | 1, DG[0]);
    reset_gl(&gl);

    // 100
    set_glyphs_0_to_999(100, gl);
    TEST_ARR3_EQUAL(gl, DG[1], DG[0] | 1, DG[0]);
    reset_gl(&gl);
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_set_glyphs_0_to_999);

    UNITY_END();
    return 0;
}
