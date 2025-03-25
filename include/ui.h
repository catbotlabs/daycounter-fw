/**
 * @file include/ui.h
 * @brief OLED User Interface code.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef INCLUDE_UI_H_
#define INCLUDE_UI_H_

#include <ssd1306.h>
#include "buttons.h"

void ui_pps();
void ui_btn_input(btn_helper btn, bool is_repeat);

#endif  // INCLUDE_UI_H_
