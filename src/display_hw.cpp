/**
 * @file src/display_hw.cpp
 * @brief Hardware-related code for driving the numeric display.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, CatBotLabs
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "display.h"
#include <avr/io.h>
#include <avr/cpufunc.h>

/**
 * Display-related pins:
 * PA2: UART/OE  [Blanks display during transitions to eliminate ghosting]
 * PA5: DG0 sink enable
 * PA7: Shift Register Latch
 * PB0: Shift Register Clock
 * PB1: Shift Register Data
 */

/**
 * Note that the DG1 output is the shift register's LSB, and
 * the DG2 output is [ (DG0) NOR (DG1) ] in the hardware.
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
 * 
 * This function is inlined to reduce function call overhead time;
 * letting display_glyphs_oe run faster and more frequently.
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

/**
 * A blocking function that draws each glyph once
 * on the display, starting at DG0: (DG2 <- DG1 <- DG0).
 * 
 * Note: The LSB must be set for glyphs[1],
 * but must be cleared for glyphs[0] and glyphs[2].
 * 
 * Each digit needs to be drawn for an equal amount of time,
 * as this affects the perceived brightness.
 * 
 * This function has been balanced with NOP calls,
 * because otherwise the hundreds (DG2) digit would be drawn the
 * longest (due to the function return/calls and loop conditions).
 * 
 * The timing of this function is optimized for where it is called
 * in the main loop:
 
    do {
        display_glyphs_oe(glyphs);
    } while (busy_flags == 0x00);

 * 
 * See the "display_digit_balance" notes in docs/ for more information.
 */
void display_glyphs_oe(const DisplayGlyphs glyphs) {
    /** Digit 0: Ones place */
    display_send_byte(glyphs[0]);  // write shift register
    PORTA |= (1 << PA2);   // OE high
    _NOP();
    PORTA |= (1 << PA5);   // set DGO, DG2 goes low.
    PORTA |= (1 << PA7);   // set latch
    PORTA &= ~(1 << PA2);  // OE low
    PORTA &= ~(1 << PA7);  // clear latch

    // Needs a 1.5us delay for balance -> 12 clock cycles
    asm volatile (
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        ::);

    /** Digit 1: Tens place */
    display_send_byte(glyphs[1]);  // write shift register
    PORTA |= (1 << PA2);   // OE high
    PORTA |= (1 << PA7);   // set latch
    PORTA &= ~(1 << PA5);  // clear DG0
    _NOP();
    PORTA &= ~(1 << PA2);  // OE low: DG1 goes high.
    PORTA &= ~(1 << PA7);  // clear latch

    // Needs a 1.2us delay for balance -> 10 clock cycles
    asm volatile (
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        ::);

    /** Digit 2: Hundreds place */
    display_send_byte(glyphs[2]);  // write shift register

    PORTA |= (1 << PA2);   // OE high: DG1 goes low, DG2 goes high.
    _NOP();
    PORTA |= (1 << PA7);   // set latch
    PORTA &= ~(1 << PA7);  // clear latch
    PORTA &= ~(1 << PA2);  // OE low

    /**
     * The function/loop overhead main loop condition makes the
     * hundreds place drawn extra long. To balance this, NOP calls
     * are added to the ones and tens place sections, but not here.
     */
}
