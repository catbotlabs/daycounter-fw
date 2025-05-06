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
#include "buttons.h"
#include "settings.h"

/**
 * Print a Date object to the software UART.
 */
void print_date(const Date& date) {
    char date_str[11];
    date_str[10] = '\0';
    generate_date_string(date_str, date);
    swuart_write(date_str);
}

/**
 * Print a Time object to the software UART.
 */
void print_time(const Time& time) {
    char time_str[9];
    time_str[8] = '\0';
    generate_time_string(time_str, time);
    swuart_write(time_str);
}


// 'r'
void cmd_watchdog_reset() {
    swuart_writeln(PS("watchdog reset"));
    wdt_enable(WDTO_15MS);
    while (true) {}
}

// 'h'
void cmd_say_hello() {
    swuart_writeln(PS("Hi there!"));
}

// 't'
void cmd_system_test() {
    #if 0  // todo: cleanup
    swuart_writeln(PS("system test begin"));

    /** Test the OLED screen by drawing text on it */
    // ssd1306_clearScreen();
    // ssd1306_printFixedN(0, 1, "OLED screen test", STYLE_NORMAL, 1);

    /** Test the display by displaying all 8's */
    // DDRA for PA2 is set as output since we are in UART TX mode.
    for (uint16_t i = 30000; i != 0; i++) {
        DisplayGlyphs gl{DIGIT_GLYPHS[8], DIGIT_GLYPHS[8]|1, DIGIT_GLYPHS[8]};
        display_glyphs_oe(gl);
    }
    PORTA |= (1 << PA2);  // Set UART pin HIGH after being used for OE.
    swuart_writeln(PS("\r\nsystem test completed"));
    #endif
}

/**
 * Process a single character received from the software UART interface.
 * 
 * This is our chance to respond and/or run a command.
 */
void handle_uart_char(char ch) {
    swuart_mode_tx();
    swuart_write('\r'); swuart_write('\n');
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
    }
    swuart_mode_rx();
}
