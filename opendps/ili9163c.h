/*
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
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * ILI9163C driver based on TFT_ILI9163C: https://github.com/sumotoy/TFT_ILI9163C
 * Copyright (c) 2014, .S.U.M.O.T.O.Y., coded by Max MC Costa.
 *
 */

/**
 * @file ili9163c.h
 * @brief ILI9163C TFT Display Controller Driver
 *
 * This module provides a low-level driver for the ILI9163C TFT LCD
 * controller. The ILI9163C is a 128x160 pixel, 262K color display
 * controller commonly used in small TFT modules.
 *
 * ## Hardware Interface
 *
 * The driver communicates with the ILI9163C via SPI:
 * - SPI data transfer using DMA
 * - D/C (Data/Command) pin for command vs data selection
 * - RST pin for hardware reset
 * - CS (Chip Select) pin for SPI slave selection
 *
 * ## Color Format
 *
 * Colors use RGB565 format (16-bit):
 * ```
 * Bit:  15 14 13 12 11 | 10 9 8 7 6 5 | 4 3 2 1 0
 *       R4 R3 R2 R1 R0 | G5 G4 G3 G2 G1 G0 | B4 B3 B2 B1 B0
 * ```
 *
 * ## Coordinate System
 *
 * - Origin (0,0) is at top-left corner
 * - X increases to the right (0-127)
 * - Y increases downward (0-159)
 * - Rotation can change the logical orientation
 *
 * ## Drawing Model
 *
 * For efficient drawing, this driver uses a windowed approach:
 * 1. Call ili9163c_set_window() to define the drawing region
 * 2. Call ili9163c_push_color() repeatedly to fill the window
 *
 * This minimizes SPI command overhead for bulk operations.
 *
 * @see tft.h for the higher-level TFT abstraction
 * @see spi_driver.h for SPI communication
 * @see ili9163c_registers.h for ILI9163C command definitions
 * @see ili9163c_settings.h for display configuration
 */

#ifndef _ILI9163C_H_
#define _ILI9163C_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @defgroup ILI9163C_Colors Predefined Colors (RGB565)
 * @brief Standard colors in RGB565 format
 *
 * These color constants are in RGB565 format for direct use
 * with the ILI9163C display driver.
 *
 * RGB565 conversion: color = ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3)
 * @{
 */
#define BLACK       0x0000      /**< RGB(  0,   0,   0) - Pure black */
#define NAVY        0x000F      /**< RGB(  0,   0, 128) - Dark blue */
#define DARKGREEN   0x03E0      /**< RGB(  0, 128,   0) - Dark green */
#define DARKCYAN    0x03EF      /**< RGB(  0, 128, 128) - Dark cyan */
#define MAROON      0x7800      /**< RGB(128,   0,   0) - Dark red */
#define PURPLE      0x780F      /**< RGB(128,   0, 128) - Purple */
#define OLIVE       0x7BE0      /**< RGB(128, 128,   0) - Olive/khaki */
#define LIGHTGREY   0xC618      /**< RGB(192, 192, 192) - Light gray */
#define DARKGREY    0x7BEF      /**< RGB(128, 128, 128) - Dark gray */
#define BLUE        0x001F      /**< RGB(  0,   0, 255) - Pure blue */
#define GREEN       0x07E0      /**< RGB(  0, 255,   0) - Pure green */
#define CYAN        0x07FF      /**< RGB(  0, 255, 255) - Cyan */
#define RED         0xF800      /**< RGB(255,   0,   0) - Pure red */
#define MAGENTA     0xF81F      /**< RGB(255,   0, 255) - Magenta */
#define YELLOW      0xFFE0      /**< RGB(255, 255,   0) - Yellow */
#define WHITE       0xFFFF      /**< RGB(255, 255, 255) - Pure white */
#define ORANGE      0xFD20      /**< RGB(255, 165,   0) - Orange */
#define GREENYELLOW 0xAFE5      /**< RGB(173, 255,  47) - Green-yellow */
#define PINK        0xF81F      /**< RGB(255,   0, 255) - Pink (same as magenta) */
/** @} */

/**
 * @brief Initialize the ILI9163C display controller
 *
 * Performs the full initialization sequence:
 * 1. Hardware reset via RST pin
 * 2. Software reset command
 * 3. Power control configuration
 * 4. Gamma settings
 * 5. Display orientation setup
 * 6. Color mode configuration (RGB565)
 * 7. Display on
 *
 * @note Must be called before any other ili9163c functions
 * @note Requires SPI to be initialized first (spi_init())
 */
void ili9163c_init(void);

/**
 * @brief Get the display geometry
 *
 * Returns the current display dimensions, which depend on
 * the rotation setting.
 *
 * @param[out] width  Receives the display width in pixels
 * @param[out] height Receives the display height in pixels
 *
 * @note Dimensions swap when rotation is 90 or 270 degrees
 */
void ili9163c_get_geometry(uint16_t *width, uint16_t *height);

/**
 * @brief Set the drawing window for subsequent pixel writes
 *
 * Defines a rectangular region on the display. Subsequent calls
 * to ili9163c_push_color() will fill pixels within this window
 * from left to right, top to bottom.
 *
 * @param[in] x0 Left edge X coordinate (inclusive)
 * @param[in] y0 Top edge Y coordinate (inclusive)
 * @param[in] x1 Right edge X coordinate (inclusive)
 * @param[in] y1 Bottom edge Y coordinate (inclusive)
 *
 * @note Window is automatically reset after filling
 * @note Coordinates are clipped to display bounds
 */
void ili9163c_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

/**
 * @brief Write a single pixel color to the display
 *
 * Writes one RGB565 color value to the current window position.
 * The position advances automatically (left-to-right, then
 * top-to-bottom within the window).
 *
 * @param[in] color RGB565 color value
 *
 * @note Call ili9163c_set_window() first to define the drawing area
 * @note For efficiency, batch multiple colors in a loop
 */
void ili9163c_push_color(uint16_t color);

/**
 * @brief Fill the entire screen with a color
 *
 * Efficiently fills all pixels with the specified color.
 *
 * @param[in] color RGB565 color value to fill with
 */
void ili9163c_fill_screen(uint16_t color);

/**
 * @brief Draw a single pixel at specific coordinates
 *
 * Sets one pixel to the specified color. This is slower than
 * using set_window + push_color for multiple pixels.
 *
 * @param[in] x     X coordinate
 * @param[in] y     Y coordinate
 * @param[in] color RGB565 color value
 *
 * @note Does nothing if coordinates are outside display bounds
 */
void ili9163c_draw_pixel(int16_t x, int16_t y, uint16_t color);

/**
 * @brief Fill a rectangular area with a color
 *
 * Efficiently fills a rectangle with the specified color.
 *
 * @param[in] x      Left edge X coordinate
 * @param[in] y      Top edge Y coordinate
 * @param[in] w      Width in pixels
 * @param[in] h      Height in pixels
 * @param[in] color  RGB565 color value
 *
 * @note Rectangle is clipped to display bounds
 */
void ili9163c_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

/**
 * @brief Set the display rotation
 *
 * Configures the logical orientation of the display.
 *
 * @param[in] r Rotation value:
 *              - 0: Normal (portrait, connector at bottom)
 *              - 1: 90° clockwise (landscape)
 *              - 2: 180° (portrait, connector at top)
 *              - 3: 270° clockwise (landscape)
 *
 * @note Changes the effective width/height returned by get_geometry()
 */
void ili9163c_set_rotation(uint8_t r);

/**
 * @brief Enable or disable display color inversion
 *
 * When inverted, all colors are bitwise-inverted (NOT operation).
 * This can be useful for highlighting or visual effects.
 *
 * @param[in] i true to invert colors, false for normal display
 */
void ili9163c_invert_display(bool i);

/**
 * @brief Turn the display on or off
 *
 * Controls the display output. When off, the display shows
 * a blank screen but retains its frame buffer contents.
 *
 * @param[in] on true to turn display on, false to turn off
 */
void ili9163c_display(bool on);

/**
 * @brief Check if coordinates are within display bounds
 *
 * Validates that the given coordinates are within the current
 * display dimensions (accounting for rotation).
 *
 * @param[in] x X coordinate to check
 * @param[in] y Y coordinate to check
 * @return true if coordinates are valid
 * @return false if coordinates are outside display bounds
 */
bool ili9163c_boundary_check(int16_t x, int16_t y);

/**
 * @brief Draw a vertical line
 *
 * Efficiently draws a vertical line using optimized memory writes.
 *
 * @param[in] x      X coordinate of the line
 * @param[in] y      Starting Y coordinate (top)
 * @param[in] h      Height (length) of the line in pixels
 * @param[in] color  RGB565 color value
 *
 * @note Line is clipped to display bounds
 */
void ili9163c_draw_vline(int16_t x, int16_t y, int16_t h, uint16_t color);

/**
 * @brief Draw a horizontal line
 *
 * Efficiently draws a horizontal line using optimized memory writes.
 *
 * @param[in] x      Starting X coordinate (left)
 * @param[in] y      Y coordinate of the line
 * @param[in] w      Width (length) of the line in pixels
 * @param[in] color  RGB565 color value
 *
 * @note Line is clipped to display bounds
 */
void ili9163c_draw_hline(int16_t x, int16_t y, int16_t w, uint16_t color);

#endif // _ILI9163C_H_
