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
 * @file flashlock.h
 * @brief Flash Memory Lock Management
 *
 * This module provides reference-counted locking for the STM32's
 * internal flash memory. It ensures flash remains unlocked only
 * while actively being written and is automatically re-locked
 * when all operations complete.
 *
 * ## Purpose
 *
 * The STM32 flash memory is locked by default to prevent accidental
 * writes. This module provides:
 * - Safe unlock/lock with reference counting
 * - Nested operation support (multiple concurrent writers)
 * - Automatic re-locking when all operations complete
 *
 * ## Reference Counting
 *
 * Each unlock_flash() increments a counter, each lock_flash()
 * decrements it. Flash is only physically re-locked when the
 * counter reaches zero.
 *
 * ```
 * unlock_flash();  // counter = 1, flash unlocked
 * unlock_flash();  // counter = 2, still unlocked
 * lock_flash();    // counter = 1, still unlocked
 * lock_flash();    // counter = 0, flash locked
 * ```
 *
 * ## Usage Pattern
 *
 * ```c
 * void write_setting(uint32_t addr, uint32_t value) {
 *     unlock_flash();
 *
 *     // Perform flash write operations
 *     flash_program_word(addr, value);
 *
 *     lock_flash();
 * }
 * ```
 *
 * ## Important Notes
 *
 * - Always pair unlock_flash() with lock_flash()
 * - Flash writes require erase first (page-based)
 * - Keep flash unlocked for minimal time
 * - Interrupts may be disabled during flash operations
 *
 * @see past.h for persistent storage using flash
 */

#ifndef __FLASHLOCK_H__
#define __FLASHLOCK_H__

/**
 * @brief Unlock flash memory for writing
 *
 * Increments the flash lock reference counter and unlocks the
 * flash memory if this is the first unlock request.
 *
 * ## Thread Safety
 *
 * This function is NOT thread-safe. If called from multiple
 * contexts (main + ISR), external synchronization is required.
 *
 * @note Must be paired with a corresponding lock_flash() call
 * @note Flash unlock sequence: write magic keys to FLASH_KEYR
 */
void unlock_flash(void);

/**
 * @brief Lock flash memory after writing
 *
 * Decrements the flash lock reference counter. If the counter
 * reaches zero, the flash memory is physically re-locked.
 *
 * @note Must be paired with a prior unlock_flash() call
 * @note Calling without prior unlock may cause underflow
 * @note Re-locking sets the LOCK bit in FLASH_CR
 */
void lock_flash(void);

#endif // __FLASHLOCK_H__
