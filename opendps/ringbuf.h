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
 * @file ringbuf.h
 * @brief Lock-Free Ring Buffer Implementation
 *
 * This module provides a simple circular (ring) buffer for producer-consumer
 * scenarios, particularly for UART receive buffering in OpenDPS.
 *
 * ## Design
 *
 * The ring buffer uses separate read and write indices that wrap around
 * when they reach the end of the buffer. This design allows lock-free
 * operation when there is a single producer (ISR) and single consumer
 * (main loop).
 *
 * ## Buffer Layout
 *
 * ```
 * +---+---+---+---+---+---+---+---+
 * | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 * +---+---+---+---+---+---+---+---+
 *         ^read          ^write
 *         (data available: 3, 4)
 * ```
 *
 * ## Usage
 *
 * The buffer stores 16-bit values (uint16_t) to accommodate UART data
 * with status bits. For 8-bit data, only the lower byte is used.
 *
 * ```c
 * ringbuf_t rx_buf;
 * uint8_t buffer[64];
 * ringbuf_init(&rx_buf, buffer, sizeof(buffer));
 *
 * // In UART ISR:
 * ringbuf_put(&rx_buf, received_byte);
 *
 * // In main loop:
 * uint16_t data;
 * if (ringbuf_get(&rx_buf, &data)) {
 *     process_byte(data & 0xFF);
 * }
 * ```
 *
 * @note Uses 16-bit elements; size is specified in bytes but used as half
 * @note Thread-safe for single producer, single consumer scenario
 */

#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef DPS_EMULATOR
 #include <pthread.h>
#endif // DPS_EMULATOR

/**
 * @brief Ring buffer structure
 *
 * Holds the state of a ring buffer including the data pointer
 * and read/write indices. The buffer is allocated externally.
 */
typedef struct {
    uint16_t *buf;      /**< Pointer to buffer storage (uint16_t array) */
    uint32_t size;      /**< Size of buffer in elements (not bytes) */
    uint32_t read;      /**< Read index (next position to read from) */
    uint32_t write;     /**< Write index (next position to write to) */
#ifdef DPS_EMULATOR
    pthread_mutex_t mutex;  /**< Mutex for thread safety in emulator */
#endif // DPS_EMULATOR
} ringbuf_t;

/**
 * @brief Initialize a ring buffer
 *
 * Prepares a ring buffer for use by setting up the data pointer
 * and resetting the read/write indices.
 *
 * @param[out] ring Pointer to ring buffer structure to initialize
 * @param[in]  buf  Pointer to buffer storage (will be cast to uint16_t*)
 * @param[in]  size Size of buffer storage in bytes
 *
 * @note The buffer must be large enough for at least 2 elements
 * @note The actual number of elements is size/sizeof(uint16_t)
 * @note One element is always empty to distinguish full from empty
 */
void ringbuf_init(ringbuf_t *ring, uint8_t *buf, uint32_t size);

/**
 * @brief Put data into the ring buffer
 *
 * Adds a 16-bit word to the ring buffer. If the buffer is full,
 * the operation fails and the data is discarded.
 *
 * @param[in,out] ring Pointer to the ring buffer
 * @param[in]     word Data word to add to the buffer
 * @return true if the data was added successfully
 * @return false if the buffer was full (data discarded)
 *
 * @note Safe to call from ISRs
 * @note Buffer full condition loses data - increase buffer size if this occurs
 */
bool ringbuf_put(ringbuf_t *ring, uint16_t word);

/**
 * @brief Get data from the ring buffer
 *
 * Removes and returns the oldest 16-bit word from the ring buffer.
 * If the buffer is empty, the operation fails.
 *
 * @param[in,out] ring Pointer to the ring buffer
 * @param[out]    word Pointer to receive the data word
 * @return true if data was retrieved successfully
 * @return false if the buffer was empty (word unchanged)
 *
 * @note Call from main loop, not from ISRs
 * @note Check return value before using *word
 */
bool ringbuf_get(ringbuf_t *ring, uint16_t *word);

#endif // __RINGBUF_H__
