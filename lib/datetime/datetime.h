/**
 * @file lib/datetime/datetime.h
 * @brief Portable code for packing, unpacking, and manipulating
 * date and time values.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef LIB_DATETIME_DATETIME_H_
#define LIB_DATETIME_DATETIME_H_

#include <stdint.h>

#undef SS  // Undefine clashing Arduino macro

/**
 * Calendar date: Stores day-of-month, month, and year.
 */
struct Date {
    uint8_t dd;
    uint8_t mm;
    uint8_t yy;   // two-digit 00 to 99
};

/**
 * Time-of-day: Stores seconds, minutes, and hours (24-hour time).
 */
struct Time {
    uint8_t SS;
    uint8_t MM;
    uint8_t HH;   // 24-hour
};

/**
 * A Date object and a Time object IN THE SAME OBJECT.
 */
struct DateTime {
    Date date;
    Time time;
};

/**
 * A buffer for the six of the MCP7940N time-keeping registers we're
 * using. (There is a seventh, RTCWKDAY, but we're not using it.)
 * 
 * This abstractifies the DateTime code from the hardware enough
 * so the datetime_from_registers function can be tested.
 */
struct DateTimeRegisters {
    uint8_t rtcsec;
    uint8_t rtcmin;
    uint8_t rtchour;
    uint8_t rtcdate;
    uint8_t rtcmth;
    uint8_t rtcyear;
};

/**
 * Sets all members of a DateTime object using
 * the buffered raw register data from the MCP7940N RTC.
 */
void datetime_from_registers(DateTime* dt, const DateTimeRegisters& rg);

#endif  // LIB_DATETIME_DATETIME_H_
