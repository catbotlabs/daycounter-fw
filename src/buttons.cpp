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
#include <util/atomic.h>
#include "globals.h"  /** for busy_flags */

// Timer duration constants:
/** Time from TCNT0=0 to TCNT0=141 is just over 18 ms */
const uint8_t DEBOUNCE_TIMER_MATCH = 141;

/** Time from TCNT1=0 to TCNT1=5078 is about 650 ms */
const uint16_t PRE_REPEAT_TIMER_MATCH = 5078;

/** Time from TCNT1=0 to TCNT1=1000 is 128 ms */
const uint16_t REPEAT_BUTTON_TIMER_MATCH = 1000;


// Timer clock configuration constants:
/** Timer0: Stopped */
const uint8_t TCCR0B_STOPPED = 0;

/** Timer0: Clock select: (8MHz/1024) */
const uint8_t TCCR0B_RUNNING = (1 << CS02) | (1 << CS00);

/** Timer1: Stopped */
const uint8_t TCCR1B_STOPPED = 0;

/** Timer1: Clock select: (8MHz/1024), CTC disabled. */
const uint8_t TCCR1B_RUNNING_NO_CTC = (1 << CS12) | (1 << CS10);

/**
 * @brief Threshold for determining which button is pressed by
 * the button signal's ADC reading. If the reading is above this
 * value, then the "high" button is pressed.
 */
const uint16_t BUTTON_ADC_THRESHOLD = 950;


/**
 * The BTPIN value for the pin that's actively triggering
 * the button repeat events.
 */
static uint8_t repeat_pin;


/**
    Explanation of how the Timers are being used:

Timer0 (8-bit):   Freq=(8MHz / 1024) = 128us per tick.
Purpose: Debouncing (A).
--------------------------------------------------------------

                 Start
                   |
TCNT0 count:  0x00 |#################### A #########| 0xFF
                                         |
                               OCR0A: Debounce delay
                                      (~18ms)

 *
 * Whenever the button pin change PCINT0 interrupt is called,
 * Timer0's clock is started. After ~18ms, OCR0A is reached, which
 * triggers an interrupt signifying that debouncing is complete and
 * the button pins can be safely read.
 * 
 * Note that the second Output Compare, OCR0B, is unused.
 *

Timer1 (16-bit):  Freq=(8MHz / 1024) = 128us per tick.
Purpose: Button repeat events (A), initial pre-repeat delay (B)
--------------------------------------------------------------


                    | <-- <-- <-|
TCNT1 count: 0x0000 |########## A #################### B ##########| 0xFFFF
                       ->   ->  |  ->    ->   ->   ->  |
                          OCR1A: Repeat event        OCR1B: Pre-repeat timer
                             (~128ms)                     (~650ms)
                               [CTC]                ISR clears TCNT1 on match.

*
* Button repeats make it so the user can "hold down" the button to
* keep scrolling/incrementing/decrementing without having to repeatedly
* mash the button.
*
* Before the "repeat press" events begin, there is first an initial wait
* through which a single button pin must be held. The wait must be long
* enough so that repeat presses aren't triggered on accident (~650ms).
* This initial wait is called the "pre-repeat" timer.

* When a debounced button is pressed, the pre-repeat timer begins.
* In this configuration, the OCR1A interrupt and CTC are both disabled.
* When the count reaches OCR1B, its interrupt then resets the count,
* enables the OCR1A interrupt, and enables CTC (effectively disabling OCR1B).
*
* Now the timer is operating in button repeat mode, where the timer automatically
* clears when it his OCR1A and triggers an interrupt signifying a button repeat
* press event.
*
* Whenever any debounced buttons are pressed OR released,
* Timer1 is reset and stopped, which stops the pre-repeat timer and
* the cyclic button repeat timer. Precisely, the steps are to:
*   - Stop timer clock
*   - Disable CTC
*   - Reset timer count to 0
*   - Disable OCR1A interrupt
*/

/** 
 * Configures button pins, enables PCINTs, and configures timers
 * for the buttons subsystem.
 * 
 * @note: This should be called with interrupts disabled.
 */
void setup_buttons() {
    /** Set the pullup resistor on BTSET */
    PORTA |= (1 << PA1);

    /** 
     * Enable ADC and set prescaler to F_CPU/64.
     * Since F_CPU=8MHz, this gives us and ADC frequency of 125kHz.
     */
    ADCSRA |= (1 << ADEN) | (0b110 << ADPS0);

    /** Configure Timer0 for debouncing */
    OCR0A = DEBOUNCE_TIMER_MATCH;  // Set the "duration" of the timer
    TIMSK0 |= (1 << OCIE0A);       // Enable interrupt on OCR0A

    /** Configure Timer1 for repeats, pre-repeat */
    OCR1A = REPEAT_BUTTON_TIMER_MATCH;  // Set button repeat timer duration
    OCR1B = PRE_REPEAT_TIMER_MATCH;     // Set pre-repeat timer duration
    TIMSK1 |= (1 << OCIE1B);  // OCR1B (pre-repeat) interrupt stays enabled.

    /** Enable PCINT0 events for the three button pins */
    PCMSK0 = BTN_PINS_MASK;

    /** Enable the PCINT0 interrupt */
    GIMSK |= (1 << PCIE0);
}


/**
 * Any button pin changed, rising or falling.
 */
ISR(PCINT0_vect) {
    /** Read the current button pin states */
    uint8_t btn_pin_states = PINA & BTN_PINS_MASK;

    /** Previous value of btn_pin_states */
    static uint8_t last_btn_pin_states = DEFAULT_BTN_PIN_STATES;

    /** These are the button pins that changed states */
    uint8_t changed_buttons = btn_pin_states ^ last_btn_pin_states;

    last_btn_pin_states = btn_pin_states;  // save the current state

    /**
     * Disable PCINTs on the button pins that have changed state,
     * to stop listening to the dirty signal.
     * 
     * The PCINTS are re-enabled when the debounce timer elapses.
     */
    PCMSK0 &= ~changed_buttons;

    /** Start the debounce timer: */
    TCNT0 = 0;                // Reset Timer0 count
    TCCR0B = TCCR0B_RUNNING;  // Start Timer0 clock
}

/**
 * Debounce timer fires 18ms after the last button pin change event.
 */
ISR(TIM0_COMPA_vect) {
    TCCR0B = TCCR0B_STOPPED;  // Stop Timer0 clock
    PCMSK0 = BTN_PINS_MASK;   // Re-enable all button PCINTs

    /** Tell the main loop it can now read the buttons. */
    busy_flags |= BF_BTN_DEBOUNCE;
}

/**
 * Begin the "pre-repeat" timer (OCR1B) from the stopped state.
 * Interrupts should be disabled when you call this.
 * 
 * @note Important: If the repeat timer (ORC1A w/CTC) is running,
 * you must first call stop_repeat_timer.
 */
void start_pre_repeat_timer() {
    TCNT1 = 0;                       // Set Timer1 count to 0
    TCCR1B = TCCR1B_RUNNING_NO_CTC;  // Start Timer1 clock
}

/**
 * Stop the button repeat timer (OCR1A w/CTC). Also stops the
 * pre-repeat timer (OCR1B) if that's running.
 * Interrupts should be disabled when you call this.
 * 
 * This function resets Timer1 to its initial (post setup_buttons) state.
 * 
 * It disables Clear Timer on Compare (CTC) and the OCR1A interrupt on Timer1.
 * This is critical because the OCR1B (pre-repeat) timer is at a
 * later TCNT1 position than OCR1A (repeat timer),
 * so OCR1A fires before OCR1B (pre-repeat) is reached
 * unless CTC/OCR1A is disabled.
 */
void stop_repeat_timer() {
    TCCR1B = TCCR0B_STOPPED;   // Stop clock and disable CTC
    TCNT1 = 0;                 // Reset Timer1 count to 0
    TIMSK1 &= ~(1 << OCIE1A);  // Disable OCR1A interrupt
}

/**
 * Button repeat timer (OCR1A) repeatedly fires every 128ms after
 * the "pre-repeat" (OCR1B) ~650ms delay timer.
 * 
 * Since Clear Timer on Compare (CTC) is set on Timer1, the timer
 * count is hw-reset right before this interrupt fires. Thus this
 * interrupt automatically repeats itself.
 */
ISR(TIM1_COMPA_vect) {
    /** 
     * Tell main loop there's a repeat press event.
     * 
     * The pin that is causing the repeat event is stored in `repeat_pin`.
     * 
     * The main loop then uses `adc_button_differentiate` to tell
     * which of the two buttons is actively held.
     */
    busy_flags |= BF_BTN_REPEAT;
}

/**
 * Pre-repeat delay (OCR1B) fires when a button has been held for ~650ms.
 * This signals that the button repeat events can begin every ~128ms
 * on timer OCR1A.
 */
ISR(TIM1_COMPB_vect) {
    /** Reset timer count to 0 (since there's no CTC option for OCR1B) */
    TCNT1 = 0;

    /**
     * Enable Clear Timer on Compare (CTC) with OCR1A.
     * This effectively disables the OCR1B (pre-repeat) timer
     * until CTC is disabled.
     */
    TCCR1B |= (1 << WGM12);   // Enable CTC so the timer wraps at OCR1A

    /** Enable button repeat timer (OCR1A) */
    TIFR1  |= (1 << OCF1A);   // Clear "OCR1A interrupt" flag
    TIMSK1 |= (1 << OCIE1A);  // Enable "OCR1A interrupt"
}

/**
 * Determines if repeat events need to be stopped or started
 * configures the repeat timers and `repeat_pin` accordingly.
 * 
 * Technically this could simply be part of ISR(TIM0_COMPA_vect)
 * (the debounce timer interrupt), but methinks its slightly too
 * heavy. Thus it gets run in the main loop.
 */
void post_debounce_update_repeats() {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        /** Button pins where the buttons are actively pressed */
        uint8_t active_btn_pins = (PINA & BTN_PINS_MASK) ^ (1 << BTPIN_BTSET);
        /** Previous value of active_btn_pins */
        static uint8_t last_active_btn_pins = 0;

        /** Button pins on buttons that were pressed */
        uint8_t ev_pressed_btn_pins = active_btn_pins & ~last_active_btn_pins;

        if (active_btn_pins ^ last_active_btn_pins) {
            // Disable the repeat timer, whether in pre-repeat or repeating.
            stop_repeat_timer();
        }
        if (ev_pressed_btn_pins & ADC_BTN_PINS_MASK) {
            if (ev_pressed_btn_pins & (1 << BTPIN_BTHZ)) {
                repeat_pin = BTPIN_BTHZ;
            } else {
                repeat_pin = BTPIN_BTVT;
            }
            start_pre_repeat_timer();
        }

        last_active_btn_pins = active_btn_pins;
    };  // ATOMIC_BLOCK
}

/**
 * Use the ADC to determine if the "high" button is pressed or not,
 * i.e. if channel == BTPIN_BTHZ, 0 means BTN_LEFT and 1 means BTN_RIGHT,
 *   or if channel == BTPIN_BTVT, O means BTN_DOWN and 1 means BTN_UP.
 * @note: this function assumes interrupts are disabled.
 * 
 * @param channel: ADC input channel, must be between 0 and 7. (not checked)
 */
bool adc_button_differentiate(uint8_t channel) {
    ADMUX = channel;        // Select ADC channel. (REFS is default: VCC)
    ADCSRA |= (1 << ADSC);  // Start ADC conversion
    while (ADCSRA & (1 << ADSC)) {}   // Wait for the conversion to complete
    if (ADC > BUTTON_ADC_THRESHOLD) {
        return true;  // The high, dominant button is pressed (Right or Up)
    }
    return false;     // The lower, weaker button is pressed (Left or Down)
}

/**
 * Reads the button pins to determine which buttons are pressed.
 * 
 * Returns a byte containing the BTN bit flags.
 * If the bit is 1, the button is pressed.
 * If the bit is 0. the button is not pressed.
 * 
 * @note: If both Right and Left are pressed, only Right will register.
 *       If both Up and Down are pressed, only Up will register.
 * 
 */
uint8_t get_pressed_button_flags() {
    /** Which buttons are pressed according to BTN_ flags */
    uint8_t pressed_buttons = 0;

    uint8_t pina = PINA;  /** Read the pin states */

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        /** Account for BTN_LEFT and BTN_RIGHT */
        if (pina & (1 << BTPIN_BTHZ)) {
            /** Left OR Right is pressed, use the ADC to determine which. */
            /** Note that if both are pressed, Right will dominate. */
            bool is_right = adc_button_differentiate(BTPIN_BTHZ);
            pressed_buttons |= (1 << (BTN_LEFT + is_right));
        }

        /** Account for BTN_DOWN and BTN_UP */
        if (pina & (1 << BTPIN_BTVT)) {
            /** Down OR Up is pressed, use the ADC to determine which. */
            /** Note that if both are pressed, Up will dominate. */
            bool is_up = adc_button_differentiate(BTPIN_BTVT);
            pressed_buttons |= (1 << (BTN_DOWN + is_up));
        }
    }  // ATOMIC_BLOCK

    /** Account for BTN_SET. BTSET is active low. */
    if (!(pina & (1 << BTPIN_BTSET))) {
        pressed_buttons |= (1 << BTN_SET);
    }

    return pressed_buttons;
}

/**
 * Use the stored `repeat_pin` and the ADC to determine
 * which button is actively causing repeat events.
 */
uint8_t get_repeated_button_flag() {
    uint8_t repeat_button;

    /** Start by setting repeat_button to the "low" button on its pin. */
    switch (repeat_pin) {
        case BTPIN_BTHZ:
            repeat_button = BTN_LEFT;
            break;
        case BTPIN_BTVT:
            repeat_button = BTN_DOWN;
            break;
    }
    /** Use the ADC to determine it's the "high" button being pressed. */
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        repeat_button += adc_button_differentiate(repeat_pin);
    }
    return repeat_button;
}
