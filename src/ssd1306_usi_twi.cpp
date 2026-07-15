/**
 * @file src/ssd1306_usi_twi.cpp
 * @brief Integrate the SSD1306 OLED module with the USIWire library.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, CatBotLabs
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "intf/ssd1306_interface.h"
#include "intf/i2c/ssd1306_i2c.h"  /** for SSD1306_SA */
#include <TinyI2CMaster.h>

/**
 * This code is based on ssd1306/src/ssd1306_hal/arduino/platform.cpp
 * and optimized for this project. Credit to lexus2k for the great library.
 * 
 * This code and the code it was derived from are provided under
 * the terms of the MIT License.
 */

static void ssd1306_i2cStart_Wire() {
    TinyI2C.start(SSD1306_SA, 0);
}

static void ssd1306_i2cStop_Wire() {
    TinyI2C.stop();
}

static void ssd1306_i2cSendByte_Wire(uint8_t data) {
    TinyI2C.write(data);
}

static void ssd1306_i2cSendBytes_Wire(const uint8_t *buffer, uint16_t size) {
    while (size--) {
        TinyI2C.write(*buffer);
        buffer++;
    }
}

void ssd1306_platform_i2cInit(int8_t busId, uint8_t addr,
        ssd1306_platform_i2cConfig_t * cfg) {
    // Not using busId nor addr.
    ssd1306_intf.spi = 0;
    ssd1306_intf.start = ssd1306_i2cStart_Wire;
    ssd1306_intf.stop = ssd1306_i2cStop_Wire;
    ssd1306_intf.send = ssd1306_i2cSendByte_Wire;
    ssd1306_intf.send_buffer = ssd1306_i2cSendBytes_Wire;
}
