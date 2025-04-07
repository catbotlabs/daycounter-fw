/**
 * @file lib/datetime/str_fmt.cpp
 * @brief String formatting tools related to Date and Time
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
 * Writes a string of format "mmm DD 'YY" to the given buffer
 * using the given Date object.
 * 
 * @note Exactly 11 characters are written to the buffer.
 * A '\0' terminating character is written at the end.
 */
void generate_date_string(char* buffer, const Date& date) {
    const char* month_name_P = get_month_name_P(date.month());

    char* mmm = buffer;
    char* dd = buffer + 4;
    char* yy = buffer + 8;

    buffer[3] =  buffer[6] = ' ';  // Set space bars
    buffer[7] = '\'';                // Apostrophe before year
    memcpy_P(mmm, month_name_P, 3);   // Copy in month name
    two_digit_itoa(dd, date.day());    // Set day digits
    two_digit_itoa(yy, date.year());   // Set year digits
    buffer[10] = '\0';            // terminating character
}


/**
 * Writes a string of format "HH:MM:SS" to the given buffer
 * using the given Time object.
 * 
 * @note Exactly 9 characters are written to the buffer.
 * A '\0' terminating character is written at the end.
 */
void generate_time_string(char* buffer, const Time& time) {
    char* hh = buffer;
    char* mm = buffer + 3;
    char* ss = buffer + 6;

    buffer[2] = buffer[5] = ':';   // Colons between fields
    two_digit_itoa(hh, time.hour());
    two_digit_itoa(mm, time.minute());
    two_digit_itoa(ss, time.second());
    buffer[8] = '\0';              // terminating character
}
