/**
 * @file src/settings.cpp
 * @brief Nonvolatile EEPROM-stored, CRC-verified settings implementation.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "settings.h"
#include <util/crc16.h>
#include <avr/eeprom.h>

/** Starting seed for our CRC (something non-zero) */
const uint16_t CRC16_SEED = 0x87da;

/** EEPROM address where the StoredSettings object is located */
const uintptr_t SETTINGS_DATA_ADDR = 0x00;

/**
 * This is the structure that is written and read on the EEPROM.
 * 
 * It simply encapsulates a Settings structure and a 16-bit CRC.
 */
struct StoredSettings {
    Settings data;   // 3 bytes
    uint16_t crc;    // 2 bytes
};

static_assert(sizeof(StoredSettings) == 5, "Should be 5 bytes");


/**
 * Calculate 16-bit CRC for data at given address with the given length.
 */
uint16_t calculate_crc(const void* data, uint8_t length) {
    uint16_t crc = CRC16_SEED;
    for (uint8_t i = 0; i < length; i++) {
        uint8_t byte = reinterpret_cast<const uint8_t*>(data)[i];
        crc = _crc16_update(crc, byte);
    }
    return crc;
}

/**
 * Stores the Settings object and its CRC in EEPROM.
 * 
 * @returns The CRC checksum of the given settings.
 */
uint16_t write_settings(const Settings& settings) {
    // The StoredSettings package encapsulates the Settings and its CRC
    StoredSettings package;
    package.data = settings;

    // Calculate the Settings's CRC, then put it at the end of the package.
    package.crc = calculate_crc(&settings, sizeof(Settings));

    // Write the StoredSettings package to the EEPROM.
    // Only the changed bytes are actually writtten.
    eeprom_update_block(
        &package,
        reinterpret_cast<void*>(SETTINGS_DATA_ADDR),
        sizeof(StoredSettings));

    return package.crc;
}

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
bool read_settings_checked(Settings* settings) {
    StoredSettings package;

    /** First, load StoredSettings package from EEPROM */
    eeprom_read_block(
        &package,
        reinterpret_cast<void*>(SETTINGS_DATA_ADDR),
        sizeof(StoredSettings));

    /**
     * Calculate the CRC checksum of the Settings object.
     * If the data is valid, its CRC should compute to zero.
     */
    if (calculate_crc(&package, sizeof(StoredSettings))) {
        /** Nonzero CRC: Data is invalid! */

        // Reset the Settings object to factory defaults.
        *settings = Settings();
        // Write the defaults to EEPROM.
        write_settings(*settings);
        return true;
    }
    /** Zero CRC: Data is valid! */
    *settings = package.data;
    return false;
}

