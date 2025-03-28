/**
 * @file src/ui.cpp
 * @brief OLED User Interface code.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#include "ui.h"
#include "buttons.h"
#include "globals.h"
#include "util.h"

/**
 * Casts an enum to uint8_t and toggles the 1's bit.
 */
template<class etype>
void toggle_binary_enum(etype* value) {
    *value = static_cast<etype>(
        static_cast<uint8_t>(*value) ^ 1);
}

/**
 * Declares a string with the given name and size,
 * and writes a terminating character to the end of the string.
 */
#define string_init(name, size) \
    char name[size]; \
    name[size-1] = '\0';

/**
 * Declares a string with the given name and size,
 * copies the given literal to the start of the string (without
 * the literal's terminating char), and then writes a terminating
 * character to the end of the string.
 */
#define string_init_start(name, size, start_literal) \
    char name[size]; \
    name[size-1] = '\0'; \
    memcpy_P(name, PSTR(start_literal), sizeof(start_literal)-1);

enum class Layer {
    Blank,
    MainMenu,
    FieldEdit
};

enum class MainMenuPage {
    TargetDate,
    CurrentTime
};

enum class TimeOrDate {
    Time,
    Date
};

struct UIState {
    Layer layer = Layer::Blank;
    MainMenuPage mm_page = MainMenuPage::TargetDate;

    bool operator!=(const UIState& other) const {
        return layer != other.layer ||
               mm_page != other.mm_page;
    }
};

static UIState ui_state;
void ui_draw(const UIState& state);

void ui_pps() {
}

void ui_logic_layer_blank(btn_helper btn, bool is_repeat);
void ui_logic_layer_mainmenu(btn_helper btn, bool is_repeat);
void ui_logic_layer_fieldedit(btn_helper btn, bool is_repeat);

void ui_btn_input(btn_helper btn, bool is_repeat) {
    if (btn.bits() == 0) {
        return;
    }

    switch (ui_state.layer) {
        case Layer::Blank: {
            ui_logic_layer_blank(btn, is_repeat);
            break;
        }
        case Layer::MainMenu: {
            ui_logic_layer_mainmenu(btn, is_repeat);
            break;
        }
        case Layer::FieldEdit: {
            ui_logic_layer_fieldedit(btn, is_repeat);
            break;
        }
    }

    ui_draw(ui_state);
}

void ui_logic_layer_blank(btn_helper btn, bool is_repeat) {
    ui_state.layer = Layer::MainMenu;
    ui_state.mm_page = MainMenuPage::TargetDate;
}

void ui_logic_layer_mainmenu(btn_helper btn, bool is_repeat) {
    if (btn.left()) {
        ui_state.layer = Layer::Blank;
        return;
    }

    if (btn.up() || btn.down()) {
        bool no_wrap = (btn.down() &&
            ui_state.mm_page == MainMenuPage::TargetDate) ||
            (btn.up() && ui_state.mm_page == MainMenuPage::CurrentTime);

        if (!is_repeat || (is_repeat && no_wrap)) {
            toggle_binary_enum(&ui_state.mm_page);
        }
    }
}

void ui_logic_layer_fieldedit(btn_helper btn, bool is_repeat) {
}


void ui_draw_layer_mainmenu(const UIState& state, const UIState& last_state);
void ui_draw_layer_fieldedit(const UIState& state, const UIState& last_state);

void ui_draw(const UIState& state) {
    static UIState last_state;
    if (!(state != last_state)) return;

    switch (state.layer) {
        case Layer::Blank: {
            ssd1306_clearScreen();
            break;
        }
        case Layer::MainMenu: {
            ui_draw_layer_mainmenu(state, last_state);
            break;
        }
        case Layer::FieldEdit: {
            ui_draw_layer_fieldedit(state, last_state);
            break;
        }
    }

    last_state = state;
}




void ui_draw_layer_mainmenu(const UIState& state, const UIState& last_state) {
    if (state.mm_page != last_state.mm_page) {
        ssd1306_clearScreen();
    }

    ssd1306_drawVLine(127, 0, 31);
    switch (state.mm_page) {
        case MainMenuPage::TargetDate: {
            ssd1306_drawVLine(125, 1, 12);
            ssd1306_drawVLine(126, 1, 12);
            ssd1306_printFixedN(0, 0, "* SET TARGET DATE", STYLE_BOLD, 0);

            string_init(date_str, 11);
            generate_date_string(date_str, running_date);
            ssd1306_printFixedN(0, 16, date_str, STYLE_NORMAL, 1);
            break;
        }
        case MainMenuPage::CurrentTime: {
            ssd1306_drawVLine(125, 19, 30);
            ssd1306_drawVLine(126, 19, 30);
            ssd1306_printFixedN(0, 0, "* SET RTC CLOCK", STYLE_BOLD, 0);

            // "Time: HH:MM:SS" -> 15 chars w/term
            string_init_start(time_str, 15, "Time: ");
            generate_time_string(time_str + 6, running_time);

            // "Date: mmm DD 'YY" -> 17 chars w/term
            string_init_start(date_str, 17, "Date: ");
            generate_date_string(date_str + 6, running_date);

            ssd1306_printFixedN(0, 16, time_str, STYLE_NORMAL, 0);
            ssd1306_printFixedN(0, 24, date_str, STYLE_NORMAL, 0);
            break;
        }
    }
}

void ui_draw_layer_fieldedit(const UIState& state, const UIState& last_state) {
}
