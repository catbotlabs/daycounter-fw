/**
 * @file include/settings.h
 * @brief EEPROM-stored persistent settings structures, with saving
 * and loading functions.
 * 
 * These settings let the DayCounter store the target date.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef INCLUDE_SETTINGS_H_
#define INCLUDE_SETTINGS_H_

#include "datetime.h"

/**
 * Persistent settings for the DayCounter.
 * 
 * The default values of the member variables are the "factory"
 * default values used if the data is found to be invalid.
 * 
 * @note: This struct isn't "packed" because Date isn't a POD-type.
 * Since AVR's are 8-bit, this shouldn't make a difference.
 */
struct Settings {
    Date target_date = Date(1, 1, 50);  // 3 bytes
};

static_assert(sizeof(Settings) == 3, "Should be 3 bytes");

/**
 * Stores the Settings object and its CRC in EEPROM.
 * 
 * @returns The CRC checksum of the given settings.
 */
uint16_t write_settings(const Settings& settings);

/**
 * Loads the stored Settings object from the EEPROM and checks
 * its CRC checksum. If checksum fails, then the settings are
 * set to factory defaults and are written to the EEPROM.
 * 
 * @param settings: Pointer to put the output.
 * 
 * @returns true if stored settings were reset to defaults, or
 *          false if stored settings were valid.
 */
bool read_settings_checked(Settings* settings);

#endif  // INCLUDE_SETTINGS_H_
