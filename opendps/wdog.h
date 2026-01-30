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
 * @file wdog.h
 * @brief Watchdog Timer Interface
 *
 * This module provides an interface to the STM32's Independent Watchdog
 * (IWDG) peripheral. The watchdog ensures system recovery from software
 * lockups by forcing a reset if not periodically refreshed.
 *
 * ## Purpose
 *
 * The watchdog timer provides:
 * - Automatic reset on software hang/lockup
 * - Protection against infinite loops
 * - Recovery from unexpected states
 *
 * ## Watchdog Operation
 *
 * 1. Initialize watchdog with wdog_init()
 * 2. Watchdog counter starts counting down
 * 3. Main loop must call wdog_kick() before timeout
 * 4. If wdog_kick() not called in time, system resets
 *
 * ## Timeout Period
 *
 * The watchdog timeout is configured during initialization.
 * Typical timeout is several seconds, allowing the main loop
 * to handle normal operations without timeout.
 *
 * ## Usage Pattern
 *
 * ```c
 * int main(void) {
 *     // Initialize hardware
 *     hw_init();
 *     wdog_init();
 *
 *     while (1) {
 *         // Process events
 *         handle_events();
 *
 *         // Refresh watchdog
 *         wdog_kick();
 *     }
 * }
 * ```
 *
 * ## Important Notes
 *
 * - Once started, IWDG cannot be stopped (only reset stops it)
 * - IWDG runs from independent LSI oscillator
 * - Works even if main clock fails
 * - Debug mode may pause IWDG (configurable)
 *
 * @see hw.h for hardware initialization
 */

#ifndef __WDOG_H__
#define __WDOG_H__

/**
 * @brief Initialize and start the watchdog timer
 *
 * Configures the Independent Watchdog (IWDG) peripheral with
 * the appropriate prescaler and reload value for the desired
 * timeout period, then starts the watchdog.
 *
 * @warning Once started, the watchdog cannot be stopped except
 *          by a system reset. Ensure wdog_kick() is called
 *          regularly in your main loop.
 *
 * @note Called during system initialization
 * @note Timeout period is hardware-defined (typically 2-5 seconds)
 */
void wdog_init(void);

/**
 * @brief Refresh (kick) the watchdog timer
 *
 * Resets the watchdog countdown timer to prevent a system reset.
 * Must be called periodically within the timeout period.
 *
 * ## Best Practices
 *
 * - Call from the main loop, not from interrupts
 * - Call only if system is in a healthy state
 * - Don't call from multiple locations (makes debugging harder)
 *
 * @note Should be called at least once per timeout period
 * @note Calling more frequently than needed is harmless
 */
void wdog_kick(void);

#endif // __WDOG_H__
