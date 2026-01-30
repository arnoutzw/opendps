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
 * @file dbg_printf.h
 * @brief Debug Printf Interface
 *
 * This module provides debug output functionality that can be enabled
 * or disabled at compile time. In release builds, debug output is
 * completely eliminated to save code space.
 *
 * ## Build Configurations
 *
 * | Define | dbg_printf | emu_printf |
 * |--------|------------|------------|
 * | DPS_EMULATOR | printf | printf |
 * | CONFIG_DEBUG | UART output | no-op |
 * | (none) | no-op | no-op |
 *
 * ## Usage
 *
 * ```c
 * dbg_printf("Voltage set to %d mV\n", voltage);
 * emu_printf("Emulator-only debug: state = %d\n", state);
 * ```
 *
 * ## Output Destination
 *
 * - **Emulator**: Standard output (console)
 * - **Hardware with CONFIG_DEBUG**: UART serial port
 * - **Release build**: No output (macros expand to nothing)
 *
 * @see mini-printf.h for the printf implementation
 */

#ifndef __DBG_PRINTF_H__
#define __DBG_PRINTF_H__

#include <stdarg.h>

#ifdef DPS_EMULATOR
 #include <stdio.h>
 /**
  * @brief Debug printf - outputs to console in emulator
  *
  * In emulator builds, this maps directly to standard printf().
  */
 #define dbg_printf printf
 /**
  * @brief Emulator-only printf
  *
  * Outputs debug messages only when running in the emulator.
  * Useful for emulator-specific debugging that would not make
  * sense on real hardware.
  */
 #define emu_printf printf
#else // DPS_EMULATOR
 #ifdef CONFIG_DEBUG
  /**
   * @brief Debug printf function
   *
   * Outputs formatted debug messages to the UART serial port.
   * Only available when CONFIG_DEBUG is defined.
   *
   * @param fmt Format string (printf-style)
   * @param ... Variable arguments
   * @return Number of characters written
   *
   * @note Uses mini_snprintf internally
   * @note Output goes to UART at configured baud rate
   */
  int dbg_printf(const char *fmt, ...);
  /** @brief No-op in non-emulator builds */
  #define emu_printf(...)
 #else
  /** @brief No-op in release builds */
  #define dbg_printf(...)
  /** @brief No-op in non-emulator builds */
  #define emu_printf(...)
 #endif
#endif // DPS_EMULATOR

#endif // __DBG_PRINTF_H__
