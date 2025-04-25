/**
 * @file lib/datetime/datetime.cpp
 * @brief Portable code for packing, unpacking, and manipulating
 * date and time values.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the “Software”), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/** 
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
 */

#include "datetime.h"
#include <stdlib.h>  /** for abs() */

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
        const AscTimeField field = static_cast<AscTimeField>(i);
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

    // Increment date by one, starting with day of month
    for (uint8_t i = 0; i < 3; i++) {
        const AscDateField field = static_cast<AscDateField>(i);
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


/**
 * Returns true if the "now" date is before the "ref" date.
 */
bool Date::is_before(const Date& other) const {
    if (year() == other.year()) {
        // The year is the same, so compare the month.
        if (month() == other.month()) {
            // The month is the same, so compare the day.
            return day() < other.day();  // is this day before the other?
        }
        return month() < other.month();  // is this month before the other?
    }

    // Signed delta years:
    int8_t delta_years = int8_t(other.year()) - year();
    if (delta_years > 50 || delta_years < -50) {
        /**
         * If the distance (up or down) between the given year
         * and the ref year exceeds 50 years, then we treat the
         * ref year as being in the next century.
         * 
         * Since the years now "wrap" around the century, we flip
         * the comparison since the years are now approaching ref
         * from the other direction.
         */
        return year() > other.year();  // Years wrap around century
    }

    // No wrapping, simple behavior:
    return year() < other.year();  // is this year before the other?
}


/**
 * NOTE: set_glyph should have its out-of-bounds handling removed
 * so its more like set_glyphs_from_count.
 */


/**
 * @brief Returns the number of days until the given date.
 * 
 * @note Always assumes that the "target" date is ahead of
 * "this" date, so do not use this method to count backwards!
 * 
 * THIS FUNCTION IS NOT SUPER OPTIMIZED:
 * Takes one loop iteration for each month that has to go by.
 * Maximum iterations: 50 (years) * 12 = 600 iterations.
 * 
 * After benchmarking, this function takes about 2 milliseconds
 * worst case (counting a full 50 year time delta). That's
 *  *probably* good enough for now, but I may come back and
 * optimize this if it's too much of a hangup.
 * 
 * Instead of iterating through months,
 * I suppose the optimized algorithm would be to:
 *  - Count days till beginning of next month
 *  - Count days till beginning of next year
 *  - Determine how many leap years are between now and the beginning
 *     of the target year.
 *  - Count how many days are in those years (365 in a year, or 366
 *     in a leap year)
 *  - Count days till beginning of target month
 *  - Count days till target date
 * 
 * It may also be necessary to add a "tick_date_one_day" function,
 * kind of like tick_datetime_one_second, to properly increment by one
 * day while wrapping.
 */
uint16_t Date::count_days_until(const Date& target) const {
    // Copy of "this" date, which will be incremented until target is reached.
    Date now = *this;

    // The going count of days.
    uint16_t count = 0;

    // While target year and month haven't been reached
    while (!(now.year() == target.year() && now.month() == target.month())) {
        // Add the remaining days of this month to the count.
        count += now.field_max_value(ADF_DAY) - now.day() + 1;

        // Set "now" to first day of the NEXT month.
        now.day() = now.field_min_value(ADF_DAY);
        now.month()++;

        // Did we go past December?
        if (now.month() > now.field_max_value(ADF_MONTH)) {
            // Wrap the month back to January.
            now.month() = now.field_min_value(ADF_MONTH);
            now.year()++;

            // Did we go past the year XX99?
            if (now.year() > now.field_max_value(ADF_YEAR)) {
                // Wrap the year back to XX00
                now.year() = now.field_min_value(ADF_YEAR);
            }
        }
    }

    // "now" and "target" are on the same month of the same year.
    // Return the going count plus the number of days from "now" to "target".
    return count + (target.day() - now.day());
}

// TODO: document
const uint16_t HOUR_COUNT_MAX_DAY_COUNT = (1000 / 24) + 2;

/** TODO: document
 * 
 * @param target: The date we are counting towards
 * @param now_d, now_t: Current date and time.
 * @param count_unit: Whether given count is in hours, days, or weeks.
 * @param direction: AHEAD if we're counting from `now` to `target`
 *                or BEHIND if we're counting from `target` to `now`
 *
 * @param count: Points to where the counted number will be stored.
 * 
 * @returns: - OK if count was successful.
 *             The value of `count` is set between 0 and 999.
 *
 *           - NEGATIVE if the target and date are backwards verses
 *             the given `direction`.
 *             The value of `count` is unchanged.
 *
 *           - OVERFLOW if the count is in the right direction, but the
 *             result would exceed 999.
 *             The value of `count` is unchanged.
 */
TimeCountResult count_time_units_until(const Date& target,
        const Date& now_d, const Time& now_t,
        TimeCountUnit count_unit, TimeCountDirection direction,
        uint16_t* count) {
    /** Positive day delta between now and target dates */
    uint16_t day_count;
    /** How many hours the given Time adds or subtracts from the day count */
    int8_t hours_delta = now_t.hour();

    if (direction == TimeCountDirection::Ahead) {
        /** We are counting from "now" to "target" */

        if (target.is_before(now_d)) {
            /** Target is before now! Cannot count backwards. */
            return TimeCountResult::Negative;
        }
        day_count = now_d.count_days_until(target);
        hours_delta *= -1;
    } else {  // BEHIND
        /** We are counting from "target" to "now" */

        if (now_d.is_before(target)) {
            /** Now is before target! Cannot count backwards. */
            return TimeCountResult::Negative;
        }
        day_count = target.count_days_until(now_d);
    }

    switch (count_unit) {
        case TimeCountUnit::Days: {
            if (day_count > 999) {
                return TimeCountResult::Overflow;
            }
            *count = day_count;
            return TimeCountResult::OK;
        }
        case TimeCountUnit::Weeks: {
            // if day_count==6999, then week count is 999.
            // if day_count==7000, then week count is 1000.
            if (day_count > 6999) {
                return TimeCountResult::Overflow;
            }
            *count = day_count / 7;
            return TimeCountResult::OK;
        }
        default: {  // Hours
            uint16_t hour_count;

            /** Make sure we do not overflow hour_count. */
            if (day_count > HOUR_COUNT_MAX_DAY_COUNT) {
                return TimeCountResult::Overflow;
            }
            hour_count = day_count * 24;

            /**
             * Catch the scenario when counting AHEAD,
             * where the day_count is 0 and the hours_delta
             * is negative.
             */
            if (hours_delta < 0 && uint8_t(-hours_delta) > hour_count) {
                    return TimeCountResult::Negative;
            }

            // Subtract the hours already past in today's time
            hour_count += hours_delta;

            if (hour_count > 999) {
                return TimeCountResult::Overflow;
            }
            *count = hour_count;
            return TimeCountResult::OK;
        }
    }
}

/**
 * Remaining functionality to add:
 *   - Date delta In_Future/In_Past calculator  / Check if delta is within display bounds
 *   - Time delta calculator function
 *   - Date / Time specialty individual field increment/decrement functions
 */

