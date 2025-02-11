/**
 * @file src/uart.cpp
 * @brief Software UART using Analog Comparator interrupts.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "uart.h"
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/cpufunc.h>
#include <avr/pgmspace.h>

#include "globals.h"

/**
 * TX and RX functionality are shared on PA2.
 * -------------------------------
 * 
 *                           [SECONDARY STATE]
 *  [PA2] ---------------->  TX (from MCU)
 *             | 
 *            |=|  (resistor)
 *            |=|
 *             |
 *             -----------<  RX (to MCU)
 *                           [DEFAULT STATE]
 * 
 * 
 * The shared lines will cause echoing back to the source.
 * Our interface will only require single-character commands
 * so this isn't a big deal.
 * 
 */

/* ============================================================

The following section is based on code from attiny13-software-uart-library
by lpodkalicki (https://github.com/lpodkalicki/attiny13-software-uart-library).
In keeping with the license of the attiny13-software-uart-library,
the following code is distributed under the terms of the BSD 2-Clause License.


"Copyright (c) 2017, Łukasz Podkalicki
All rights reserved."

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * "Copyright (c) 2017, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * The ASM code is based on Ralph Doncaster's project
 * (https://github.com/nerdralph/nerdralph/tree/master/avr/libs/bbuart)
 * Software UART for ATtiny13"
 */


// [19200 was too slow for this implementation running at 8MHz.]
// [115200 is too fast to work reliably without calibrating the oscillator.]
#define UART_BAUDRATE 57600

#define TXDELAY         (int)(((F_CPU/UART_BAUDRATE)-7 +1.5)/3)
#define RXDELAY         (int)(((F_CPU/UART_BAUDRATE)-5 +1.5)/3)
#define RXDELAY2        (int)((RXDELAY*1.5)-2.5)
#define RXROUNDED       (((F_CPU/UART_BAUDRATE)-5 +2)/3)
#if RXROUNDED > 127
# error Low baud rates are not supported - use higher, UART_BAUDRATE
#endif

/// It's assumed that interrupts are disabled within this context.
__attribute__((always_inline)) inline uint8_t rx_byte_sw_uart() {
    uint8_t rxbyte;
    asm volatile(
        " ldi r18, %[rxdelay2] \n\t" // 1.5 bit delay
        " ldi %0, 0x80 \n\t" // bit shift counter
        "RxBit%=: \n\t"
        // 6 cycle loop + delay - total = 5 + 3*r22
        // delay (3 cycle * r18) -1 and clear carry with subi
        " subi r18, 1 \n\t"
        " brne RxBit%= \n\t"
        " ldi r18, %[rxdelay] \n\t"
        " sbic %[uart_port]-2, %[uart_pin] \n\t" // check UART PIN
        " sec \n\t"
        " ror %0 \n\t"
        " brcc RxBit%= \n\t"
        "StopBit%=: \n\t"
        " dec r18 \n\t"
        " brne StopBit%= \n\t"
        : "=r" (rxbyte)
        : [uart_port] "I" (_SFR_IO_ADDR(PORTA)),
        [uart_pin] "I" (PA2),
        [rxdelay] "I" (RXDELAY),
        [rxdelay2] "I" (RXDELAY2)
        : "r0","r18","r19"
    );
    return rxbyte;
}

void tx_byte_sw_uart(uint8_t txbyte) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        asm volatile(
            " cbi %[uart_port], %[uart_pin] \n\t" // start bit
            " in r0, %[uart_port] \n\t"
            " ldi r30, 3 \n\t" // stop bit + idle state
            " ldi r28, %[txdelay] \n\t"
            "TxLoop%=: \n\t"
            // 8 cycle loop + delay - total = 7 + 3*r22
            " mov r29, r28 \n\t"
            "TxDelay%=: \n\t"
            // delay (3 cycle * delayCount) - 1
            " dec r29 \n\t"
            " brne TxDelay%= \n\t"
            " bst %[ch], 0 \n\t"
            " bld r0, %[uart_pin] \n\t"
            " lsr r30 \n\t"
            " ror %[ch] \n\t"
            " out %[uart_port], r0 \n\t"
            " brne TxLoop%= \n\t"
            :
            : [uart_port] "I" (_SFR_IO_ADDR(PORTA)),
            [uart_pin] "I" (PA2),
            [txdelay] "I" (TXDELAY),
            [ch] "r" (txbyte)
            : "r0","r28","r29","r30"
        );
    }  // ATOMIC_BLOCK
}

/* End of code derived from attiny13-software-uart-library
============================================================ */


volatile char swuart_rx_char = '\0';

/**
 * Analog comparator interrupt: It's being triggered on
 * falling edge of UART RX line.
 */
ISR(ANA_COMP_vect) {
    char rx_char = rx_byte_sw_uart();  // Read the incoming byte
    ACSR |= (1 << ACI);                // Reset comparator interrupt flag
    if (swuart_rx_char != '\0') {
        // Stored character has not been consumed yet.
        // Discard the new character.
        return;
    }
    swuart_rx_char = rx_char;
    busy_flags |= BF_UARTRX;
}

void setup_swuart() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Analog comparator: Set positive input to bandgap voltage,
        // trigger interrupts on rising ACO edge.
        ACSR |= (1 << ACBG) | (1 << ACIS0) | (1 << ACIS1);
        swuart_mode_rx();
    }  // ATOMIC_BLOCK
}

void swuart_mode_rx() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Pin as input, pullup disabled
        PORTA &= ~(1 << PA2);
        DDRA &= ~(1 << PA2);
        // Enable Analog Comparator interrupts
        ACSR |= (1 << ACIE);
        ACSR |= (1 << ACI);  // clear interrupt flag
    }
}

void swuart_mode_tx() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // Pin as output, HIGH.
        PORTA |= (1 << PA2);
        DDRA |= (1 << PA2);
        // Disable Analog Comparator interrupts
        ACSR &= ~(1 << ACIE);
        ACSR |= (1 << ACI);  // clear interrupt flag
    }
}

// write single character.
void swuart_write(char out) {
    tx_byte_sw_uart(out);
}

// write until terminating character.
void swuart_write(const char* out) {
    while (*out) {
        tx_byte_sw_uart(*out++);
    }
}

// write until terminating character, strings using F().
void swuart_write(const __FlashStringHelper* fptr) {
    PGM_P pgm_ptr = reinterpret_cast<PGM_P>(fptr);
    while (char out = pgm_read_byte(pgm_ptr++)) {
        tx_byte_sw_uart(out);
    }
}


// assumes dg is between 0 and 9.
__attribute__((always_inline)) inline void swuart_print_digit(uint8_t dg) {
    tx_byte_sw_uart('0' + dg);
}


// print a 16-bit signed integer.
void swuart_print_int(int16_t num) {
    if (num < 0) {
        tx_byte_sw_uart('-1');
        num *= -1;
    }
    if (num > 10000) {
        swuart_print_digit(num / 10000);
        num %= 10000;
    }
    if (num > 1000) {
        swuart_print_digit(num / 1000);
        num %= 1000;
    }
    if (num > 100) {
        swuart_print_digit(num / 100);
        num %= 100;
    }
    if (num > 10) {
        swuart_print_digit(num / 10);
        num %= 10;
    }
    swuart_print_digit(num);
}
