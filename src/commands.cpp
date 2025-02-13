/**
 * @file include/commands.h
 * @brief Various functions that can be called by characters
 * received over the software uart. This is for development purposes.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "commands.h"
#include <avr/wdt.h>
#include "uart.h"
#include "glyphdefs.h"
#include "display.h"
#include "ui.h"
#include "datetime.h"
#include "rtc.h"

// 'r'
void cmd_watchdog_reset() {
    swuart_write(F("watchdog reset\r\n"));
    wdt_enable(WDTO_15MS);
    while (true) {};
}

void cmd_say_hello() {
    swuart_write(F("Hi there!\r\n"));
}

// 't'
void cmd_system_test() {
    swuart_write(F("system test begin\r\n"));

    /** Test the OLED screen by drawing text on it */
    ssd1306_clearScreen();
    ssd1306_printFixedN(0, 1, "OLED screen test", STYLE_NORMAL, 1);

    /** Test the display by displaying all 8's */
    // DDRA for PA2 is set as output since we are in UART TX mode.
    for (uint16_t i=30000; i!=0; i++) {
        DisplayGlyphs gl{DIGIT_GLYPHS[8], DIGIT_GLYPHS[8]|1, DIGIT_GLYPHS[8]};
        display_glyphs_oe(gl);
    }
    PORTA |= (1 << PA2);  // Set UART pin HIGH after being used for OE.
    swuart_write(F("\r\nsystem test completed\r\n"));
}

void print_datetime(const DateTime& dt) {
    swuart_write("mm ");
    swuart_print_int(dt.date.mm);
    swuart_write(", dd ");
    swuart_print_int(dt.date.dd);
    swuart_write(", yy ");
    swuart_print_int(dt.date.yy);
    swuart_write(", HH ");
    swuart_print_int(dt.time.HH);
    swuart_write(", MM ");
    swuart_print_int(dt.time.MM);
    swuart_write(", SS ");
    swuart_print_int(dt.time.SS);
    swuart_newline();
}

void cmd_read_rtc_time() {
    DateTime dt;
    rtc_read_datetime(&dt);
    print_datetime(dt);
}

void handle_uart_char(char ch) {
    swuart_mode_tx();
    swuart_write(F("\r\n"));
    switch (ch) {
        case 'r':
          cmd_watchdog_reset();
          break;
        case 'h':
          cmd_say_hello();
          break;
        case 't':
          cmd_system_test();
          break;
        case 'd':
          cmd_read_rtc_time();
          break;
    }
    swuart_mode_rx();
}
