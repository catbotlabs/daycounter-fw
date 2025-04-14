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
#include "glyphdefs.h"

/** {dg0, dg1, dg2} */
using DisplayGlyphs = uint8_t[3];

/**
 * Configure the three pins that drive the display shift register
 * as outputs with data 0.
 */
void setup_display();

/**
 * Set the three glyph bytes using a three-digit number.
 * 
 * @param count: The given three-digit number. Must be between 0 and 999,
 * no bounds checking.
 */
void set_glyphs(int16_t count, DisplayGlyphs glyphs);


void display_glyphs_oe(const DisplayGlyphs glyphs);

#endif  // LIB_DISPLAY_DISPLAY_H_
