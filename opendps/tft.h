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
 * @file tft.h
 * @brief High-Level TFT Display Driver for OpenDPS
 *
 * This module provides the high-level interface for the TFT display used
 * in OpenDPS. It abstracts the underlying ILI9163C display controller
 * and provides drawing primitives for the user interface.
 *
 * ## Display Characteristics
 *
 * - Resolution: 128x160 pixels
 * - Color depth: 16-bit BGR565 format
 * - Controller: ILI9163C (or compatible)
 * - Interface: SPI with DMA
 *
 * ## Color Format
 *
 * Colors are in BGR565 format (16 bits per pixel):
 * - Bits 15-11: Blue (5 bits, 0-31)
 * - Bits 10-5: Green (6 bits, 0-63)
 * - Bits 4-0: Red (5 bits, 0-31)
 *
 * Use the color constants from ili9163c.h (BLACK, WHITE, RED, GREEN, etc.)
 *
 * ## Font System
 *
 * The display supports multiple font sizes:
 * - FONT_FULL_SMALL: Complete ASCII character set, small size (for menus)
 * - FONT_METER_SMALL: Digits and units only, small size
 * - FONT_METER_MEDIUM: Digits and units only, medium size
 * - FONT_METER_LARGE: Digits and units only, large size
 *
 * The METER fonts contain only digits 0-9, decimal point, minus sign, and
 * common unit characters (m, V, A, etc.) to minimize flash usage.
 *
 * Fonts use 2-bit-per-pixel encoding for anti-aliasing with 4 gray levels.
 *
 * ## Coordinate System
 *
 * - Origin (0,0) is at the top-left corner
 * - X increases to the right (0-127)
 * - Y increases downward (0-159)
 *
 * @see ili9163c.h for low-level display control and color constants
 * @see font-*.c for font definitions
 */

#ifndef __TFT_H__
#define __TFT_H__

/**
 * @brief Available font sizes for text rendering
 *
 * These font sizes are optimized for the 128x160 display resolution.
 * FONT_FULL_SMALL contains the complete ASCII character set (32-127),
 * while the METER fonts contain only digits, decimal point, and unit
 * characters for displaying voltage/current values efficiently.
 */
typedef enum
{
    /** @brief Full ASCII character set, small size (~8 pixels high) */
    FONT_FULL_SMALL,
    /** @brief Meter font: digits 0-9, '.', '-', units, small size */
    FONT_METER_SMALL,
    /** @brief Meter font: digits 0-9, '.', '-', units, medium size */
    FONT_METER_MEDIUM,
    /** @brief Meter font: digits 0-9, '.', '-', units, large size (~32 pixels) */
    FONT_METER_LARGE
} tft_font_size_t;

/**
 * @brief Initialize the TFT display module
 *
 * Initializes the display hardware and software by:
 * - Initializing the ILI9163C controller
 * - Setting display rotation to landscape mode
 * - Loading display inversion setting from persistent storage
 * - Clearing the display to black
 *
 * @note Must be called after hw_init() which configures SPI
 * @note Must be called before any other tft_* functions
 */
void tft_init(void);

/**
 * @brief Clear the entire display to black
 *
 * Fills the entire 128x160 display area with black (0x0000).
 * This is optimized for speed compared to calling tft_fill().
 */
void tft_clear(void);

/**
 * @brief Get the horizontal spacing between glyphs
 *
 * Returns the number of pixels to add between characters when
 * rendering text with the specified font size. This spacing
 * ensures readable text without characters running together.
 *
 * @param[in] size Font size to query
 * @return Horizontal spacing in pixels (typically 1-3)
 */
uint8_t tft_get_glyph_spacing(tft_font_size_t size);

/**
 * @brief Get the dimensions of a character glyph
 *
 * Returns the width and height of a specific character in the
 * specified font. Used for calculating text layout and positioning.
 *
 * @param[in]  size         Font size to use
 * @param[in]  ch           Character to measure (must be in font's character set)
 * @param[out] glyph_width  Receives the character width in pixels
 * @param[out] glyph_height Receives the character height in pixels
 *
 * @note For variable-width fonts, each character may have different width
 * @note Returns zero dimensions for unsupported characters
 */
void tft_get_glyph_metrics(tft_font_size_t size, char ch, uint32_t *glyph_width, uint32_t *glyph_height);

/**
 * @brief Get the pixel data for a character glyph
 *
 * Returns a pointer to the compressed 2bpp pixel data for rendering
 * a character. The data is encoded with 4 gray levels for anti-aliasing.
 *
 * @param[in]  size          Font size to use
 * @param[in]  ch            Character to get (must be in font's character set)
 * @param[out] glyph_pixdata Receives pointer to compressed pixel data
 * @param[out] glyph_size    Receives size of pixel data in bytes
 *
 * @note The pixel data must be decoded with tft_decode_glyph() before display
 * @see tft_decode_glyph()
 */
void tft_get_glyph_pixdata(tft_font_size_t size, char ch, const uint8_t **glyph_pixdata, uint32_t *glyph_size);

/**
 * @brief Decode a 2bpp glyph to BGR565 format
 *
 * Converts compressed 2-bit-per-pixel font data to the native 16-bit
 * BGR565 format for the TFT. The result is stored in an internal
 * blit buffer ready for tft_blit().
 *
 * The 2bpp encoding provides 4 levels of anti-aliasing:
 * - 0: Background (transparent/black)
 * - 1: 33% foreground intensity
 * - 2: 66% foreground intensity
 * - 3: 100% foreground (full color)
 *
 * @param[in] pixdata Pointer to 2bpp compressed glyph data
 * @param[in] nbytes  Number of bytes in the glyph data
 * @param[in] invert  If true, swap foreground and background
 * @param[in] color   Foreground color in BGR565 format
 *
 * @note Result is placed in internal blit_buffer, not returned
 */
void tft_decode_glyph(const uint8_t *pixdata, size_t nbytes, bool invert, uint16_t color);

/**
 * @brief Blit raw pixel data to the display
 *
 * Copies a rectangular region of pixels directly to the display using
 * SPI with DMA. This is the fastest way to draw graphics and images.
 *
 * @param[in] bits   Pointer to pixel data in BGR565 format (row-major order)
 * @param[in] width  Width of the source rectangle in pixels
 * @param[in] height Height of the source rectangle in pixels
 * @param[in] x      X coordinate of top-left corner on display (0-127)
 * @param[in] y      Y coordinate of top-left corner on display (0-159)
 *
 * @note Data must be in BGR565 format (2 bytes per pixel)
 * @note No bounds checking is performed; ensure coordinates are valid
 */
void tft_blit(uint16_t *bits, uint32_t width, uint32_t height, uint32_t x, uint32_t y);

/**
 * @brief Draw a single character to the display
 *
 * Renders a character at the specified position with the given color.
 * The character is decoded from the font, converted to BGR565, and
 * blitted to the display.
 *
 * @param[in] size   Font size to use
 * @param[in] ch     Character to draw (must be in font's character set)
 * @param[in] x      X coordinate of the character position
 * @param[in] y      Y coordinate of the character position
 * @param[in] w      Width of the bounding box (for clipping)
 * @param[in] h      Height of the bounding box (for clipping)
 * @param[in] color  Text color in BGR565 format
 * @param[in] invert If true, draw with inverted colors (for highlighting)
 * @return Width of the character drawn in pixels
 */
uint8_t tft_putch(tft_font_size_t size, char ch, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t color, bool invert);

/**
 * @brief Calculate the pixel dimensions of a string
 *
 * Computes the total width and height required to render a string
 * with the specified font, including inter-character spacing.
 *
 * @param[in]  size          Font size to use
 * @param[in]  str           Null-terminated string to measure
 * @param[out] string_width  Receives total width in pixels
 * @param[out] string_height Receives total height in pixels (max glyph height)
 *
 * @note Useful for centering text or calculating layout
 */
void tft_get_string_metrics(tft_font_size_t size, const char *str, uint32_t *string_width, uint32_t *string_height);

/**
 * @brief Draw a string to the display
 *
 * Renders a null-terminated string at the specified position.
 * The position is anchored to the bottom-left of the string baseline.
 * Characters are drawn left-to-right.
 *
 * @param[in] size   Font size to use
 * @param[in] str    Null-terminated string to draw
 * @param[in] x      X coordinate (left side of string)
 * @param[in] y      Y coordinate (bottom of string baseline)
 * @param[in] w      Width of bounding box (for clipping)
 * @param[in] h      Height of bounding box (for clipping)
 * @param[in] color  Text color in BGR565 format
 * @param[in] invert If true, draw with inverted colors
 * @return Total width of the string drawn in pixels
 */
uint16_t tft_puts(tft_font_size_t size, const char *str, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t color, bool invert);

/**
 * @brief Fill a rectangular area with a repeating pattern
 *
 * Fills the specified area by repeating the given pattern buffer.
 * Useful for creating textured backgrounds, gradients, or dithered fills.
 *
 * @param[in] x1        X coordinate of top-left corner
 * @param[in] y1        Y coordinate of top-left corner
 * @param[in] x2        X coordinate of bottom-right corner
 * @param[in] y2        Y coordinate of bottom-right corner
 * @param[in] fill      Pattern buffer containing BGR565 pixel data
 * @param[in] fill_size Size of pattern buffer in bytes
 *
 * @note Pattern wraps around if area is larger than pattern
 */
void tft_fill_pattern(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint8_t *fill, uint32_t fill_size);

/**
 * @brief Fill a rectangular area with a solid color
 *
 * Fills the specified rectangular area with a single solid color.
 * Optimized for speed when clearing regions or drawing solid backgrounds.
 *
 * @param[in] x     X coordinate of top-left corner
 * @param[in] y     Y coordinate of top-left corner
 * @param[in] w     Width of the area in pixels
 * @param[in] h     Height of the area in pixels
 * @param[in] color Fill color in BGR565 format
 */
void tft_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t color);

/**
 * @brief Draw a rectangle outline
 *
 * Draws a one-pixel-wide rectangle outline (frame) at the specified
 * position. The rectangle is not filled.
 *
 * @param[in] xpos   X coordinate of top-left corner
 * @param[in] ypos   Y coordinate of top-left corner
 * @param[in] width  Width of the rectangle (outer dimension)
 * @param[in] height Height of the rectangle (outer dimension)
 * @param[in] color  Line color in BGR565 format
 */
void tft_rect(uint32_t xpos, uint32_t ypos, uint32_t width, uint32_t height, uint16_t color);

/**
 * @brief Enable or disable display color inversion
 *
 * Enables or disables hardware color inversion on the display.
 * When enabled, all colors are inverted (black becomes white, etc.).
 * This setting is persisted to flash storage.
 *
 * @param[in] invert true to enable color inversion, false for normal mode
 *
 * @note Some displays may have inverted default colors due to hardware
 * @note Use this to compensate for displays with opposite color polarity
 */
void tft_invert(bool invert);

/**
 * @brief Check if display color inversion is enabled
 *
 * Returns whether display color inversion is currently active.
 *
 * @return true if the display colors are inverted
 * @return false if the display is in normal mode
 */
bool tft_is_inverted(void);

#ifdef DPS_EMULATOR
/**
 * @brief Update the emulator display window
 *
 * When running in the PC emulator, this function updates the SDL
 * window to show the current display buffer contents. Must be called
 * after drawing operations to see the result on screen.
 *
 * @note Only available when compiling with DPS_EMULATOR defined
 * @note On real hardware, display updates happen during blit operations
 */
void emul_tft_draw(void);
#endif // DPS_EMULATOR

#endif // __TFT_H__
