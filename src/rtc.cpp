/**
 * @file src/rtc.cpp
 * @brief MCP7940N RTC I2C interfacing
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "rtc.h"
#include <util/delay.h>   // for _delay_us()
#include <TinyI2CMaster.h>
#include "datetime.h"

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
    TinyI2C.start(RTC_ADDR, 0);
    TinyI2C.write(addr);
    if (!TinyI2C.restart(RTC_ADDR, 1)) {
        return 1;
    }
    *data = TinyI2C.read();
    TinyI2C.stop();
    return 0;
}

/**
 * Write the RTC's register at \c addr with \c data;
 * Returns 0 on success, 1 on failure.
 */
bool rtc_write_byte(uint8_t addr, uint8_t data) {
    TinyI2C.start(RTC_ADDR, 0);
    TinyI2C.write(addr);
    if (!TinyI2C.write(data)) {
        return true;
    }
    TinyI2C.stop();
    return false;
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
RTCInitResult setup_rtc() {
    RTCInitResult result = RTCInitResult::OK;

    // one-byte read buffer.
    uint8_t reg = 0;

    /** RTCSEC: Make sure ST is set. */
    if (rtc_read_byte(RTCSEC, &reg)) {
        return RTCInitResult::Fail;
    }
    if (!(reg & (1 << ST))) {
        rtc_write_byte(RTCSEC, reg | (1 << ST));
        result = RTCInitResult::Unset;
    }

    /** CONTROL: Make sure SQWEN is set. */
    // SQWEN is the only bit we want set in CONTROL
    const uint8_t wanted_control_reg = (1 << SQWEN);
    if (rtc_read_byte(CONTROL, &reg)) {
        return RTCInitResult::Fail;
    }
    if (reg != wanted_control_reg) {
        rtc_write_byte(CONTROL, wanted_control_reg);
        result = RTCInitResult::Unset;
    }

    /** RTCWKDAY: Make sure VBATEN is set. */
    if (rtc_read_byte(RTCWKDAY, &reg)) {
        return RTCInitResult::Fail;
    }
    if (!(reg & (1 << VBATEN))) {
        rtc_write_byte(RTCWKDAY, reg | (1 << VBATEN));
        result = RTCInitResult::Unset;
    }

    /** Return whether any of the registers were modified. */
    return result;
}

/**
 * Read the seven timekeeping registers of the MCP7940N RTC
 * into the given DateTimeRegisters object.
 * 
 * @returns 0 on success, 1 on failure.
 */
bool rtc_read_timekeeping_registers(DateTimeRegisters* rg) {
    /** Request to read 7 bytes starting at 0x00. */
    TinyI2C.start(RTC_ADDR, 0);
    TinyI2C.write(0x00);  // start address to read from
    if (!TinyI2C.restart(RTC_ADDR, 7)) {  // request to read 7 bytes
        return 1;
    }

    // Put those bytes in the DateTimeRegisters object
    for (int i = 0; i < 7; i++) {
        *(reinterpret_cast<uint8_t*>(rg) + i) = TinyI2C.read();
    }
    TinyI2C.stop();
    return 0;
}

/**
 * Write the buffered registers in the given DateTimeRegisters
 * object to the RTC's memory.
 * 
 * @returns 0 on success, 1 on failure.
 */
bool rtc_write_timekeeping_registers(const DateTimeRegisters& rg) {
    if (!TinyI2C.start(RTC_ADDR, 0)) {
        return false;
    }
    TinyI2C.write(0x00);  // 0x00 is the starting address we're writing to.

    /** Send the register bytes to the RTC */
    for (int i = 0; i < 7; i ++) {
        if (!TinyI2C.write(reinterpret_cast<const uint8_t*>(&rg)[i])) {
            return false;
        }
    }
    TinyI2C.stop();
    return true;
}

/**
 * Read the date and time from the RTC and write their values
 * to the given Date and Time objects.
 */
void rtc_read_datetime(Date* date, Time* time) {
    DateTimeRegisters rg;
    rtc_read_timekeeping_registers(&rg);
    datetime_from_registers(date, time, rg);
}

/**
 * Set the RTC's current date and time to the given values.
 * 
 * This function will block for over a millisecond, as the
 * oscillator must be stopped before writing the new date and time values.
 */
void rtc_write_datetime(const Date& date, const Time& time) {
    /** Oscillator must be stopped when writing date/time to RTC. */

    // Buffer for our RTCWKDAY and RTCSEC operations.
    uint8_t reg;

    // Clear ST bit, don't care about contents of RTCSEC
    if (rtc_write_byte(RTCSEC, 0)) {
        return;
    }

    // Wait for OSCRUN to clear, meaning, wait for oscillator to stop.
    // (If communication were broken, the code would lock up here.)
    // MCP7940N datasheet states this should take a minimum of 1 ms.
    do {
        _delay_us(400);
        rtc_read_byte(RTCWKDAY, &reg);
    } while (reg & (1 << OSCRUN));

    // Read current timekeeping registers
    DateTimeRegisters registers;
    rtc_read_timekeeping_registers(&registers);

    // Write new time and date
    update_registers_from_datetime(&registers, date, time);
    rtc_write_timekeeping_registers(registers);

    // Set the ST bit to start the oscillator again.
    rtc_read_byte(RTCSEC, &reg);
    reg |= (1 << ST);
    rtc_write_byte(RTCSEC, reg);
}
