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
    return 10 * ((bcd >> 4) & 7) + (bcd & 15);
}

/**
 * Sets all members of a DateTime object using
 * the buffered raw register data from the MCP7940N RTC.
 */
void datetime_from_registers(DateTime* dt, const DateTimeRegisters& rg) {
    dt->time.SS = int_from_bcd(rg.rtcsec & 127);  // mask out Bit 7 (ST)
    dt->time.MM = int_from_bcd(rg.rtcmin);
    dt->time.HH = int_from_bcd(rg.rtchour & 31);  // mask out Bits 6 (12/24) and 5 (AM/PM)

    dt->date.dd = int_from_bcd(rg.rtcdate);
    dt->date.mm = int_from_bcd(rg.rtcmth & 63);  // mask out Bit 5 (LPYR)
    dt->date.yy = int_from_bcd(rg.rtcyear);
}
