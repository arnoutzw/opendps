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
 * @file uui.h
 * @brief Universal User Interface Framework for OpenDPS
 *
 * This module provides the user interface framework for OpenDPS. It implements
 * a screen-based UI system where each operating function (CV, CC, CL, etc.)
 * has its own screen with configurable UI items.
 *
 * ## Architecture Overview
 *
 * The UI is organized hierarchically:
 * - **uui_t**: The top-level UI container holding multiple screens
 * - **ui_screen_t**: A screen representing an operating function
 * - **ui_item_t**: A UI element on a screen (number input, icon, etc.)
 *
 * ```
 * uui_t (Application UI)
 *   |
 *   +-- ui_screen_t (CV Screen)
 *   |     +-- ui_item_t (Voltage input)
 *   |     +-- ui_item_t (Current limit input)
 *   |     +-- ui_item_t (Power icon)
 *   |
 *   +-- ui_screen_t (CC Screen)
 *         +-- ui_item_t (Current input)
 *         +-- ui_item_t (Voltage limit input)
 * ```
 *
 * ## Item Types
 *
 * - **ui_item_number**: Editable numeric value (see uui_number.h)
 * - **ui_item_icon**: Static or animated icon (see uui_icon.h)
 *
 * ## Focus System
 *
 * Items can receive focus for editing:
 * - Only items with `can_focus=true` can be selected
 * - Rotary encoder moves between focusable items
 * - When focused, encoder adjusts the item's value
 * - SEL button confirms changes
 *
 * ## Event Flow
 *
 * 1. Hardware events (buttons, encoder) generate event_t values
 * 2. Events are passed to uui_handle_screen_event()
 * 3. The current screen's focused item processes the event
 * 4. Items update their state and request redraw
 *
 * ## Parameter System
 *
 * Each screen can have named parameters for remote control:
 * - Parameters are accessed by name (e.g., "voltage", "current")
 * - set_parameter() and get_parameter() callbacks handle values
 * - Parameters have units and SI prefixes for proper formatting
 *
 * @see event.h for event types
 * @see uui_number.h for numeric input items
 * @see uui_icon.h for icon items
 */

#ifndef __UUI_H__
#define __UUI_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "event.h"
#include "tick.h"
#include "pwrctl.h"
#include "past.h"

/**
 * @def MAX_SCREENS
 * @brief Maximum number of screens in the UI
 *
 * Can be overridden by defining CONFIG_UI_MAX_SCREENS at compile time.
 */
#ifdef CONFIG_UI_MAX_SCREENS
 #define MAX_SCREENS (CONFIG_UI_MAX_SCREENS)
#else
 #define MAX_SCREENS (6)
#endif

/**
 * @def MAX_PARAMETERS
 * @brief Maximum number of parameters per screen
 *
 * Can be overridden by defining CONFIG_UI_MAX_PARAMETERS at compile time.
 */
#ifdef CONFIG_UI_MAX_PARAMETERS
 #define MAX_PARAMETERS (CONFIG_UI_MAX_PARAMETERS)
#else
 #define MAX_PARAMETERS (6)
#endif

/**
 * @def MAX_PARAMETER_NAME
 * @brief Maximum length of parameter name strings
 *
 * Can be overridden by defining CONFIG_UI_MAX_PARAMETER_NAME at compile time.
 */
#ifdef CONFIG_UI_MAX_PARAMETER_NAME
 #define MAX_PARAMETER_NAME (CONFIG_UI_MAX_PARAMETER_NAME)
#else
 #define MAX_PARAMETER_NAME (10)
#endif

/**
 * @def XPOS_ICON
 * @brief X position for the screen/function icon on the status bar
 */
#define XPOS_ICON    (43)


/**
 * @brief Physical units for parameter values
 *
 * Describes the physical unit of a parameter value for proper formatting
 * and display. Keep in sync with dpsctl/dpsctl.py: def unit_name(unit).
 */
typedef enum {
    unit_none = 0,    /**< No unit (dimensionless) */
    unit_ampere,      /**< Current in amperes (A) */
    unit_volt,        /**< Voltage in volts (V) */
    unit_watt,        /**< Power in watts (W) */
    unit_second,      /**< Time in seconds (s) */
    unit_hertz,       /**< Frequency in hertz (Hz) */
    unit_furlong,     /**< Length in furlongs (for testing) */
    unit_last = 0xff  /**< Sentinel value */
} unit_t;

/**
 * @brief SI unit prefixes for scaling
 *
 * Represents SI prefixes as powers of 10. Used to scale parameter
 * values for display and conversion (e.g., mV = millivolts, si_milli).
 */
typedef enum {
    si_micro = -6,    /**< 10^-6 (micro, μ) */
    si_milli = -3,    /**< 10^-3 (milli, m) */
    si_centi = -2,    /**< 10^-2 (centi, c) */
    si_deci = -1,     /**< 10^-1 (deci, d) */
    si_none = 0,      /**< 10^0 (no prefix) */
    si_deca = 1,      /**< 10^1 (deca, da) */
    si_hecto = 2,     /**< 10^2 (hecto, h) */
    si_kilo = 3,      /**< 10^3 (kilo, k) */
    si_mega = 4,      /**< 10^6 (mega, M) */
} si_prefix_t;

/**
 * @brief UI item type identifiers
 *
 * Identifies the specific type of a UI item for proper handling
 * and type-safe casting.
 */
typedef enum {
    /** @brief Numeric value input control (ui_number_t) */
    ui_item_number,
    /** @brief Icon display control (ui_icon_t) */
    ui_item_icon,
    /** @brief Sentinel value for iteration */
    ui_item_last = 0xff
} ui_item_type_t;

/**
 * @brief Text alignment options for UI elements
 */
typedef enum {
    ui_text_left_aligned,   /**< Align text to the left edge */
    ui_text_right_aligned   /**< Align text to the right edge */
} ui_text_alignment_t;

/**
 * @brief Return status codes for set_parameter operations
 *
 * These codes indicate the result of attempting to set a parameter
 * value, either locally or via remote control.
 */
typedef enum {
    ps_ok = 0,          /**< Parameter was set successfully */
    ps_unknown_name,    /**< Parameter name not recognized */
    ps_range_error,     /**< Value is outside valid range */
    ps_not_supported,   /**< Parameter cannot be modified */
    ps_flash_error,     /**< Error writing to persistent storage */
} set_param_status_t;

/**
 * @brief Parameter descriptor structure
 *
 * Describes a named parameter that can be accessed via the remote
 * control protocol. Each screen has an array of these descriptors
 * defining its configurable parameters.
 */
typedef struct ui_parameter_t {
    char name[MAX_PARAMETER_NAME];  /**< Parameter name (e.g., "voltage") */
    unit_t unit;                    /**< Physical unit (e.g., unit_volt) */
    si_prefix_t prefix;             /**< SI prefix (e.g., si_milli for mV) */
} ui_parameter_t;

/* Forward declaration for circular reference */
typedef struct ui_screen ui_screen_t;

/**
 * @brief Base class for UI items
 *
 * This is the base structure for all UI items. Specific item types
 * (ui_number_t, ui_icon_t) extend this structure with additional fields.
 *
 * The function pointers provide polymorphic behavior:
 * - got_focus: Called when item receives input focus
 * - lost_focus: Called when item loses input focus
 * - got_event: Called to process user input events
 * - get_value: Returns the item's current value
 * - draw: Renders the item on the display
 */
typedef struct ui_item_t {
    uint8_t id;                 /**< Unique item identifier within screen */
    ui_item_type_t type;        /**< Item type (ui_item_number, ui_item_icon) */
    bool can_focus;             /**< True if item can receive focus for editing */
    bool has_focus;             /**< True if item currently has input focus */
    bool needs_redraw;          /**< True if item needs to be redrawn */
    uint16_t x, y;              /**< Position on screen (top-left corner) */
    ui_screen_t *screen;        /**< Parent screen containing this item */

    /** @brief Called when item receives input focus */
    void (*got_focus)(struct ui_item_t *item);
    /** @brief Called when item loses input focus */
    void (*lost_focus)(struct ui_item_t *item);
    /** @brief Called to process a user input event */
    void (*got_event)(struct ui_item_t *item, event_t event);
    /** @brief Returns the item's current value (type-specific interpretation) */
    uint32_t (*get_value)(struct ui_item_t *item);
    /** @brief Renders the item on the TFT display */
    void (*draw)(struct ui_item_t *item);
} ui_item_t;

/**
 * @def MCALL
 * @brief Macro for calling operations on UI elements
 *
 * Provides a convenient way to call method-like functions on UI items
 * with proper type casting. Supports variable arguments for methods
 * that take parameters beyond the item pointer.
 *
 * @param item Pointer to the UI item
 * @param operation Name of the operation/function to call
 * @param ... Additional arguments to pass to the operation
 */
#define MCALL(item, operation, ...) ((ui_item_t*) (item))->operation((ui_item_t*) item, ##__VA_ARGS__)

/**
 * @brief Screen structure representing an operating function
 *
 * A screen corresponds to an operating function like CV (Constant Voltage),
 * CC (Constant Current), etc. Each screen has its own UI items, parameters,
 * and callback functions.
 *
 * Screens are registered with the UI using uui_add_screen() and can be
 * switched using uui_next_screen(), uui_prev_screen(), or uui_set_screen().
 */
struct ui_screen {
    uint8_t id;                     /**< Unique screen ID (must be unique across all screens) */
    char *name;                     /**< Screen name (e.g., "cv", "cc") for remote control */
    uint8_t *icon_data;             /**< Pointer to icon bitmap data */
    uint32_t icon_data_len;         /**< Length of icon data in bytes */
    uint32_t icon_width;            /**< Icon width in pixels */
    uint32_t icon_height;           /**< Icon height in pixels */
    bool is_enabled;                /**< True if power output is enabled for this screen */
    uint8_t num_items;              /**< Number of UI items on this screen */
    uint8_t cur_item;               /**< Index of currently focused item */
    ui_parameter_t parameters[MAX_PARAMETERS];  /**< Parameter descriptors */

    /** @brief Called when the screen becomes active (switched to) */
    void (*activated)(void);
    /** @brief Called when switching away from this screen */
    void (*deactivated)(void);
    /** @brief Called when the enable button is pressed */
    void (*enable)(bool _enable);
    /** @brief Called periodically for housekeeping (e.g., display updates) */
    void (*tick)(void);
    /** @brief Called to save screen state to persistent storage */
    void (*past_save)(past_t *past);
    /** @brief Called to restore screen state from persistent storage */
    void (*past_restore)(past_t *past);
    /** @brief Called to set a parameter value by name */
    set_param_status_t (*set_parameter)(char *name, char *value);
    /** @brief Called to get a parameter value by name */
    set_param_status_t (*get_parameter)(char *name, char *value, uint32_t value_len);
    /** @brief Flexible array of UI items on this screen */
    ui_item_t *items[];
};

/**
 * @brief Top-level UI container structure
 *
 * The uui_t structure is the root of the UI hierarchy, containing
 * all screens and managing screen transitions and visibility.
 */
typedef struct {
    uint8_t num_screens;            /**< Number of registered screens */
    uint8_t cur_screen;             /**< Index of currently active screen */
    bool is_visible;                /**< True if UI is visible (not hidden) */
    ui_screen_t *screens[MAX_SCREENS];  /**< Array of registered screens */
    past_t *past;                   /**< Persistent storage for settings */
} uui_t;

/**
 * @brief Initialize the UUI instance
 *
 * Initializes the UI framework with the given persistent storage.
 * Must be called before adding screens or handling events.
 *
 * @param[in,out] ui   Pointer to the UI structure to initialize
 * @param[in]     past Pointer to initialized PAST structure for persistence
 */
void uui_init(uui_t *ui, past_t *past);

/**
 * @brief Refresh items needing redraw on the current screen
 *
 * Iterates through all items on the current screen and redraws
 * those that have their needs_redraw flag set.
 *
 * @param[in] ui    Pointer to the UI structure
 * @param[in] force If true, redraws all items regardless of needs_redraw flag
 */
void uui_refresh(uui_t *ui, bool force);

/**
 * @brief Activate the current screen
 *
 * Calls the activated() callback of the current screen and draws
 * all items. Should be called after uui_init() or after screen changes.
 *
 * @param[in] ui Pointer to the UI structure
 */
void uui_activate(uui_t *ui);

/**
 * @brief Add a screen to the UI
 *
 * Registers a screen with the UI. Screens should be added in the
 * desired order during initialization.
 *
 * @param[in,out] ui     Pointer to the UI structure
 * @param[in]     screen Pointer to the screen to add
 *
 * @note Maximum of MAX_SCREENS screens can be added
 */
void uui_add_screen(uui_t *ui, ui_screen_t *screen);

/**
 * @brief Process a user input event
 *
 * Routes an event to the appropriate handler based on the event type
 * and current screen/focus state.
 *
 * @param[in] ui    Pointer to the UI structure
 * @param[in] event The event to process
 *
 * @see event_t for available event types
 */
void uui_handle_screen_event(uui_t *ui, event_t event);

/**
 * @brief Switch to the next screen
 *
 * Deactivates the current screen and activates the next one in sequence.
 * Wraps around to the first screen after the last.
 *
 * @param[in,out] ui Pointer to the UI structure
 */
void uui_next_screen(uui_t *ui);

/**
 * @brief Switch to the previous screen
 *
 * Deactivates the current screen and activates the previous one in sequence.
 * Wraps around to the last screen before the first.
 *
 * @param[in,out] ui Pointer to the UI structure
 */
void uui_prev_screen(uui_t *ui);

/**
 * @brief Switch to a specific screen by index
 *
 * Deactivates the current screen and activates the screen at the
 * specified index.
 *
 * @param[in,out] ui         Pointer to the UI structure
 * @param[in]     screen_idx Index of the screen to activate (0-based)
 */
void uui_set_screen(uui_t *ui, uint32_t screen_idx);

/**
 * @brief Initialize a UI item
 *
 * Performs base initialization for a UI item. Should be called by
 * specific item type initializers (e.g., number_init, icon_init).
 *
 * @param[out] item Pointer to the item to initialize
 */
void ui_item_init(ui_item_t *item);

/**
 * @brief Periodic tick handler for the UI
 *
 * Should be called regularly from the main loop. Calls the tick()
 * callback of the current screen for periodic updates.
 *
 * @param[in] ui Pointer to the UI structure
 */
void uui_tick(uui_t *ui);

/**
 * @brief Show or hide the UI
 *
 * Controls visibility of the entire UI. When hidden, the UI is not
 * drawn and does not process events.
 *
 * @param[in,out] ui   Pointer to the UI structure
 * @param[in]     show true to show the UI, false to hide it
 */
void uui_show(uui_t *ui, bool show);

/**
 * @brief Disable power output for the current screen
 *
 * Calls the enable(false) callback of the current screen and updates
 * its is_enabled flag.
 *
 * @param[in,out] ui Pointer to the UI structure
 */
void uui_disable_cur_screen(uui_t *ui);

#endif // __UUI_H__
