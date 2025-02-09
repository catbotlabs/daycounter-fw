/**
 * @file src/display_hw.cpp
 * @brief Hardware-related code relating to the digital display.
 * This code isn't included in unit testing.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "display.h"
#include <avr/io.h>

/**
 * Display-related pins:
 * PA2: UART/OE  [Blanks display during transitions to eliminate ghosting]
 * PA5: DG0/DG1  [Select DG0 if HIGH or DG1 when LOW]
 * PA7: Shift Register Latch
 * PB0: Shift Register Clock
 * PB1: Shift Register Data
 */

void setup_display() {
    // Set PB0 and PB1 as outputs, set low.
    DDRB |= (1 << PB0);
    DDRB |= (1 << PB1);

    // Set PA5 and PA7 as outputs, set low.
    DDRA |= (1 << PA5);
    DDRA |= (1 << PA7);
}

/**
 * Shift out one byte MSB-first to the display shift register.
 * TODO: Benchmark to see if inlining this function actually makes
 * things faster.
 */
__attribute__((always_inline)) inline void display_send_byte(uint8_t data) {
    for (uint8_t i = 0; i < 8; i++) {
        // set data pin to bit value
        if (data & 0x80) {
            PORTB |= (1 << PB1);
        } else {
            PORTB &= ~(1 << PB1);
        }

        PORTB &= ~(1 << PB0);  // clock set low

        data <<= 1;

        PORTB |= (1 << PB0);   // clock rising edge
    }
}

void display_glyphs_oe(const DisplayGlyphs& glyphs) {
    /// DIGIT 0: Ones place
    display_send_byte(glyphs.dg[0]);  // write shift register
    PORTA |= (1 << PA2);   // OE high
    PORTA |= (1 << PA7);   // set latch
    PORTA |= (1 << PA5);   // set DGO
    PORTA &= ~(1 << PA2);  // OE low
    PORTA &= ~(1 << PA7);  // clear latch

    asm volatile ("nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        ::);  // Six extra clock cycles for balance

    /// DIGIT 1: Tens place
    display_send_byte(glyphs.dg[1]);  // write shift register
    PORTA |= (1 << PA2);   // OE high
    PORTA |= (1 << PA7);   // set latch
    PORTA &= ~(1 << PA5);  // clear DG0
    PORTA &= ~(1 << PA2);  // OE low
    PORTA &= ~(1 << PA7);  // clear latch

    asm volatile ("nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        ::);  // Six extra clock cycles for balance

    /// DIGIT 2: Hundreds place
    display_send_byte(glyphs.dg[2]);  // write shift register
    PORTA |= (1 << PA2);   // OE high
    PORTA |= (1 << PA7);   // set latch
    PORTA &= ~(1 << PA2);  // OE low
    PORTA &= ~(1 << PA7);  // clear latch

    /* Note: The main loop condition makes the hundreds place drawn for
       6 additional clock cycles, so NOP instructions have been added
       to pad the Ones and Tens places. */
}


// Here's an display glyphs function that doesn't use the OE pin.
#if 0
void display_glyphs_no_oe(const DisplayGlyphs& glyphs) {
    for (uint8_t i = 0; i < 3; i++) {
        display_send_byte(glyphs.dg[i]);  // write shift register
        PORTA |= (1 << PA7);   // set latch
        if (i == 0) {
            PORTA |= (1 << PA5);   // set DGO
        }
        if (i == 1) {
            PORTA &= ~(1 << PA5);   // clear DG0
        }
        PORTA &= ~(1 << PA7);  // clear latch
    }
}
#endif
