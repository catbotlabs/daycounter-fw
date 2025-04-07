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
#include "rtc.h"

/**
 * When in the main menu, how many seconds of
 * inactivity before the screen blanks
 */
const uint8_t MAINMENU_TIMEOUT_S = 6;

enum class Layer : uint8_t {
    Blank,
    MainMenu,
    FieldEdit
};

enum class MainMenuPage : uint8_t {
    TargetDate,
    RtcClock,
    CountMode
};

enum class TimeOrDate : uint8_t {
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
    TimeOrDate time_or_date = TimeOrDate::Time;

    /** Currently selected field (index from the left of the screen) */
    uint8_t active_field = 0;

    /** Seconds before inactivity timeout in Main Menu */
    uint8_t mm_timeout = MAINMENU_TIMEOUT_S;

    /** The pixel width of the header on the Main Menu layer */
    uint8_t mmpg_header_width = 0;

    /** The y-location in pixels of the top of the Main Menu scrollbar nub */
    uint8_t mm_scrollbar_top = 0;

    /** The x-location in pixels of the left of the Field Edit arrows */
    uint8_t fe_arrow_left = 0;

    /** General-purpose needs-to-be-redrawn flag */
    bool dirty = false;

    bool operator!=(const UIState& other) const {
        return layer != other.layer ||
               mm_page != other.mm_page ||
               active_field != other.active_field ||
               dirty != other.dirty;
    }
};

static UIState ui_state;

// Longest string: "Date: mmm DD 'YY" -> 17 chars w/term
/** General-purpose string buffer for UI */
static char ui_strbuf[17];


void ui_draw(UIState* state);

void ui_pps() {
    // If we're looking at the RTC time, then redraw.
    if (ui_state.layer != Layer::Blank &&
            ui_state.mm_page == MainMenuPage::RtcClock) {
        ui_state.dirty = true;
    }
    
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

/** Save edited fields to the RTC or EEPROM */
void ui_fieldedit_save();


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
    ui_state.time_or_date = TimeOrDate::Date;
    ui_state.active_field = 0;
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
    // The maximum value of the active_field mapped by which
    // Main Menu page we're on.
    static const uint8_t MAX_FIELD_VALUE_PAGE_MAP[3] = {2, 5, 1};

    uint8_t max_field_value = MAX_FIELD_VALUE_PAGE_MAP[static_cast<uint8_t>(ui_state.mm_page)];

    if (btn.left()) {
        if (ui_state.active_field == 0) {
            if (!is_repeat) {
                // Allow wrapping if it's not a repeat press
                ui_state.active_field = max_field_value;
            }
        } else {
            // Increment active field index
            ui_state.active_field--;
        }
    } else if (btn.right()) {
        if (ui_state.active_field == max_field_value) {
            if (!is_repeat) {
                // Allow wrapping if it's not a repeat press
                ui_state.active_field = 0;
            }
        } else {
            // Increment active field index
            ui_state.active_field++;
        }
    }

    if (ui_state.active_field > 2) {
        ui_state.time_or_date = TimeOrDate::Time;
    } else {
        ui_state.time_or_date = TimeOrDate::Date;
    }

    // Press SET to confirm the field's value
    if (btn.set()) {
        // Save values or whatever...
        ui_fieldedit_save();
        ui_transition_fieldedit_to_mainmenu();
    }
}

/** Save edited fields to the RTC or EEPROM */
void ui_fieldedit_save() {
    switch (ui_state.mm_page) {
        case MainMenuPage::TargetDate: {
            write_settings(running_settings);
            break;
        }
        case MainMenuPage::RtcClock: {
            // TODO: Only update RTC if clock was modified.
            rtc_write_datetime(running_date, running_time);
            break;
        }
        case MainMenuPage::CountMode: {
            write_settings(running_settings);
            break;
        }
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

    state->dirty = false;

    last_state = *state;
}

void ui_draw_layer_mainmenu(UIState* state, const UIState& last_state) {
    bool page_changed = state->mm_page != last_state.mm_page ||
                        state->layer != last_state.layer;

    // Clear the "information block" area
    if (page_changed) {
        ssd1306_clearBlock((6*10), 2, (12*10)-(6*10), 16);
    }
    switch (state->mm_page) {
        case MainMenuPage::TargetDate: {
            state->mm_scrollbar_top = 0;
            state->mmpg_header_width = (6*17);

            // Draw the header
            ssd1306_printFixedN(0, 0, "* SET TARGET DATE", STYLE_BOLD, 0);

            // "mmm DD 'YY" -> 11 chars w/term
            generate_date_string(ui_strbuf, running_settings.target_date);
            ssd1306_printFixedN(0, 16, ui_strbuf, STYLE_NORMAL, 1);
            break;
        }
        case MainMenuPage::RtcClock: {
            state->mm_scrollbar_top = 10;
            state->mmpg_header_width = (6*15);

            // Draw the header
            if (page_changed) {
                ssd1306_printFixedN(0, 0, "* SET RTC CLOCK", STYLE_BOLD, 0);
            }

            // "Date: mmm DD 'YY" -> 17 chars w/term
            memcpy_P(ui_strbuf, PSTR("Date: "), 6);
            generate_date_string(ui_strbuf + 6, running_date);
            ssd1306_printFixedN(0, 16, ui_strbuf, STYLE_NORMAL, 0);  // print Date

            // "Time: HH:MM:SS" -> 15 chars w/term
            memcpy_P(ui_strbuf, PSTR("Tim"), 3);
            generate_time_string(ui_strbuf + 6, running_time);
            ssd1306_printFixedN(0, 24, ui_strbuf, STYLE_NORMAL, 0);  // print Time
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
    
    if (page_changed) {
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
}

/** 9x8 pixel bitmap of an Up arrow, aligned to the top of the canvas */
//static const uint8_t PROGMEM ARROW_UP[9]   = {0x10, 0x18, 0x1c, 0x1e, 0x1f, 0x1e, 0x1c, 0x18, 0x10};

/** 9x8 pixel bitmap of a Down arrow, aligned to the bottom of the canvas */
//static const uint8_t PROGMEM ARROW_DOWN[9] = {0x08, 0x18, 0x38, 0x78, 0xf8, 0x78, 0x38, 0x18, 0x08};

static const uint8_t fe_date_arrow_pos[3] = {15, 57, 105};
static const uint8_t fe_time_arrow_pos[3] = {9+15, 45+15, 81+15};

/** Whether an arrow image is pointing up or down */
enum class ArrowDirection : uint8_t {
    Up,
    Down
};

// TODO: document
// y should be aligned by 8
// y is top location
// x is bottom location
/**
 * Procedurally draw an Up or Down arrow (9 x 8) pixels,
 * given its top-left coordinates.
*/
void draw_arrow(ArrowDirection direction, uint8_t x, uint8_t y) {
    // range of h: [0, 4]
    for (int i=0; i<9; i++) {
        uint8_t h = 4 - abs(4 - i);

        uint8_t top_offset;
        uint8_t bottom_offset;

        // up: align bottom
        // down: align top
        if (direction == ArrowDirection::Up) {
            top_offset = 4 - h;
            bottom_offset = 4;
        } else {
            top_offset = 2;
            bottom_offset = 2 + h;
        }

        ssd1306_drawVLine(x + i, y + top_offset, y + bottom_offset);
    }
}

void ui_draw_layer_fieldedit(UIState* state, const UIState& last_state) {
    if (state->time_or_date != last_state.time_or_date) {
        ssd1306_clearScreen();
    }

    switch (state->mm_page) {
        case MainMenuPage::TargetDate:
        case MainMenuPage::RtcClock: {
            if (state->time_or_date == TimeOrDate::Date) {
                // date
                state->fe_arrow_left = fe_date_arrow_pos[state->active_field];

                // "mmm DD 'YY" -> 11 chars w/term
                if (state->mm_page == MainMenuPage::TargetDate) {
                    // We're editing the target date, so show that.
                    generate_date_string(ui_strbuf, running_settings.target_date);
                } else {
                    // We're editing the RTC date, so show that.
                    generate_date_string(ui_strbuf, running_date);
                    // TODO: fix visual glitch with ellipses
                    ssd1306_printFixedN(110, 24, "...", STYLE_NORMAL, 0);  // there's more ahead
                }
                ssd1306_printFixedN(0, 8, ui_strbuf, STYLE_NORMAL, 1);
            } else {
                // time
                state->fe_arrow_left = fe_time_arrow_pos[state->active_field - 3];

                // "HH:MM:SS" -> 9 chars w/term
                generate_time_string(ui_strbuf, running_time);
                ssd1306_printFixedN(15, 8, ui_strbuf, STYLE_NORMAL, 1);
                ssd1306_printFixedN(0, 24, "...", STYLE_NORMAL, 0);  // there's more behind
            }
            break;
        }
        case MainMenuPage::CountMode: {
            break;
        }
    }

    if (state->fe_arrow_left != last_state.fe_arrow_left ||
            state->layer != last_state.layer) {
        ssd1306_clearBlock(last_state.fe_arrow_left, 0, 9, 8);
        ssd1306_clearBlock(last_state.fe_arrow_left, 3, 9, 8);
        draw_arrow(ArrowDirection::Up, state->fe_arrow_left, 0);
        draw_arrow(ArrowDirection::Down, state->fe_arrow_left, 24);
//        ssd1306_drawBitmap(state->fe_arrow_left, 0, 9, 8, ARROW_UP);
//        ssd1306_drawBitmap(state->fe_arrow_left, 3, 9, 8, ARROW_DOWN);
    }
}
