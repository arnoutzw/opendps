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
 * @file bootcom.h
 * @brief Bootloader-Application Communication Interface
 *
 * This module provides a mechanism for communication between the
 * bootloader (DPSBoot) and the main application (OpenDPS) through
 * a shared RAM buffer that persists across resets.
 *
 * ## Purpose
 *
 * The bootcom interface enables:
 * - Application requesting firmware upgrade (triggers bootloader)
 * - Bootloader passing status back to application
 * - Sharing state across software resets
 *
 * ## Memory Location
 *
 * The bootcom buffer is located at a fixed RAM address that both
 * the bootloader and application agree upon. The location is chosen
 * to survive a software reset (not cleared by startup code).
 *
 * ## Data Format
 *
 * The buffer contains:
 * - Magic word to validate data presence
 * - Two 32-bit data words for general-purpose use
 *
 * Common uses:
 * - w1: Command/status code
 * - w2: Additional parameter or flags
 *
 * ## Usage Flow (Firmware Upgrade)
 *
 * 1. Application receives upgrade command via serial
 * 2. Application writes upgrade request to bootcom
 * 3. Application triggers software reset
 * 4. Bootloader reads bootcom, finds upgrade request
 * 5. Bootloader performs firmware upgrade
 * 6. Bootloader clears bootcom and boots application
 *
 * @see dpsboot for bootloader implementation
 * @see protocol.h for upgrade commands
 */

#ifndef __BOOTCOM_H__
#define __BOOTCOM_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Write data to the bootcom buffer
 *
 * Stores two 32-bit words in the shared bootcom buffer and sets
 * the magic word to indicate valid data is present. This data
 * will be preserved across a software reset.
 *
 * @param[in] w1 First data word (typically command/status)
 * @param[in] w2 Second data word (typically parameter)
 *
 * ## Usage Example (Request Upgrade)
 *
 * ```c
 * // Tell bootloader to expect firmware upgrade
 * bootcom_put(BOOTCOM_CMD_UPGRADE, firmware_size);
 *
 * // Reset to bootloader
 * scb_reset_system();
 * ```
 *
 * @note Data persists until cleared or power cycle
 * @note Call before software reset for bootloader communication
 */
void bootcom_put(uint32_t w1, uint32_t w2);

/**
 * @brief Read data from the bootcom buffer
 *
 * Checks if valid bootcom data is present (magic word check)
 * and if so, retrieves the stored data words. The buffer is
 * cleared after successful read to prevent stale data.
 *
 * @param[out] w1 Receives first data word
 * @param[out] w2 Receives second data word
 *
 * @return true  Valid data was present and retrieved
 * @return false No valid bootcom data (magic word mismatch)
 *
 * ## Usage Example (Bootloader Check)
 *
 * ```c
 * uint32_t cmd, param;
 * if (bootcom_get(&cmd, &param)) {
 *     if (cmd == BOOTCOM_CMD_UPGRADE) {
 *         // Handle firmware upgrade request
 *         start_upgrade(param);
 *     }
 * }
 * ```
 *
 * @note Buffer is cleared on successful read
 * @note Returns false on first boot (no prior bootcom)
 */
bool bootcom_get(uint32_t *w1, uint32_t *w2);

#endif // __BOOTCOM_H__
