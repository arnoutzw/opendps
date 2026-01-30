# Hardware Abstraction Layer

This document describes the hardware abstraction layer (HAL) that interfaces OpenDPS with the STM32F100 microcontroller and DPS5005 hardware.

## Microcontroller

**STM32F100C8T6** (Value Line)
- ARM Cortex-M3 core
- 24 MHz maximum clock
- 64 KB Flash
- 8 KB SRAM
- 37 GPIO pins
- 2x 12-bit ADC (16 channels)
- 2x 12-bit DAC

## Pin Assignments

### Power Control

| Function | Port | Pin | Description |
|----------|------|-----|-------------|
| V_OUT_DAC | PA4 | DAC1_OUT | Voltage setpoint output |
| I_OUT_DAC | PA5 | DAC2_OUT | Current limit setpoint |
| V_ADC | PA0 | ADC_IN0 | Output voltage feedback |
| I_ADC | PA1 | ADC_IN1 | Output current feedback |
| VIN_ADC | PA2 | ADC_IN2 | Input voltage measurement |

### User Interface

| Function | Port | Pin | Description |
|----------|------|-----|-------------|
| ENC_A | PB6 | TIM4_CH1 | Encoder channel A |
| ENC_B | PB7 | TIM4_CH2 | Encoder channel B |
| ENC_BTN | PB5 | GPIO | Encoder push button |
| SEL_BTN | PA15 | GPIO | Select button (active low) |
| M1_BTN | PB3 | GPIO | M1 button (active low) |
| M2_BTN | PB4 | GPIO | M2 button (active low) |

### Display (ILI9163C via SPI1)

| Function | Port | Pin | Description |
|----------|------|-----|-------------|
| TFT_SCK | PA5 | SPI1_SCK | SPI clock |
| TFT_MOSI | PA7 | SPI1_MOSI | SPI data out |
| TFT_CS | PB1 | GPIO | Chip select (active low) |
| TFT_RESET | PB10 | GPIO | Reset (active low) |
| TFT_A0 | PB11 | GPIO | Data/Command select |

### Serial Communication

| Function | Port | Pin | Description |
|----------|------|-----|-------------|
| UART_TX | PA9 | USART1_TX | Serial transmit |
| UART_RX | PA10 | USART1_RX | Serial receive |

### Power Control Signals

| Function | Port | Pin | Description |
|----------|------|-----|-------------|
| PWR_ENABLE | ? | GPIO | Output enable control |

## ADC Configuration

The ADC is configured for:
- 12-bit resolution (0-4095)
- Continuous conversion mode
- DMA transfer to memory
- Scan mode for multiple channels

### ADC Channels

```c
#define ADC_CHA_IOUT    (0)  // Output current
#define ADC_CHA_VOUT    (1)  // Output voltage
#define ADC_CHA_VIN     (2)  // Input voltage
```

### Reading ADC Values

```c
uint16_t i_out, v_in, v_out;
hw_get_adc_values(&i_out, &v_in, &v_out);

// Values are raw 12-bit ADC readings
// Convert using calibration coefficients:
// actual_mV = (v_out * V_ADC_K) + V_ADC_C
```

## DAC Configuration

Dual 12-bit DACs for voltage and current control:

- DAC1 (PA4): Voltage setpoint
- DAC2 (PA5): Current limit setpoint

### Setting DAC Values

```c
// Set voltage DAC (0-4095)
hw_set_voltage_dac(dac_value);

// Set current DAC (0-4095)
hw_set_current_dac(dac_value);
```

## Timer Configuration

### SysTick Timer
- 1ms tick rate
- Used for delays and periodic tasks
- Button debouncing

### TIM4 - Encoder Interface
- Configured in encoder mode
- Channels 1 & 2 for quadrature decoding
- 4x resolution (counts both edges of both channels)

### TIM2 - PWM (if applicable)
- Used for additional PWM outputs

## SPI Configuration

SPI1 is used for TFT display communication:

- Master mode
- 8-bit data frame
- MSB first
- Mode 0 (CPOL=0, CPHA=0)
- DMA enabled for efficient transfers

### SPI Transfer

```c
// Initialize SPI
spi_init();

// Transfer with DMA
uint8_t tx_data[] = {0x2C, 0x00, 0x00};
spi_dma_transceive(tx_data, sizeof(tx_data), NULL, 0);
```

## UART Configuration

USART1 for serial communication:

- Baud rate: 115200
- 8 data bits
- 1 stop bit
- No parity
- No flow control

### Serial Communication

```c
// Transmit byte
hw_send_byte(data);

// Receive handled via interrupt
void usart1_isr(void) {
    char c = usart_recv(USART1);
    serial_handle_rx_char(c);
}
```

## Button Handling

Buttons are active-low with internal pull-ups:

```c
// Button press detection (in ISR)
if (!gpio_get(GPIOB, GPIO5)) {  // Encoder button
    event_put(event_rot_press);
}
```

Debouncing is performed in the SysTick handler with a ~50ms delay.

## Display Interface

The ILI9163C display controller is connected via SPI:

### Display Specifications
- Resolution: 128x160 pixels
- Color depth: 16-bit (RGB565/BGR565)
- Interface: 4-wire SPI

### Control Signals

```c
// Command mode (A0 low)
gpio_clear(TFT_A0_PORT, TFT_A0_PIN);
spi_send_command(cmd);

// Data mode (A0 high)
gpio_set(TFT_A0_PORT, TFT_A0_PIN);
spi_send_data(data, len);
```

## Power Management

### Output Enable

The power output can be enabled/disabled:

```c
// Enable output
hw_enable_output(true);

// Disable output
hw_enable_output(false);
```

### Overcurrent Protection

Hardware OCP triggers an interrupt when current exceeds limit:

```c
void exti_isr(void) {
    if (overcurrent_detected()) {
        event_put(event_ocp);
    }
}
```

## Flash Memory

Internal flash is used for:
- Bootloader: 0x08000000 - 0x08001FFF (8KB)
- Application: 0x08002000 - 0x0800DFFF (48KB)
- PAST Storage: 0x0800E000 - 0x0800FFFF (8KB)

### Flash Operations

```c
// Unlock before writing
unlock_flash();

// Erase page (1KB per page)
flash_erase_page(address);

// Program word
flash_program_word(address, data);

// Lock after writing
lock_flash();
```

## Clock Configuration

System clock tree:
- HSI: 8 MHz internal oscillator
- PLL: 3x multiplier
- SYSCLK: 24 MHz
- AHB: 24 MHz
- APB1: 24 MHz
- APB2: 24 MHz

## Initialization Sequence

```c
void hw_init(void) {
    // 1. Configure clocks
    rcc_clock_setup_in_hsi_out_24mhz();

    // 2. Enable peripheral clocks
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_ADC1);
    rcc_periph_clock_enable(RCC_DAC);
    rcc_periph_clock_enable(RCC_SPI1);
    rcc_periph_clock_enable(RCC_USART1);

    // 3. Configure GPIO
    setup_gpio();

    // 4. Configure peripherals
    setup_adc();
    setup_dac();
    setup_spi();
    setup_usart();
    setup_timers();

    // 5. Configure interrupts
    setup_nvic();
}
```

## Hardware Abstraction API

### Core Functions

| Function | Description |
|----------|-------------|
| `hw_init()` | Initialize all hardware |
| `hw_get_adc_values()` | Read ADC channels |
| `hw_set_voltage_dac()` | Set voltage DAC |
| `hw_set_current_dac()` | Set current DAC |
| `hw_enable_output()` | Enable/disable output |
| `hw_send_byte()` | Send byte via UART |

### Timing Functions

| Function | Description |
|----------|-------------|
| `delay_ms()` | Blocking delay |
| `get_ticks()` | Get system tick count |
| `systick_init()` | Initialize system tick |
