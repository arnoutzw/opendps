/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Cyril Russo (github.com/X-Ryl669)
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
 * @file func_gen.h
 * @brief Function Generator Mode
 *
 * This module implements a function generator operating mode for OpenDPS.
 * It generates time-varying voltage waveforms useful for testing and
 * signal simulation.
 *
 * ## Supported Waveforms
 *
 * - **Sine**: Sinusoidal waveform
 * - **Square**: Square wave with adjustable duty cycle
 * - **Sawtooth**: Linear ramp waveform
 * - **Triangle**: Symmetric triangle wave
 *
 * ## Parameters
 *
 * | Name      | Unit | Description |
 * |-----------|------|-------------|
 * | waveform  | -    | Waveform type (sine, square, etc.) |
 * | frequency | Hz   | Output frequency |
 * | amplitude | mV   | Peak-to-peak voltage |
 * | offset    | mV   | DC offset voltage |
 * | duty      | %    | Duty cycle (for square wave) |
 *
 * ## Limitations
 *
 * Due to hardware constraints:
 * - Maximum frequency is limited by DAC update rate (~1 kHz)
 * - High frequencies may have reduced amplitude accuracy
 * - Output capacitance affects high-frequency response
 * - Cannot generate negative voltages (offset must compensate)
 *
 * ## Implementation
 *
 * The function generator uses the funcgen_tick callback from hw.h
 * which is called at a high rate (~50 kHz) from the ADC ISR.
 * This provides consistent timing for waveform generation.
 *
 * @note This function is only available when CONFIG_FUNCGEN_ENABLE is defined
 * @see hw.h for the funcgen_tick mechanism
 * @see uui.h for the UI framework
 */

#ifndef __FUNC_GEN_H__
#define __FUNC_GEN_H__

#include "uui.h"

/**
 * @brief Initialize and register the function generator
 *
 * Creates the function generator screen and registers it with the UI
 * framework. This includes:
 * - Creating waveform, frequency, amplitude, and offset input items
 * - Setting up the waveform generation timer callback
 * - Initializing lookup tables for waveform synthesis
 * - Setting up parameter handlers for remote control
 *
 * @param[in,out] ui The user interface to add the function generator to
 *
 * @note Called once during system initialization
 * @note Only available when CONFIG_FUNCGEN_ENABLE is defined
 * @note Must be called after uui_init() and past_init()
 */
void func_gen_init(uui_t *ui);

#endif // __FUNC_GEN_H__
