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

#ifndef	DCFW_INCLUDE_UART_H_
#define	DCFW_INCLUDE_UART_H_

#include <Arduino.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


/**
 * This is an arduino Stream compatable abstraction of
 * our software UART implementation.
 * 
 * It has one main quirk: In the DayCounter circuit, the GPIO
 * we're using for TX is also being used as the SET button
 * input whenever we're not transmitting. So the @ref write
 * function needs to configure the pin as an output and then
 * back to being an input.
 */
class DayCounterSWUART : public Stream {
 public:
    void begin();
    virtual int available() override;
    virtual int peek() override;
    virtual int read() override;
    virtual size_t write(uint8_t ch) override;

    /**
     * Multi-byte write implementation saves time by not having
     * to change the tx pin mode back and forth.
     */
    virtual size_t write(const uint8_t* buffer, size_t size) override;
    virtual void flush() override {};
};

extern DayCounterSWUART sw_uart;

extern volatile bool got_rx;

#endif  // DCFW_INCLUDE_UART_H_
