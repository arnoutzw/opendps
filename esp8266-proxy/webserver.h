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

#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include <stdint.h>
#include <stdbool.h>
#include "uframe.h"

/** HTTP server port */
#define HTTP_PORT 80

/** Callback function type for UART communication
 *  Takes a frame to send and receives the response in the same frame
 *  Returns true on success, false on timeout/error
 */
typedef bool (*uart_comm_func_t)(frame_t *frame);

/**
 * @brief Initialize the web server
 * @param comm_func Function to use for UART communication with DPS
 */
void webserver_init(uart_comm_func_t comm_func);

/**
 * @brief Web server task - call this from FreeRTOS
 * @param pvParameters unused
 */
void webserver_task(void *pvParameters);

#endif // __WEBSERVER_H__
