/**
 * @file include/rtc.h
 * @brief Functions to configure, read from and write to the
 * MCP7940N battery-backed RTC.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef INCLUDE_RTC_H_
#define INCLUDE_RTC_H_

#include <stdint.h>
#include "datetime.h"

/**
 * @brief Makes sure the RTC's registers are set properly.

 * @returns 0 if all registers are set as expected.
 *          1 if any registers were adjusted.
 *          2 if failed to communicate with RTC.
 */
uint8_t setup_rtc();

/**
 * Populate a DateTime object with the date and time from the RTC.
 */
void rtc_read_datetime(DateTime* dt);

#endif  // INCLUDE_RTC_H_
