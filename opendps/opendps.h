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
 * @file opendps.h
 * @brief Main OpenDPS application interface
 *
 * This header defines the main application programming interface for OpenDPS,
 * a FOSS firmware replacement for DPS5005 and compatible programmable power
 * supplies (DPS3003, DPS3005, DPS5015, DPS5020, DP50V5A).
 *
 * OpenDPS provides:
 * - Multiple operating modes (CV, CC, CL, Function Generator)
 * - Remote control via serial/WiFi using a binary protocol
 * - TFT display with user interface
 * - Calibration and settings persistence
 *
 * The main functions in this module handle:
 * - Function (operating mode) selection and management
 * - Parameter get/set operations
 * - Power output control
 * - Status and temperature monitoring
 * - UI state management (lock, screen changes)
 * - Firmware upgrade initiation
 *
 * @see protocol.h for communication protocol details
 * @see uui.h for user interface framework
 * @see pwrctl.h for power control implementation
 */

#ifndef __OPENDPS_H__
#define __OPENDPS_H__

#include <stdint.h>
#include <stdbool.h>
#include "protocol.h"

/**
 * @def OPENDPS_MAX_PARAMETERS
 * @brief Maximum number of parameters that can be passed to a function
 *
 * This defines the upper limit on the number of parameters that can be
 * configured for any operating function (CV, CC, CL, etc.). Each function
 * may use fewer parameters, but cannot exceed this limit.
 */
#define OPENDPS_MAX_PARAMETERS  (8)

/**
 * @brief Enable the specified operating function by index
 *
 * Switches the power supply to a different operating mode (function) such as
 * Constant Voltage (CV), Constant Current (CC), Current Limit (CL), or
 * Function Generator. The function index corresponds to the order in which
 * functions were registered during initialization.
 *
 * When switching functions:
 * - The current function is deactivated
 * - Power output is disabled for safety
 * - The new function's UI is displayed
 * - The new function is activated
 *
 * @param[in] func_idx Zero-based index of the function to enable
 * @return true if the function was successfully enabled
 * @return false if the index is out of range or the function failed to enable
 *
 * @note The function index must be less than the number of registered functions
 * @see opendps_get_function_names() to get the list of available functions
 */
bool opendps_enable_function_idx(uint32_t func_idx);

/**
 * @brief Get the list of available function names
 *
 * Retrieves the names of all registered operating functions. This is typically
 * used by the remote control protocol to list available modes.
 *
 * Example function names: "cv", "cc", "cl", "funcgen"
 *
 * @param[out] names Array of char pointers to receive function name strings
 * @param[in]  size  Maximum number of names the array can hold
 * @return Number of function names written to the array
 *
 * @note The returned pointers point to static strings and must not be freed
 * @note The array should have at least MAX_SCREENS elements
 */
uint32_t opendps_get_function_names(char* names[], size_t size);

/**
 * @brief Get the name of the currently active function
 *
 * Returns the name string of the currently active operating mode.
 *
 * @return Pointer to the current function name string (e.g., "cv", "cc")
 * @return NULL if no function is active (should not happen in normal operation)
 *
 * @note The returned pointer points to static memory and must not be freed
 */
const char* opendps_get_curr_function_name(void);

/**
 * @brief Get the DPSBoot bootloader git hash
 *
 * Retrieves the git commit hash of the bootloader stored in persistent
 * storage (PAST). This is useful for version tracking and debugging.
 *
 * @param[out] git_hash Pointer to receive the address of the git hash string
 * @return Length of the git hash string on success
 * @return 0 if the git hash is not available
 *
 * @note The git hash is stored in flash during bootloader compilation
 */
uint32_t opendps_get_boot_git_hash(const char** git_hash);

/**
 * @brief Get the OpenDPS application git hash
 *
 * Retrieves the git commit hash of the main application firmware.
 * This is useful for version tracking and debugging.
 *
 * @param[out] git_hash Pointer to receive the address of the git hash string
 * @return Length of the git hash string on success
 * @return 0 if the git hash is not available
 *
 * @note The git hash is embedded at compile time via GIT_VERSION define
 */
uint32_t opendps_get_app_git_hash(const char** git_hash);

/**
 * @brief Get the parameters of the current function
 *
 * Retrieves an array of parameter descriptors for the currently active
 * operating function. Each parameter contains its name, unit, and SI prefix.
 *
 * @param[out] parameters Pointer to receive the address of the parameter array
 * @return Number of parameters in the current function
 *
 * @note The returned pointer points to the function's internal parameter array
 * @see ui_parameter_t for the parameter structure definition
 */
uint32_t opendps_get_curr_function_params(ui_parameter_t **parameters);

/**
 * @brief Get the value of a named parameter from the current function
 *
 * Retrieves the current value of a parameter as a formatted string.
 * This is used by the remote control protocol to query parameter values.
 *
 * @param[in]  name      Name of the parameter to query (e.g., "voltage", "current")
 * @param[out] value     Buffer to receive the string representation of the value
 * @param[in]  value_len Size of the value buffer in bytes
 * @return true if the parameter exists and its value was written
 * @return false if the parameter does not exist in the current function
 *
 * @note The value string format depends on the parameter type and unit
 */
bool opendps_get_curr_function_param_value(char *name, char *value, uint32_t value_len);

/**
 * @brief Set a parameter to a new value
 *
 * Sets the value of a named parameter in the current function. The value
 * is parsed from a string representation. This is the primary interface
 * for remote control parameter setting.
 *
 * @param[in] name  Name of the parameter to set (e.g., "voltage", "current")
 * @param[in] value New value as a string (e.g., "5000" for 5V in mV)
 * @return ps_ok if the parameter was successfully set
 * @return ps_unknown_name if the parameter name is not recognized
 * @return ps_range_error if the value is outside valid bounds
 * @return ps_not_supported if the parameter cannot be modified
 *
 * @see set_param_status_t for all possible return values
 */
set_param_status_t opendps_set_parameter(char *name, char *value);

/**
 * @brief Set calibration data for ADC/DAC conversion
 *
 * Updates the calibration coefficients used for converting between
 * raw ADC/DAC values and real-world voltage/current values.
 * The new calibration data is stored in persistent storage.
 *
 * Valid calibration variable names:
 * - "A_ADC_K", "A_ADC_C" - Current ADC slope and offset
 * - "A_DAC_K", "A_DAC_C" - Current DAC slope and offset
 * - "V_ADC_K", "V_ADC_C" - Voltage ADC slope and offset
 * - "V_DAC_K", "V_DAC_C" - Voltage DAC slope and offset
 * - "VIN_ADC_K", "VIN_ADC_C" - Input voltage ADC slope and offset
 *
 * @param[in] name  Name of the calibration variable to set
 * @param[in] value Pointer to the float value to set
 * @return ps_ok if the calibration was successfully set
 * @return ps_unknown_name if the calibration name is not recognized
 * @return ps_flash_error if writing to persistent storage failed
 */
set_param_status_t opendps_set_calibration(char *name, float *value);

/**
 * @brief Clear all calibration data and restore defaults
 *
 * Erases all stored calibration coefficients from persistent storage,
 * causing the device to use the default calibration values defined
 * in dps-model.h for the specific hardware model.
 *
 * @return true if calibration data was successfully cleared
 * @return false if erasing from persistent storage failed
 *
 * @warning After clearing calibration, measurements may be inaccurate
 *          until the device is recalibrated
 */
bool opendps_clear_calibration(void);

/**
 * @brief Enable or disable power output
 *
 * Controls the power output of the current operating function.
 * When enabled, the power supply outputs voltage/current according
 * to the current function's settings.
 *
 * @param[in] enable true to enable output, false to disable
 * @return true if the output state was successfully changed
 * @return false if the operation failed (e.g., UI is locked)
 *
 * @note Output is automatically disabled when:
 *       - Switching between functions
 *       - OCP (Over Current Protection) triggers
 *       - Temperature alarm activates
 */
bool opendps_enable_output(bool enable);

/**
 * @brief Update the power enable status indicator on the display
 *
 * Updates the power status icon on the TFT display to reflect
 * whether power output is currently enabled or disabled.
 *
 * @param[in] enabled true if power output is enabled, false otherwise
 *
 * @note This is typically called after opendps_enable_output()
 */
void opendps_update_power_status(bool enabled);

/**
 * @brief Update the WiFi status indicator on the display
 *
 * Updates the WiFi status icon on the TFT display to show the
 * current connection state of the ESP8266 WiFi module.
 *
 * @param[in] status Current WiFi connection status
 *
 * @see wifi_status_t for possible status values:
 *      - wifi_off: WiFi module not active
 *      - wifi_connecting: Connection in progress
 *      - wifi_connected: Successfully connected
 *      - wifi_error: Connection error
 *      - wifi_upgrading: Firmware upgrade in progress
 */
void opendps_update_wifi_status(wifi_status_t status);

/**
 * @brief Handle a ping command from the remote control
 *
 * Processes a ping request, typically used to check if the device
 * is responsive. May also update the WiFi status indicator.
 *
 * @note This function is called by the protocol handler
 */
void opendps_handle_ping(void);

/**
 * @brief Lock or unlock the user interface
 *
 * When locked, the UI ignores button presses and rotary encoder input,
 * preventing accidental changes. A padlock icon is displayed when locked.
 * This is typically used for remote control scenarios where the user
 * should not accidentally change settings.
 *
 * @param[in] lock true to lock the UI, false to unlock
 *
 * @note Locking can be triggered via remote command or button combination
 */
void opendps_lock(bool lock);

/**
 * @brief Lock or unlock the UI due to temperature alarm
 *
 * Similar to opendps_lock() but specifically for thermal protection.
 * When a temperature alarm is triggered, this function locks the UI
 * and disables output to protect the device.
 *
 * @param[in] lock true to lock due to high temperature, false to unlock
 *
 * @note This is an automatic safety feature, not user-controllable
 * @note The device must cool down before the lock can be released
 */
void opendps_temperature_lock(bool lock);

/**
 * @brief Set temperature sensor readings
 *
 * Updates the internal temperature values received from external
 * temperature sensors (typically connected via ESP8266). These values
 * are used for thermal monitoring and protection.
 *
 * @param[in] temp1 First temperature reading (in 0.1°C units)
 * @param[in] temp2 Second temperature reading (in 0.1°C units)
 *
 * @note A value of INVALID_TEMPERATURE (0xFFFF) indicates no sensor
 * @note Temperature unit interpretation is user-defined (C, F, or K)
 * @see opendps_get_temperature() to read current temperatures
 */
void opendps_set_temperature(int16_t temp1, int16_t temp2);

/**
 * @brief Get current temperature sensor readings
 *
 * Retrieves the most recent temperature sensor readings and
 * whether a thermal shutdown has occurred.
 *
 * @param[out] temp1 Pointer to receive first temperature (0.1°C units)
 * @param[out] temp2 Pointer to receive second temperature (0.1°C units)
 * @param[out] temp_shutdown Pointer to receive thermal shutdown status
 *
 * @note NULL may be passed for any parameter that is not needed
 * @note A value of INVALID_TEMPERATURE (0xFFFF) indicates no sensor
 */
void opendps_get_temperature(int16_t *temp1, int16_t *temp2, bool *temp_shutdown);

/**
 * @brief Initiate firmware upgrade process
 *
 * Called when a firmware upgrade is requested via the serial protocol.
 * This function prepares the device for upgrade by:
 * 1. Saving upgrade parameters to the bootcom area
 * 2. Setting the upgrade magic number
 * 3. Performing a system reset to enter the bootloader
 *
 * The bootloader will then handle the actual firmware download.
 *
 * @note This function does not return - the device resets
 * @see protocol.h for upgrade protocol details
 */
void opendps_upgrade_start(void);

/**
 * @brief Switch to a different screen
 *
 * Changes the currently displayed screen. Screens include the main
 * operating screen and settings/calibration screens.
 *
 * @param[in] screen_id ID of the screen to switch to:
 *                      - 0: Main operating screen
 *                      - 1: Settings/calibration screen
 * @return true if the screen was successfully changed
 * @return false if the screen_id is invalid
 */
bool opendps_change_screen(uint8_t screen_id);

#endif // __OPENDPS_H__
