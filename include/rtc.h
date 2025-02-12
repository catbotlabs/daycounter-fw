/**
 * @file include/rtc.h
 * @brief Software UART using Analog Comparator interrupts.
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

/**
 * @brief Makes sure the RTC's registers are set properly.

 * @returns 0 if all registers are set as expected.
 *          1 if any registers were adjusted.
 *          2 if failed to communicate with RTC.
 */
uint8_t setup_rtc();

#endif  // INCLUDE_RTC_H_
