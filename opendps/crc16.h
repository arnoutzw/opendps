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
 * @file crc16.h
 * @brief CRC-16 CCITT Checksum Calculation
 *
 * This module provides CRC-16 CCITT checksum calculation for data integrity
 * verification in the OpenDPS communication protocol.
 *
 * ## Algorithm
 *
 * Uses CRC-16 CCITT with the following parameters:
 * - Polynomial: 0x1021 (x^16 + x^12 + x^5 + 1)
 * - Initial value: 0x0000
 * - Input reflection: No
 * - Output reflection: No
 * - Final XOR: 0x0000
 *
 * ## Usage
 *
 * ### Block CRC (all data available)
 * ```c
 * uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
 * uint16_t checksum = crc16(data, sizeof(data));
 * ```
 *
 * ### Streaming CRC (data arrives incrementally)
 * ```c
 * uint16_t crc = 0;  // Initialize to 0
 * while (data_available()) {
 *     crc = crc16_add(crc, get_byte());
 * }
 * // crc now contains the final checksum
 * ```
 *
 * ## Protocol Usage
 *
 * In the uframe protocol:
 * 1. CRC is calculated over the payload (after SOF, before EOF)
 * 2. CRC bytes are transmitted big-endian (MSB first)
 * 3. Receiver calculates CRC and compares with received value
 *
 * @see uframe.h for frame structure
 */

#ifndef __CRC16_H__
#define __CRC16_H__

#include <stdint.h>

/**
 * @brief Add a byte to an ongoing CRC calculation
 *
 * Updates the CRC with a single byte of data. Use this function
 * for streaming applications where data is processed byte-by-byte.
 *
 * @param[in] crc  Current CRC value (0 to start new calculation)
 * @param[in] byte Byte to add to the CRC
 * @return Updated CRC value including the new byte
 *
 * @note Initialize crc to 0 when starting a new calculation
 * @note This is the core CRC calculation; crc16() is built on this
 *
 * @par Example
 * @code
 * uint16_t crc = 0;
 * crc = crc16_add(crc, 0x01);
 * crc = crc16_add(crc, 0x02);
 * // crc now contains CRC of {0x01, 0x02}
 * @endcode
 */
uint16_t crc16_add(uint16_t crc, uint8_t byte);

/**
 * @brief Calculate CRC-16 checksum of a data buffer
 *
 * Computes the CRC-16 CCITT checksum of an entire buffer. This is
 * a convenience wrapper around crc16_add() for block operations.
 *
 * @param[in] data   Pointer to the data buffer
 * @param[in] length Number of bytes in the buffer
 * @return 16-bit CRC checksum
 *
 * @note For empty data (length=0), returns 0
 *
 * @par Example
 * @code
 * uint8_t buffer[100];
 * fill_buffer(buffer, 100);
 * uint16_t checksum = crc16(buffer, 100);
 * @endcode
 */
uint16_t crc16(uint8_t *data, uint16_t length);

#endif // __CRC16_H__
