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
 * @file pwrctl.h
 * @brief Power Control Module for OpenDPS
 *
 * This module provides the power control layer for OpenDPS, handling:
 * - Voltage and current output control
 * - ADC to physical value conversion
 * - Physical value to DAC conversion
 * - Calibration coefficient management
 * - Output enable/disable control
 *
 * ## Calibration System
 *
 * The DPS uses linear calibration for ADC and DAC conversions:
 *
 * For ADC (measuring actual values):
 *   Physical_Value = K * ADC_Value + C
 *
 * For DAC (setting output values):
 *   DAC_Value = K * Physical_Value + C
 *
 * Where K is the slope (angle factor) and C is the offset.
 *
 * Calibration coefficients are stored in persistent storage (PAST) and
 * can be overridden from the default values in dps-model.h.
 *
 * ## Calibration Procedure
 *
 * To calibrate voltage ADC:
 * 1. Set a known voltage output
 * 2. Read the ADC value from CLI stat command
 * 3. Measure actual voltage with reference meter
 * 4. Repeat at different voltage levels
 * 5. Calculate K and C from the measurements
 *
 * Example:
 *   ADC 394 = 5001 mV measured
 *   ADC 782 = 10030 mV measured
 *   K = (10030-5001)/(782-394) = 12.97 mV/ADC
 *   C = 5001 - K*394 = -108 mV
 *
 * @see dps-model.h for default calibration coefficients per model
 * @see settings_calibration.h for calibration UI
 */

#ifndef __PWRCTL_H__
#define __PWRCTL_H__

#include <stdint.h>
#include <stdbool.h>
#include "past.h"

/**
 * @defgroup Calibration_Coefficients Calibration Coefficients
 * @brief Global calibration coefficient variables
 *
 * These variables hold the current calibration coefficients used for
 * ADC/DAC conversions. They are initialized from PAST (persistent storage)
 * or from default values in dps-model.h.
 *
 * Naming convention:
 * - a_* : Current (Ampere) related
 * - v_* : Output voltage related
 * - vin_* : Input voltage related
 * - *_adc_* : ADC conversion (raw to physical)
 * - *_dac_* : DAC conversion (physical to raw)
 * - *_k_coef : Slope (K) coefficient
 * - *_c_coef : Offset (C) coefficient
 * @{
 */

/** @brief Raw ADC value used for current limit comparison in ISR */
extern uint32_t pwrctl_i_limit_raw;

/** @brief Raw ADC value used for voltage limit comparison in ISR */
extern uint32_t pwrctl_v_limit_raw;

/** @brief Current ADC slope coefficient: I_ma = K * ADC + C */
extern float a_adc_k_coef;

/** @brief Current ADC offset coefficient */
extern float a_adc_c_coef;

/** @brief Current DAC slope coefficient: DAC = K * I_ma + C */
extern float a_dac_k_coef;

/** @brief Current DAC offset coefficient */
extern float a_dac_c_coef;

/** @brief Voltage ADC slope coefficient: V_mv = K * ADC + C */
extern float v_adc_k_coef;

/** @brief Voltage ADC offset coefficient */
extern float v_adc_c_coef;

/** @brief Voltage DAC slope coefficient: DAC = K * V_mv + C */
extern float v_dac_k_coef;

/** @brief Voltage DAC offset coefficient */
extern float v_dac_c_coef;

/** @brief Input voltage ADC slope coefficient */
extern float vin_adc_k_coef;

/** @brief Input voltage ADC offset coefficient */
extern float vin_adc_c_coef;

/** @} */ // end of Calibration_Coefficients

/**
 * @brief Initialize the power control module
 *
 * Initializes the power control subsystem by:
 * - Loading calibration coefficients from persistent storage
 * - Falling back to default values if not stored
 * - Initializing output state to disabled
 *
 * @param[in] past Pointer to initialized PAST structure for reading calibration
 *
 * @note Must be called after past_init() and before any other pwrctl functions
 */
void pwrctl_init(past_t *past);

/**
 * @brief Set the output voltage
 *
 * Sets the target output voltage by programming the appropriate DAC value.
 * The actual DAC value is calculated using the calibration coefficients.
 *
 * @param[in] value_mv Desired output voltage in millivolts
 * @return true if the requested voltage is within the valid range
 * @return false if the voltage exceeds the maximum for this model
 *
 * @note The actual output depends on the input voltage and load
 * @note Maximum voltage is limited by CONFIG_DPS_MAX_CURRENT in dps-model.h
 * @see pwrctl_get_vout() to read the current setting
 */
bool pwrctl_set_vout(uint32_t value_mv);

/**
 * @brief Set the output current (for constant current mode)
 *
 * Sets the target output current for constant current (CC) mode operation.
 * The DAC is programmed with the appropriate value using calibration.
 *
 * @param[in] value_ma Desired output current in milliamps
 * @return true if the requested current is within the valid range
 * @return false if the current exceeds the maximum for this model
 *
 * @note This is used in CC mode for setting the constant current output
 * @see pwrctl_set_ilimit() for setting the current limit in CV mode
 */
bool pwrctl_set_iout(uint32_t value_ma);

/**
 * @brief Get the current output current setting
 *
 * Returns the currently configured output current setting in milliamps.
 * This is the target value, not necessarily the actual measured current.
 *
 * @return Current setting in milliamps
 *
 * @see pwrctl_calc_iout() to get the actual measured current
 */
uint32_t pwrctl_get_iout(void);

/**
 * @brief Get the current output voltage setting
 *
 * Returns the currently configured output voltage setting in millivolts.
 * This is the target value, not necessarily the actual measured voltage.
 *
 * @return Voltage setting in millivolts
 *
 * @see pwrctl_calc_vout() to get the actual measured voltage
 */
uint32_t pwrctl_get_vout(void);

/**
 * @brief Set the current limit for over-current protection
 *
 * Sets the current limit used for over-current protection (OCP) in
 * constant voltage (CV) mode. When the output current exceeds this
 * limit, the OCP event is triggered.
 *
 * @param[in] value_ma Current limit in milliamps
 * @return true if the requested limit is within the valid range
 * @return false if the limit exceeds the maximum for this model
 *
 * @note OCP triggers an event and may disable output depending on configuration
 * @see pwrctl_get_ilimit() to read the current limit setting
 */
bool pwrctl_set_ilimit(uint32_t value_ma);

/**
 * @brief Get the current limit setting
 *
 * Returns the currently configured current limit in milliamps.
 *
 * @return Current limit setting in milliamps
 */
uint32_t pwrctl_get_ilimit(void);

/**
 * @brief Set the voltage limit for over-voltage protection
 *
 * Sets the voltage limit used for over-voltage protection (OVP).
 * When the output voltage exceeds this limit, the OVP event is triggered.
 *
 * @param[in] value_mv Voltage limit in millivolts
 * @return true if the requested limit is within the valid range
 * @return false if the limit is invalid
 *
 * @note OVP is a safety feature to protect connected loads
 */
bool pwrctl_set_vlimit(uint32_t value_mv);

/**
 * @brief Get the voltage limit setting
 *
 * Returns the currently configured voltage limit in millivolts.
 *
 * @return Voltage limit setting in millivolts
 */
uint32_t pwrctl_get_vlimit(void);

/**
 * @brief Enable or disable the power output
 *
 * Controls whether the power supply actually outputs voltage/current.
 * When disabled, the output terminals are essentially disconnected.
 *
 * @param[in] enable true to enable output, false to disable
 *
 * @note Disabling output is the safe state for the device
 * @note Output is automatically disabled on OCP/OVP events
 */
void pwrctl_enable_vout(bool enable);

/**
 * @brief Check if power output is currently enabled
 *
 * Returns the current state of the power output.
 *
 * @return true if power output is enabled
 * @return false if power output is disabled
 */
bool pwrctl_vout_enabled(void);

/**
 * @brief Calculate input voltage from raw ADC value
 *
 * Converts a raw ADC reading from the input voltage sensing circuit
 * to the actual input voltage in millivolts using calibration coefficients.
 *
 * Formula: V_in_mv = vin_adc_k_coef * raw + vin_adc_c_coef
 *
 * @param[in] raw Raw ADC value (0-4095 for 12-bit ADC)
 * @return Input voltage in millivolts
 *
 * @see hw_get_adc_values() to get the raw ADC value
 */
uint32_t pwrctl_calc_vin(uint16_t raw);

/**
 * @brief Calculate output voltage from raw ADC value
 *
 * Converts a raw ADC reading from the output voltage sensing circuit
 * to the actual output voltage in millivolts using calibration coefficients.
 *
 * Formula: V_out_mv = v_adc_k_coef * raw + v_adc_c_coef
 *
 * @param[in] raw Raw ADC value (0-4095 for 12-bit ADC)
 * @return Output voltage in millivolts
 *
 * @see hw_get_adc_values() to get the raw ADC value
 */
uint32_t pwrctl_calc_vout(uint16_t raw);

/**
 * @brief Calculate DAC value for desired output voltage
 *
 * Converts a target output voltage to the raw DAC value needed
 * to achieve that voltage, using calibration coefficients.
 *
 * Formula: DAC = v_dac_k_coef * V_out_mv + v_dac_c_coef
 *
 * @param[in] v_out_mv Desired output voltage in millivolts
 * @return DAC value to program (0-4095 for 12-bit DAC)
 *
 * @see hw_set_voltage_dac() to write the DAC value
 */
uint16_t pwrctl_calc_vout_dac(uint32_t v_out_mv);

/**
 * @brief Calculate output current from raw ADC value
 *
 * Converts a raw ADC reading from the output current sensing circuit
 * to the actual output current in milliamps using calibration coefficients.
 *
 * Formula: I_out_ma = a_adc_k_coef * raw + a_adc_c_coef
 *
 * @param[in] raw Raw ADC value (0-4095 for 12-bit ADC)
 * @return Output current in milliamps
 *
 * @see hw_get_adc_values() to get the raw ADC value
 */
uint32_t pwrctl_calc_iout(uint16_t raw);

/**
 * @brief Calculate expected ADC value for given current limit
 *
 * Calculates what raw ADC value corresponds to a given current limit.
 * This is used for fast OCP comparison in the ISR without floating point.
 *
 * @param[in] i_limit_ma Current limit in milliamps
 * @return Expected raw ADC value at that current
 *
 * @note The result is stored in pwrctl_i_limit_raw for ISR use
 */
uint32_t pwrctl_calc_ilimit_adc(uint16_t i_limit_ma);

/**
 * @brief Calculate expected ADC value for given voltage limit
 *
 * Calculates what raw ADC value corresponds to a given voltage limit.
 * This is used for fast OVP comparison in the ISR without floating point.
 *
 * @param[in] v_limit_mv Voltage limit in millivolts
 * @return Expected raw ADC value at that voltage
 *
 * @note The result is stored in pwrctl_v_limit_raw for ISR use
 */
uint32_t pwrctl_calc_vlimit_adc(uint16_t v_limit_mv);

/**
 * @brief Calculate DAC value for desired output current
 *
 * Converts a target output current to the raw DAC value needed
 * to achieve that current in constant current mode.
 *
 * Formula: DAC = a_dac_k_coef * I_out_ma + a_dac_c_coef
 *
 * @param[in] i_out_ma Desired output current in milliamps
 * @return DAC value to program (0-4095 for 12-bit DAC)
 *
 * @see hw_set_current_dac() to write the DAC value
 */
uint16_t pwrctl_calc_iout_dac(uint32_t i_out_ma);

#endif // __PWRCTL_H__
