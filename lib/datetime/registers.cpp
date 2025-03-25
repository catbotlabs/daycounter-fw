/**
 * @file lib/datetime/registers.cpp
 * @brief Register abstraction-related functions for DateTime
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 *
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
    /* RTCSEC: Mask out Bit 7 (ST) */
    time->second() = int_from_bcd(rg.rtcsec & TKMSK_RTCSEC);
    time->minute() = int_from_bcd(rg.rtcmin);
    /* RTCHOUR: Mask out Bit 6 (12/24), but keep Bit 5 (AM/PM,HRTEN1). */
    time->hour() = int_from_bcd(rg.rtchour & TKMSK_RTCHOUR);
    date->day() = int_from_bcd(rg.rtcdate);
    /* RTCMTH: Mask out Bit 5 (LPYR) */
    date->month() = int_from_bcd(rg.rtcmth & TKMSK_RTCMTH);
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
