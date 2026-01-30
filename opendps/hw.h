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
 * @file hw.h
 * @brief Hardware Abstraction Layer for OpenDPS
 *
 * This header provides the hardware abstraction layer (HAL) for the OpenDPS
 * firmware running on the STM32F100 microcontroller. It defines:
 *
 * - GPIO pin mappings for buttons, TFT display, and rotary encoder
 * - ADC channel definitions for voltage and current sensing
 * - Hardware initialization and configuration functions
 * - DAC control for voltage and current output
 * - TFT backlight control
 * - Button and rotary encoder handling
 *
 * The DPS power supply hardware consists of:
 * - STM32F100C8T6 microcontroller (64KB flash, 8KB RAM)
 * - 1.8" TFT display (128x160 pixels, ILI9163C controller)
 * - 4 buttons (SEL, M1, M2, ENABLE)
 * - Rotary encoder with push button
 * - Dual 12-bit DAC for voltage and current control
 * - 12-bit ADC for voltage and current sensing
 *
 * @note This file must be paired with the model-specific definitions in dps-model.h
 * @see dps-model.h for ADC/DAC calibration coefficients
 */

#ifndef __HW_H__
#define __HW_H__
#include "dps-model.h"

/**
 * @def V_IO_DELTA
 * @brief Maximum voltage drop between input and output (in millivolts)
 *
 * The DPS power supply is a buck converter and requires the input voltage
 * to be at least V_IO_DELTA millivolts higher than the desired output voltage.
 * This constant is used to calculate the maximum achievable output voltage
 * based on the current input voltage.
 *
 * Formula: Max Vout = Vin - V_IO_DELTA
 */
#define V_IO_DELTA (800)

/**
 * @defgroup ADC_Channels ADC Channel Definitions
 * @brief ADC channel assignments for voltage and current sensing
 * @{
 */

/** @brief ADC channel for output current measurement (I_out) */
#define ADC_CHA_IOUT  (7)

/** @brief ADC channel for input voltage measurement (V_in) */
#define ADC_CHA_VIN   (8)

/** @brief ADC channel for output voltage measurement (V_out) */
#define ADC_CHA_VOUT  (9)

/** @} */ // end of ADC_Channels

/**
 * @defgroup TFT_GPIO TFT Display GPIO Configuration
 * @brief GPIO pin assignments for the ILI9163C TFT display
 * @{
 */

/** @brief TFT reset signal port */
#define TFT_RST_PORT GPIOB
/** @brief TFT reset signal pin (active low) */
#define TFT_RST_PIN  GPIO12
/** @brief TFT A0 (data/command) signal port */
#define TFT_A0_PORT  GPIOB
/** @brief TFT A0 signal pin (low=command, high=data) */
#define TFT_A0_PIN   GPIO14
#ifdef DPS5015
/** @brief TFT chip select port (DPS5015 specific) */
#define TFT_CSN_PORT GPIOA
/** @brief TFT chip select pin (active low, DPS5015 specific) */
#define TFT_CSN_PIN  GPIO8
#endif

/** @} */ // end of TFT_GPIO

/**
 * @defgroup Button_GPIO Button GPIO Configuration
 * @brief GPIO pin assignments and interrupt configuration for physical buttons
 *
 * The DPS power supply has 4 physical buttons plus a rotary encoder with push:
 * - SEL: Select/confirm button
 * - M1: Memory/function button 1
 * - M2: Memory/function button 2
 * - ENABLE: Power output enable/disable button
 * - Rotary encoder: Adjusts values, with push button for selection
 * @{
 */

/** @brief SEL button GPIO port */
#define BUTTON_SEL_PORT GPIOA
/** @brief SEL button GPIO pin */
#define BUTTON_SEL_PIN  GPIO2
/** @brief SEL button external interrupt line */
#define BUTTON_SEL_EXTI EXTI2
/** @brief SEL button interrupt service routine name */
#define BUTTON_SEL_isr  exti2_isr
/** @brief SEL button NVIC interrupt number */
#define BUTTON_SEL_NVIC NVIC_EXTI2_IRQ

/** @brief M1 button GPIO port */
#define BUTTON_M1_PORT GPIOA
/** @brief M1 button GPIO pin */
#define BUTTON_M1_PIN  GPIO3
/** @brief M1 button external interrupt line */
#define BUTTON_M1_EXTI EXTI3
/** @brief M1 button interrupt service routine name */
#define BUTTON_M1_isr  exti3_isr
/** @brief M1 button NVIC interrupt number */
#define BUTTON_M1_NVIC NVIC_EXTI3_IRQ

/** @brief M2 button GPIO port */
#define BUTTON_M2_PORT GPIOA
/** @brief M2 button GPIO pin */
#define BUTTON_M2_PIN  GPIO1
/** @brief M2 button external interrupt line */
#define BUTTON_M2_EXTI EXTI1
/** @brief M2 button interrupt service routine name */
#define BUTTON_M2_isr  exti1_isr
/** @brief M2 button NVIC interrupt number */
#define BUTTON_M2_NVIC NVIC_EXTI1_IRQ

/** @brief ENABLE button GPIO port */
#define BUTTON_ENABLE_PORT GPIOB
/** @brief ENABLE button GPIO pin */
#define BUTTON_ENABLE_PIN  GPIO4
/** @brief ENABLE button external interrupt line */
#define BUTTON_ENABLE_EXTI EXTI4
/** @brief ENABLE button interrupt service routine name */
#define BUTTON_ENABLE_isr  exti4_isr
/** @brief ENABLE button NVIC interrupt number */
#define BUTTON_ENABLE_NVIC NVIC_EXTI4_IRQ

/** @brief Rotary encoder press button GPIO port */
#define BUTTON_ROT_PRESS_PORT GPIOB
/** @brief Rotary encoder press button GPIO pin */
#define BUTTON_ROT_PRESS_PIN  GPIO5
/** @brief Rotary encoder press button external interrupt line */
#define BUTTON_ROT_PRESS_EXTI EXTI5
/** @brief Rotary encoder channel A GPIO port */
#define BUTTON_ROT_A_PORT     GPIOB
/** @brief Rotary encoder channel A GPIO pin */
#define BUTTON_ROT_A_PIN      GPIO8
/** @brief Rotary encoder channel A external interrupt line */
#define BUTTON_ROT_A_EXTI     EXTI8
/** @brief Rotary encoder channel B GPIO port */
#define BUTTON_ROT_B_PORT     GPIOB
/** @brief Rotary encoder channel B GPIO pin */
#define BUTTON_ROT_B_PIN      GPIO9
/** @brief Rotary encoder channel B external interrupt line */
#define BUTTON_ROT_B_EXTI     EXTI9
/** @brief Rotary encoder interrupt service routine name (shared EXTI5-9) */
#define BUTTON_ROTARY_isr     exti9_5_isr
/** @brief Rotary encoder NVIC interrupt number */
#define BUTTON_ROTARY_NVIC    NVIC_EXTI9_5_IRQ

/** @} */ // end of Button_GPIO


/**
 * @brief Initialize all hardware subsystems
 *
 * Performs complete hardware initialization including:
 * - System clock configuration (24MHz from 8MHz HSI)
 * - GPIO configuration for buttons, LEDs, and TFT control signals
 * - ADC configuration for voltage/current sensing with DMA
 * - DAC configuration for voltage/current output
 * - UART configuration for serial communication (115200 baud)
 * - External interrupt configuration for buttons
 * - Watchdog timer initialization
 * - SPI initialization for TFT display
 *
 * This function must be called once at startup before any other
 * hardware-related functions.
 *
 * @note This function enables global interrupts
 */
void hw_init(void);

/**
 * @brief Read the latest ADC measurements
 *
 * Returns the most recent raw ADC values from the continuous DMA-based
 * ADC conversion. These values need to be converted to physical units
 * using the calibration coefficients.
 *
 * @param[out] i_out_raw Pointer to receive raw output current ADC value (0-4095)
 * @param[out] v_in_raw  Pointer to receive raw input voltage ADC value (0-4095)
 * @param[out] v_out_raw Pointer to receive raw output voltage ADC value (0-4095)
 *
 * @note Any pointer may be NULL if that value is not needed
 * @see pwrctl_calc_iout() to convert i_out_raw to milliamps
 * @see pwrctl_calc_vin() to convert v_in_raw to millivolts
 * @see pwrctl_calc_vout() to convert v_out_raw to millivolts
 */
void hw_get_adc_values(uint16_t *i_out_raw, uint16_t *v_in_raw, uint16_t *v_out_raw);

/**
 * @brief Set the output voltage DAC value
 *
 * Writes a raw value to the voltage control DAC (DAC Channel 1).
 * The DAC output controls the buck converter's voltage reference.
 *
 * @param[in] v_dac Raw DAC value (0-4095 for 12-bit DAC)
 *
 * @note Use pwrctl_calc_vout_dac() to convert millivolts to DAC value
 * @see pwrctl_set_vout() for the high-level voltage setting function
 */
void hw_set_voltage_dac(uint16_t v_dac);

/**
 * @brief Set the output current DAC value
 *
 * Writes a raw value to the current control DAC (DAC Channel 2).
 * The DAC output controls the current limit or constant current reference.
 *
 * @param[in] i_dac Raw DAC value (0-4095 for 12-bit DAC)
 *
 * @note Use pwrctl_calc_iout_dac() to convert milliamps to DAC value
 * @see pwrctl_set_ilimit() for the high-level current limit function
 * @see pwrctl_set_iout() for the high-level current setting function
 */
void hw_set_current_dac(uint16_t i_dac);

/**
 * @brief Initialize and enable the TFT backlight
 *
 * Configures Timer 4 in PWM mode to drive the TFT backlight LED.
 * This function must be called before the display can be used.
 *
 * @param[in] brightness Initial backlight brightness (0-100 percent)
 *
 * @note Brightness of 0 turns the backlight off completely
 * @see hw_set_backlight() to change brightness after initialization
 */
void hw_enable_backlight(uint8_t brightness);

/**
 * @brief Set the TFT backlight brightness
 *
 * Adjusts the PWM duty cycle controlling the TFT backlight LED.
 *
 * @param[in] brightness Backlight brightness percentage (0-100)
 *
 * @note 0 = backlight off, 100 = maximum brightness
 * @note The brightness setting is stored in persistent storage
 */
void hw_set_backlight(uint8_t brightness);

/**
 * @brief Get the current TFT backlight brightness
 *
 * Returns the current backlight brightness setting.
 *
 * @return Current brightness percentage (0-100)
 */
uint8_t hw_get_backlight(void);

/**
 * @brief Get the current value that triggered OCP
 *
 * When Over Current Protection (OCP) triggers, this function returns
 * the current value in milliamps that caused the protection to activate.
 *
 * @return Current value in milliamps that triggered OCP
 *
 * @note This value is only valid after an OCP event
 * @see event_t::event_ocp
 */
uint16_t hw_get_itrig_ma(void);

/**
 * @brief Get the voltage value that triggered OVP
 *
 * When Over Voltage Protection (OVP) triggers, this function returns
 * the voltage value in millivolts that caused the protection to activate.
 *
 * @return Voltage value in millivolts that triggered OVP
 *
 * @note This value is only valid after an OVP event
 * @see event_t::event_ovp
 */
uint16_t hw_get_vtrig_mv(void);

/**
 * @brief Check for long button press and inject event
 *
 * Called periodically from the main loop to detect if a button has been
 * held for longer than the long-press threshold. If detected, a long
 * press event is injected into the event queue.
 *
 * Long press is typically used for:
 * - Entering settings/calibration mode
 * - Resetting parameters
 * - Special functions
 *
 * @note This function should be called regularly from the main loop
 */
void hw_longpress_check(void);

/**
 * @brief Check if the SEL button is currently pressed
 *
 * Reads the current state of the SEL button GPIO pin.
 * This is used for detecting button combinations and during boot
 * to check for forced upgrade mode.
 *
 * @return true if the SEL button is currently pressed
 * @return false if the SEL button is not pressed
 */
bool hw_sel_button_pressed(void);

#ifdef CONFIG_ADC_BENCHMARK
/**
 * @brief Print ADC timing information for benchmarking
 *
 * Outputs the ADC conversion timing statistics for performance analysis.
 * This function is only available when CONFIG_ADC_BENCHMARK is defined.
 *
 * @note Debug/development feature only
 */
void hw_print_ticks(void);
#endif // CONFIG_ADC_BENCHMARK

#ifdef CONFIG_FUNCGEN_ENABLE
/**
 * @brief Function pointer for function generator tick callback
 *
 * This pointer is called on each ADC update (approximately 50kHz) to
 * allow the function generator to update its output waveform in real-time.
 *
 * By using a function pointer instead of a conditional check, we avoid
 * branch prediction overhead in the time-critical ADC ISR, ensuring
 * consistent latency for waveform generation.
 *
 * @note Set to fg_noop() when function generator is not active
 * @see fg_noop()
 */
extern void (*funcgen_tick)(void);

/**
 * @brief No-operation function for function generator tick
 *
 * An empty function used as the default value for funcgen_tick when
 * the function generator is not active. This maintains consistent
 * timing in the ADC ISR by always calling a function.
 *
 * @see funcgen_tick
 */
void fg_noop(void);

/**
 * @brief Get current time in microseconds
 *
 * Returns a high-resolution timestamp in microseconds, updated by
 * a hardware timer. Used for precise timing in the function generator.
 *
 * @return Current time in microseconds (wraps at 32-bit overflow)
 *
 * @note Overflow occurs approximately every 71 minutes
 */
uint32_t cur_time_us(void);
#endif // CONFIG_FUNCGEN_ENABLE

#endif // __HW_H__
