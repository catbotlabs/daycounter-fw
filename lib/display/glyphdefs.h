/**
 * @file lib/display/glyphdefs.h
 * @brief Definitions of the glyph constants.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
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
 *    bit meaning:     A G F E D C B s
 * 
 * Where "s" is reserved for the DG1 sink enable signal,
 * which is set per-digit in the display draw function.
 * 
 * The glyph is shifted out MSB first, so
 * [Qa, Qb, Qc, ... Qh] on the shift register corresponds to
 * [s,  B,  C,  ... A ] in our glyph packing.
 * 
 */

// Glyphs:
//      0bAGFEDCBs
//  '0' 0b10111110
//  '1' 0b00000110
//  '2' 0b11011010
//  '3' 0b11001110
//  '4' 0b01100110
//  '5' 0b11101100
//  '6' 0b11111100
//  '7' 0b10000110
//  '8' 0b11111110
//  '9' 0b11101110
//  ' ' 0b00000000
//  '-' 0b01000000

/**
 * These are the seven-segment glyphs for integers 0-9,
 * conveniently mapped in an array.
 * 
 * (For instance, DIGIT_GLYPHS[3] is the glyph for the number '3').
 */
static const uint8_t PROGMEM DIGIT_GLYPHS[10] = {0xbe, 0x06, 0xda, 0xce,
                                                 0x66, 0xec, 0xfc, 0x86,
                                                 0xfe, 0xee};


static const uint8_t GLYPH_DASH = 0x40;
static const uint8_t GLYPH_NONE = 0x00;

#endif  // LIB_DISPLAY_GLYPHDEFS_H_
