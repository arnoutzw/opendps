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
 * @file past.h
 * @brief Persistent Application Storage (PAST) Module
 *
 * This module provides wear-leveled persistent storage using the STM32's
 * internal flash memory. It stores key-value pairs (units) identified by
 * numeric IDs, with automatic garbage collection.
 *
 * ## Design
 *
 * PAST uses two flash blocks in a ping-pong arrangement:
 * - One block is always active (contains current data)
 * - The other is either erased or being prepared
 * - When active block is full, data is copied to the other block
 *   (garbage collection), omitting deleted entries
 *
 * ## Unit Format
 *
 * Each unit (key-value pair) is stored as:
 * ```
 * +--------+--------+--------+--------+
 * | ID(32) | Len(32)| Data...| Pad... |
 * +--------+--------+--------+--------+
 * ```
 *
 * - ID: 32-bit unique identifier (0 or 0xFFFFFFFF are reserved)
 * - Length: 32-bit data length in bytes
 * - Data: Actual data bytes
 * - Padding: Alignment padding to 4-byte boundary
 *
 * ## Wear Leveling
 *
 * - New data is always appended at the end of the block
 * - Updates create new copies (old data marked as deleted)
 * - Garbage collection recovers space by copying only valid data
 * - Two-block design ensures power-fail safety
 *
 * ## Usage Example
 *
 * ```c
 * past_t past;
 * past.blocks[0] = 0x08007000;  // First flash block address
 * past.blocks[1] = 0x08007400;  // Second flash block address
 *
 * if (!past_init(&past)) {
 *     // Handle initialization failure
 * }
 *
 * // Write data
 * uint32_t value = 12345;
 * past_write_unit(&past, MY_SETTING_ID, &value, sizeof(value));
 *
 * // Read data
 * const void *data;
 * uint32_t len;
 * if (past_read_unit(&past, MY_SETTING_ID, &data, &len)) {
 *     uint32_t read_value = *(uint32_t*)data;
 * }
 * ```
 *
 * @see pastunits.h for defined unit IDs
 */

#ifndef __PAST_H__
#define __PAST_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Unit identifier type
 *
 * A 32-bit value uniquely identifying each stored unit. Predefined
 * IDs are listed in pastunits.h. Custom IDs should start above
 * the highest predefined value.
 *
 * Reserved values:
 * - 0x00000000: Indicates deleted/invalid unit
 * - 0xFFFFFFFF: Indicates erased flash (unused space)
 */
typedef uint32_t past_id_t;

/**
 * @brief PAST instance structure
 *
 * Holds the state of a PAST storage instance. Users must initialize
 * the blocks[] array with flash block addresses before calling past_init().
 * All other fields are managed internally and should not be modified.
 */
typedef struct {
    /** @brief Flash block addresses (user must initialize before past_init) */
    uint32_t blocks[2];
    /** @brief Index of currently active block (0 or 1) - internal use */
    uint32_t _cur_block;
    /** @brief Generation counter for GC tracking - internal use */
    uint32_t _counter;
    /** @brief Next write address in current block - internal use */
    uint32_t _end_addr;
    /** @brief true if PAST is initialized and valid - internal use */
    bool _valid;
} past_t;

/**
 * @brief Initialize the PAST system
 *
 * Prepares the PAST for use by:
 * 1. Reading block headers to find the active block
 * 2. Scanning the active block to find the end of valid data
 * 3. Performing garbage collection if needed
 * 4. Formatting both blocks if no valid data is found
 *
 * @param[in,out] past PAST structure with blocks[] initialized to flash addresses
 * @return true if initialization succeeded
 * @return false if initialization failed (flash error, corruption)
 *
 * @note The blocks[] array must be initialized before calling
 * @note On first use, PAST is automatically formatted
 */
bool past_init(past_t *past);

/**
 * @brief Read a unit from PAST
 *
 * Searches for a unit by ID and returns a pointer to its data.
 * The returned pointer points directly into flash memory.
 *
 * @param[in]  past   Initialized PAST structure
 * @param[in]  id     Unit ID to search for
 * @param[out] data   Receives pointer to unit data in flash
 * @param[out] length Receives length of unit data in bytes
 * @return true if the unit was found
 * @return false if the unit was not found
 *
 * @note The data pointer is valid until the next PAST write operation
 * @note Do not modify data through the returned pointer
 */
bool past_read_unit(past_t *past, past_id_t id, const void **data, uint32_t *length);

/**
 * @brief Write a unit to PAST
 *
 * Stores a unit with the given ID. If a unit with the same ID already
 * exists, the old copy is invalidated and a new copy is written.
 *
 * @param[in,out] past   Initialized PAST structure
 * @param[in]     id     Unit ID (must not be 0 or 0xFFFFFFFF)
 * @param[in]     data   Pointer to data to store
 * @param[in]     length Length of data in bytes
 * @return true if the unit was successfully written
 * @return false if writing failed (flash error, PAST full and GC failed)
 *
 * @note May trigger garbage collection if block is nearly full
 * @note Data is copied to flash; source buffer can be freed after call
 */
bool past_write_unit(past_t *past, past_id_t id, void *data, uint32_t length);

/**
 * @brief Erase a unit from PAST
 *
 * Marks a unit as deleted. The space is reclaimed during the next
 * garbage collection cycle.
 *
 * @param[in,out] past Initialized PAST structure
 * @param[in]     id   Unit ID to erase
 * @return true if the unit was found and marked as deleted
 * @return false if the unit was not found or erase failed
 */
bool past_erase_unit(past_t *past, past_id_t id);

/**
 * @brief Format the PAST area
 *
 * Erases both flash blocks and initializes the first one with a
 * valid header. All stored data is lost.
 *
 * @param[in,out] past Initialized or partially initialized PAST structure
 * @return true if formatting succeeded
 * @return false if flash erase or write failed
 *
 * @warning This destroys all stored data
 * @note Called automatically by past_init() if no valid data is found
 */
bool past_format(past_t *past);

/**
 * @brief Check if garbage collection is needed and perform it
 *
 * Checks the fill level of the current block and performs garbage
 * collection if it's nearly full. GC copies valid units to the other
 * block and switches the active block.
 *
 * @param[in,out] past Initialized PAST structure
 * @return true if GC was performed
 * @return false if GC was not needed or failed
 *
 * @note Usually called automatically by past_write_unit()
 * @note Safe to call explicitly to pre-emptively free space
 */
bool past_gc_check(past_t *past);

#endif // __PAST_H__
