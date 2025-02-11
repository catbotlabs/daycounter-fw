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
#include "globals.h"
#include "uart.h"
#include "ui.h"
#include "display.h"
#include "rtc.h"

// 1: UARTTX/OE pin used as OE.
// 0: UARTTX/OE pin used as UARTTX.
bool oe_mode = false;
volatile uint8_t busy_flags = 0;

enum
{
    OEMODE_UART,
    OEMODE_OE
};

DisplayGlyphs glyphs;

void setup()
{
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
        swuart_mode_rx();
    }
    else if (oe_mode == OEMODE_OE)
    {
        DDRA |= (1 << PA2);  // OE pin: output
        PORTA |= (1 << PA2); // OE pin: high (disable display)
    }

    setup_display();

    Wire.setClock(300000); // enable USI TWI fast mode
    Wire.begin();

    setup_rtc();

    ssd1306_128x32_i2c_init();

    ssd1306_fillScreen(0x00);
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
}

void loop()
{
    do
    {
        display_glyphs_oe(glyphs);
    } while (busy_flags == 0x00);

    if (busy_flags & BF_PPS)
    {
        // pulse-per-second
    }
    else if (busy_flags & BF_UARTRX)
    {
        // uart byte received.

        // We'll just echo back the character we received.
        swuart_mode_tx();
        swuart_write("\r\n");
        char msg[] = "[x]\r\n";
        msg[1] = swuart_rx_char;
        swuart_write(msg);
        swuart_mode_rx();
        busy_flags &= ~BF_UARTRX;

        swuart_rx_char = '\0'; // consume character so we can take more.
    }
}
