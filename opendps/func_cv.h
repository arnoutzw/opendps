/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Johan Kanflo (github.com/kanflo)
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
 * @file func_cv.h
 * @brief Constant Voltage (CV) Function Mode
 *
 * This module implements the Constant Voltage operating mode for OpenDPS.
 * In CV mode, the power supply maintains a constant output voltage while
 * the current varies based on the connected load.
 *
 * ## Parameters
 *
 * | Name    | Unit | Description |
 * |---------|------|-------------|
 * | voltage | mV   | Target output voltage |
 * | current | mA   | Current limit (OCP threshold) |
 *
 * ## Behavior
 *
 * - Output voltage is regulated to the set value
 * - If load draws more than the current limit, OCP triggers
 * - Display shows: set voltage, current limit, actual V and I
 *
 * ## OCP (Over Current Protection)
 *
 * When the output current exceeds the configured limit:
 * 1. Output is automatically disabled
 * 2. OCP event is generated and logged
 * 3. Display shows OCP indicator
 * 4. User must manually re-enable output
 *
 * @see pwrctl.h for low-level voltage/current control
 * @see uui.h for the UI framework
 */

#ifndef __FUNC_CV_H__
#define __FUNC_CV_H__

#include "uui.h"

/**
 * @brief Initialize and register the CV function
 *
 * Creates the CV screen and registers it with the UI framework.
 * This includes:
 * - Creating voltage and current limit input items
 * - Setting up parameter handlers for remote control
 * - Restoring saved settings from PAST
 *
 * @param[in,out] ui The user interface to add the CV function to
 *
 * @note Called once during system initialization
 * @note Must be called after uui_init() and past_init()
 */
void func_cv_init(uui_t *ui);

#endif // __FUNC_CV_H__
