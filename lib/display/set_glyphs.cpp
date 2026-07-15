/**
 * @file lib/display/set_glyphs.cpp
 * @brief Implementation of integer to display character conversion function.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, CatBotLabs
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "display.h"
#include "glyphdefs.h"

void set_glyphs_0_to_999(int16_t count, DisplayGlyphs glyphs) {
    if (count > 99) {
        // set hundreds digit
        glyphs[2] = pgm_read_byte(&DIGIT_GLYPHS[count / 100]);
    } else {
        // blank hundreds digit
        glyphs[2] = GLYPH_NONE;
    }

    // Always set the DG1 bit on the tens place digit glyph.
    if (count > 9) {
        // set tens digit
        glyphs[1] = pgm_read_byte(&DIGIT_GLYPHS[count % 100 / 10]) | 1;
    } else {
        // blank tens digit
        glyphs[1] = GLYPH_NONE | 1;
    }

    // set ones digit
    glyphs[0] = pgm_read_byte(&DIGIT_GLYPHS[count % 10]);
}
