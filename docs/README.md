# OpenDPS Documentation

OpenDPS is an open-source firmware replacement for the DPS5005 and similar programmable power supply modules. This documentation provides comprehensive information about the firmware architecture, APIs, and usage.

## Table of Contents

- [Architecture Overview](architecture.md) - System design and component interaction
- [Hardware Abstraction](hardware.md) - GPIO, ADC, DAC, and peripheral interfaces
- [Communication Protocol](protocol.md) - Serial and WiFi remote control protocol
- [User Interface](ui-framework.md) - Display and input handling
- [Power Control](power-control.md) - Voltage/current regulation and calibration
- [Operating Modes](operating-modes.md) - CV, CC, CL, and function generator modes
- [Persistent Storage](storage.md) - Settings and calibration storage

## Supported Hardware

| Model | Voltage | Current | Notes |
|-------|---------|---------|-------|
| DPS5005 | 0-50V | 0-5A | Primary development target |
| DPS3005 | 0-30V | 0-5A | Supported |
| DPS5015 | 0-50V | 0-15A | Supported |
| DPS5020 | 0-50V | 0-20A | Supported |
| DPS3003 | 0-30V | 0-3A | Supported |
| DP50V5A | 0-50V | 0-5A | Supported |

## Quick Start

### Building the Firmware

```bash
# Clone the repository
git clone https://github.com/kanflo/opendps.git
cd opendps

# Initialize submodules
git submodule init
git submodule update

# Build the firmware
make -C opendps
make -C dpsboot
```

### Flashing

```bash
# Flash bootloader (required once)
make -C dpsboot flash

# Flash application
make -C opendps flash
```

## Architecture Summary

```
+------------------+     +------------------+
|    User Input    |     |  Serial/WiFi     |
|  (Encoder/Btns)  |     |    Commands      |
+--------+---------+     +--------+---------+
         |                        |
         v                        v
+--------+---------+     +--------+---------+
|   Event Queue    |<--->|  Protocol Handler|
+--------+---------+     +------------------+
         |
         v
+--------+---------+
|  UI Framework    |
|  (Screens/Items) |
+--------+---------+
         |
         v
+--------+---------+     +------------------+
|  Power Control   |---->|   DAC/ADC        |
|  (pwrctl)        |<----|   Hardware       |
+------------------+     +------------------+
```

## Key Modules

### Core Application (`opendps.h`)
Main application logic, function management, and system initialization.

### Hardware Abstraction (`hw.h`)
Low-level hardware interface for GPIO, ADC, DAC, and peripherals.

### Power Control (`pwrctl.h`)
Voltage and current regulation with calibration support.

### User Interface (`uui.h`, `tft.h`)
Screen management, input handling, and display rendering.

### Communication (`protocol.h`, `uframe.h`)
Binary protocol for remote control via serial or WiFi.

### Storage (`past.h`)
Wear-leveled persistent storage for settings and calibration.

## Remote Control

OpenDPS can be controlled remotely using:

1. **Serial Connection**: Direct UART connection at 115200 baud
2. **WiFi**: Via ESP8266 module running the esp8266-proxy firmware
3. **Python Tool**: `dpsctl.py` command-line utility

Example:
```bash
# Set voltage to 5V and current limit to 1A
./dpsctl.py -d /dev/ttyUSB0 -V 5000 -I 1000

# Enable output
./dpsctl.py -d /dev/ttyUSB0 --enable

# Query status
./dpsctl.py -d /dev/ttyUSB0 --status
```

## License

OpenDPS is released under the MIT License. See the LICENSE file for details.
