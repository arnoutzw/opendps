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
 * @file tick.h
 * @brief System Tick Timer Module for OpenDPS
 *
 * This module provides millisecond-resolution timing services using the
 * ARM Cortex-M SysTick timer. It is used for:
 *
 * - Timing delays (e.g., display initialization, debouncing)
 * - Measuring elapsed time
 * - Periodic task scheduling
 * - Timeout detection
 *
 * ## Implementation
 *
 * The SysTick timer is configured to generate an interrupt every 1ms.
 * The ISR increments a 64-bit counter that can be read via get_ticks().
 *
 * ## Usage Example
 *
 * ```c
 * // Measure operation duration
 * uint64_t start = get_ticks();
 * do_something();
 * uint64_t duration = get_ticks() - start;
 * printf("Operation took %lu ms\n", (uint32_t)duration);
 *
 * // Implement timeout
 * uint64_t timeout = get_ticks() + 1000;  // 1 second timeout
 * while (condition && get_ticks() < timeout) {
 *     // wait...
 * }
 * ```
 *
 * @note The tick counter wraps after ~584 million years at 1ms resolution
 */

#ifndef __TICK_H__
#define __TICK_H__

#include <stdint.h>

/**
 * @brief Initialize the SysTick timer module
 *
 * Configures the ARM Cortex-M SysTick timer to generate interrupts
 * at 1ms intervals. The interrupt handler increments the tick counter.
 *
 * @note Must be called during system initialization
 * @note Requires the system clock to be configured first
 */
void systick_init(void);

/**
 * @brief Blocking delay for a specified number of milliseconds
 *
 * Performs a busy-wait delay by polling the tick counter. The CPU
 * is fully occupied during the delay.
 *
 * @param[in] delay Delay duration in milliseconds
 *
 * @warning This is a blocking function - no other code executes
 *          (except ISRs) during the delay
 * @warning Do not use for delays > 1000ms; prefer non-blocking designs
 */
void delay_ms(uint32_t delay);

/**
 * @brief Get the current system tick count
 *
 * Returns the number of milliseconds since system startup (power-on
 * or reset). The counter is 64-bit to avoid wraparound issues.
 *
 * @return Number of milliseconds since startup
 *
 * @note Safe to call from ISRs
 * @note On STM32, reading 64-bit value is not atomic; brief interrupts
 *       may cause glitches, but this is generally acceptable
 */
uint64_t get_ticks(void);

#endif // __TICK_H__
