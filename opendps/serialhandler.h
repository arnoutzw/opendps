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
 * @file serialhandler.h
 * @brief Serial Communication Handler
 *
 * This module handles incoming serial data, buffering characters and
 * dispatching complete frames or lines to the appropriate handler.
 *
 * ## Operation Modes
 *
 * The serial handler supports two communication modes:
 *
 * 1. **Binary Protocol Mode**: Receives uframe-encoded binary commands
 *    for remote control via dpsctl or ESP8266 WiFi module
 *
 * 2. **CLI Mode**: Receives text commands for interactive debugging
 *    when CONFIG_DEBUG is enabled
 *
 * ## Data Flow
 *
 * ```
 * UART RX Interrupt
 *       |
 *       v
 * serial_handle_rx_char()
 *       |
 *       +---> uframe decoder (binary protocol)
 *       |           |
 *       |           v
 *       |     protocol handler
 *       |
 *       +---> line buffer (CLI mode)
 *                   |
 *                   v
 *              cli_run()
 * ```
 *
 * ## Usage
 *
 * Called from the UART receive interrupt handler:
 *
 * ```c
 * void usart1_isr(void) {
 *     if (USART_SR(USART1) & USART_SR_RXNE) {
 *         char c = usart_recv(USART1);
 *         serial_handle_rx_char(c);
 *     }
 * }
 * ```
 *
 * @see protocol.h for binary protocol handling
 * @see cli.h for CLI command processing
 * @see uframe.h for frame decoding
 */

#ifndef __SERIALHANDER_H__
#define __SERIALHANDER_H__

/**
 * @brief Handle a received serial character
 *
 * Processes a single character received from the UART. The character
 * is added to the appropriate buffer (binary frame or CLI line) and
 * complete messages are dispatched to their handlers.
 *
 * @param[in] c The received character
 *
 * ## Processing
 *
 * - Binary mode: Character passed to uframe decoder
 * - CLI mode: Character added to line buffer, processed on newline
 *
 * @note Called from UART interrupt context
 * @note Must be fast to avoid missing characters
 * @note Non-reentrant (not thread-safe)
 */
void serial_handle_rx_char(char c);

#endif // __SERIALHANDER_H__
