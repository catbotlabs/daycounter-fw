/**
 * @file src/buttons.cpp
 * @brief Button reading / debouncing
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "buttons.h"
#include "globals.h"  /** for busy_flags */
#include "uart.h"     /** TODO: remove after dev */

/**
 * Button pins:
 * 
 * - PA0 on PCINT0: BTVT (UP / DOWN)      [ ACTIVE HIGH ]
 * - PA1 on PCINT1: BTSET                 [ ACTIVE LOW  ]
 * - PA3 on PCINT3: BTHZ (RIGHT / LEFT)   [ ACTIVE HIGH ]
 */

using ButtonPinStates = uint8_t;

const uint8_t BUTTON_PIN_MASK = (1 << PA3) | (1 << PA1) | (1 << PA0);

// By default, BTVT and BTHZ are LOW and BTSET is HIGH.
const uint8_t DEFAULT_BUTTON_STATES = (1 << PA1);

uint8_t button_pin_state;

void setup_button_pins();
void setup_button_timers();

void setup_buttons() {
    // TODO: disable interrupts?

    /** Enable ADC */
    ADCSRA |= (1 << ADEN);

    /** Set the pullup resistor on BTSET */
    PORTA |= (1 << PA1); 

    /** Enable all PCINTs (start listening) */
    PCMSK0 |= (1 << PCINT0);
    PCMSK0 |= (1 << PCINT1);
    PCMSK0 |= (1 << PCINT3);

    // TODO: Enable PCINT interrupt

    // TODO: Enable timers
}

ISR(PCINT0_vect) {
    // button states mapped according to PORTA
    static uint8_t button_states = DEFAULT_BUTTON_STATES;
    uint8_t new_button_states = PORTA & BUTTON_PIN_MASK;

    // the buttons that changed states according to PORTA
    uint8_t changed_buttons = button_states ^ new_button_states;

    button_states = new_button_states;


    // KICK OFF DEBOUNCE TIMER
}

void DEBOUNCE_TIMER_ELAPSE() {
    // stop this timer
    // set this timer to 0

    // disable repeats timer interrupt

    // set busy flag debounce_timer_end
}

/**
 * Use the ADC to read the voltage on the given ADC input.
 * 
 * @param adc_mux: ADC input channel, MUST be between 0 and 7.
 */
uint16_t adc_read_blocking(uint8_t adc_mux) {
    ADMUX = adc_mux;         /** Leave REFS as the default: Vcc */
    ADCSRA |= (1 << ADSC);   /** ADC start conversion */
    while (ADCSRA & (1 << ADSC)) {};   /** Wait for conversion to finish */
    return ADC;  /** Return high and low bytes */
}

/**
 * Reads the button pins to determine which buttons are pressed.
 * 
 * Returns a byte containing the BTN bit flags.
 * If the bit is 1, the button is pressed.
 * If the bit is 0. the button is not pressed.
 * 
 * @note If both Right and Left are pressed, only Right will register.
 *       If both Up and Down are pressed, only Up will register.
 * 
 */
uint8_t read_button_states() {
    uint8_t state = 0;
    uint8_t pina = PINA;  /** Read the pin states */

    /** Account for BTN_RIGHT and BTN_LEFT */
    if (pina & (1 << BTPIN_BTHZ)) {
        /** Right OR Left is pressed, use the ADC to determine which. */
        /** Note if both are pressed, Right will dominate */
        uint16_t adc_result = adc_read_blocking(BTPIN_BTHZ);
            swuart_write("BTHZ ADC: ");
            swuart_print_int(adc_result);
            swuart_newline();
    }

    /** Account for BTN_UP and BTN_DOWN */
    if (pina & (1 << BTPIN_BTVT)) {
        /** Up OR Down is pressed, use the ADC to determine which. */
        /** Note if both are pressed, Up will dominate */
        uint16_t adc_result = adc_read_blocking(BTPIN_BTVT);
            swuart_write("BTVT ADC: ");
            swuart_print_int(adc_result);
            swuart_newline();
    }

    /** Account for BTN_SET. BTSET is active low. */
    if (!(pina & (1 << BTPIN_BTSET))) {
        state |= (1 << BTN_SET);
    }
    return state;
}
