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

// TODO: document

/**
 * Tell the UI that a PPS tick has received from RTC
 * 
 * This may cause a redraw.
 */
void ui_pps();

/**
 * Tell the UI that the given buttons have been pressed.
 * The UI will then perform logic, such as editing the
 * running_settings or running_date/time objects, or saving
 * those objects to nonvolatile storage.
 * 
 * The UI will redraw if necessary.
 * 
 * @param btn: Button flags for buttons that were PRESSED
 * since last ui_btn_input call. This can be multiple buttons.
 * If there are repeats, then this is the flag of the repeating button.
 * 
 * @param repeats: The number of repeat presses since the button was pressed.
 * If 0, then this is not a repeat press.
 */
void ui_btn_input(btn_helper btn, uint8_t repeats);

#endif  // INCLUDE_UI_H_
