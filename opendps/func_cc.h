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
 * @file func_cc.h
 * @brief Constant Current (CC) Function Mode
 *
 * This module implements the Constant Current operating mode for OpenDPS.
 * In CC mode, the power supply maintains a constant output current while
 * the voltage adjusts based on the load resistance.
 *
 * ## Parameters
 *
 * | Name    | Unit | Description |
 * |---------|------|-------------|
 * | current | mA   | Target output current |
 * | voltage | mV   | Voltage limit (OVP threshold) |
 *
 * ## Behavior
 *
 * - Output current is regulated to the set value
 * - Voltage adjusts automatically based on load resistance
 * - If voltage would exceed limit, OVP triggers
 * - Display shows: set current, voltage limit, actual V and I
 *
 * ## Use Cases
 *
 * - Battery charging (constant current phase)
 * - LED testing and driving
 * - Electroplating
 * - Any application requiring controlled current
 *
 * ## OVP (Over Voltage Protection)
 *
 * When the output voltage would exceed the configured limit:
 * 1. Output is automatically disabled
 * 2. OVP event is generated
 * 3. Display shows OVP indicator
 *
 * @see pwrctl.h for low-level voltage/current control
 * @see uui.h for the UI framework
 */

#ifndef __FUNC_CC_H__
#define __FUNC_CC_H__

#include "uui.h"

/**
 * @brief Initialize and register the CC function
 *
 * Creates the CC screen and registers it with the UI framework.
 * This includes:
 * - Creating current and voltage limit input items
 * - Setting up parameter handlers for remote control
 * - Restoring saved settings from PAST
 *
 * @param[in,out] ui The user interface to add the CC function to
 *
 * @note Called once during system initialization
 * @note Must be called after uui_init() and past_init()
 */
void func_cc_init(uui_t *ui);

#endif // __FUNC_CC_H__
