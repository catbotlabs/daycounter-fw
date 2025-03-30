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
 * When in the main menu, how many seconds of
 * inactivity before the screen blanks
 */
const uint8_t MAINMENU_TIMEOUT_S = 10;

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
    RtcClock,
    CountMode
};

enum class TimeOrDate {
    Time,
    Date
};

struct UIState {
    Layer layer = Layer::Blank;
    MainMenuPage mm_page = MainMenuPage::TargetDate;

    /**
     * Whether we are editing the Date or the Time of the
     * current RTC time.
     */
    TimeOrDate rtc_set_field = TimeOrDate::Time;

    /** Seconds before inactivity timeout in Main Menu */
    uint8_t mm_timeout = MAINMENU_TIMEOUT_S;

    bool operator!=(const UIState& other) const {
        return layer != other.layer ||
               mm_page != other.mm_page;
    }

    /** The pixel width of the header on the Main Menu layer */
    uint8_t mmpg_header_width = 0;

    /** The y-location in pixels of the top of the scrollbar num */
    uint8_t mm_scrollbar_top = 0;
};

static UIState ui_state;
void ui_draw(UIState* state);

void ui_pps() {
    switch (ui_state.layer) {
        case Layer::Blank: {
            break;
        }
        case Layer::MainMenu: {
            // Tick main menu inactivity blank
            if (--ui_state.mm_timeout == 0) {
                ui_state.layer = Layer::Blank;
            }
            break;
        }
        case Layer::FieldEdit: {
            break;
        }
    }

    ui_draw(&ui_state);
}


/** Transitions between UI layers */
void ui_transition_blank_to_mainmenu();
void ui_transition_mainmenu_to_blank();
void ui_transition_mainmenu_to_fieldedit();
void ui_transition_fieldedit_to_mainmenu();

/** Process UI logic in the MainMenu or FieldEdit layers. */
void ui_logic_layer_mainmenu(btn_helper btn, bool is_repeat);
void ui_logic_layer_fieldedit(btn_helper btn, bool is_repeat);


void ui_btn_input(btn_helper btn, bool is_repeat) {
    if (btn.bits() == 0) {
        return;
    }

    switch (ui_state.layer) {
        case Layer::Blank: {
            ui_transition_blank_to_mainmenu();
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

    ui_draw(&ui_state);
}

void ui_transition_blank_to_mainmenu() {
    /** Activity on Blank layer means transition to Main Menu layer */
    ui_state.layer = Layer::MainMenu;
    ui_state.mm_timeout = MAINMENU_TIMEOUT_S;  // Set inactivity timer
}

void ui_transition_mainmenu_to_blank() {
    ui_state.layer = Layer::Blank;
}

void ui_transition_mainmenu_to_fieldedit() {
    ui_state.layer = Layer::FieldEdit;
    ui_state.rtc_set_field = TimeOrDate::Time;
}

void ui_transition_fieldedit_to_mainmenu() {
    ui_state.layer = Layer::MainMenu;
    ui_state.mm_timeout = MAINMENU_TIMEOUT_S;  // Set inactivity timer
}

void ui_logic_layer_mainmenu(btn_helper btn, bool is_repeat) {
    // Reset inactivity timer
    ui_state.mm_timeout = MAINMENU_TIMEOUT_S;
    if (btn.left()) {
        ui_state.mm_page = MainMenuPage::TargetDate;
        ui_transition_mainmenu_to_blank();
        return;
    }

    // Scroll up or down between the pages
    if (btn.down()) {
        if (ui_state.mm_page == MainMenuPage::CountMode) {
            if (!is_repeat) {
                // Allow wrapping if it's not a repeat press
                ui_state.mm_page = MainMenuPage::TargetDate;
            }
        } else {
            // Increment Main Menu page
            reinterpret_cast<uint8_t&>(ui_state.mm_page)++;
        }
    } else if (btn.up()) {
        // Allow wrapping if it's not a repeat press
        if (ui_state.mm_page == MainMenuPage::TargetDate) {
            if (!is_repeat) {
                ui_state.mm_page = MainMenuPage::CountMode;
            }
        } else {
            // Decrement Main Menu page
            reinterpret_cast<uint8_t&>(ui_state.mm_page)--;
        }
    }

    // Press SET to edit the selected item
    if (btn.set()) {
        ui_transition_mainmenu_to_fieldedit();
    }
}

void ui_logic_layer_fieldedit(btn_helper btn, bool is_repeat) {
    // Press SET to confirm the field's value
    if (btn.set()) {
        // Save values or whatever...
        ui_transition_fieldedit_to_mainmenu();
    }
}


void ui_draw_layer_mainmenu(UIState* state, const UIState& last_state);
void ui_draw_layer_fieldedit(UIState* state, const UIState& last_state);

void ui_draw(UIState* state) {
    static UIState last_state;
    if (!(*state != last_state)) return;

    if (state->layer != last_state.layer) {
        ssd1306_clearScreen();
    }

    switch (state->layer) {
        case Layer::Blank: {
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

    last_state = *state;
}

void ui_draw_layer_mainmenu(UIState* state, const UIState& last_state) {
    // Clear the "information block" area
    ssd1306_clearBlock((6*10), 2, (12*10)-(6*10), 16);
    switch (state->mm_page) {
        case MainMenuPage::TargetDate: {
            state->mm_scrollbar_top = 0;
            state->mmpg_header_width = (6*17);

            // Draw the header
            ssd1306_printFixedN(0, 0, "* SET TARGET DATE", STYLE_BOLD, 0);

            // "mmm DD 'YY" -> 11 chars w/term
            string_init(date_str, 11);
            generate_date_string(date_str, running_date);
            ssd1306_printFixedN(0, 16, date_str, STYLE_NORMAL, 1);
            break;
        }
        case MainMenuPage::RtcClock: {
            state->mm_scrollbar_top = 10;
            state->mmpg_header_width = (6*15);

            // Draw the header
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
        case MainMenuPage::CountMode: {
            state->mm_scrollbar_top = 20;
            state->mmpg_header_width = (6*16);

            // Draw the header
            ssd1306_printFixedN(0, 0, "* SET COUNT MODE", STYLE_BOLD, 0);

            ssd1306_printFixedN(0, 16, "Unit: Days", STYLE_NORMAL, 0);
            ssd1306_printFixedN(0, 24, "Target: Ahead", STYLE_NORMAL, 0);
            break;
        }
    }

    // Clear trailing end delta of header
    int8_t delta_header_width =
        last_state.mmpg_header_width - state->mmpg_header_width;
    if (delta_header_width > 0) {
        ssd1306_clearBlock(state->mmpg_header_width, 0,
                           delta_header_width, 8);
    }

    // Clear last scrollbar nub
    ssd1306_clearBlock(124, (last_state.mm_scrollbar_top / 8), 3, 16);
    for (uint8_t x = 124; x < 127; x++) {
        // Draw scrollbar nub
        ssd1306_drawVLine(x, state->mm_scrollbar_top,
                                state->mm_scrollbar_top + 11);
    }
    // Draw scrollbar line
    ssd1306_drawVLine(127, 0, 31);
}

void ui_draw_layer_fieldedit(UIState* state, const UIState& last_state) {
    ssd1306_clearScreen();
    switch (state->mm_page) {
        case MainMenuPage::TargetDate:
        case MainMenuPage::RtcClock: {
            break;
        }
        case MainMenuPage::CountMode: {
            break;
        }
    }
}
