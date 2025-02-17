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
    return 10 * ((bcd >> 4) & 7) + (bcd & 15);
}

/**
 * Sets all members of a DateTime object using
 * the buffered raw register data from the MCP7940N RTC.
 */
void datetime_from_registers(Date* date, Time* time,
        const DateTimeRegisters& rg) {
    time->second() = int_from_bcd(rg.rtcsec & 127);  // mask out Bit 7 (ST)
    time->minute() = int_from_bcd(rg.rtcmin);

    // mask out Bit 6 (12/24) and 5 (AM/PM)
    time->hour() = int_from_bcd(rg.rtchour & 31);

    date->day() = int_from_bcd(rg.rtcdate);
    date->month() = int_from_bcd(rg.rtcmth & 63);  // mask out Bit 5 (LPYR)
    date->year() = int_from_bcd(rg.rtcyear);
}

/**
 * Increment the time by one second and perform all necessary carries.
 */
void tick_datetime_one_second(Date* date, Time* time) {
    // Increment time by one, starting with seconds
    for (uint8_t i = 0; i < 3; i++) {
        const AscendingDateField field = static_cast<AscendingDateField>(i);
        uint8_t& field_value = date->field(field);
        field_value++;
        if (field_value <= date->field_max_value(field)) {
            // no carry, operation complete
        }
        // reset this field to its minimum value
        field_value = date->field_min_value(field);
        // then go to the next field.
    }

    // Increment date by one, starting by seconds
    for (uint8_t i = 0; i < 3; i++) {
        const AscendingTimeField field = static_cast<AscendingTimeField>(i);
        uint8_t& field_value = time->field(field);
        field_value++;
        if (field_value <= time->field_max_value(field)) {
            // no carry, operation complete
            break;
        }
        field_value = time->fields_min_value();
    }
}

