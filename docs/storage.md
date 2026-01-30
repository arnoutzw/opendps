# Persistent Storage (PAST)

This document describes the Persistent Application Storage (PAST) system used in OpenDPS for storing settings and calibration data.

## Overview

PAST provides wear-leveled storage in the STM32's internal flash memory. It safely stores:
- Calibration coefficients
- User settings
- Power settings
- Display preferences

## Memory Layout

```
Flash Memory Map:
0x0800 E000 +------------------+
           |   PAST Page 0    |  4 KB
           |   (active/old)   |
0x0800 F000 +------------------+
           |   PAST Page 1    |  4 KB
           |   (backup/new)   |
0x0801 0000 +------------------+
```

## Unit Format

Each stored value is a "unit" with this format:

```
+--------+--------+--------+----------+
|  ID    | Length | Data   | Padding  |
| (8-bit)| (8-bit)|(var)   | (to 4B)  |
+--------+--------+--------+----------+
```

- **ID**: Unique identifier (see pastunits.h)
- **Length**: Data length in bytes
- **Data**: The actual stored value
- **Padding**: Alignment to 4-byte boundary

## Storage Units

### Power Settings

| ID | Name | Type | Description |
|----|------|------|-------------|
| 1 | past_power | uint32_t | `[I_limit:16] | [V_out:16]` |

### Display Settings

| ID | Name | Type | Description |
|----|------|------|-------------|
| 2 | past_tft_inversion | uint8_t | 0=normal, 1=inverted |
| 14 | past_tft_brightness | uint8_t | 0-100 brightness level |

### Version Information

| ID | Name | Type | Description |
|----|------|------|-------------|
| 3 | past_boot_git_hash | string | Bootloader git hash |
| 4 | past_app_git_hash | string | Application git hash |

### Calibration Coefficients

| ID | Name | Type | Description |
|----|------|------|-------------|
| 5 | past_A_ADC_K | float | Current ADC slope |
| 6 | past_A_ADC_C | float | Current ADC offset |
| 7 | past_A_DAC_K | float | Current DAC slope |
| 8 | past_A_DAC_C | float | Current DAC offset |
| 9 | past_V_DAC_K | float | Voltage DAC slope |
| 10 | past_V_DAC_C | float | Voltage DAC offset |
| 11 | past_V_ADC_K | float | Voltage ADC slope |
| 12 | past_V_ADC_C | float | Voltage ADC offset |
| 13 | past_VIN_ADC_K | float | Input voltage ADC slope |
| 14 | past_VIN_ADC_C | float | Input voltage ADC offset |

### System Flags

| ID | Name | Type | Description |
|----|------|------|-------------|
| 0xFF | past_upgrade_started | flag | Upgrade in progress |

## API Reference

### Initialization

```c
/**
 * Initialize PAST storage
 * @return true if valid storage found, false if formatted
 */
bool past_init(void);
```

### Reading Data

```c
/**
 * Read a unit from storage
 * @param id Unit identifier
 * @param data Buffer to receive data
 * @param length Maximum bytes to read
 * @return Actual bytes read, 0 if not found
 */
uint32_t past_read_unit(uint8_t id, void *data, uint32_t length);
```

**Example:**

```c
float v_dac_k;
if (past_read_unit(past_V_DAC_K, &v_dac_k, sizeof(float)) > 0) {
    // Calibration loaded
} else {
    // Use default
    v_dac_k = DEFAULT_V_DAC_K;
}
```

### Writing Data

```c
/**
 * Write a unit to storage
 * @param id Unit identifier
 * @param data Data to write
 * @param length Data length in bytes
 * @return true if successful
 */
bool past_write_unit(uint8_t id, const void *data, uint32_t length);
```

**Example:**

```c
float v_dac_k = 0.0815f;
if (!past_write_unit(past_V_DAC_K, &v_dac_k, sizeof(float))) {
    // Write failed
}
```

### Formatting

```c
/**
 * Format storage (erase all data)
 * @return true if successful
 */
bool past_format(void);
```

## Wear Leveling

PAST implements wear leveling to extend flash life:

### Write Algorithm

1. Find end of active page
2. Append new unit at end
3. If page full, garbage collect to backup page
4. Swap active/backup pages

### Garbage Collection

When active page is full:

```
Active Page          Backup Page
+-----------+       +-----------+
| Unit A    |       |           |
| Unit A'   | --->  | Unit A'   |  (latest A)
| Unit B    |       | Unit B    |
| Unit C    | --->  | Unit C    |  (only copy)
| Unit C'   |       |           |
+-----------+       +-----------+
  (full)              (compacted)
```

Only the latest version of each unit is copied.

### Page Swap

After garbage collection:
1. Erase old active page
2. Swap page pointers
3. New writes go to (now empty) backup page

## Usage Patterns

### Saving Settings on Change

```c
void on_voltage_changed(ui_number_t *item) {
    // Apply to hardware
    pwrctl_set_vout(item->value);

    // Save to storage
    uint32_t power = (current_limit << 16) | item->value;
    past_write_unit(past_power, &power, sizeof(power));
}
```

### Loading Settings at Startup

```c
void load_settings(void) {
    uint32_t power;
    if (past_read_unit(past_power, &power, sizeof(power)) > 0) {
        voltage_setpoint = power & 0xFFFF;
        current_limit = power >> 16;
    } else {
        // Use defaults
        voltage_setpoint = DEFAULT_VOLTAGE;
        current_limit = DEFAULT_CURRENT;
    }
}
```

### Saving Calibration

```c
void save_calibration(void) {
    past_write_unit(past_V_DAC_K, &v_dac_k, sizeof(float));
    past_write_unit(past_V_DAC_C, &v_dac_c, sizeof(float));
    past_write_unit(past_V_ADC_K, &v_adc_k, sizeof(float));
    past_write_unit(past_V_ADC_C, &v_adc_c, sizeof(float));
    past_write_unit(past_A_DAC_K, &a_dac_k, sizeof(float));
    past_write_unit(past_A_DAC_C, &a_dac_c, sizeof(float));
    past_write_unit(past_A_ADC_K, &a_adc_k, sizeof(float));
    past_write_unit(past_A_ADC_C, &a_adc_c, sizeof(float));
}
```

## Error Handling

### Corrupt Storage

If PAST detects corruption:
1. Storage is reformatted
2. All settings reset to defaults
3. `past_init()` returns false

### Flash Write Failure

Flash writes can fail if:
- Flash not properly unlocked
- Power loss during write
- Flash worn out (unlikely with wear leveling)

Always check return values:

```c
if (!past_write_unit(id, data, len)) {
    // Handle error
    dbg_printf("PAST write failed for unit %d\n", id);
}
```

## Limitations

- Maximum unit size: ~256 bytes
- Maximum total storage: ~4KB (minus overhead)
- Write endurance: ~10,000 cycles per page
- With wear leveling: Effectively much higher

## Flash Locking

PAST automatically manages flash locking:

```c
// Internal to PAST
void past_write_internal(void) {
    unlock_flash();
    // ... perform writes ...
    lock_flash();
}
```

For manual flash operations, use `flashlock.h`:

```c
unlock_flash();
flash_program_word(address, data);
lock_flash();
```
