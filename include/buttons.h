/**
 * @file src/buttons.h
 * @brief Button reading / debouncing
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef DCFW_INCLUDE_BUTTONS_H_
#define DCFW_INCLUDE_BUTTONS_H_

#include <avr/io.h>

/** Button bit positions */
enum {
    BTN_SET,
    BTN_DOWN,
    BTN_UP,
    BTN_RIGHT,
    BTN_LEFT
};

static const uint8_t BTPIN_BTVT  = PA0;
static const uint8_t BTPIN_BTSET = PA1;
static const uint8_t BTPIN_BTHZ  = PA3;

/** Set up everything necessary for the buttons subsystem */
void setup_buttons();

/** 
 * Read all the button pins to determine which buttons are pressed.
 * Uses the ADC, so it's a little blocking.
 * 
 * @returns The button states as bits, according to the BTN bit positions.
 */
uint8_t read_button_states();

#endif  // DCFW_INCLUDE_BUTTONS_H_
