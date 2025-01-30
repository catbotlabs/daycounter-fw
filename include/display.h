/**
 * @file include/display.h
 * @brief Digital display code.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef DCFW_INCLUDE_DISPLAY_H_
#define DCFW_INCLUDE_DISPLAY_H_

#include <stdint.h>

struct DisplayGlyphs {
    /**
     * dg = {dg2, dg1, dg0}
     * 
     * dg2: hundreds place glyph
     * dg1: tens place glyph
     * dg0: ones place glyph
     */
    uint8_t dg[3];
};

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
void set_glyphs(uint16_t count, DisplayGlyphs& glyphs);  // TEST this function
void display_glyphs(const DisplayGlyphs& glyphs);

#endif  // DCFW_INCLUDE_DISPLAY_H_
