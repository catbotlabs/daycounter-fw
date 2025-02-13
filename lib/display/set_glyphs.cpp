/**
 * @file lib/display/set_glyphs.cpp
 * @brief Implementation of set_glyphs function
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "display.h"
#include "glyphdefs.h"

void set_glyphs(int16_t count, DisplayGlyphs glyphs) {
    // If count is below zero, set display to "  -"
    if (count < 0) {
        glyphs[0] = GLYPH_DASH;
        glyphs[2] = glyphs[1] = GLYPH_NONE;
    } else if (count > 999) {
        // If count exceeds 999, set display to "---"
        glyphs[2] = glyphs[1] = glyphs[0] = GLYPH_DASH;
    } else {
        if (count > 99) {
            // set hundreds digit
            glyphs[2] = pgm_read_byte(&DIGIT_GLYPHS[count / 100]);
        } else {
            // blank hundreds digit
            glyphs[2] = GLYPH_NONE;
        }

        if (count > 9) {
            // set tens digit
            glyphs[1] = pgm_read_byte(&DIGIT_GLYPHS[count % 100 / 10]);
        } else {
            // blank tens digit
            glyphs[1] = GLYPH_NONE;
        }

        // set ones digit
        glyphs[0] = pgm_read_byte(&DIGIT_GLYPHS[count % 10]);
    }

    // Always set the DG1 bit on the tens place digit glyph.
    glyphs[1] |= 1;
}
