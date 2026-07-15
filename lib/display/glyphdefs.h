/**
 * @file lib/display/glyphdefs.h
 * @brief Definitions of the characters for the numeric display.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, CatBotLabs
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */
#ifndef LIB_DISPLAY_GLYPHDEFS_H_
#define LIB_DISPLAY_GLYPHDEFS_H_

#ifndef PIO_UNIT_TESTING
#include <avr/pgmspace.h>
#else
// Unit testing: Quick mock up for pgmspace tools
#define PROGMEM
#define pgm_read_byte(ptr) (*(ptr))
#endif  // PIO_UNIT_TESTING

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
 *    bit meaning:     G F E D C B A s
 * 
 * Where "s" is reserved for the DG1 sink enable signal,
 * which must be set per-digit before being passed to the
 * display draw function.
 * 
 * The glyph is shifted out MSB first, so
 * [Qa, Qb, Qc, ... Qh] on the shift register corresponds to
 * [s,  A,  B,  ... G ] in our glyph packing.
 * 
 */

// Glyphs:
//      0bGFEDCBAs

//  '0' 0b01111110
//  '1' 0b00001100
//  '2' 0b10110110
//  '3' 0b10011110
//  '4' 0b11001100
//  '5' 0b11011010
//  '6' 0b11111010
//  '7' 0b00001110
//  '8' 0b11111110
//  '9' 0b11011110
//  ' ' 0b00000000
//  '-' 0b10000000

/**
 * These are the seven-segment glyphs for integers 0-9,
 * conveniently mapped in an array.
 * 
 * (For instance, DIGIT_GLYPHS[3] is the glyph for the number '3').
 */
static const uint8_t PROGMEM DIGIT_GLYPHS[10] = {0x7e, 0x0c, 0xb6, 0x9e,
                                                 0xcc, 0xda, 0xfa, 0x0e,
                                                 0xfe, 0xde};


static const uint8_t GLYPH_DASH = 0x80;
static const uint8_t GLYPH_NONE = 0x00;

#endif  // LIB_DISPLAY_GLYPHDEFS_H_
