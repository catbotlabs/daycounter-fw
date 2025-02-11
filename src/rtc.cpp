/**
 * @file src/rtc.cpp
 * @brief RTC hardware/I2C interfacing
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "rtc.h"
#include <Wire.h>
#include <uart.h>

const uint8_t RTC_ADDR = 0x6f;

/**
 * RTC configurations:
 *  - MFP: Square Wave Clock Output:
 *    SQWEN set 1.  (RTCC bit 6)
 */

/// REGISTERS:
const uint8_t CONTROL = 0x07;


/// BITS:
// CONTROL:
const uint8_t SQWEN = 6;


// Read one byte from RTC at addr to data.
// Returns 0 on success, 1 on failure.
bool read_byte(uint8_t addr, uint8_t* data) {
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(addr);
    if (Wire.endTransmission()) {
        return 1;
    }
    Wire.requestFrom(RTC_ADDR, 1);
    *data = Wire.read();
    return 0;
}


void setup_rtc() {
    uint8_t control_byte = 177;

    swuart_mode_tx();
    if (read_byte(CONTROL, &control_byte)) {
        swuart_write("control byte fail\r\n");
    } else {
        swuart_write("success\r\n");
    }

    swuart_print_int(control_byte);
    swuart_mode_rx();
}



