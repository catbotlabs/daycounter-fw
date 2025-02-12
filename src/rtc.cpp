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


/// == RTC Register Definitions ==
const uint8_t RTCSEC = 0x00;
const uint8_t RTCMIN = 0x01;
const uint8_t RTCHOUR = 0x02;
const uint8_t RTCWKDAY = 0x03;
const uint8_t RTCDATE = 0x04;
const uint8_t RTCMTH = 0x05;
const uint8_t RTCYEAR = 0x06;
const uint8_t CONTROL = 0x07;

// == RTC Bit Definitions ==
// 0x00 RTCSEC
const uint8_t ST = 7;

// 0x03 RTCWKDAY
const uint8_t VBATEN = 3;
const uint8_t OSCRUN = 5;

// 0x07 CONTROL:
const uint8_t SQWEN = 6;



/**
 * Read the RTC's register at \c addr to \c data;
 * Returns 0 on success, 1 on failure.
 */
bool rtc_read_byte(uint8_t addr, uint8_t* data) {
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(addr);
    if (Wire.endTransmission()) {
        return 1;
    }
    Wire.requestFrom(RTC_ADDR, (uint8_t)1);
    *data = Wire.read();
    return 0;
}

/**
 * Write the RTC's register at \c addr with \c data;
 * Returns 0 on success, 1 on failure.
 */
bool rtc_write_byte(uint8_t addr, uint8_t data) {
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(addr);
    Wire.write(data);
    return Wire.endTransmission();
}


/**
 * RTC configurations to configure on startup:
 *  - MFP: Square Wave Clock Output:
 *    SQWEN set 1.  (CONTROL bit 6)
 * 
 *  - ST: Set to enable oscillator  (RTCSEC bit 7)
 *        Disable and wait for OSCRUN == 0 when setting time
 * 
 *  - VBATEN: Set 1. (RTCWKDAY bit 1)
 */


/**
 * @brief Make sure the RTC's registers are set properly.
 * 
 * @details This function reads three registers and ensures that each
 * is configured as necessary. If a register does not meet the below conditions,
 * it is re-written. If any of the registers were written, this function returns 1.
 *    In RTCSEC, the ST bit must be set (to 1) to start the oscillator.
 *    In CONTROL, only the SQWEN bit must be set. This will enable a
 *      1Hz square wave output on the RTC's MFP pin (tied to PB2).
 *    In RTCWKDAY, the VBATEN bit must be set. This enables the backup battery.
 * 
 * @returns 0 if all registers are set as expected.
 *          1 if any registers were adjusted.
 *          2 if failed to communicate with RTC.
 */
uint8_t setup_rtc() {
    bool mod = false;

    // one-byte read buffer.
    uint8_t reg = 0;

    /** RTCSEC: Make sure ST is set. */
    if (rtc_read_byte(RTCSEC, &reg)) {
        return 2;
    }
    if (!(reg & (1 << ST))) {
        rtc_write_byte(RTCSEC, reg | (1 << ST));
        mod = true;
    }

    /** CONTROL: Make sure SQWEN is set. */
    // SQWEN is the only bit we want set in CONTROL
    const uint8_t wanted_control_reg = (1 << SQWEN);
    if (rtc_read_byte(CONTROL, &reg)) {
        return 2;
    }
    if (reg != wanted_control_reg) {
        rtc_write_byte(CONTROL, wanted_control_reg);
        mod = true;
    }

    /** RTCWKDAY: Make sure VBATEN is set. */
    if (rtc_read_byte(RTCWKDAY, &reg)) {
        return 2;
    }
    if (!(reg & (1 << VBATEN))) {
        rtc_write_byte(RTCWKDAY, reg | (1 << VBATEN));
    }

    /** Return whether any of the registers were modified. */
    return mod;
}


