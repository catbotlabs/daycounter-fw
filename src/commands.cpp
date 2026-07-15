/**
 * @file include/commands.h
 * @brief Various functions that can be called by characters
 * received over the software uart. This is for development purposes.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, CatBotLabs
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
#include "globals.h"
#include "settings.h"


/**
 * This firmware does not have much free flash space, especially
 * when built in debug UART mode. These functions demonstrate
 * how to do basic debug tasks, but they use precious space. If
 * you were to implement custom functionality, you would want to
 * disable code here to free up space.
 */



/**
 * Set the display to "888" and draw text
 * on the OLED screen.
 *
 * This can be called on startup by holding the SET button
 * during power-up.
 *
 */
void test_screen_and_display() {
    swuart_writeln(PS("test begin"));

    /** Test the OLED screen by drawing text on it */
    ssd1306_clearScreen();
    ssd1306_printFixedN(0, 1, "OLED screen testing.", STYLE_NORMAL, 1);

    /** Test the display by displaying all 8's */
    // DDRA for PA2 is already set as output since if are in UART TX mode.
    for (uint16_t i = 30000; i; i++) {
        DisplayGlyphs gl{DIGIT_GLYPHS[8], (uint8_t)(DIGIT_GLYPHS[8]|1), DIGIT_GLYPHS[8]};
        display_glyphs_oe(gl);
    }
    ssd1306_clearScreen();

    PORTA |= (1 << PA2);  // Set UART pin HIGH after being used for OE.

    swuart_writeln(PS("done"));
}



/**
 * Print a Date object to the software UART.
 */
void print_date(const Date& date) {
    char date_str[11];
    generate_date_string(date_str, date);
    swuart_write(date_str);
}

/**
 * Print a Time object to the software UART.
 */
void print_time(const Time& time) {
    char time_str[9];
    generate_time_string(time_str, time);
    swuart_write(time_str);
}


// 'r'
void cmd_watchdog_reset() {
    swuart_writeln(PS("wd reset"));
    wdt_enable(WDTO_15MS);
    while (true) {}
}

// 'h'
void cmd_say_hello() {
    swuart_writeln(PS("Hi"));
}

// 'n'
void cmd_print_time_now() {
    swuart_writeln(PS("rtc now"));

    char str[21];
    generate_date_string(str, running_date);
    str[10] = ' ';   // seperating space
    generate_time_string(str + 11, running_time);
    swuart_write(str);
    swuart_write('\r'); swuart_write('\n');

#if 0
    // The generate_date_string will generate buggy
    // text if the date field numbers are out of range.
    // This implementation is extra safe:

    /* Malfunction-safe date print: */
    swuart_writeln(PS("\r\ndate: mm dd yy"));
    // mm dd yy
    char buffer[10];
    buffer[2] = ' ';
    buffer[5] = ' ';
    buffer[8] = '\r';
    buffer[9] = '\n';

    two_digit_itoa(buffer, running_date.month());
    two_digit_itoa(buffer+3, running_date.day());
    two_digit_itoa(buffer+6, running_date.year());
    swuart_write(buffer);
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
        case 'n':
          cmd_print_time_now();
    }
    swuart_mode_rx();
}
