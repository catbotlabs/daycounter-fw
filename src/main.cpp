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
#include <Arduino.h>
#include "globals.h"
#include "uart.h"

volatile uint8_t busy_flags = 0;

void setup() {
    //GIMSK |= (1 << PCIE0);  // enable PCINT0 interrupts
    setup_swuart();
    swuart_mode_tx();
    swuart_write(F("\r\n== CatBotLabs DayCounter K001 ==\r\n"
                 "built " __DATE__ __TIME__ "\r\n"));
    swuart_mode_rx();
}

void loop() {
    do {
        // tight display loop
    } while (busy_flags == 0);

    if (busy_flags & BF_PPS) {
        // pulse-per-second
    } else if (busy_flags & BF_UARTRX) {
        // uart byte received.

        // We'll just echo back the character we received.
        swuart_mode_tx();
        swuart_write("\r\n");
        char msg[] = "[x]\r\n";
        msg[1] = swuart_rx_char;
        swuart_write(msg);
        swuart_mode_rx();
        busy_flags &= ~BF_UARTRX;

        swuart_rx_char = '\0';  // consume character so we can take more.
    }
}
