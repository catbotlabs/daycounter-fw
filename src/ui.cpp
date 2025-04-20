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

/** Switch to "fast repeat" mode on the given Nth repeat. */
const uint8_t FAST_REPEAT_BUILDUP = 5;

/** When in "fast repeat" mode, make this the minimum increment */
const uint8_t FAST_REPEAT_INCREMENTS = 5;

// Labels for the CountUnit field
// It is 3 substrings that are 5 chars long each.
static const char CountUnitLabels[] PROGMEM = "Days\0HoursWeeks";

// Labels for the CountDirection field
// It is 2 substrings that are 6 chars long each.
static const char CountDirectionLabels[] PROGMEM = "Ahead\0Behind";

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

enum class LeftOrRight : uint8_t {
    Left,
    Right
};

struct UIState {
    Layer layer = Layer::Blank;
    MainMenuPage mm_page = MainMenuPage::TargetDate;

    /**
     * Whether we are editing the left or right horizontal
     * page in field edit mode (for RtcClock and CountMode)
     */
    LeftOrRight hz_page = LeftOrRight::Left;

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
void ui_logic_layer_mainmenu(btn_helper btn, uint8_t repeats);
void ui_logic_layer_fieldedit(btn_helper btn, uint8_t repeats);

/** Save edited fields to the RTC or EEPROM */
void ui_fieldedit_save();


void ui_btn_input(btn_helper btn, uint8_t repeats) {
    if (btn.bits() == 0) {
        return;
    }

    switch (ui_state.layer) {
        case Layer::Blank: {
            ui_transition_blank_to_mainmenu();
            break;
        }
        case Layer::MainMenu: {
            ui_logic_layer_mainmenu(btn, repeats);
            break;
        }
        case Layer::FieldEdit: {
            ui_logic_layer_fieldedit(btn, repeats);
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
    ui_state.hz_page = LeftOrRight::Left;
    ui_state.active_field = 0;
}

void ui_transition_fieldedit_to_mainmenu() {
    ui_state.layer = Layer::MainMenu;
    ui_state.mm_timeout = MAINMENU_TIMEOUT_S;  // Set inactivity timer
}

void ui_logic_layer_mainmenu(btn_helper btn, uint8_t repeats) {
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
            if (!repeats) {
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
            if (!repeats) {
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

void ui_logic_layer_fieldedit(btn_helper btn, uint8_t repeats) {
    // The maximum value of the active_field mapped by which
    // Main Menu page we're on.
    static const uint8_t MAX_FIELD_VALUE_PAGE_MAP[3] = {2, 5, 1};

    uint8_t max_field_value = MAX_FIELD_VALUE_PAGE_MAP[
        static_cast<uint8_t>(ui_state.mm_page)];

    if (btn.left()) {
        if (ui_state.active_field == 0) {
            if (!repeats) {
                // Allow wrapping if it's not a repeat press
                ui_state.active_field = max_field_value;
            }
        } else {
            // Increment active field index
            ui_state.active_field--;
        }
    } else if (btn.right()) {
        if (ui_state.active_field == max_field_value) {
            if (!repeats) {
                // Allow wrapping if it's not a repeat press
                ui_state.active_field = 0;
            }
        } else {
            // Increment active field index
            ui_state.active_field++;
        }
    }

    /** Important: hz_page has implications for drawing AND logic  */
    if (ui_state.mm_page == MainMenuPage::CountMode) {
        ui_state.hz_page = static_cast<LeftOrRight>(ui_state.active_field);
    } else {
        ui_state.hz_page = static_cast<LeftOrRight>(ui_state.active_field > 2);
    }

    if (btn.down() || btn.up()) {
        uint8_t* field;
        uint8_t f_min;
        uint8_t f_max;
        Date* active_date = nullptr;

        /**
         * Get pointer to the field we're editing (field),
         * and get its minimum/maximum bounds.
         */
        switch (ui_state.mm_page) {
            case MainMenuPage::TargetDate: {
                // Editing a Date: target_date
                active_date = &running_settings.target_date;
                break;
            }
            case MainMenuPage::RtcClock: {
                if (ui_state.hz_page == LeftOrRight::Left) {  // Date
                    // Editing a Date: running_date
                    active_date = &running_date;
                } else {  // Time
                    StdTimeField stf = static_cast<StdTimeField>
                                       (ui_state.active_field - 3);
                    AscendingTimeField atf = to_ascending(stf);
                    field = &running_time.field(atf);
                    f_min = running_time.fields_min_value();
                    f_max = running_time.field_max_value(atf);
                }
                break;
            }
            case MainMenuPage::CountMode: {
                if (ui_state.active_field == 0) {
                    field = reinterpret_cast<uint8_t*>(&running_settings.count_unit);
                } else {
                    field = reinterpret_cast<uint8_t*>(&running_settings.count_direction);
                }
                f_min = 0;
                f_max = 1;
                break;
            }
        }

        /** Are we are editing the Month field of a Date object? */
        bool editing_months = false;

        if (active_date) {
            /** A Date is being edited, so use the Date field accessors. */
            StdDateField sdf = static_cast<StdDateField>
                (ui_state.active_field);
            if (sdf == StdDateField::Month) {
                editing_months = true;
            }
            AscendingDateField adf = to_ascending(sdf);
            field = &(active_date->field(adf));
            f_min = active_date->field_min_value(adf);
            f_max = active_date->field_max_value(adf);
        }

        uint8_t prev_field_val = *field;

        /** Signed value of the field as we're editing it */
        int8_t s_field_val = *field;

        /**
         * True if we should increment/decrement quickly because
         * the user has been holding the button for enough repeats.
         */
        bool fast_repeats = (repeats >= FAST_REPEAT_BUILDUP) && !editing_months;

        // TODO: FIX field_val going negative and wrapping!

        if (!fast_repeats) {
            /** Increment / decrement the field */
            if (btn.up()) {
                s_field_val++;
            } else {
                s_field_val--;
            }
        } else {
            /** Fast repeat mode! */
            if (btn.up()) {
                /** Jump to the next increment */
                uint8_t current_increment = s_field_val / FAST_REPEAT_INCREMENTS;
                s_field_val = (current_increment + 1) * FAST_REPEAT_INCREMENTS;
            } else {
                /** Jump to the previous increment */

                // This will round down any parital-increment amount
                uint8_t increment = s_field_val / FAST_REPEAT_INCREMENTS;
                if ((s_field_val % FAST_REPEAT_INCREMENTS) == 0) {
                    // There is no more partial increment amount, so
                    // go down by one whole increment.
                    increment--;
                }
                s_field_val = increment * FAST_REPEAT_INCREMENTS;
            }
        }

        /** Bounds checking */
        // If this is not a repeat press,
        //    then wrap to the other min/max bound.
        // If this is a repeat press,
        //    then clamp to the bound that was exceeded.
        if (s_field_val < f_min) {
            if (repeats) {
                s_field_val = f_min;  // Repeats: clamp
            } else {
                s_field_val = f_max;  // Real press: wrap
            }
        }
        if (s_field_val > f_max) {
            if (repeats) {
                s_field_val = f_max;  // Repeats: clamp
            } else {
                s_field_val = f_min;  // Real press: wrap
            }
        }

        /** Apply the clamped, edited field value */
        *field = s_field_val;

        // If editing a Date, then always clamp Day Of Month to its max bound,
        // because Day Of Month bounds change Month-to-month or
        // year-to-year (because of leap year).
        if (active_date) {
            uint8_t& day = active_date->day();
            uint8_t max_day = active_date->field_max_value(ADF_DAY);
            if (day > max_day) {
                day = max_day;
            }
        }

        // Redraw screen and recount display if the active field has changed.
        if (*field != prev_field_val) {
            ui_state.dirty = true;
            busy_flags |= BF_RECOUNT;
        }
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

    /** Reset general-purpose redraw flag */
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
            state->mm_scrollbar_top = 0;  // TODO: collapse to array?
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
            ssd1306_printFixedN(0, 16, ui_strbuf, STYLE_NORMAL, 0);

            // "Time: HH:MM:SS" -> 15 chars w/term
            memcpy_P(ui_strbuf, PSTR("Tim"), 3);
            generate_time_string(ui_strbuf + 6, running_time);
            ssd1306_printFixedN(0, 24, ui_strbuf, STYLE_NORMAL, 0);
            break;
        }
        case MainMenuPage::CountMode: {
            state->mm_scrollbar_top = 20;
            state->mmpg_header_width = (6*16);

            // Draw the header
            ssd1306_printFixedN(0, 0, "* SET COUNT MODE", STYLE_BOLD, 0);

            memcpy_P(ui_strbuf, PSTR("Unit: "), 6);
            uint8_t field_val = static_cast<uint8_t>(running_settings.count_unit);
            memcpy_P(ui_strbuf + 6, CountUnitLabels + (field_val * 5), 5);
            ui_strbuf[11] = '\0';
            ssd1306_printFixedN(0, 16, ui_strbuf, STYLE_NORMAL, 0);

            memcpy_P(ui_strbuf, PSTR("Target: "), 8);
            field_val = static_cast<uint8_t>(running_settings.count_direction);
            memcpy_P(ui_strbuf + 8, CountDirectionLabels + (field_val * 6), 6);
            ui_strbuf[14] = '\0';
            ssd1306_printFixedN(0, 24, ui_strbuf, STYLE_NORMAL, 0);
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

/** X-position of the arrows for each field when editing a Date */
static const uint8_t fe_date_arrow_pos[3] = {15, 57, 105};

/** X-position of the arrows for each field when editing a Time */
static const uint8_t fe_time_arrow_pos[3] = {9+15, 45+15, 81+15};

/** Whether an arrow image is pointing up or down */
enum class ArrowDirection : uint8_t {
    Up,
    Down
};



/**
 * @brief Procedurally draw an arrow given its direction (up/down)
 * and its top-left coordinates.
 * 
 * The arrow is 8 pixels tall and 9 pixels wide.
 * Using this function instead of drawBitmap reduces the program size,
 * since drawBitmap isn't used anywhere else.
 * 
 * @param direction: Whether this arrow points up or down
 * @param x: Left-aligned position of the arrow in pixels
 * @param y: Y-position of the arrow in blocks (8 pixel units).
 * 
*/
void draw_arrow(ArrowDirection direction, uint8_t x, uint8_t y) {
    /** Draw the arrow with vertical lines */
    for (int i = 0; i < 9; i++) {
        /** Height of the arrow in pixels */
        // y = 4 - |4 - x|
        uint8_t h = 4 - abs(4 - i);
        // h ranges between 0 and 4, inclusive.

        uint8_t top_offset;     // Line top y-coordinate relative to `y`
        uint8_t bottom_offset;  // Line bottom y-coordinate relative to `y`

        if (direction == ArrowDirection::Up) {
            // Up arrow: the arrow is aligned to the bottom of the block
            top_offset = 4 - h;
            bottom_offset = 4;
        } else {
            // Down arrow: the arrow is aligned to the top of the block
            top_offset = 2;
            bottom_offset = 2 + h;
        }

        ssd1306_drawVLine(x + i, y + top_offset, y + bottom_offset);
    }
}

void ui_draw_layer_fieldedit(UIState* state, const UIState& last_state) {
    // Clear screen if the "horizontal page" changed
    if (state->hz_page != last_state.hz_page) {
        ssd1306_clearScreen();
    }

    // The x position of the left of the big main text.
    uint8_t main_text_x = 0;

    switch (state->mm_page) {
        case MainMenuPage::TargetDate:
        case MainMenuPage::RtcClock: {
            if (state->hz_page == LeftOrRight::Left) {  // Left page: Date
                // date
                state->fe_arrow_left = fe_date_arrow_pos[state->active_field];

                // "mmm DD 'YY" -> 11 chars w/term
                if (state->mm_page == MainMenuPage::TargetDate) {
                    // We're editing the target date, so show that.
                    generate_date_string(ui_strbuf, running_settings.target_date);
                } else {
                    // We're editing the RTC date, so show that.
                    generate_date_string(ui_strbuf, running_date);
                }
            } else {  // Right page: Time
                // time
                state->fe_arrow_left = fe_time_arrow_pos[state->active_field - 3];

                // "HH:MM:SS" -> 9 chars w/term
                generate_time_string(ui_strbuf, running_time);
                main_text_x = 15;
            }
            break;
        }
        case MainMenuPage::CountMode: {



            if (state->hz_page == LeftOrRight::Left) {  // Left page: Count Unit
                uint8_t field_val = static_cast<uint8_t>
                                    (running_settings.count_unit);
                memcpy_P(ui_strbuf, CountUnitLabels + (field_val * 5), 5);
                main_text_x = 40;
                state->fe_arrow_left = 46;
                ui_strbuf[5] = '\0';  // terminating character
                ssd1306_printFixedN(0, 0, "Units", STYLE_NORMAL, 0);
            } else {  // Right page: Count Direction
                uint8_t field_val = static_cast<uint8_t>
                                    (running_settings.count_direction);
                memcpy_P(ui_strbuf, CountDirectionLabels + (field_val * 6), 6);
                main_text_x = 24;
                state->fe_arrow_left = 30;
                ui_strbuf[6] = '\0';  // terminating character
                ssd1306_printFixedN(0, 0, "Direction", STYLE_NORMAL, 0);
            }
            break;
        }
    }

    // Draw the big main text
    ssd1306_printFixedN(main_text_x, 8, ui_strbuf, STYLE_NORMAL, 1);

                    // TODO: fix visual glitch with ellipses
    /** Draw ellipses to show there's more fields to the left/right */
    if (state->mm_page != MainMenuPage::TargetDate) {   // If ellipses_x != 255
        uint8_t ellipses_x = 0;
        if (state->hz_page == LeftOrRight::Left) {
            ellipses_x = 110;
        }
        ssd1306_printFixedN(ellipses_x, 24, "...", STYLE_NORMAL, 0);
    }

    /** Redraw arrows if they moved, or if we just entered FieldEdit layer */
    if (state->fe_arrow_left != last_state.fe_arrow_left ||
            state->layer != last_state.layer) {
        /** Clear arrows at the previous location */
        ssd1306_clearBlock(last_state.fe_arrow_left, 0, 9, 8);
        ssd1306_clearBlock(last_state.fe_arrow_left, 3, 9, 8);
        /** Draw arrows at the new location */
        draw_arrow(ArrowDirection::Up, state->fe_arrow_left, 0);
        draw_arrow(ArrowDirection::Down, state->fe_arrow_left, 24);
    }
}
