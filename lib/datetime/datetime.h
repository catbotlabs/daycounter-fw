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

#ifndef PIO_UNIT_TESTING
#include <avr/pgmspace.h>
#else
// Unit testing: Quick mock up for pgmspace tools
#define PROGMEM
#define memcpy_P memcpy
#endif  // PIO_UNIT_TESTING

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
enum class AscendingDateField : uint8_t {
    Day,
    Month,
    Year
};

/** Shorter AscendingDateField names for convience. */
constexpr auto ADF_DAY = AscendingDateField::Day;
constexpr auto ADF_MONTH = AscendingDateField::Month;
constexpr auto ADF_YEAR = AscendingDateField::Year;

/**
 * Provides a standardized way of referencing and
 * accessing the fields of a Time object.
 * 
 * These names are in "ascending" order, since they
 * progress from Second to Minute to Hour,
 * and Seconds < Minutes < Hours in duration of time.
 */
enum class AscendingTimeField : uint8_t {
    Second,
    Minute,
    Hour
};

/** Shorter AscendingTimeField names for convience. */
constexpr auto ATF_SECOND = AscendingTimeField::Second;
constexpr auto ATF_MINUTE = AscendingTimeField::Minute;
constexpr auto ATF_HOUR = AscendingTimeField::Hour;

/** Return value for count_time_units_until */
enum class TimeCountResult : uint8_t {
    OK,          /** Result is usable and resides within expected limits */
    Negative,    /** Result would be negative */
    Overflow     /** Result would exceed 999  */
};

/** Whether we are counting in days, weeks, or hours */
enum class TimeCountUnit : uint8_t {
    Days,
    Hours,
    Weeks
};

/**
 * Setting that determines if the target date is ahead of
 * or behind the current RTC time.
 * 
 * If AHEAD, then we count the days until the target date.
 * 
 * If BEHIND, then we count the days since the target date.
 */
enum class TimeCountDirection : uint8_t {
    Ahead,
    Behind
};

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
 * All the three-character month names, "glued" together
 * and stored in program space.
 * 
 * Note that for the "month" field in the Date object,
 * January is month=1. In this string January is at index 0, etc.A
 */
static const char MONTH_NAMES_P[(3 * 12) + 1] PROGMEM =
    "JanFebMarAprMayJunJulAugSepOctNovDec";

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
    uint8_t fields[3]/* = {1, 1, 0}*/;  // Default value: January 1, XX00.

 public:
    Date() = default;

    /**
     * Note that the arguments are NOT in the same order as the stored fields.
     * 
     * @note: This constructor makes Date a non-POD type, but the convience
     * and clarity outweighs the potential performance benefits.
     */
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
     * @returns TRUE if year is a multiple of 4, UNLESS
     * year is 0 (century year), then returns FALSE.
     */
    static inline bool is_leap_year(uint8_t in_year) {
        return in_year && ((in_year % 4) == 0);
    }

    /**
     * Returns true if the "now" date is before the "ref" date.
     */
    bool is_before(const Date& other) const;

    uint16_t count_days_until(const Date& target) const;

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
        if (field == AscendingDateField::Year) {
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
            case ADF_YEAR:  // saves flash space to not have "default"
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
    uint8_t fields[3]/* = {0, 0, 0}*/;  // Default value: 00:00:00

 public:
    Time() = default;

    /**
     * Note that the arguments are NOT in the same order as the stored fields.
     * 
     * @note: This constructor makes Time a non-POD type, but the convience
     * and clarity outweighs the potential performance benefits.
     */
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
 * A buffer for the seven MCP7940N time-keeping registers we're
 * using. (RTCWKDAY is ignored except for being buffered for future writes.)
 * 
 * This abstractifies the DateTime code from the hardware enough
 * so the datetime_from_registers function can be tested.
 */
struct __attribute__((packed)) DateTimeRegisters {
    uint8_t rtcsec;
    uint8_t rtcmin;
    uint8_t rtchour;
    uint8_t rtcwkday;  // Unused & untested outside of I2C-interfacing code
    uint8_t rtcdate;
    uint8_t rtcmth;
    uint8_t rtcyear;
};

static_assert(sizeof(DateTimeRegisters) == 7,
    "DateTimeRegisters must be 7 bytes long!");


#ifdef PIO_UNIT_TESTING  // Only expose for unit testing.
uint8_t int_from_bcd(uint8_t bcd);
uint8_t bcd_from_int(uint8_t num);
#endif

/**
 * Sets all members of a DateTime object using
 * the buffered raw register data from the MCP7940N RTC.
 */
void datetime_from_registers(Date* date, Time* time,
    const DateTimeRegisters& rg);

void update_registers_from_datetime(DateTimeRegisters* rg, const Date& date,
    const Time& time);

uint8_t tick_datetime_one_second(Date* date, Time* time);


/** String-formatting tools: */

/**
 * @brief Convert a two-digit integer into the first two
 * ASCII characters of a string.
 * 
 * @param num: The given integer to be converted to a string.
 * The integer must be between 0 and 99, inclusive. Bounds
 * are not checked!
 * 
 * @param cstr: Pointer to the character where the tens digit
 * will be placed. The ones digit will be placed at (cstr + 1).
 * No terminating character is added, only cstr[0] and cstr[1]
 * are modified.
 * 
 * Behavior example:
 * 
 *    num     cstr
 *  ----------------
 *     0   |  '00'
 *     1   |  '01'
 *     9   |  '09'
 *     24  |  '24'
 *     70  |  '70'
 *     99  |  '99'
 *    100  |  ':0'  (error: undefined!)
 *    ...  |  undefined
 *  ----------------
 * 
 */
inline void two_digit_itoa(char* cstr, uint8_t num) {
    cstr[0] = '0' + (num / 10);
    cstr[1] = '0' + (num % 10);
}

/**
 * Returns a program-space pointer to a 3-character month name string.
 * 
 * @param month: The month field of a Date object. (1 = January, 12 = December).
 * 
 * @note The string is not '\0'-terminated. You must read only 3 characters.
 */
constexpr const char* get_month_name_P(uint8_t month) {
    return MONTH_NAMES_P + ((month - 1) * 3);
}

/**
 * Writes a string of format "mmm DD 'YY" to the given buffer
 * using the given Date object.
 * 
 * @note Exactly 11 characters are written to the buffer.
 * A '\0' terminating character is written at the end.
 */
void generate_date_string(char* buffer, const Date& date);

/**
 * Writes a string of format "HH:MM:SS" to the given buffer
 * using the given Time object.
 * 
 * @note Exactly 9 characters are written to the buffer.
 * A '\0' terminating character is written at the end.
 */
void generate_time_string(char* buffer, const Time& time);

/**
 * TODO: document
 */
TimeCountResult count_time_units_until(const Date& target,
        const Date& now_d, const Time& now_t,
        TimeCountUnit count_unit, TimeCountDirection direction,
        uint16_t* count);


#endif  // LIB_DATETIME_DATETIME_H_
