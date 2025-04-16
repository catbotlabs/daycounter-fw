/**
 * @file include/buttons.h
 * @brief Button reading / debouncing
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef INCLUDE_BUTTONS_H_
#define INCLUDE_BUTTONS_H_

#include <avr/io.h>
#include <stdint.h>

/**
 * Vocabulary:
 * 
 *    - Button pin: The bit position that represents the _pin_ a button is on.
 *         For instance, PA1 for BTSET, or PA0 for UP/DOWN.
 * 
 *    - Button bit / "Button": A bit position that represents a button itself.
 *         For instance, BTN_SET for Set, or BTN_DOWN for Down.
*/

/**
 * Button pins:
 * 
 * - PA0 on PCINT0: BTVT (UP / DOWN)    - Active high, told apart by ADC.
 * - PA1 on PCINT1: BTSET               - Active low single button.
 * - PA3 on PCINT3: BTHZ (RIGHT / LEFT) - Active high, told apart by ADC.
 */

/** Button pin definitions */
static const uint8_t BTPIN_BTVT  = PA0;
static const uint8_t BTPIN_BTSET = PA1;
static const uint8_t BTPIN_BTHZ  = PA3;

// By default, BTVT and BTHZ are LOW and BTSET is HIGH.
static const uint8_t DEFAULT_BTN_PIN_STATES = (1 << BTPIN_BTSET);

/** Mask of all button pins (they're all on Port A) */
static const uint8_t BTN_PINS_MASK =
    (1 << BTPIN_BTHZ) | (1 << BTPIN_BTSET) | (1 << BTPIN_BTVT);

/** Mask of the "shared" button pins (that use the ADC to tell them apart). */
static const uint8_t ADC_BTN_PINS_MASK =
    (1 << BTPIN_BTVT) | (1 << BTPIN_BTHZ);

/** Physical button bit position definitions */
enum {
    BTN_SET,
    BTN_DOWN,
    BTN_UP,     // UP must be DOWN + 1
    BTN_LEFT,
    BTN_RIGHT,  // RIGHT must be LEFT + 1
};

/**
 * Helper class that gives easy access to button bits.
 */
class btn_helper {
    uint8_t data;

 public:
    /** Initialize btn_helper from a byte of the button bits. */
    explicit btn_helper(uint8_t bits) : data(bits) {}

    bool set()   const {return data & (1 << BTN_SET); }
    bool down()  const {return data & (1 << BTN_DOWN); }
    bool up()    const {return data & (1 << BTN_UP); }
    bool left()  const {return data & (1 << BTN_LEFT); }
    bool right() const {return data & (1 << BTN_RIGHT); }

    /** Get the underlying byte */
    uint8_t bits() const {return data; }
};

/** Set up pins, interrupts, and timers for the buttons subsystem. */
void setup_buttons();

/** Run after debounce in the main loop to start/stop button repeats */
void post_debounce_update_repeats();

/** Use after debounce to get which buttons are pressed right now. */
uint8_t get_pressed_button_flags();

/**
 * Use the stored pin and the ADC to determine which
 * is actively causing repeat events.
 */
uint8_t get_repeated_button_flag();

#endif  // INCLUDE_BUTTONS_H_
