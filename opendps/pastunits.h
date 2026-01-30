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
 * @file pastunits.h
 * @brief PAST Storage Unit Identifiers
 *
 * This file defines the unit IDs for the Persistent Application Storage
 * (PAST) system. Each setting stored in flash has a unique identifier.
 *
 * ## Unit Categories
 *
 * | Category | IDs | Description |
 * |----------|-----|-------------|
 * | Power settings | 1 | Voltage/current output settings |
 * | Display settings | 2, 14 | TFT inversion, brightness |
 * | Version info | 3-4 | Git hashes for boot/app |
 * | Calibration | 5-13 | ADC/DAC calibration coefficients |
 * | System | 0xFF | Upgrade status flag |
 *
 * ## Adding New Units
 *
 * When adding a new persistent setting:
 * 1. Add a new enum value here (use next available ID)
 * 2. Document the data format in a comment
 * 3. Use past_read_unit() and past_write_unit() to access
 *
 * @see past.h for the storage system
 * @see pwrctl.h for calibration usage
 */

#ifndef __PASTUNITS_H__
#define __PASTUNITS_H__

/**
 * @brief Identifiers for persistent storage units
 *
 * Each enum value represents a unique setting that can be stored
 * in flash memory using the PAST system.
 */
typedef enum {
    /** @brief Power settings: [I_limit:16] | [V_out:16] */
    past_power = 1,
    /** @brief TFT inversion: 0 (normal) or 1 (inverted) */
    past_tft_inversion,
    /**
     * @brief Bootloader git hash (string)
     * @warning Moving this ID requires recompiling DPSBoot!
     */
    past_boot_git_hash,
    /** @brief Application git hash (string) */
    past_app_git_hash,
    /** @brief Current ADC slope coefficient K (float) */
    past_A_ADC_K,
    /** @brief Current ADC offset coefficient C (float) */
    past_A_ADC_C,
    /** @brief Current DAC slope coefficient K (float) */
    past_A_DAC_K,
    /** @brief Current DAC offset coefficient C (float) */
    past_A_DAC_C,
    /** @brief Voltage DAC slope coefficient K (float) */
    past_V_DAC_K,
    /** @brief Voltage DAC offset coefficient C (float) */
    past_V_DAC_C,
    /** @brief Voltage ADC slope coefficient K (float) */
    past_V_ADC_K,
    /** @brief Voltage ADC offset coefficient C (float) */
    past_V_ADC_C,
    /** @brief Input voltage ADC slope coefficient K (float) */
    past_VIN_ADC_K,
    /** @brief Input voltage ADC offset coefficient C (float) */
    past_VIN_ADC_C,
    /** @brief TFT brightness level (0-100) */
    past_tft_brightness,
    /**
     * @brief Upgrade in progress flag
     * Presence indicates incomplete upgrade; bootloader won't boot app
     */
    past_upgrade_started = 0xff
} parameter_id_t;


#endif // __PASTUNITS_H__
