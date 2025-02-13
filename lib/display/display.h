/**
 * @file lib/display/display.h
 * @brief Digital display code.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef LIB_DISPLAY_DISPLAY_H_
#define LIB_DISPLAY_DISPLAY_H_

#include <stdint.h>

/** {dg0, dg1, dg2} */
using DisplayGlyphs = uint8_t[3];

/**
 * Configure the three pins that drive the display shift register
 * as outputs with data 0.
 */
void setup_display();

/**
 * Set the three glyph bytes according to the given number, `count`.
 * `count` represents the number of days until the target date (or
 * hours or weeks, in whatever direction we are counting.)
 * 
 * If count is negative, then glyphs are set to "  -".
 * If count exceeds 999, then the glyphs are set to "---".
 * Otherwise, the digits are set to their base-10 digit
 * values according to count.
 */
void set_glyphs(int16_t count, DisplayGlyphs glyphs);
void display_glyphs_oe(const DisplayGlyphs glyphs);
//void display_glyphs_no_oe(const DisplayGlyphs glyphs);

#endif  // LIB_DISPLAY_DISPLAY_H_
