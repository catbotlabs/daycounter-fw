/**
 * @file lib/datetime/datetime.cpp
 * @brief Portable code for packing, unpacking, and manipulating
 * date and time values.
 * 
 * @details This DateTime code implements the following functions:
 *   - Reading DateTime object from raw RTC registers
 *   - Writing DateTime object to raw RTC registers
 *   - Incrementing a DateTime by 1 second, performing all carries. (for PPS)
 *   - Incrementing individual parameters of a DateTime or Date object
 *       by one or more, w/o carries but with proper clamping and/or wrapping.
 *   - Calculating the delta between the current time and a Date.
 * 
 * This code is all abstract, as in, it does not interact with the MCU or RTC's
 * hardware directly. Thus, all the above functions are portable enough to
 * be unit tested on a native platform.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "datetime.h"

/**
 * Interpret BCD packed this way as an integer from 0 to 99.
 *   MSB 0000   0000 LSB
 *      [tens] [ones]
 * 
 * Since some of the MCP7940N RTC's timekeeping registers store
 * configuration parameters in certain bits, some additional bit-masking
 * is necessary before passing the raw register data as the argument.
 */
uint8_t int_from_bcd(uint8_t bcd) {
    return 10 * (bcd >> 4) + (bcd & 15);
}

/**
 * Return two-digit packed BCD byte from an integer from 0 to 99.
 * 
 * Since some of the MCP7940N RTC's timekeeping registers store
 * configuration parameters in certain bits, some bit-masking may
 * be necessary before using the result to set the register data.
 * 
 * @param num the given number, which must be between 0 and 99.
 */
uint8_t bcd_from_int(uint8_t num) {
    return ((num / 10) << 4) | (num % 10);
}

/** 
 * Timekeeping register masks. The zeroes are on bits
 * that should not be used for timekeeping.
 */
#define TKMSK_RTCSEC  127  // RTCSEC: mask out Bit 7 (ST)
#define TKMSK_RTCHOUR 63   // RTCHOUR: mask out Bit 6 (12/24)
#define TKMSK_RTCMTH  31   // RTCMTH: mask out LPYR

/**
 * Sets all members of a DateTime object using
 * the buffered raw register data from the MCP7940N RTC.
 * 
 * @note It's assumed that we're using 24-hour time
 * (12/24 bit set to 0).
 */
void datetime_from_registers(Date* date, Time* time,
        const DateTimeRegisters& rg) {
    time->second() = int_from_bcd(rg.rtcsec & TKMSK_RTCSEC);  // mask out Bit 7 (ST)
    time->minute() = int_from_bcd(rg.rtcmin);

    // mask out Bit 6 (12/24). Keep bit 5 (AM/PM,HRTEN1) since we're doing 24-hour time.
    time->hour() = int_from_bcd(rg.rtchour & TKMSK_RTCHOUR);

    date->day() = int_from_bcd(rg.rtcdate);
    date->month() = int_from_bcd(rg.rtcmth & TKMSK_RTCMTH);  // mask out Bit 5 (LPYR)
    date->year() = int_from_bcd(rg.rtcyear);
}

/**
 * Sets the timekeeping bits of the MCP7940N RTC using
 * the values from a Date and a Time object.
 * 
 * We're not masking the RTCMIN, RTCDATE, and RTCYEAR registers
 * because, a) The unimplented bits won't be set to 1 unless the
 * Date/Time objects are out-of-bounds, which won't happen in normal
 * operation, and, b) I'm pretty sure if you do write a 1 to them,
 * nothing would happen anyway.
 */
void update_registers_from_datetime(DateTimeRegisters* rg, const Date& date,
        const Time& time) {
    rg->rtcsec &= ~TKMSK_RTCSEC;    // clear timekeeping bits
    rg->rtcsec |= bcd_from_int(time.second()) & TKMSK_RTCSEC;
    
    rg->rtcmin = bcd_from_int(time.minute());

    rg->rtchour &= ~TKMSK_RTCHOUR;  // clear timekeeping bits
    rg->rtchour |= bcd_from_int(time.hour()) & TKMSK_RTCHOUR;

    rg->rtcdate = bcd_from_int(date.day());

    rg->rtcmth &= ~TKMSK_RTCMTH;    // clear timekeeping bits
    rg->rtcmth |= bcd_from_int(date.month()) & TKMSK_RTCMTH;

    rg->rtcyear = bcd_from_int(date.year());
}

/**
 * Increment the time by one second and perform all necessary carries.
 * 
 * If any of the fields are out of bound,
 * then undefined behaviour will result!
 * 
 * @returns The greatest field that was incremented.
 * Possible return values:
 *   0:  +1 second (no carry)
 *   1:  +1 minute (seconds wrap)
 *   2:  +1 hour   (minutes wrap)
 *   3:  +1 day    (hours wrap)
 *   4:  +1 month  (days wrap)
 *   5:  +1 year   (months wrap)
 *   6:  years wraps from 99 to 0.
 * 
 * @note Fails to follow century leap year rules. All century years
 * are leap years according to this function.
 * 
 */
uint8_t tick_datetime_one_second(Date* date, Time* time) {
    // Increment time by one, starting with seconds
    for (uint8_t i = 0; i < 3; i++) {
        const AscendingTimeField field = static_cast<AscendingTimeField>(i);
        uint8_t& field_value = time->field(field);
        field_value++;
        if (field_value <= time->field_max_value(field)) {
            // no carry, operation complete
            return i;
        }
        // reset this field to its minimum value
        field_value = time->fields_min_value();
        // then go to the next field.
    }

    // Increment date by one, starting by seconds
    for (uint8_t i = 0; i < 3; i++) {
        const AscendingDateField field = static_cast<AscendingDateField>(i);
        uint8_t& field_value = date->field(field);
        field_value++;
        if (field_value <= date->field_max_value(field)) {
            // no carry, operation complete
            return i + 3;
        }
        field_value = date->field_min_value(field);
    }
    return 6;
}

