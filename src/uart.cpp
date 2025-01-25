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


#ifndef UART_BAUDRATE
// 19200 was too slow for this implementation running at 8MHz.
// 115200 is too fast to work reliably without calibrating the oscillator.
#define UART_BAUDRATE 57600
#endif

#define PIN_TX PA1  // TX pin is shared with the SET button pin.
#define PIN_RX PA2  // Use PA2 because it has AIN1 for interrupt.
// TX PIN: PA1
// RX PIN: PA2

#define	TXDELAY         	(int)(((F_CPU/UART_BAUDRATE)-7 +1.5)/3)
#define RXDELAY         	(int)(((F_CPU/UART_BAUDRATE)-5 +1.5)/3)
#define RXDELAY2        	(int)((RXDELAY*1.5)-2.5)
#define RXROUNDED       	(((F_CPU/UART_BAUDRATE)-5 +2)/3)
#if RXROUNDED > 127
# error Low baud rates are not supported - use higher, UART_BAUDRATE
#endif

/// It's assumed that interrupts are disabled within this context.
__attribute__((always_inline)) inline uint8_t rx_byte_sw_uart() {
    uint8_t rxbyte;
    __asm volatile(
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

/// It's assumed that interrupts are disabled within this context.
void tx_byte_sw_uart(uint8_t txbyte) {
    __asm volatile(
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
        [uart_pin] "I" (PIN_TX),
        [txdelay] "I" (TXDELAY),
        [ch] "r" (txbyte)
        : "r0","r28","r29","r30"
    );
}

/* End of code derived from attiny13-software-uart-library
============================================================ */

#define SWUART_RX_BUFFER_LEN 32

template<uint8_t buffer_len>
class ByteRingBuffer {
 public:
    inline bool is_empty() volatile {
        return read_index == write_index;
    }

    bool put(uint8_t in) volatile {
        if ((write_index + 1) % buffer_len == read_index) {
            return false;  // buffer is full, avoid overflow
        }
        buffer[write_index] = in;
        write_index = (write_index + 1) % buffer_len;
        return true;
    }

    bool get(uint8_t& out) volatile {
        if (is_empty()) {
            return false;  // buffer is empty
        }

        out = buffer[read_index];
        read_index = (read_index + 1) % buffer_len;
        return true;
    }

    bool peek(uint8_t& out) volatile {
        if (is_empty()) {
            return false;
        }
        out = buffer[read_index];
        return true;
    }

 protected:
    uint8_t buffer[buffer_len];
    uint8_t write_index = 0;
    uint8_t read_index = 0;
};

DayCounterSWUART sw_uart;
volatile ByteRingBuffer<SWUART_RX_BUFFER_LEN> rx_buffer;

volatile bool got_rx = false;

ISR(ANA_COMP_vect) {
    uint8_t rx_char = rx_byte_sw_uart();
    rx_buffer.put(rx_char);
    ACSR |= (1 << ACI);  // clear comparator interrupt flag
    got_rx = true;
    // impl: set busy flag for RX received
}

/// It's assumed that interrupts are disabled within this context.
__attribute__((always_inline)) inline void txpin_set_output_mode() {
    PCMSK0 &= ~(1 << PCINT1);  // disable PCINT for this pin
    PORTA |= (1 << PA1);       // logic 1
    DDRA |= (1 << PA1);        // output
}

/// It's assumed that interrupts are disabled within this context.
__attribute__((always_inline)) inline void txpin_set_input_mode() {
    DDRA &= ~(1 << PA1);       // input
    PORTA &= ~(1 << PA1);      // no pullup (have an external)
    GIFR |= (1 << PCIF0);
    PCMSK0 |= (1 << PCINT1);   // enable PCINT for this pin
}


void DayCounterSWUART::begin() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        // - Configure RX as input through Analog Comparator:
        // Analog comparator: Set positive input to bandgap voltage,
        // enable interrupts, trigger interrupts on rising ACO edge.
        ACSR |= (1 << ACBG) | (1 << ACIE) | (1 << ACIS0) | (1 << ACIS1);
        // RX pin as input, pullup disabled
        PORTA &= ~(1 << PIN_RX);
        DDRA &= ~(1 << PIN_RX);
        // - Configure TX as an input so it can listen for the SET button
        //txpin_set_input_mode();

        txpin_set_output_mode();
    }
}

int DayCounterSWUART::available() {
    return !rx_buffer.is_empty();
}

int DayCounterSWUART::peek() {
    uint8_t ch;
    if (!rx_buffer.peek(ch)) {
        return -1;
    }
    return ch;
}

int DayCounterSWUART::read() {
    uint8_t ch;
    if (!rx_buffer.get(ch)) {
        return -1;
    }
    return ch;
}

size_t DayCounterSWUART::write(uint8_t ch) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
//        txpin_set_output_mode(); 	// configure TX as output
        tx_byte_sw_uart(ch); 	// transmit one byte
//        txpin_set_input_mode();     // configure TX as input for SET button
    }
    return 1;
}

size_t DayCounterSWUART::write(const uint8_t* buffer, size_t size) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
//        txpin_set_output_mode(); 	// configure TX as output
        while (size--) {
            tx_byte_sw_uart(*buffer++);  // transmit one byte
        }
//        txpin_set_input_mode();     // configure TX as input for SET button
    }
    return size;
}