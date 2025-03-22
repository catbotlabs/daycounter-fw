/**
 * @file lib/util/util.h
 * @brief General-purpose utility functions
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef LIB_UTIL_UTIL_H_
#define LIB_UTIL_UTIL_H_

#include <stdint.h>
#include <math.h>

/**
 * @brief Returns the position of the least significant bit
 * that is set to 1.
 * 
 * @note If the byte is zero, returns -1.
 * 
 * Example: 0b00100000  ->  5
 *              ^
 * 
 * Example: 0b00110101  ->  0
 *                   ^
 */
inline int8_t get_least_set_bit(uint8_t byte) {
    for (int i = 0; i < 8; i++) {
        if ((byte >> i) & 1) {
            return i;
        }
    }
    return -1;
}

#endif  // LIB_UTIL_UTIL_H_
