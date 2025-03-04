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
#include <string.h>

#undef SS  // Undefine clashing Arduino macro

/**
 * Date and Time fields are stored in ascending order:
 *  [second, minute, hour] and [day, month, year]
 */


/**
 * Provides a standardized way of referencing and
 * accessing the fields of a Date object.
 * 
 * These names are in "ascending" order, since they
 * progress from Day to Month to Year,
 * and Days < Months < Years in duration of time.
 */
enum class AscendingDateField {
    DAY,
    MONTH,
    YEAR
};

/** Shorter AscendingDateField names for convience. */
constexpr auto ADF_DAY = AscendingDateField::DAY;
constexpr auto ADF_MONTH = AscendingDateField::MONTH;
constexpr auto ADF_YEAR = AscendingDateField::YEAR;

/**
 * Provides a standardized way of referencing and
 * accessing the fields of a Time object.
 * 
 * These names are in "ascending" order, since they
 * progress from Second to Minute to Hour,
 * and Seconds < Minutes < Hours in duration of time.
 */
enum class AscendingTimeField {
    SECOND,
    MINUTE,
    HOUR
};

/** Shorter AscendingTimeField names for convience. */
constexpr auto ATF_SECOND = AscendingTimeField::SECOND;
constexpr auto ATF_MINUTE = AscendingTimeField::MINUTE;
constexpr auto ATF_HOUR = AscendingTimeField::HOUR;


/**
 * The number of days in each month, before accounting for leap years.
 * 
 * Note that in this array, index 0 is January.
 * In the Date object, January is Month 1.
 * 
 * So it's necessary to get the days in the month as such:
 *  @code DAYS_IN_MONTH[date.month() - 1]  @endcode
 */
static const uint8_t DAYS_IN_MONTH[12] =
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/**
 * Calendar date: Stores day-of-month, month, and year.
 * 
 * The Day and Month are stored the way they are typically
 * interpreted: Month 1 is January, Month 3 is March.
 *              Day 1 is first day of month, etc.
 * So, Month 0 is invalid, and Day 0 is invalid.
 * 
 * The Year is an integer ranging from 0 to 99, representing
 * the last two digits of the current year A.D.
 */
class Date {
    /** Stored in ascending order: {day, month, year} */
    uint8_t fields[3] = {1, 1, 0};  // Default value: January 1, XX00.

 public:
    Date() = default;

    /** Note that the arguments are NOT in the same order as the fields. */
    Date(uint8_t month, uint8_t day, uint8_t year) :
        fields{day, month, year} {}

    /** Const-qualified accessors for day, month, and year */
    const uint8_t& day()   const { return fields[0]; }
    const uint8_t& month() const { return fields[1]; }
    const uint8_t& year()  const { return fields[2]; }

    /** Accessors for day, month, and year */
    uint8_t& day()   { return fields[0]; }
    uint8_t& month() { return fields[1]; }
    uint8_t& year()  { return fields[2]; }

    /**
     * Returns true if the given year is a leap year.
     * 
     * @param in_year: The given year, from 0 to 99.
     */
    static inline bool is_leap_year(uint8_t in_year) {
        return (in_year % 4) == 0;
    }

    /**
     * Get a reference to the given date field.
     */
    inline uint8_t& field(AscendingDateField field) {
        return fields[static_cast<uint8_t>(field)];
    }

    /**
     * Returns the minimum allowable value for the given field.
     */
    uint8_t field_min_value(AscendingDateField field) const {
        if (field == AscendingDateField::YEAR) {
            return 0;  // Years number starts at zero.
        }
        return 1;      // Month number and Day-Of-Month number start at 1.
    }

    /**
     * Returns the maximum allowable value for the given field,
     * factoring in how the other fields affect this field.
     * 
     * For instance, the value of the Month field affects the
     * maximum allowable value of the Day field.
     */
    uint8_t field_max_value(AscendingDateField field) const {
        switch (field) {
            case ADF_MONTH:
                return 12;
            case ADF_DAY: {
                uint8_t max = DAYS_IN_MONTH[month() - 1];
                if ((month() == 2) && is_leap_year(year())) {
                    max++;  // February has 29 days if it's a leap year.
                }
                return max;
            }
            default:  // ADF_YEAR
                return 99;
        }
    }
};

/**
 * Time-of-day: Stores seconds, minutes, and hours.
 * 
 * Time is stored as 24-hour time (Hours go up to 23, no AM/PM).
 */
struct Time {
    /** Stored in ascending order: {second, minute, hour} */
    uint8_t fields[3] = {0, 0, 0};  // Default value: 00:00:00

 public:
    Time() = default;

    /** Note that the arguments are NOT in the same order as the fields. */
    Time(uint8_t hour, uint8_t minute, uint8_t second) :
        fields{second, minute, hour} {}

    /** Const-qualified accessors for second, minute, and hour */
    const uint8_t& second() const { return fields[0]; }
    const uint8_t& minute() const { return fields[1]; }
    const uint8_t& hour()   const { return fields[2]; }

    /** Accessors for second, minute, and hour */
    uint8_t& second() { return fields[0]; }
    uint8_t& minute() { return fields[1]; }
    uint8_t& hour()   { return fields[2]; }

    /**
     * Get a reference to the given time field.
     */
    inline uint8_t& field(AscendingTimeField field) {
        return fields[static_cast<uint8_t>(field)];
    }

    /**
     * Returns the minimum allowable value for all of the
     * time fields, which is 0.
     */
    uint8_t fields_min_value() const {
        return 0;
    }

    /**
     * Returns the maximum allowable value for the given time
     * field. For Hours this is 23, for Seconds or Minutes this is 59.
     */
    uint8_t field_max_value(AscendingTimeField field) const {
        if (field == ATF_HOUR) {
            return 23;
        }
        return 59;
    }
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
void datetime_from_registers(Date* date, Time* time,
    const DateTimeRegisters& rg);

uint8_t tick_datetime_one_second(Date* date, Time* time);

#endif  // LIB_DATETIME_DATETIME_H_
