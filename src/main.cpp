/**
 * @file day-counter/attiny_first_test/src/main.cpp
 * @brief DayCounter ATTINY84 initial testing
 *
 * Copyright (c) 2025 Winston Malinowski
 */

#include <Arduino.h>
#include <util/atomic.h>

#include <SoftwareSerial.h>
SoftwareSerial soft(2, 3);

void setup() {
    soft.begin(19200);
    // Set PB0 as output
    DDRB |= (1<<PB0);

    //TCCR0B = (0b010 << CS00);
}

void loop() {
    // Toggle PB0 so it's 1Hz
//    PORTB ^= (1<<PB0);
    delay(500);
    PINB |= (1<<PB0);
    soft.println("hi");
}
