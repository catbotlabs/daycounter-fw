/**
 * @file src/main.cpp
 * @brief Main routine for the CatBotLabs K001 Day Counter
 *
 * Part of the CatBotLabs K001 Day Counter firmware.
 *
 * Copyright (c) 2025, CatBotLabs
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the “Software”), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <avr/wdt.h>
#include <TinyI2CMaster.h>
#include "globals.h"
#include "datetime.h"
#include "rtc.h"
#include "settings.h"
#include "display.h"
#include "glyphdefs.h"
#include "buttons.h"
#include "ui.h"
#include "uart.h"
#include "commands.h"

/** globals.h definitions */
Date running_date;
Time running_time;
Settings running_settings;
bool oe_mode = OEMODE_OE;
volatile uint8_t busy_flags = 0;

/** Three-byte buffer for the digital display */
static DisplayGlyphs glyphs;

/** Number of repeat presses since button held */
static uint8_t num_repeats;

/**
 * Print an error message and display the global `glyphs`
 * on the digital display in an infinite loop.
 */
void fatal_infinite_display_loop(const ProgmemString* msg) {
    swuart_writeln(PS("fatal:"));
    swuart_writeln(msg);
    while (true) {
        display_glyphs_oe(glyphs);
    }
}

/**
 * Hardware interrupt 0 (PB2) triggered by rising edge of the
 * pulse-per-second signal. This is the MCU's primary method
 * of keeping time.
 * PB2 is tied to the RTC's multi-function (MFP) pin.
 */
ISR(INT0_vect) {
    busy_flags |= BF_PPS;
}

/**
 * Called when the code times out (locks up) on the
 * initial test I2C transaction.
 * 
 * Since we are relying on the pullup resistors in the SSD1306 module,
 * this could be caused by poor connection with the OLED module.
 */
ISR(WATCHDOG_vect) {
    wdt_disable();
    glyphs[2] = 0x60;                // 'I'
    glyphs[1] = DIGIT_GLYPHS[2]|1;   // '2'
    glyphs[0] = 0x72;                // 'C'
    fatal_infinite_display_loop(PS("I2C bus failure! Check OLED wiring."));
}

/** 
 * Initial startup routine
 */
void setup() {
    /** Disable watchdog in case of watchdog reset. */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /**
     * IF PA2 is HIGH:
     *   UART is plugged in, set oe_mode to 0.
     * IF PA2 is LOW:
     *   Standard configuration. Use OE to eliminate ghosting, set oe_mode to 1.
     */
#ifdef OEMODE_CHECK
    oe_mode = !((PINA & (1 << PA2)) >> PA2);
#endif
    if (oe_mode == OEMODE_UART) {
        setup_swuart();
        swuart_mode_tx();
        swuart_writeln(PS("\r\n== CatBotLabs DayCounter K001 ==\r\n"
                       "compiled " __DATE__ ", " __TIME__));
    } else if (oe_mode == OEMODE_OE) {
        DDRA |= (1 << PA2);   // OE pin: output
        PORTA |= (1 << PA2);  // OE pin: high (disable display)
    }

    /** Initialize pins that drive the digital display. */
    setup_display();

    /** Initialize I2C bus */
    TinyI2C.init();


    sei();  // temporarily enable interrupts for this
    wdt_enable(WDTO_15MS);  // start watchdog ~15ms timer
    WDTCSR |= (1 << WDIE);  // enable Watchdog ISR to escape lock-up

    /** If I2C wiring is bad, this will lock up and trigger the watchdog. */
    /** See ISR(WATCHDOG_vect) */
    bool oled_fail = !TinyI2C.start(0x3C, 0);  // 0x3C = SSD1306 address
    TinyI2C.write(0xAE);  // DISPLAY OFF (typical startup command)
    TinyI2C.stop();

    cli();
    wdt_disable();

    if (oled_fail) {
        glyphs[2] = DIGIT_GLYPHS[5];  // 'S'
        glyphs[1] = 0x73;             // 'C'
        glyphs[0] = 0xa0;             // 'r'
        fatal_infinite_display_loop(
            PS("OLED comm fail"));
    }

    /** Enable button interrupts after I2C test */
    setup_buttons();

    /** Setup and configure OLED screen. */
    ssd1306_128x32_i2c_init();
    ssd1306_clearScreen();
    ssd1306_setFixedFont(ssd1306xled_font6x8);

    /** Check if the SET button is pressed on startup. */
    /** If it is, do a quick demo of the OLED and number display. */
    if (!(PINA & (1 << PA1))) {
        test_screen_and_display();
    }

    /** Setup PPS interrupts */
    PORTB |= (1 << PB2);  // enable pullup resistor (MFP pin is open-drain)
    MCUCR |= (1 << ISC01) | (1 << ISC00);  // INT0 interrupts on rising edge
    GIMSK |= (1 << INT0);   // enable INT0 interrupts

    /** Setup RTC */
    switch (setup_rtc()) {
      case RTCInitResult::Fail:
        /** Failed to communicate with RTC */
        ssd1306_clearScreen();
        ssd1306_printFixedN(0, 20, "check RTC", STYLE_NORMAL, 1);
        glyphs[2] = 0xa0;  // 'r'
        glyphs[1] = 0xf1;  // 't'
        glyphs[0] = 0xb0;  // 'c'
        fatal_infinite_display_loop(PS("RTC comm fail"));
        break;
      case RTCInitResult::Unset:
        swuart_writeln(PS("RTC unset"));  // RTC registers weren't set properly
        /** Time is unset! */
        break;
      case RTCInitResult::OK:
        break;  // RTC okay. No action necessary.
    }

    if (read_settings_checked(&running_settings)) {
        swuart_writeln(PS("settings reset"));
    }

    rtc_read_datetime(&running_date, &running_time);


    /**
     * The RTC initial value date (MM DD YY) is
     * 01 01 01 ONLY IF the RTC powers on from the primary power.
     *
     * But it starts at 00 00 00 if the RTC battery is inserted
     * while the circuit is off.
     *
     * Zero is an illegal value for Month/Day for both the RTC
     * and this codebase. So we fix it here.
     */
    if (running_date.day() == 0 || running_date.month() == 0) {
        if (running_date.day() == 0) {
            running_date.day() = 1;
        }
        if (running_date.month() == 0) {
            running_date.month() = 1;
        }
        rtc_write_datetime(running_date, running_time);
    }


    busy_flags |= BF_RECOUNT;  // We'll need to recount/set glyphs.

    swuart_writeln(PS("ready."));
    /** If oe_mode == OEMODE_UART, then ready RX mode on UART. */
    swuart_mode_rx();

    sei();  // Enable interrupts!
}

/**
 * Post-startup loop routine
 */
inline void loop() {
    /** Focus on drawing to the matrix-encoded digital display */
    do {
        display_glyphs_oe(glyphs);
    } while (busy_flags == 0x00);

    uint8_t flags = busy_flags;  // copy busy flags
    busy_flags = 0;              // clear all busy flags

    // Check & process each busy flag:
    if (flags & BF_PPS) {
        /** Pulse-per-second event received. */
        uint8_t result = tick_datetime_one_second(&running_date, &running_time);

        /** Re-count at the top of the hour */
        if (result >= 2) {
            busy_flags |= BF_RECOUNT;
        }

        ui_pps();
    }
    if (flags & BF_UARTRX) {
        /** Character received from UART link. */
        handle_uart_char(swuart_rx_char);
        swuart_rx_char = '\0';  // consume character so we can take more.
    }
    if (flags & BF_BTN_DEBOUNCE) {
        /** Debounce timer elapsed */

        // Update button repeats
        if (post_debounce_update_repeats()) {
            // Pre-repeat just began, so reset our repeat counter.
            num_repeats = 0;
        }

        /** Get which buttons are pressed right now */
        uint8_t pressed_buttons = get_pressed_button_flags();
        /** Previous value of pressed_buttons */
        static uint8_t last_pressed_buttons = 0;

        /** These are the buttons that have been pressed since last debounce */
        uint8_t ev_pressed = pressed_buttons & ~last_pressed_buttons;

        /** Process the press events in the UI code */
        ui_btn_input(btn_helper(ev_pressed), 0);

        last_pressed_buttons = pressed_buttons;
    }
    if (flags & BF_BTN_REPEAT) {
        /** Button repeat timer triggered */
        num_repeats++;
        uint8_t repeated_button = get_repeated_button_flag();

        /** Process the repeat event in the UI code */
        ui_btn_input(btn_helper(1 << repeated_button), num_repeats);
    }
    if (flags & BF_RECOUNT) {
        uint16_t count;
        TimeCountResult result = count_time_units_until(
            running_settings.target_date,
            running_date, running_time,
            running_settings.count_unit, running_settings.count_direction,
            &count);
        switch (result) {
            case TimeCountResult::OK: {
                set_glyphs_0_to_999(count, glyphs);
                break;
            }
            case TimeCountResult::Negative: {
                glyphs[0] = glyphs[2] = 0x00;
                glyphs[1] = GLYPH_DASH | 1;
                break;
            }
            case TimeCountResult::Overflow: {
                glyphs[0] = glyphs[1] = glyphs[2] = GLYPH_DASH;
                glyphs[1] |= 1;
                break;
            }
        }
    }
}

int main() {
    setup();

    for (;;) {
        loop();
    }
    return 0;
}
