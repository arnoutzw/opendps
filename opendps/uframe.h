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
 * @file uframe.h
 * @brief UART Frame Protocol for OpenDPS Communication
 *
 * This module implements a byte-stuffed framing protocol for reliable
 * serial communication between OpenDPS and external controllers (dpsctl,
 * ESP8266 WiFi proxy).
 *
 * ## Frame Format
 *
 * ```
 * +-----+------------------+--------+--------+-----+
 * | SOF | Escaped Payload  | CRC-HI | CRC-LO | EOF |
 * +-----+------------------+--------+--------+-----+
 *  0x7E     Variable         16-bit CRC       0x7F
 * ```
 *
 * ## Byte Stuffing (Escaping)
 *
 * To allow any byte value in the payload, special characters are escaped:
 *
 * | Byte | Escaped As |
 * |------|------------|
 * | 0x7E (SOF) | 0x7D 0x5E |
 * | 0x7F (EOF) | 0x7D 0x5F |
 * | 0x7D (DLE) | 0x7D 0x5D |
 *
 * The escape sequence is: DLE + (original_byte XOR 0x20)
 *
 * ## Frame Building
 *
 * 1. Call set_frame_header() to initialize frame
 * 2. Use pack8/pack16/pack32/pack_cstr to add payload
 * 3. Call end_frame() to finalize (adds CRC and EOF)
 * 4. Transmit frame->buffer[0..frame->length-1]
 *
 * ## Frame Parsing
 *
 * 1. Receive bytes until EOF is seen
 * 2. Call uframe_extract_payload() to validate and extract payload
 * 3. Use unpack8/unpack16/unpack32 to read payload fields
 *
 * ## CRC Protection
 *
 * A 16-bit CRC-CCITT checksum is calculated over the unescaped payload
 * and appended (with escaping) before the EOF marker. The receiver
 * recalculates the CRC and compares it to detect transmission errors.
 *
 * @see protocol.h for the command protocol built on uframe
 * @see crc16.h for CRC calculation
 */

#ifndef __UFRAME_H__
#define __UFRAME_H__

#include "dbg_printf.h"

/**
 * @defgroup Frame_Constants Frame Protocol Constants
 * @brief Special byte values used in the frame protocol
 * @{
 */

/** @brief Start Of Frame marker (0x7E) */
#define _SOF 0x7e

/** @brief Data Link Escape character (0x7D) - starts escape sequence */
#define _DLE 0x7d

/** @brief XOR value for escaping (0x20) - escaped_byte = original ^ _XOR */
#define _XOR 0x20

/** @brief End Of Frame marker (0x7F) */
#define _EOF 0x7f

/** @} */ // end of Frame_Constants

/**
 * @defgroup Frame_Errors Frame Error Codes
 * @brief Error codes returned by uframe_extract_payload()
 * @{
 */

/** @brief Frame is too short or too long to be valid */
#define E_LEN 1

/** @brief Frame has no SOF/EOF markers or invalid structure */
#define E_FRM 2

/** @brief CRC checksum mismatch - data corruption detected */
#define E_CRC 3

/** @} */ // end of Frame_Errors

/**
 * @def FRAME_OVERHEAD
 * @brief Calculate maximum frame size for a given payload size
 *
 * Worst case: SOF + (2 * payload for all bytes escaped) + (2 * 2 for CRC) + EOF
 *
 * @param size Payload size in bytes
 * @return Maximum frame size including overhead
 */
#define FRAME_OVERHEAD(size) (1 + 2*(size) + 4 + 1)

/**
 * @def MAX_FRAME_LENGTH
 * @brief Maximum size of a frame buffer
 *
 * This limits the maximum payload size. Larger frames would require
 * fragmentation at the application level.
 */
#define MAX_FRAME_LENGTH (128)

/**
 * @brief Frame structure for building and parsing frames
 *
 * This structure holds the frame buffer and state for both
 * building outgoing frames and parsing incoming frames.
 */
typedef struct
{
    uint8_t buffer[MAX_FRAME_LENGTH];   /**< Frame data buffer */
    uint32_t length;                    /**< Current length of data in buffer */
    uint16_t crc;                       /**< Running CRC value during building */
    uint32_t unpack_pos;                /**< Current read position for unpacking */
} frame_t;

/**
 * @brief Initialize a frame for building
 *
 * Prepares a frame structure for adding payload data. Writes the
 * SOF marker and initializes the CRC.
 *
 * @param[out] frame Frame structure to initialize
 */
void set_frame_header(frame_t *frame);

/**
 * @brief Finalize a frame after adding payload
 *
 * Appends the CRC checksum and EOF marker to complete the frame.
 * After this call, frame->buffer contains the complete frame ready
 * for transmission.
 *
 * @param[in,out] frame Frame to finalize
 */
void end_frame(frame_t *frame);

/**
 * @brief Prepare a frame for unpacking payload
 *
 * Resets the unpack position to the beginning of the payload.
 * Call this before using unpack8/16/32 functions.
 *
 * @param[in,out] frame Frame to prepare for reading
 */
void start_frame_unpacking(frame_t *frame);

/**
 * @brief Pack an 8-bit value into the frame with escaping
 *
 * Adds a byte to the frame payload, escaping it if necessary
 * (if it's SOF, EOF, or DLE). Updates the CRC.
 *
 * @param[in,out] frame Frame to add data to
 * @param[in]     data  Byte value to add
 */
void pack8(frame_t *frame, uint8_t data);

/**
 * @brief Add a raw byte to the frame without escaping
 *
 * Adds a byte directly to the frame buffer without escape processing.
 * Used for adding frame markers (SOF, EOF) or already-escaped data.
 *
 * @param[in,out] frame Frame to add data to
 * @param[in]     data  Byte to add (not escaped)
 *
 * @warning Only use for special bytes that should not be escaped
 */
void stuff8(frame_t *frame, uint8_t data);

/**
 * @brief Pack a 16-bit value into the frame (big-endian)
 *
 * Adds a 16-bit value to the frame as two bytes, MSB first.
 * Both bytes are escaped if necessary.
 *
 * @param[in,out] frame Frame to add data to
 * @param[in]     data  16-bit value to add
 */
void pack16(frame_t *frame, uint16_t data);

/**
 * @brief Pack a 32-bit value into the frame (big-endian)
 *
 * Adds a 32-bit value to the frame as four bytes, MSB first.
 * All bytes are escaped if necessary.
 *
 * @param[in,out] frame Frame to add data to
 * @param[in]     data  32-bit value to add
 */
void pack32(frame_t *frame, uint32_t data);

/**
 * @brief Pack a 32-bit floating point value into the frame
 *
 * Adds a float to the frame by treating it as a 32-bit integer
 * and packing it in big-endian order.
 *
 * @param[in,out] frame Frame to add data to
 * @param[in]     data  Float value to add
 */
void pack_float(frame_t *frame, float data);

/**
 * @brief Pack a null-terminated string into the frame
 *
 * Adds a C string to the frame including the null terminator.
 * Each byte is escaped if necessary.
 *
 * @param[in,out] frame Frame to add data to
 * @param[in]     data  Null-terminated string to add
 */
void pack_cstr(frame_t *frame, const char *data);

/**
 * @brief Unpack an 8-bit value from the frame
 *
 * Reads the next byte from the frame payload and advances the
 * read position.
 *
 * @param[in,out] frame Frame to read from
 * @param[out]    data  Pointer to receive the byte value
 * @return Number of bytes remaining in frame after this read
 */
uint32_t unpack8(frame_t *frame, uint8_t *data);

/**
 * @brief Unpack a 16-bit value from the frame (big-endian)
 *
 * Reads the next two bytes from the frame as a 16-bit value
 * (MSB first) and advances the read position.
 *
 * @param[in,out] frame Frame to read from
 * @param[out]    data  Pointer to receive the 16-bit value
 * @return Number of bytes remaining in frame after this read
 */
uint32_t unpack16(frame_t *frame, uint16_t *data);

/**
 * @brief Unpack a 32-bit value from the frame (big-endian)
 *
 * Reads the next four bytes from the frame as a 32-bit value
 * (MSB first) and advances the read position.
 *
 * @param[in,out] frame Frame to read from
 * @param[out]    data  Pointer to receive the 32-bit value
 * @return Number of bytes remaining in frame after this read
 */
uint32_t unpack32(frame_t *frame, uint32_t *data);

/**
 * @defgroup Unpack_Macros Unpack Helper Macros
 * @brief Type-casting macros to avoid compiler warnings
 *
 * These macros cast the data pointer to the correct type to
 * avoid "incompatible pointer type" warnings when using enums
 * or other integer types.
 * @{
 */

/** @brief Unpack 8-bit value with type cast */
#define UNPACK8(frame, data) unpack8(frame, (uint8_t*) data)

/** @brief Unpack 16-bit value with type cast */
#define UNPACK16(frame, data) unpack16(frame, (uint16_t*) data)

/** @brief Unpack 32-bit value with type cast */
#define UNPACK32(frame, data) unpack32(frame, (uint32_t*) data)

/** @} */ // end of Unpack_Macros

/**
 * @brief Extract payload from received frame data
 *
 * Processes raw received bytes by removing framing, unescaping,
 * and verifying the CRC. On success, the frame structure contains
 * the clean payload ready for unpacking.
 *
 * @param[out] frame  Frame structure to receive the extracted payload
 * @param[in]  data   Raw received data including SOF and EOF
 * @param[in]  length Length of raw data in bytes
 * @return Payload length on success
 * @return -E_LEN if frame is too short or too long
 * @return -E_FRM if frame structure is invalid (no SOF/EOF)
 * @return -E_CRC if CRC verification failed
 *
 * @note This function copies data into frame->buffer
 * @see uframe_extract_payload_inplace() for in-place processing
 */
int32_t uframe_extract_payload(frame_t *frame, uint8_t *data, uint32_t length);

/**
 * @brief Extract payload from received frame data (in-place)
 *
 * Like uframe_extract_payload() but modifies the input buffer
 * directly instead of copying. More efficient but destroys the
 * original data.
 *
 * @param[in,out] data   Raw data buffer (modified in-place)
 * @param[in]     length Length of raw data in bytes
 * @return Payload length on success (payload starts at data[0])
 * @return -E_LEN if frame is too short or too long
 * @return -E_FRM if frame structure is invalid
 * @return -E_CRC if CRC verification failed
 *
 * @warning Input data is modified - original content is lost
 */
int32_t uframe_extract_payload_inplace(uint8_t *data, uint32_t length);

/**
 * @brief Initialize a frame_t from already-extracted payload
 *
 * Sets up a frame structure for unpacking when the payload has
 * already been extracted (e.g., by uframe_extract_payload_inplace).
 *
 * @param[out] frame  Frame structure to initialize
 * @param[in]  data   Pointer to extracted payload data
 * @param[in]  length Length of payload in bytes
 *
 * @note Prefer uframe_extract_payload() for most use cases
 */
void uframe_from_extracted_payload(frame_t *frame, const uint8_t *data, uint32_t length);

#endif // __UFRAME_H__
