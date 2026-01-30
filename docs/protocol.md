# Communication Protocol

This document describes the binary communication protocol used for remote control of OpenDPS via serial or WiFi.

## Overview

OpenDPS uses a binary protocol with the following characteristics:
- Byte-stuffed framing (uframe)
- CRC-16 error detection
- Request/response model
- Little-endian byte order

## Physical Layer

### Serial (UART)
- Baud rate: 115200
- Data bits: 8
- Stop bits: 1
- Parity: None
- Flow control: None

### WiFi (ESP8266)
- TCP connection to ESP8266 proxy
- Default port: 5005
- Same protocol as serial

## Frame Format

All communication uses the uframe encoding:

```
+-------+-------------------+----------+-------+
| START |      PAYLOAD      |   CRC    |  END  |
| 0x7E  | (escaped bytes)   | (16-bit) | 0x7F  |
+-------+-------------------+----------+-------+
```

### Escape Sequences

Special bytes within the payload are escaped:

| Byte | Escape Sequence |
|------|-----------------|
| 0x7E (START) | 0x7D 0x5E |
| 0x7F (END) | 0x7D 0x5F |
| 0x7D (ESCAPE) | 0x7D 0x5D |

### CRC-16 Calculation

- Algorithm: CRC-16 CCITT
- Polynomial: 0x1021
- Initial value: 0xFFFF
- Calculated over unescaped payload

## Command Format

```
+----------+------------------+
| COMMAND  |    PARAMETERS    |
|  (8-bit) |    (variable)    |
+----------+------------------+
```

## Commands

### Query (0x00)

Request device status.

**Request:**
```
+------+
| 0x00 |
+------+
```

**Response:**
```
+------+--------+-------+-------+--------+--------+--------+
| 0x80 | V_out  | I_out | V_in  | Output | Func   | Temp   |
|      | (16b)  | (16b) | (16b) | Enable | Index  | (opt)  |
+------+--------+-------+-------+--------+--------+--------+
```

| Field | Size | Description |
|-------|------|-------------|
| V_out | 16-bit | Output voltage in mV |
| I_out | 16-bit | Output current in mA |
| V_in | 16-bit | Input voltage in mV |
| Output Enable | 8-bit | 0=off, 1=on |
| Func Index | 8-bit | Current function index |
| Temp | 8-bit | Temperature (optional) |

### Set Voltage and Current (0x01)

Set output voltage and current limit.

**Request:**
```
+------+--------+--------+
| 0x01 | V_set  | I_set  |
|      | (16b)  | (16b)  |
+------+--------+--------+
```

**Response:**
```
+------+--------+
| 0x81 | Status |
+------+--------+
```

Status codes:
- 0x00: Success
- 0x01: Voltage out of range
- 0x02: Current out of range

### Enable Output (0x02)

Enable or disable the power output.

**Request:**
```
+------+--------+
| 0x02 | Enable |
|      | (8-bit)|
+------+--------+
```

Enable: 0 = off, 1 = on

**Response:**
```
+------+--------+
| 0x82 | Status |
+------+--------+
```

### WiFi Status (0x04)

Query WiFi module status (ESP8266).

**Request:**
```
+------+
| 0x04 |
+------+
```

**Response:**
```
+------+----------+------+
| 0x84 | Status   | RSSI |
|      | (8-bit)  | (8b) |
+------+----------+------+
```

WiFi Status:
- 0: Off
- 1: Connecting
- 2: Connected
- 3: Error

### Lock (0x05)

Lock or unlock the device (prevent local control).

**Request:**
```
+------+------+
| 0x05 | Lock |
|      | (8b) |
+------+------+
```

Lock: 0 = unlock, 1 = lock

**Response:**
```
+------+--------+
| 0x85 | Status |
+------+--------+
```

### Set Calibration (0x0B)

Set calibration coefficients.

**Request:**
```
+------+-------+---------+
| 0x0B | Index | Value   |
|      | (8b)  | (float) |
+------+-------+---------+
```

Calibration indices:
- 0: A_ADC_K (current ADC slope)
- 1: A_ADC_C (current ADC offset)
- 2: A_DAC_K (current DAC slope)
- 3: A_DAC_C (current DAC offset)
- 4: V_ADC_K (voltage ADC slope)
- 5: V_ADC_C (voltage ADC offset)
- 6: V_DAC_K (voltage DAC slope)
- 7: V_DAC_C (voltage DAC offset)
- 8: VIN_ADC_K (input voltage ADC slope)
- 9: VIN_ADC_C (input voltage ADC offset)

**Response:**
```
+------+--------+
| 0x8B | Status |
+------+--------+
```

### Set Function (0x0C)

Change the operating function (mode).

**Request:**
```
+------+-------+
| 0x0C | Index |
|      | (8b)  |
+------+-------+
```

Function indices:
- 0: Constant Voltage (CV)
- 1: Constant Current (CC)
- 2: Current Limit (CL)
- 3: Function Generator

**Response:**
```
+------+--------+
| 0x8C | Status |
+------+--------+
```

### Set Parameter (0x0D)

Set a function-specific parameter.

**Request:**
```
+------+-------+-------+
| 0x0D | Param | Value |
|      | (8b)  | (32b) |
+------+-------+-------+
```

Parameters depend on the active function. Example for CV mode:
- 0: Voltage setpoint (mV)

**Response:**
```
+------+--------+
| 0x8D | Status |
+------+--------+
```

### Upgrade Start (0x10)

Start firmware upgrade.

**Request:**
```
+------+------+
| 0x10 | Size |
|      | (32b)|
+------+------+
```

**Response:**
```
+------+--------+
| 0x90 | Status |
+------+--------+
```

### Upgrade Data (0x11)

Send firmware data chunk.

**Request:**
```
+------+--------+---------+
| 0x11 | Offset | Data... |
|      | (32b)  | (var)   |
+------+--------+---------+
```

**Response:**
```
+------+--------+
| 0x91 | Status |
+------+--------+
```

## Response Codes

All response commands have bit 7 set (command | 0x80).

### Status Codes

| Code | Description |
|------|-------------|
| 0x00 | Success |
| 0x01 | Invalid parameter |
| 0x02 | Out of range |
| 0x03 | Locked |
| 0x04 | Unknown command |
| 0x05 | CRC error |
| 0x06 | Framing error |

## Usage Examples

### Python Example

```python
import serial
import struct

def create_frame(payload):
    """Create uframe with CRC"""
    crc = crc16(payload)
    payload += struct.pack('<H', crc)

    # Escape special bytes
    escaped = bytearray()
    for b in payload:
        if b in (0x7D, 0x7E, 0x7F):
            escaped.append(0x7D)
            escaped.append(b ^ 0x20)
        else:
            escaped.append(b)

    return bytes([0x7E]) + escaped + bytes([0x7F])

def query_status(ser):
    """Query device status"""
    frame = create_frame(bytes([0x00]))
    ser.write(frame)
    response = read_frame(ser)

    if response[0] == 0x80:
        v_out, i_out, v_in = struct.unpack('<HHH', response[1:7])
        return {
            'voltage': v_out,
            'current': i_out,
            'input_voltage': v_in,
            'output_enabled': response[7] == 1
        }

def set_voltage_current(ser, voltage_mv, current_ma):
    """Set voltage and current"""
    payload = struct.pack('<BHH', 0x01, voltage_mv, current_ma)
    frame = create_frame(payload)
    ser.write(frame)
    response = read_frame(ser)
    return response[1] == 0x00  # Success

def enable_output(ser, enable):
    """Enable or disable output"""
    payload = bytes([0x02, 1 if enable else 0])
    frame = create_frame(payload)
    ser.write(frame)
    response = read_frame(ser)
    return response[1] == 0x00
```

### dpsctl Usage

```bash
# Query status
./dpsctl.py -d /dev/ttyUSB0 --status

# Set voltage to 5.00V
./dpsctl.py -d /dev/ttyUSB0 -V 5000

# Set current limit to 1.00A
./dpsctl.py -d /dev/ttyUSB0 -I 1000

# Enable output
./dpsctl.py -d /dev/ttyUSB0 --enable

# Disable output
./dpsctl.py -d /dev/ttyUSB0 --disable

# Set voltage and enable in one command
./dpsctl.py -d /dev/ttyUSB0 -V 12000 -I 2000 --enable
```

## Timing Considerations

- Command timeout: 1000 ms
- Inter-frame gap: 10 ms minimum
- Maximum frame size: 256 bytes
- Upgrade chunk size: 128 bytes

## Error Recovery

If communication fails:
1. Wait for any pending frame to complete (timeout)
2. Send a sync byte (0x7E) to reset framing
3. Retry the command

```c
// Sync and retry
void sync_and_retry(command_t cmd) {
    uart_send_byte(0x7E);  // Sync
    delay_ms(10);
    send_command(cmd);
}
```
