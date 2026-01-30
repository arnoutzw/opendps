# OpenDPS Architecture

This document describes the overall architecture of the OpenDPS firmware.

## System Overview

OpenDPS runs on an STM32F100 microcontroller with the following resources:

| Resource | Size | Usage |
|----------|------|-------|
| Flash | 64 KB | Bootloader (8KB) + Application (48KB) + PAST (8KB) |
| RAM | 8 KB | Stack, heap, buffers, UI state |
| Clock | 24 MHz | System clock |

## Memory Map

```
0x0800 0000 +------------------+
           |    Bootloader    |  8 KB
           |    (dpsboot)     |
0x0800 2000 +------------------+
           |                  |
           |   Application    |  48 KB
           |    (opendps)     |
           |                  |
0x0800 E000 +------------------+
           |   PAST Storage   |  8 KB
           | (2 x 4KB pages)  |
0x0801 0000 +------------------+
```

## Component Architecture

### Layer Diagram

```
+---------------------------------------------------------------+
|                      Application Layer                         |
|  +------------------+  +------------------+  +---------------+ |
|  | Operating Modes  |  |   UI Screens     |  |   Settings    | |
|  | (func_cv, cc..)  |  | (main, settings) |  | (calibration) | |
|  +------------------+  +------------------+  +---------------+ |
+---------------------------------------------------------------+
|                      Framework Layer                           |
|  +------------------+  +------------------+  +---------------+ |
|  |  Power Control   |  |  UI Framework    |  |   Protocol    | |
|  |    (pwrctl)      |  |     (uui)        |  |   Handler     | |
|  +------------------+  +------------------+  +---------------+ |
+---------------------------------------------------------------+
|                      Driver Layer                              |
|  +--------+  +--------+  +--------+  +--------+  +----------+ |
|  |  TFT   |  |  ADC   |  |  DAC   |  |  UART  |  |   SPI    | |
|  +--------+  +--------+  +--------+  +--------+  +----------+ |
+---------------------------------------------------------------+
|                      Hardware (STM32F100)                      |
+---------------------------------------------------------------+
```

### Core Components

#### 1. Main Application (`opendps.c/h`)

The central coordinator that:
- Initializes all subsystems
- Manages the main event loop
- Handles function (operating mode) switching
- Processes remote commands
- Coordinates UI updates

```c
// Main loop structure
while (1) {
    event_t event = event_get();
    switch (event) {
        case event_rot_left:
        case event_rot_right:
        case event_rot_press:
            // Handle encoder input
            uui_handle_event(&event);
            break;
        case event_uart_rx:
            // Handle serial command
            process_serial_command();
            break;
        // ... other events
    }
    uui_tick();  // Update UI
}
```

#### 2. Event System (`event.c/h`)

Lock-free event queue for ISR-to-mainloop communication:

```
+-------------+     +--------------+     +-------------+
|   Button    |     |              |     |             |
|    ISR      |---->|    Event     |---->|    Main     |
+-------------+     |    Queue     |     |    Loop     |
+-------------+     |  (ringbuf)   |     |             |
|   Encoder   |---->|              |---->|             |
|    ISR      |     +--------------+     +-------------+
+-------------+
```

Event types include:
- `event_rot_left/right` - Encoder rotation
- `event_rot_press` - Encoder button press
- `event_button_m1/m2` - M1/M2 button presses
- `event_uart_rx` - Serial data received
- `event_ocp` - Overcurrent protection triggered

#### 3. Power Control (`pwrctl.c/h`)

Manages voltage and current regulation:

```
                    Setpoint
                       |
                       v
+------------------+   |   +------------------+
|   Calibration    |   |   |    Feedback      |
|   Coefficients   |   |   |    (ADC)         |
|  (K*x + C)       |   |   |                  |
+--------+---------+   |   +--------+---------+
         |             |            |
         v             v            v
+--------+-------------+------------+---------+
|              Control Loop                   |
|         (set DAC, read ADC)                 |
+---------------------+-----------------------+
                      |
                      v
               +------+------+
               |    DAC      |
               |   Output    |
               +-------------+
```

Calibration converts between:
- **User units**: millivolts (mV), milliamps (mA)
- **DAC/ADC units**: Raw 12-bit values (0-4095)

#### 4. UI Framework (`uui.c/h`)

Hierarchical screen-based UI system:

```
uui_t (Global UI State)
  |
  +-- current_screen
  |
  +-- screens[]
        |
        +-- ui_screen_t (Main Screen)
        |     |
        |     +-- items[]
        |           +-- ui_number_t (Voltage)
        |           +-- ui_number_t (Current)
        |           +-- ui_icon_t (Status)
        |
        +-- ui_screen_t (Settings Screen)
              |
              +-- items[]
                    +-- ...
```

UI Items:
- `ui_number_t` - Editable numeric value with digits
- `ui_icon_t` - Selectable icon from a set

#### 5. Communication Protocol (`protocol.c/h`)

Binary protocol over UART/WiFi:

```
Frame Structure (uframe encoding):
+-------+----------+-------+---------+-------+
| START | PAYLOAD  |  CRC  | PAYLOAD |  END  |
| 0x7E  | (escaped)| (16b) | LENGTH  | 0x7F  |
+-------+----------+-------+---------+-------+

Command Frame:
+--------+--------+--------+
|  CMD   | PARAM1 | PARAM2 |
| (8b)   | (var)  | (var)  |
+--------+--------+--------+
```

### Operating Modes (Functions)

Each operating mode is implemented as a "function" with standardized interface:

```c
typedef struct {
    char *name;                    // Display name
    ui_screen_t *(*setup)(void);   // Initialize and return screen
    void (*enable)(bool enable);   // Called when mode activated/deactivated
    void (*tick)(void);            // Periodic update (100Hz)
    void (*set_param)(uint8_t idx, uint32_t val);  // Remote parameter set
    uint32_t (*get_param)(uint8_t idx);            // Remote parameter get
} function_t;
```

Available modes:
- **CV (Constant Voltage)**: Regulate output voltage
- **CC (Constant Current)**: Regulate output current
- **CL (Current Limit)**: CV with adjustable current limit
- **Function Generator**: Output waveforms (sine, square, etc.)

### Data Flow

#### User Input Flow
```
Encoder/Button -> ISR -> Event Queue -> Main Loop -> UI Framework -> Power Control
```

#### Remote Command Flow
```
UART RX -> ISR -> Ring Buffer -> uframe Decoder -> Protocol Handler -> Power Control
```

#### Display Update Flow
```
UI Framework -> TFT Driver -> SPI DMA -> ILI9163C Controller
```

## Initialization Sequence

```
1. hw_init()           - Configure clocks, GPIO, peripherals
2. pwrctl_init()       - Initialize power control, load calibration
3. tft_init()          - Initialize display
4. event_init()        - Initialize event queue
5. past_init()         - Initialize persistent storage
6. uui_init()          - Initialize UI framework
7. func_*_init()       - Initialize operating modes
8. Enable interrupts
9. Enter main loop
```

## Interrupt Priorities

| IRQ | Priority | Handler |
|-----|----------|---------|
| SysTick | 0 | System tick (1ms), button debounce |
| TIM2 | 1 | Encoder reading |
| USART1 | 2 | Serial receive |
| DMA | 3 | SPI transfers |

## Thread Safety

OpenDPS is single-threaded with interrupt handlers:
- **Main loop**: All application logic
- **ISRs**: Only queue events, don't process

Communication between ISRs and main loop uses:
- Lock-free ring buffers (`ringbuf.h`)
- Atomic event flags
- Volatile variables for shared state
