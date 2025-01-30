/**
 * @file src/display.cpp
 * @brief Code relating to the digital display.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "display.h"
#include <avr/pgmspace.h>

/**
 * Digital display glyphs are 7 bits wide, one bit for each
 * segment.
 * 
 *  A_
 * F[_]B   (G is center bar)
 * E[_]C
 *   D
 * 
 * Each glyph is packed in a uint8_t as such:
 * 
 *    bit position:   [7 6 5 4 3 2 1 0]
 *    bit meaning:     x G F E D C B A
 * 
 * Where "x" is reserved for the DG0 sink enable signal,
 * which is set per-digit in the display draw function.
 * 
 * The glyph is shifted out MSB first, so
 * [Qa, Qb, Qc, ... Qh] on the shift register corresponds to
 * [A,  B,  C,  ... x ] in our glyph packing.
 * 
 */

//      0bGFEDCBA
//  '0' 0b0111111
//  '1' 0b0000110
//  '2' 0b1011011
//  '3' 0b1001111
//  '4' 0b1100110
//  '5' 0b1101101
//  '6' 0b1111101
//  '7' 0b0000111
//  '8' 0b1111111
//  '9' 0b1101111
//  ' ' 0b0000000
//  '-' 0b1000000

static const PROGMEM uint8_t DIGIT_GLYPHS[10] = {0x3f, 0x06, 0x4f, 0x5b,
                                                 0x66, 0x6d, 0x7d, 0x07,
                                                 0x7f, 0x6f};

static const uint8_t GLYPH_DASH = 0x40;
static const uint8_t GLYPH_NONE = 0x00;

/**
 * Display-related pins:
 * PB0: Shift Register Clock
 * PB1: Shift Register Data
 * PA7: Shift Register Latch
 * 
 */


void setup_display() {
    // Set PB0 and PB1 as outputs with data 0.
    PORTB &= ~((1 << PB1) | (1 << PB0));
    DDRB |= (1 << PB1) | (1 << PB0);

    // Set PA7 as output with data 0.
    PORTA &= ~(1 << PA7);
    DDRA |= (1 << PA7);
}


void set_glyphs(uint16_t count, DisplayGlyphs& glyphs) {
    // If count is below zero, set display to "  -"
    if (count < 0) {
        glyphs.dg[0] = GLYPH_DASH;
        glyphs.dg[2] = glyphs.dg[1] = GLYPH_NONE;
    } else if (count > 999) {
        // If count exceeds 999, set display to "---"
        glyphs.dg[2] = glyphs.dg[1] = glyphs.dg[0] = GLYPH_DASH;
    } else {
        if (count > 99) {
            // set hundreds digit
            glyphs.dg[2] = DIGIT_GLYPHS[count / 100];
        } else {
            // blank hundreds digit
            glyphs.dg[2] = GLYPH_NONE;
        }

        if (count > 9) {
            // set tens digit
            glyphs.dg[1] = DIGIT_GLYPHS[count % 100 / 10];
        } else {
            // blank tens digit
            glyphs.dg[1] = GLYPH_NONE;
        }

        // set ones digit
        glyphs.dg[0] = DIGIT_GLYPHS[count % 10];
    }
}

/**
 * Shift out one byte MSB-first to the display shift register.
 */
__attribute__((always_inline)) inline void display_send_byte(uint8_t data) {
    // latch set low
    PORTA &= ~(1 << PA7);
    for (uint8_t i = 0; i < 8; i++) {
        // set data pin to bit value
        if (data & 0x80) {
            PORTB |= (1 << PB1);
        } else {
            PORTB &= ~(1 << PB1);
        }

        // clock set low
        PORTB &= ~(1 << PB0);

        data <<= 1;

        // clock rising edge
        PORTB |= (1 << PB0);

    }
    // latch rising edge
    PORTA |= (1 << PA7);
};


void display_glyphs(const DisplayGlyphs& glyphs) {
    display_send_byte(glyphs.dg[0] | 0x80);
    display_send_byte(glyphs.dg[1]);
    display_send_byte(glyphs.dg[2]);
}
