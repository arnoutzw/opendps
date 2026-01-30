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
 * @file spi_driver.h
 * @brief SPI Communication Driver with DMA Support
 *
 * This module provides SPI communication functionality for the STM32F100
 * microcontroller with DMA (Direct Memory Access) support for efficient
 * data transfers.
 *
 * ## Hardware Configuration
 *
 * The SPI driver is configured for:
 * - SPI1 peripheral on the STM32F100
 * - Master mode operation
 * - 8-bit data frame
 * - MSB first transmission
 * - Mode 0 (CPOL=0, CPHA=0) or configurable
 *
 * ## DMA Operation
 *
 * DMA is used for both transmit and receive operations:
 * - TX: Memory to SPI data register
 * - RX: SPI data register to memory
 *
 * This allows the CPU to perform other tasks during transfers
 * and provides consistent timing for display updates.
 *
 * ## Primary Usage
 *
 * The SPI driver is primarily used for:
 * - TFT display communication (ILI9163C)
 * - Potential external peripherals on SPI bus
 *
 * ## Usage Example
 *
 * ```c
 * // Initialize SPI
 * spi_init();
 *
 * // Send display command
 * uint8_t cmd = ILI9163C_CASET;
 * spi_dma_transceive(&cmd, 1, NULL, 0);
 *
 * // Send data
 * uint8_t data[] = {0x00, 0x00, 0x00, 0x7F};
 * spi_dma_transceive(data, sizeof(data), NULL, 0);
 *
 * // Full-duplex read/write
 * uint8_t tx_buf[4] = {0x9F, 0x00, 0x00, 0x00};
 * uint8_t rx_buf[4];
 * spi_dma_transceive(tx_buf, 4, rx_buf, 4);
 * ```
 *
 * @see ili9163c.h for TFT display driver
 * @see hw.h for GPIO pin configuration
 */

#ifndef __SPI_DRIVER_H__
#define __SPI_DRIVER_H__

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize the SPI driver
 *
 * Configures the SPI1 peripheral and associated DMA channels:
 * - GPIO pins for SPI (MOSI, MISO, SCK)
 * - SPI peripheral configuration
 * - DMA channels for TX and RX
 *
 * @note Must be called before any SPI communication
 * @note Called automatically by hw_init()
 */
void spi_init(void);

/**
 * @brief Perform SPI data transfer using DMA
 *
 * Transmits data and optionally receives data simultaneously
 * using DMA for efficient transfer. The function blocks until
 * the transfer is complete.
 *
 * ## Transfer Modes
 *
 * | tx_buf | rx_buf | Operation |
 * |--------|--------|-----------|
 * | valid  | NULL   | Transmit only |
 * | valid  | valid  | Full duplex |
 * | NULL   | valid  | Receive only (sends dummy bytes) |
 *
 * @param[in]  tx_buf Transmit buffer containing data to send
 * @param[in]  tx_len Number of bytes to transmit
 * @param[out] rx_buf Receive buffer for incoming data (may be NULL)
 * @param[in]  rx_len Number of bytes to receive (may be 0)
 *
 * @return true  Transfer completed successfully
 * @return false Invalid parameters or DMA error
 *
 * @note tx_len and rx_len must be equal for full-duplex operation
 * @note Function blocks until DMA transfer completes
 * @note Caller must manage chip select (CS) pin
 */
bool spi_dma_transceive(uint8_t *tx_buf, uint32_t tx_len, uint8_t *rx_buf, uint32_t rx_len);

#endif // __SPI_DRIVER_H__
