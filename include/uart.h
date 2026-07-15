/**
 * @file include/uart.h
 * @brief Software UART using Analog Comparator interrupts.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, CatBotLabs
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef INCLUDE_UART_H_
#define INCLUDE_UART_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/**
 * The character that has been received from the soft UART.
 * Effectively a single-byte buffer. Once set, it will not
 * be overwritten until swuart_rx_char is reset to '\0'.
 */
extern volatile char swuart_rx_char;

/** Helper type to mark strings stored in program space. */
class ProgmemString;

/** Casts a C-string to a pointer-to-ProgmemString */
#define PS(string_literal) \
    (reinterpret_cast<const ProgmemString*>(PSTR(string_literal)))


// Sets up the software UART by configuring the Analog Comparator and
// calling swuart_mode_rx.
void setup_swuart();

// Configures UART pin as input, sets up pin's interrupts.
// Does nothing if oe_mode is set to OEMODE_OE.
// Always returns 0.
bool swuart_mode_rx();

// Configures UART pin as output, disables pin's interrupts.
// Does nothing if oe_mode is set to OEMODE_OE.
// Always returns 1.
bool swuart_mode_tx();

/** Write a single character over software UART. */
void swuart_write(char out);

/** Write C-string over software UART. */
void swuart_write(const char* out);

/** Write program-space string and newline over software UART. */
void swuart_writeln(const ProgmemString* pstr);

/**
 * Print a signed 16-bit integer as base 10 text to
 * software UART.
 */
void swuart_print_int(int16_t num);

/**
 * Block wrapper that calls swuart_mode_tx() before the block
 * and swuart_mode_rx() after the block.
 * 
 * Example:
 * 
 * UART_TX_BLOCK {    // (calls swuart_mode_tx())
 *    [swuart_write calls]
 * }  // (calls swuart_mode_rx())
 * 
 * Note: Since this is based on a simple for loop,
 * it will not call swuart_mode_rx if you break out
 * using return.
 */
#define UART_TX_BLOCK for (uint8_t i=swuart_mode_tx(); i; i=swuart_mode_rx())

#endif  // INCLUDE_UART_H_
