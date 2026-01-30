/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Johan Kanflo (github.com/kanflo)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file uui_number.h
 * @brief Editable Number UI Widget
 *
 * This module provides an editable numeric input widget for the OpenDPS
 * user interface. It displays formatted numbers with configurable digits,
 * decimal places, and SI unit prefixes.
 *
 * ## Features
 *
 * - Configurable number of integer and decimal digits
 * - Per-digit editing with encoder rotation
 * - Minimum and maximum value constraints
 * - SI prefix support (milli, kilo, etc.)
 * - Customizable colors and fonts
 * - Change notification callback
 *
 * ## Display Format
 *
 * Numbers are displayed as: `[digits].[decimals] [prefix][unit]`
 *
 * Examples:
 * - `12.50 V` (2 digits, 2 decimals, voltage)
 * - `1.234 A` (1 digit, 3 decimals, current)
 * - `500 mV` (3 digits, 0 decimals, millivolts)
 *
 * ## Editing Behavior
 *
 * When focused:
 * 1. SEL button cycles through digits (including decimals)
 * 2. Encoder rotation changes the selected digit
 * 3. Value is clamped to min/max automatically
 * 4. Changed callback is invoked after each modification
 *
 * ## Usage Example
 *
 * ```c
 * static ui_number_t voltage_input = {
 *     .ui = {
 *         .x = 10,
 *         .y = 20,
 *         .type = ui_item_number,
 *         .can_focus = true,
 *     },
 *     .unit = unit_volt,
 *     .color = WHITE,
 *     .font_size = FONT_METER_LARGE,
 *     .num_digits = 2,
 *     .num_decimals = 2,
 *     .min = 0,
 *     .max = 50000,  // 50.00V in millivolts
 *     .value = 12000, // 12.00V
 *     .si_prefix = si_milli,
 *     .changed = on_voltage_changed,
 * };
 * number_init(&voltage_input);
 * ```
 *
 * @see uui.h for the UI framework
 * @see uui_icon.h for the icon widget
 * @todo Add support for negative numbers
 */

#ifndef __UUI_NUMBER_H__
#define __UUI_NUMBER_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "tft.h"
#include "uui.h"

/**
 * @brief Editable number UI item structure
 *
 * Represents a numeric input field with formatting options.
 * The number is stored internally in the smallest unit (e.g., millivolts)
 * and displayed with appropriate SI prefix scaling.
 *
 * ## Internal Value Representation
 *
 * The `value` field stores the number in base units without decimal scaling.
 * For example, with si_prefix = si_milli:
 * - value = 12500 displays as "12.50" (for 2 decimals)
 * - value = 5000 displays as "5.00"
 *
 * ## Digit Navigation
 *
 * The `cur_digit` field tracks which digit is being edited:
 * - 0 = leftmost (most significant) digit
 * - Increments rightward through integer and decimal digits
 *
 * @note Initialize with number_init() before use
 * @note The structure should be statically allocated
 */
typedef struct ui_number_t {
    /** @brief Base UI item (must be first for polymorphism) */
    ui_item_t ui;
    /** @brief Physical unit type (volt, ampere, etc.) */
    unit_t unit;
    /** @brief Display color in BGR565 format */
    uint16_t color;
    /** @brief Font size for rendering */
    tft_font_size_t font_size;
    /** @brief Text alignment within bounding box */
    ui_text_alignment_t alignment;
    /** @brief If true, decimal point has same width as digits */
    bool pad_dot;
    /** @brief SI prefix for display scaling (milli, kilo, etc.) */
    si_prefix_t si_prefix;
    /** @brief Number of integer digits to display */
    uint8_t num_digits;
    /** @brief Number of decimal digits to display */
    uint8_t num_decimals;
    /** @brief Currently selected digit index (0 = leftmost) */
    uint8_t cur_digit;
    /** @brief Current value in base units */
    int32_t value;
    /** @brief Minimum allowed value */
    int32_t min;
    /** @brief Maximum allowed value */
    int32_t max;
    /**
     * @brief Callback invoked when value changes
     * @param item Pointer to this number item
     *
     * Called after each value modification from user input.
     * Use this to apply the new value (e.g., set DAC output).
     */
    void (*changed)(struct ui_number_t *item);
} ui_number_t;

/**
 * @brief Initialize a number UI item
 *
 * Sets up the function pointers for the number widget's draw, focus,
 * and event handling callbacks. Must be called before the item is
 * added to a screen.
 *
 * @param[in,out] item The number item to initialize
 *
 * @note All other fields should be set before calling this function
 * @note The item's ui.type should be set to ui_item_number
 */
void number_init(ui_number_t *item);

#endif // __UUI_NUMBER_H__
