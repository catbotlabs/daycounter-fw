/**
 * @file include/global/Arduino.h
 * @brief Boilerplate placeholder file and defines for compatibility
 * with TinyI2C library.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, CatBotLabs
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef INCLUDE_GLOBAL_ARDUINO_H_
#define INCLUDE_GLOBAL_ARDUINO_H_

#define DDR_USI DDRA
#define PIN_USI PINA
#define PORT_USI PORTA
#define PIN_USI_SDA PINA6
#define PIN_USI_SCL PINA4
#define DDR_USI_CL DDR_USI
#define PIN_USI_CL PIN_USI
#define PORT_USI_CL PORT_USI

#endif  // INCLUDE_GLOBAL_ARDUINO_H_
