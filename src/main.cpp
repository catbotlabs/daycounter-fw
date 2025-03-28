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
#include <USIWire.h>
#include <avr/wdt.h>
#include "globals.h"
#include "rtc.h"
#include "datetime.h"
#include "display.h"
#include "glyphdefs.h"
#include "buttons.h"
#include "ui.h"
#include "uart.h"
#include "commands.h"
#include "settings.h"

/** globals.h definitions */
Date running_date;
Time running_time;
Settings running_settings;
bool oe_mode = false;
volatile uint8_t busy_flags = 0;

/** Three-byte buffer for the digital display */
static DisplayGlyphs glyphs;

/**
 * Print an error message and display whatever is in @ref glyphs
 * on the digital display in an infinite loop.
 */
void fatal_infinite_display_loop(const ProgmemString* msg) {
    swuart_write(PS("fatal: "));
    swuart_write(msg);
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
 * Handles I2C bus failure, which would normally cause a lock-up.
 * 
 * Since we are relying on the pullup resistors in the SSD1306 module,
 * this could be caused by an open connection with the OLED.
 */
ISR(WATCHDOG_vect) {
    wdt_disable();
    glyphs[2] = 0x30;                // 'I'
    glyphs[1] = DIGIT_GLYPHS[2]|1;   // '2'
    glyphs[0] = 0xb8;                // 'C'
    fatal_infinite_display_loop(PS("I2C bus failure! Check OLED wiring."));
}

void setup() {
    /** TODO: determine if/whether interrupts (I-bit)
     *  are set in this function. */

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
    if (oe_mode == OEMODE_UART) {
        setup_swuart();
        swuart_mode_tx();
        swuart_write(PS("\r\n== CatBotLabs DayCounter K001 ==\r\n"
                       "built " __DATE__ ", " __TIME__ "\r\n"));
    } else if (oe_mode == OEMODE_OE) {
        DDRA |= (1 << PA2);   // OE pin: output
        PORTA |= (1 << PA2);  // OE pin: high (disable display)
    }

    /** Initialize pins that drive the digital display. */
    setup_display();

    /** Initialize I2C bus */
    Wire.begin();

    /** I2C bus test */
    Wire.beginTransmission(0x3C);  // 0x3C = SSD1306 address
    Wire.write(0xAE);              // DISPLAY OFF (typical startup command)
    sei();  // temporarily enable interrupts for this
    wdt_enable(WDTO_15MS);  // start watchdog ~15ms timer
    WDTCSR |= (1 << WDIE);  // enable Watchdog ISR to escape lock-up

    /** If I2C wiring is bad, this will lock up and trigger the watchdog. */
    /** See ISR(WATCHDOG_vect) */
    bool oled_fail = Wire.endTransmission();
    cli();

    wdt_disable();
    if (oled_fail) {
        glyphs[2] = DIGIT_GLYPHS[5];  // 'S'
        glyphs[1] = 0xb9;             // 'c'
        glyphs[0] = 0x50;             // 'r'
        fatal_infinite_display_loop(
            PS("failed to communicate with OLED screen"));
    }

    /** Enable button interrupts after I2C test */
    setup_buttons();

    /** Setup and configure OLED screen. */
    ssd1306_128x32_i2c_init();
    ssd1306_clearScreen();
    ssd1306_setFixedFont(ssd1306xled_font6x8);

    /** Setup PPS interrupts */
    PORTB |= (1 << PB2);  // enable pullup resistor (MFP pin is open-drain)
    MCUCR |= (1 << ISC01) | (1 << ISC00);  // INT0 interrupts on rising edge
    GIMSK |= (1 << INT0);   // enable INT0 interrupts

    /** Setup RTC */
    switch (setup_rtc()) {
      case RTCInitResult::FAIL:
        /** Failed to communicate with RTC */
        ssd1306_clearScreen();
        ssd1306_printFixedN(0, 20, "check RTC", STYLE_NORMAL, 1);
        glyphs[2] = 0x50;  // 'r'
        glyphs[1] = 0x79;  // 't'
        glyphs[0] = 0x58;  // 'c'
        fatal_infinite_display_loop(PS("failed to communicate with RTC"));
        break;
      case RTCInitResult::UNSET:
        swuart_write(PS("info: RTC registers adjusted\r\n"));
        /** Time is unset! */
        break;
      case RTCInitResult::OK:
        swuart_write(PS("info: RTC okay\r\n"));
    }

    if (read_settings_checked(&running_settings)) {
        swuart_write(PS("info: Settings invalid, reset to defaults.\r\n"));
    } else {
        swuart_write(PS("info: Settings okay\r\n"));
    }

    rtc_read_datetime(&running_date, &running_time);
    set_glyphs(running_time.second(), glyphs);

    /** If oe_mode == OEMODE_UART, then ready RX mode on UART. */
    swuart_mode_rx();

    sei();  // Enable interrupts!
}

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
        tick_datetime_one_second(&running_date, &running_time);

        /** Show the seconds on the display */
        set_glyphs(running_time.second(), glyphs);

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
        post_debounce_update_repeats();

        /** Get which buttons are pressed right now */
        uint8_t pressed_buttons = get_pressed_button_flags();
        /** Previous value of pressed_buttons */
        static uint8_t last_pressed_buttons = 0;

        /** These are the buttons that have been pressed since last debounce */
        uint8_t ev_pressed = pressed_buttons & ~last_pressed_buttons;

        ui_btn_input(btn_helper(ev_pressed), false);

        last_pressed_buttons = pressed_buttons;
    }
    if (flags & BF_BTN_REPEAT) {
        /** Button repeat timer triggered */
        uint8_t repeated_button = get_repeated_button_flag();
        ui_btn_input(btn_helper(1 << repeated_button), true);
    }
}

int main() {
    // TODO: we don't need to call them setup() and loop().
    setup();
    for (;;) {
        loop();
    }
    return 0;
}
