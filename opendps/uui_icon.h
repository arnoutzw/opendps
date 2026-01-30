/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Cyril Russo (github.com/X-Ryl669)
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
 * @file uui_icon.h
 * @brief Selectable Icon UI Widget
 *
 * This module provides a selectable icon widget for the OpenDPS user
 * interface. It displays one icon from a set of icons, with the ability
 * to cycle through them using the encoder.
 *
 * ## Features
 *
 * - Multiple icon support with index-based selection
 * - Encoder-based icon cycling when focused
 * - Customizable colors
 * - Fixed-size icons for consistent layout
 * - Change notification callback
 *
 * ## Icon Format
 *
 * Icons are stored as arrays of bitmap data. All icons in a set must
 * have the same dimensions. The format matches the TFT blit format
 * (typically 1-bit monochrome expanded during rendering).
 *
 * ## Use Cases
 *
 * - Waveform selection (sine, square, triangle, sawtooth)
 * - Mode indicators
 * - Status icons
 * - Any multi-choice graphical selector
 *
 * ## Usage Example
 *
 * ```c
 * // Waveform icons for function generator
 * static const uint8_t *waveform_icons[] = {
 *     gfx_sin, gfx_square, gfx_saw, gfx_triangle
 * };
 *
 * static ui_icon_t waveform_selector = {
 *     .ui = {
 *         .x = 10,
 *         .y = 30,
 *         .type = ui_item_icon,
 *         .can_focus = true,
 *     },
 *     .color = WHITE,
 *     .icons_width = 24,
 *     .icons_height = 24,
 *     .icons_data_len = 72,  // width * height / 8 for 1bpp
 *     .num_icons = 4,
 *     .value = 0,  // Start with sine
 *     .changed = on_waveform_changed,
 *     .icons = { gfx_sin, gfx_square, gfx_saw, gfx_triangle },
 * };
 * icon_init(&waveform_selector);
 * ```
 *
 * @see uui.h for the UI framework
 * @see uui_number.h for numeric input widget
 * @see gfx-*.h for icon bitmap data
 */

#ifndef __UUI_ICON_H__
#define __UUI_ICON_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "tft.h"
#include "uui.h"

/**
 * @brief Selectable icon UI item structure
 *
 * Represents a graphical selector that displays one icon from a set.
 * The user can cycle through icons using the encoder when the item
 * is focused.
 *
 * ## Memory Layout
 *
 * This structure uses a flexible array member for the icons pointers.
 * Allocate as:
 * ```c
 * static ui_icon_t my_icon = {
 *     // ... fields ...
 *     .icons = { icon1, icon2, icon3 },
 * };
 * ```
 *
 * @note Initialize with icon_init() before use
 * @note The structure must be statically allocated due to flexible array
 */
typedef struct ui_icon_t {
    /** @brief Base UI item (must be first for polymorphism) */
    ui_item_t ui;
    /** @brief Icon display color in BGR565 format */
    uint16_t color;
    /** @brief Size of each icon's bitmap data in bytes */
    uint32_t icons_data_len;
    /** @brief Width of each icon in pixels */
    uint32_t icons_width;
    /** @brief Height of each icon in pixels */
    uint32_t icons_height;
    /** @brief Currently selected icon index (0 to num_icons-1) */
    uint32_t value;
    /** @brief Total number of icons in the set */
    uint32_t num_icons;
    /**
     * @brief Callback invoked when selection changes
     * @param item Pointer to this icon item
     *
     * Called after each icon selection change from user input.
     * Use this to apply the new selection (e.g., change waveform type).
     */
    void (*changed)(struct ui_icon_t *item);
    /** @brief Array of pointers to icon bitmap data (flexible array) */
    const uint8_t *icons[];
} ui_icon_t;

/**
 * @brief Initialize an icon UI item
 *
 * Sets up the function pointers for the icon widget's draw, focus,
 * and event handling callbacks. Must be called before the item is
 * added to a screen.
 *
 * @param[in,out] item The icon item to initialize
 *
 * @note All fields including the icons array should be set before calling
 * @note The item's ui.type should be set to ui_item_icon
 */
void icon_init(ui_icon_t *item);

#endif // __UUI_ICON_H__
