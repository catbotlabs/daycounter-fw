/**
 * @file include/uart.h
 * @brief Software UART using Analog Comparator interrupts.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef INCLUDE_UART_H_
#define INCLUDE_UART_H_

#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/**
 * The character that has been received from the soft UART.
 * Effectively a single-byte buffer. Once set, it will not
 * be overwritten until swuart_rx_char is reset to '\0'.
 */
extern volatile char swuart_rx_char;


// Sets up the software UART by configuring the Analog Comparator and
// calling swuart_mode_rx.
void setup_swuart();

// Configures UART pin as input, sets up pin's interrupts.
void swuart_mode_rx();

// Configures UART pin as output, disables pin's interrupts.
void swuart_mode_tx();

void swuart_write(char out);
void swuart_write(const char* out);
void swuart_write(const __FlashStringHelper* fptr);

#endif  // INCLUDE_UART_H_
