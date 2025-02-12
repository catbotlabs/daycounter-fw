/**
 * @file src/main.cpp
 * @brief Main routine
 *
 * Part of the CatBotLabs K001 Day Counter firmware.
 *
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */
#include <Arduino.h>
#include <Wire.h>
#include <avr/wdt.h>
#include "globals.h"
#include "uart.h"
#include "ui.h"
#include "display.h"
#include "rtc.h"
#include "commands.h"

bool oe_mode = false;
volatile uint8_t busy_flags = 0;

DisplayGlyphs glyphs;

/**
 * Hardware interrupt 0 (PB2) triggered by rising edge of the
 * pulse-per-second signal. This is the MCU's primary method
 * of keeping time.
 * PB2 is tied to the RTC's multi-function (MFP) pin.
*/
ISR(INT0_vect) {
    busy_flags |= BF_PPS;
}

void setup() {
    /** TODO: determine if/whether interrupts (I-bit) are set in this function. */

    /** Disable watchdog in case of watchdog reset. */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /**
     * IF PA2 is HIGH:
     *   UART is plugged in, set oe_mode 0.
     * IF PA2 is LOW:
     *   Standard configuration. Use OE to eliminate ghosting, set oe_mode 1.
     */
    oe_mode = !((PINA & (1 << PA2)) >> PA2);
    if (oe_mode == OEMODE_UART)
    {
        setup_swuart();
        swuart_mode_tx();
        swuart_write(F("\r\n== CatBotLabs DayCounter K001 ==\r\n"
                       "built " __DATE__ __TIME__ "\r\n"));
    }
    else if (oe_mode == OEMODE_OE)
    {
        DDRA |= (1 << PA2);  // OE pin: output
        PORTA |= (1 << PA2); // OE pin: high (disable display)
    }

    /** Initialize pins that drive the digital display. */
    setup_display();

    /** Initialize I2C bus */
    Wire.setClock(300000); // enable USI TWI fast mode
    Wire.begin();

    /** Setup and configure OLED screen. */
    ssd1306_128x32_i2c_init();
    ssd1306_fillScreen(0x00);

    /** Setup PPS interrupts */
    PORTB |= (1 << PB2);  // enable pullup resistor (MFP pin is open-drain)
    // INT0 interrupts on rising edge
    MCUCR |= (1 << ISC01) | (1 << ISC00);
    // enable INT0 interrupts
    GIMSK |= (1 << INT0);

    /** Setup RTC */
    switch (setup_rtc()) {
        case 2:
          // TODO: show a message on the OLED screen?
          swuart_write(F("fatal: failed to communicate with RTC\r\n"));
          break;
        case 1:
          swuart_write(F("info: RTC registers adjusted\r\n"));
          break;
        case 0:
          swuart_write(F("info: RTC okay\r\n"));
    }

    /** OLED screen demo */
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_printFixedN(30, 0, "Set Date", STYLE_NORMAL, 0);
    ssd1306_setFixedFont(digital_font5x7);
    if (oe_mode)
    {
        ssd1306_printFixedN(20, 20, "oe_mode 1", STYLE_NORMAL, 1);
    }
    else
    {
        ssd1306_printFixedN(20, 20, "oe_mode 0", STYLE_NORMAL, 1);
    }
    //    ssd1306_printFixedN(20, 20, "Mar11,23", STYLE_NORMAL, 1);
    //    ssd1306_printFixedN(0, 18, "17:23:06", STYLE_NORMAL, 1);

    set_glyphs(230, &glyphs);

    /** If oe_mode == OEMODE_UART, then ready RX mode on UART. */
    swuart_mode_rx();
}

void loop()
{
    do {
        display_glyphs_oe(glyphs);
    } while (busy_flags == 0x00);

    if (busy_flags & BF_PPS) {
        // pulse-per-second
        UART_TX_BLOCK {
            static uint8_t ticks = 0;
            swuart_write("tick ");
            swuart_print_int(ticks);
            swuart_newline();
            ticks++;
        }
        busy_flags &= ~BF_PPS;
    } else if (busy_flags & BF_UARTRX) {
        // uart byte received.
        handle_uart_char(swuart_rx_char);
        swuart_rx_char = '\0';  // consume character so we can take more.
        busy_flags &= ~BF_UARTRX;
    }
}
