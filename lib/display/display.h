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

/**
 * The three "glyph" bytes to go to the numeric display,
 * one for each digit: {dg0, dg1, dg2}
 * 
 * DG0 is the ones place,
 * DG1 is the tens place,
 * DG2 is the hundreds place.
 */
using DisplayGlyphs = uint8_t[3];

/**
 * Initializes the GPIO pins used for the display.
 */
void setup_display();

/**
 * Sets the three glyph bytes to display a three-digit positive integer.
 * Also properly sets the DG1 bit (LSB) of each glyphs.
 * 
 * @param count: The given three-digit number. Must be between 0 and 999,
 * no bounds checking.
 * 
 * @param glpyhs: The three glyph bytes that will be set.
 */
void set_glyphs_0_to_999(int16_t count, DisplayGlyphs glyphs);

/**
 * Blocking function that draws each glyph once
 * on the display, starting at DG0: (DG2 <- DG1 <- DG0).
 * 
 * Note that the LSB must be cleared on glyphs[0] and glyphs[2],
 * but it must be set in glyphs[1] (to make the DG1 signal
 * function properly).
 * 
 * This function must be called continually in a loop
 * without any delay in order to draw cleanly and evenly.
 * 
 * The timing is balanced for being run in the main loop
 * in main.cpp
 * 
 * This function is defined in src/display_hw.cpp
 */
void display_glyphs_oe(const DisplayGlyphs glyphs);

#endif  // LIB_DISPLAY_DISPLAY_H_
