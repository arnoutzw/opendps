/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Johan Kanflo (github.com/kanflo)
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
 * @file protocol.h
 * @brief Serial Communication Protocol for OpenDPS
 *
 * This module defines the binary serial interface protocol for controlling
 * OpenDPS remotely. All functionality accessible via physical buttons and
 * the rotary encoder can be controlled through this protocol.
 *
 * ## Protocol Overview
 *
 * The protocol uses framed messages with the following structure:
 * - All frames are wrapped using the uframe framing protocol (see uframe.h)
 * - Commands are sent by the host, responses come from the DPS
 * - Response frames have the MSB set: (cmd_response | original_cmd)
 *
 * Basic frame structure:
 * - Host -> DPS: [cmd] [optional_payload]*
 * - DPS -> Host: [cmd_response | cmd] [success] [response_data]*
 *
 * ## Supported Commands
 *
 * | Command | Description |
 * |---------|-------------|
 * | cmd_ping | Check device connectivity |
 * | cmd_query | Get device status (V_in, V_out, I_out, etc.) |
 * | cmd_set_function | Change operating mode (CV, CC, etc.) |
 * | cmd_list_functions | Get available functions |
 * | cmd_set_parameters | Set function parameters |
 * | cmd_list_parameters | Get function parameters and values |
 * | cmd_enable_output | Turn power output on/off |
 * | cmd_wifi_status | Set WiFi indicator status |
 * | cmd_lock | Lock/unlock the UI |
 * | cmd_temperature_report | Send temperature readings |
 * | cmd_upgrade_start | Begin firmware upgrade |
 * | cmd_upgrade_data | Send firmware data chunk |
 *
 * ## Communication Interfaces
 *
 * The protocol can be used over:
 * - Direct UART connection (115200 baud, 8N1)
 * - WiFi via ESP8266 proxy (TCP socket on port 5005)
 *
 * @see uframe.h for the underlying framing protocol
 * @see dpsctl/protocol.py for the Python implementation
 */

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>
#include <stdbool.h>
#include "uframe.h"

/**
 * @brief Protocol command identifiers
 *
 * These constants identify the different commands that can be sent to
 * and received from the OpenDPS device. Commands from the host are
 * sent as-is; responses have cmd_response OR'd with the command.
 *
 * @note Keep this enum in sync with dpsctl/protocol.py
 */
typedef enum {
    /** @brief Ping the device to check connectivity */
    cmd_ping = 1,
    /** @brief Obsolete: Set output voltage (kept for enum sync) */
    __obsolete_cmd_set_vout,
    /** @brief Obsolete: Set current limit (kept for enum sync) */
    __obsolete_cmd_set_ilimit,
    /** @brief Query device status (voltages, current, power state) */
    cmd_query,
    /** @brief Obsolete: Enable/disable power output (kept for enum sync) */
    __obsolete_cmd_power_enable,
    /** @brief Set WiFi status indicator on display */
    cmd_wifi_status,
    /** @brief Lock or unlock the user interface */
    cmd_lock,
    /** @brief Over Current Protection event notification (DPS->Host) */
    cmd_ocp_event,
    /** @brief Initiate firmware upgrade session */
    cmd_upgrade_start,
    /** @brief Send firmware upgrade data chunk */
    cmd_upgrade_data,
    /** @brief Change the active operating function/mode */
    cmd_set_function,
    /** @brief Enable or disable power output */
    cmd_enable_output,
    /** @brief List available operating functions */
    cmd_list_functions,
    /** @brief Set function parameters (name=value pairs) */
    cmd_set_parameters,
    /** @brief List function parameters and their current values */
    cmd_list_parameters,
    /** @brief Report temperature sensor readings */
    cmd_temperature_report,
    /** @brief Query firmware version (git hashes) */
    cmd_version,
    /** @brief Report calibration data and raw ADC/DAC values */
    cmd_cal_report,
    /** @brief Set calibration coefficients */
    cmd_set_calibration,
    /** @brief Clear all calibration data */
    cmd_clear_calibration,
    /** @brief Change the displayed screen */
    cmd_change_screen,
    /** @brief Set display backlight brightness */
    cmd_set_brightness,
    /** @brief Response flag - OR'd with command in responses */
    cmd_response = 0x80
} command_t;

/**
 * @brief WiFi connection status values
 *
 * Used with cmd_wifi_status to indicate the current state of
 * WiFi connectivity. The DPS displays an appropriate icon.
 */
typedef enum {
    /** @brief WiFi module is off or not present */
    wifi_off = 0,
    /** @brief WiFi is attempting to connect */
    wifi_connecting,
    /** @brief WiFi is successfully connected */
    wifi_connected,
    /** @brief WiFi connection error occurred */
    wifi_error,
    /** @brief WiFi FOTA (firmware over-the-air) upgrade in progress */
    wifi_upgrading
} wifi_status_t;

/**
 * @brief Firmware upgrade status codes
 *
 * These codes are returned during the firmware upgrade process to
 * indicate the current state or any errors that occurred.
 */
typedef enum {
    /** @brief Upgrade proceeding, ready for next chunk */
    upgrade_continue = 0,
    /** @brief Error in bootcom communication area */
    upgrade_bootcom_error,
    /** @brief CRC verification of downloaded firmware failed */
    upgrade_crc_error,
    /** @brief Error while erasing flash memory */
    upgrade_erase_error,
    /** @brief Error while writing to flash memory */
    upgrade_flash_error,
    /** @brief Downloaded firmware would overflow available flash */
    upgrade_overflow_error,
    /** @brief Received upgrade data without upgrade_start */
    upgrade_protocol_error,
    /** @brief Firmware successfully received and verified */
    upgrade_success = 16
} upgrade_status_t;

/**
 * @brief Reasons for entering upgrade mode
 *
 * The bootloader reports why it entered upgrade mode, which helps
 * diagnose issues during the upgrade process.
 */
typedef enum {
    /** @brief Unknown reason for upgrade mode */
    reason_unknown = 0,
    /** @brief User forced upgrade via button press during boot */
    reason_forced,
    /** @brief PAST (persistent storage) initialization failed */
    reason_past_failure,
    /** @brief Application requested upgrade via bootcom */
    reason_bootcom,
    /** @brief Previous upgrade was interrupted and not completed */
    reason_unfinished_upgrade,
    /** @brief Application failed to start properly */
    reason_app_start_failed
} upgrade_reason_t;

/**
 * @brief Status codes for set_parameters command responses
 *
 * Each parameter in a cmd_set_parameters command receives one of
 * these status codes in the response.
 */
typedef enum {
    /** @brief Parameter was successfully set */
    sp_ok = 1,
    /** @brief Parameter name was not recognized */
    sp_unknown_parameter,
    /** @brief Parameter value was out of valid range */
    sp_illegal_value
} set_parameter_status_t;

/**
 * @def INVALID_TEMPERATURE
 * @brief Sentinel value indicating no valid temperature reading
 *
 * When a temperature sensor is not present or has an error,
 * this value is used to indicate the reading is invalid.
 */
#define INVALID_TEMPERATURE (0xffff)

/*
 * =============================================================================
 * Frame Creation Helpers
 * =============================================================================
 *
 * These functions create protocol frames ready for transmission.
 * Each function initializes a frame_t structure with the appropriate
 * command and payload, ready to be sent over UART.
 */

/**
 * @brief Create a response frame
 *
 * Creates a standard response frame with command and success flag.
 *
 * @param[out] frame   Frame structure to initialize
 * @param[in]  cmd     Original command being responded to
 * @param[in]  success 1 for success, 0 for failure
 */
void protocol_create_response(frame_t *frame, command_t cmd, uint8_t success);

/**
 * @brief Create a ping command frame
 *
 * @param[out] frame Frame structure to initialize
 */
void protocol_create_ping(frame_t *frame);

/**
 * @brief Create a power enable/disable command frame
 *
 * @param[out] frame  Frame structure to initialize
 * @param[in]  enable 1 to enable power, 0 to disable
 */
void protocol_create_power_enable(frame_t *frame, uint8_t enable);

/**
 * @brief Create a set voltage command frame
 *
 * @param[out] frame   Frame structure to initialize
 * @param[in]  vout_mv Desired output voltage in millivolts
 */
void protocol_create_vout(frame_t *frame, uint16_t vout_mv);

/**
 * @brief Create a set current limit command frame
 *
 * @param[out] frame     Frame structure to initialize
 * @param[in]  ilimit_ma Desired current limit in milliamps
 */
void protocol_create_ilimit(frame_t *frame, uint16_t ilimit_ma);

/**
 * @brief Create a status query command frame
 *
 * @param[out] frame Frame structure to initialize
 */
void protocol_create_status(frame_t *frame);

/**
 * @brief Create a query response frame with device status
 *
 * Creates a response containing all current device status values.
 *
 * @param[out] frame          Frame structure to initialize
 * @param[in]  v_in           Input voltage in millivolts
 * @param[in]  v_out_setting  Configured output voltage in millivolts
 * @param[in]  v_out          Actual output voltage in millivolts
 * @param[in]  i_out          Output current in milliamps
 * @param[in]  i_limit        Current limit in milliamps
 * @param[in]  power_enabled  1 if power output is enabled, 0 otherwise
 */
void protocol_create_query_response(frame_t *frame, uint16_t v_in,
    uint16_t v_out_setting, uint16_t v_out, uint16_t i_out,
    uint16_t i_limit, uint8_t power_enabled);

/**
 * @brief Create a WiFi status command frame
 *
 * @param[out] frame  Frame structure to initialize
 * @param[in]  status WiFi connection status
 */
void protocol_create_wifi_status(frame_t *frame, wifi_status_t status);

/**
 * @brief Create a lock command frame
 *
 * @param[out] frame  Frame structure to initialize
 * @param[in]  locked 1 to lock UI, 0 to unlock
 */
void protocol_create_lock(frame_t *frame, uint8_t locked);

/**
 * @brief Create an OCP event frame
 *
 * Sent by the DPS when over-current protection triggers.
 *
 * @param[out] frame Frame structure to initialize
 * @param[in]  i_cut Current that triggered OCP in milliamps
 */
void protocol_create_ocp(frame_t *frame, uint16_t i_cut);

/*
 * =============================================================================
 * Frame Unpacking Helpers
 * =============================================================================
 *
 * These functions extract data from received protocol frames.
 * They validate the frame type and extract the payload fields.
 * All functions return true on success, false if the frame type
 * doesn't match or the frame is too short.
 */

/**
 * @brief Unpack a response frame
 *
 * @param[in]  frame   Frame to unpack
 * @param[out] cmd     Original command that was responded to
 * @param[out] success Success flag from response
 * @return true if unpacking succeeded, false otherwise
 */
bool protocol_unpack_response(frame_t *frame, command_t *cmd, uint8_t *success);

/**
 * @brief Unpack a power enable command frame
 *
 * @param[in]  frame  Frame to unpack
 * @param[out] enable Power enable flag
 * @return true if unpacking succeeded, false otherwise
 */
bool protocol_unpack_power_enable(frame_t *frame, uint8_t *enable);

/**
 * @brief Unpack a voltage setting command frame
 *
 * @param[in]  frame   Frame to unpack
 * @param[out] vout_mv Output voltage in millivolts
 * @return true if unpacking succeeded, false otherwise
 */
bool protocol_unpack_vout(frame_t *frame, uint16_t *vout_mv);

/**
 * @brief Unpack a current limit command frame
 *
 * @param[in]  frame     Frame to unpack
 * @param[out] ilimit_ma Current limit in milliamps
 * @return true if unpacking succeeded, false otherwise
 */
bool protocol_unpack_ilimit(frame_t *frame, uint16_t *ilimit_ma);

/**
 * @brief Unpack a query response frame
 *
 * @param[in]  frame          Frame to unpack
 * @param[out] v_in           Input voltage in millivolts
 * @param[out] v_out_setting  Configured output voltage in millivolts
 * @param[out] v_out          Actual output voltage in millivolts
 * @param[out] i_out          Output current in milliamps
 * @param[out] i_limit        Current limit in milliamps
 * @param[out] power_enabled  Power output enable status
 * @return true if unpacking succeeded, false otherwise
 */
bool protocol_unpack_query_response(frame_t *frame, uint16_t *v_in,
    uint16_t *v_out_setting, uint16_t *v_out, uint16_t *i_out,
    uint16_t *i_limit, uint8_t *power_enabled);

/**
 * @brief Unpack a WiFi status command frame
 *
 * @param[in]  frame  Frame to unpack
 * @param[out] status WiFi connection status
 * @return true if unpacking succeeded, false otherwise
 */
bool protocol_unpack_wifi_status(frame_t *frame, wifi_status_t *status);

/**
 * @brief Unpack a lock command frame
 *
 * @param[in]  frame  Frame to unpack
 * @param[out] locked Lock status
 * @return true if unpacking succeeded, false otherwise
 */
bool protocol_unpack_lock(frame_t *frame, uint8_t *locked);

/**
 * @brief Unpack an OCP event frame
 *
 * @param[in]  frame Frame to unpack
 * @param[out] i_cut Current that triggered OCP in milliamps
 * @return true if unpacking succeeded, false otherwise
 */
bool protocol_unpack_ocp(frame_t *frame, uint16_t *i_cut);

/**
 * @brief Unpack an upgrade start command frame
 *
 * @param[in]  frame      Frame to unpack
 * @param[out] chunk_size Requested data chunk size in bytes
 * @param[out] crc        Expected CRC16 of complete firmware
 * @return true if unpacking succeeded, false otherwise
 */
bool protocol_unpack_upgrade_start(frame_t *frame, uint16_t *chunk_size, uint16_t *crc);


/*
 * =============================================================================
 * Protocol Command Documentation
 * =============================================================================
 *
 * === Pinging DPS ===
 * The ping command is sent by the host to check if the DPS is online.
 *
 *  HOST:   [cmd_ping]
 *  DPS:    [cmd_response | cmd_ping] [1]
 *
 *
 * === Reading the status of the DPS ===
 *
 * This command retrieves V_in, V_out, I_out, I_limit, power enable. Voltage
 * and currents are all in the 'milli' range.
 *
 *  HOST:   [cmd_query]
 *  DPS:    [cmd_response | cmd_query] [1] [V_in(15:8)] [V_in(7:0)]
 *          [V_out_setting(15:8)] [V_out_setting(7:0)] [V_out(15:8)] [V_out(7:0)]
 *          [I_out(15:8)] [I_out(7:0)] [I_limit(15:8)] [I_limit(7:0)] [<power enable>]
 *
 *
 * === Changing active function ===
 * Functions are operating modes (constant voltage, constant current, ...).
 * The cmd_set_function command sets the active function. The change will be
 * reflected on the display and the current function will be turned off.
 *
 *  HOST:   [cmd_set_function] [<function name>]
 *  DPS:    [cmd_response | cmd_set_function] [<status>]
 *
 * <status> will be 1 or 0 depending on if the function is available or not
 *
 *
 * === Listing available functions ===
 * This command is used to list the available functions on the OpenDPS.
 * <status> is always 1.
 *
 *  HOST:   [cmd_list_functions]
 *  DPS:    [cmd_response | cmd_list_functions] [<status>] <func name 1> \0 <func name 2> \0 ...
 *
 *
 * === Setting function parameters ===
 * Each function can be controlled using named parameters. Multiple parameters
 * can be sent in a single command. Names and values are sent as ASCII strings.
 *
 *  HOST:   [cmd_set_parameters] <param 1> \0 <value 1> \0 <param 2> \0 <value 2> ...
 *  DPS:    [cmd_response | cmd_set_parameters] <set_parameter_status_t 1> <set_parameter_status_t 2> ...
 *
 *
 * === Listing function parameters ===
 * Returns parameters associated with the current function and system values:
 *   - Power out status (1/0)
 *   - Input voltage
 *   - Output voltage
 *   - Output current
 *
 *  HOST:   [cmd_list_parameters]
 *  DPS:    [cmd_response | cmd_list_parameters] <param 1> \0 <value 1> \0 ...
 *
 *
 * === Receiving a temperature report ===
 * This command is sent by a WiFi companion with temperature sensors.
 * Two temperatures are signed 16-bit integers x10. 0xFFFF indicates invalid.
 *
 *  HOST:   [cmd_temperature_report] <temp1[15:8]> <temp1[7:0]> <temp2[15:8]> <temp2[7:0]>
 *  DPS:    [cmd_response | cmd_temperature_report]
 *
 *
 * === Setting wifi status ===
 *
 *  HOST:   [cmd_wifi_status] [<wifi_status_t>]
 *  DPS:    [cmd_response | cmd_wifi_status] [1]
 *
 *
 * === Locking the controls ===
 *
 *  HOST:   [cmd_lock] [<lock>]
 *  DPS:    [cmd_response | cmd_lock] [1]
 *
 *
 * === Overcurrent protection event ===
 * The DPS sends this when OCP triggers. No response expected.
 *
 *  DPS:    [cmd_ocp_event] [I_cut(7:0)] [I_cut(15:8)]
 *  HOST:   none
 *
 *
 * === DPS upgrade sessions ===
 * When cmd_upgrade_start is received:
 *  1. Chunk size and CRC are written to bootcom RAM
 *  2. Device restarts into bootloader
 *  3. Bootloader sends cmd_upgrade_start ack
 *  4. Host sends cmd_upgrade_data chunks
 *  5. Bootloader writes each chunk to flash and acks
 *  6. After last chunk, bootloader verifies CRC and boots app
 *
 *  HOST:     [cmd_upgrade_start] [chunk_size:16] [crc:16]
 *  DPS (BL): [cmd_response | cmd_upgrade_start] [<upgrade_status_t>] [<chunk_size:16>] [<upgrade_reason_t:8>]
 *
 *  HOST:   [cmd_upgrade_data] [<payload>]+
 *  DPS BL: [cmd_response | cmd_upgrade_data] [<upgrade_status_t>]
 */

#endif // __PROTOCOL_H__
