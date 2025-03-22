/**
 * @file include/globals.h
 * @brief Declares shared global variables like the `busy_flags`.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */
#ifndef INCLUDE_GLOBALS_H_
#define INCLUDE_GLOBALS_H_

#include <stdint.h>

/**
 * A single volatile byte where specific bits can be
 * set to "1" by ISRs to exit the main display loop.
 * 
 * While busy_flags == 0, the display loop runs,
 * if busy_flags != 0, we exit the display loop to process
 * other tasks like the RTC or OLED UI.
 */
extern volatile uint8_t busy_flags;

/** Busy flag bit masks */
enum {
    BF_PPS          = 0x01,  /// Pulse-per-second event from RTC
    BF_UARTRX       = 0x02,  /// Byte received on software UART
    BF_BTN_DEBOUNCE = 0x04,  /// Button debounce timer ended
    BF_BTN_REPEAT   = 0x08,  /// Button repeat timer triggered
};

enum {
    OEMODE_UART,
    OEMODE_OE
};

// false: UART/OE pin used for UART
// true: UART/OE pin used for OE
extern bool oe_mode;

#endif  // INCLUDE_GLOBALS_H_
