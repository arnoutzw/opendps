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
 * @file func_cl.h
 * @brief Current Limit (CL) Function Mode
 *
 * This module implements the Current Limit operating mode for OpenDPS.
 * CL mode is similar to CV mode but with soft current limiting instead
 * of triggering OCP when the current limit is reached.
 *
 * ## Parameters
 *
 * | Name    | Unit | Description |
 * |---------|------|-------------|
 * | voltage | mV   | Target output voltage |
 * | current | mA   | Maximum current (soft limit) |
 *
 * ## Behavior
 *
 * The CL mode operates in two states:
 *
 * 1. **CV State**: When load current is below the limit
 *    - Output voltage is regulated to the set value
 *    - Current varies with load
 *
 * 2. **CL State**: When load would draw more than the limit
 *    - Current is limited to the set value
 *    - Voltage drops to maintain current limit
 *    - Output is NOT disabled (unlike OCP)
 *
 * ## Comparison with CV Mode
 *
 * | Aspect | CV Mode | CL Mode |
 * |--------|---------|---------|
 * | Over current | Triggers OCP, disables output | Limits current, reduces voltage |
 * | User action | Must re-enable | No action needed |
 * | Use case | Protection-focused | Continuous operation |
 *
 * ## Use Cases
 *
 * - Powering devices with inrush current
 * - Testing with variable loads
 * - Applications where brief overcurrent is acceptable
 *
 * @see func_cv.h for the CV mode with hard OCP
 * @see pwrctl.h for low-level voltage/current control
 */

#ifndef __FUNC_CL_H__
#define __FUNC_CL_H__

#include "uui.h"

/**
 * @brief Initialize and register the CL function
 *
 * Creates the CL screen and registers it with the UI framework.
 * This includes:
 * - Creating voltage and current limit input items
 * - Setting up parameter handlers for remote control
 * - Restoring saved settings from PAST
 *
 * @param[in,out] ui The user interface to add the CL function to
 *
 * @note Called once during system initialization
 * @note Must be called after uui_init() and past_init()
 */
void func_cl_init(uui_t *ui);

#endif // __FUNC_CL_H__
