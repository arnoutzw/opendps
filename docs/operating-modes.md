# Operating Modes

OpenDPS supports multiple operating modes (called "functions") that define how the power supply behaves.

## Available Modes

| Mode | Name | Description |
|------|------|-------------|
| CV | Constant Voltage | Regulate output voltage |
| CC | Constant Current | Regulate output current |
| CL | Current Limit | CV with adjustable current limit |
| GEN | Function Generator | Generate waveforms |

## Mode Architecture

Each mode is implemented as a "function" with a standardized interface:

```c
// Function structure (conceptual)
typedef struct {
    char *name;
    ui_screen_t *(*setup)(void);
    void (*enable)(bool enable);
    void (*tick)(void);
    void (*set_param)(uint8_t idx, uint32_t val);
    uint32_t (*get_param)(uint8_t idx);
} function_t;
```

## Constant Voltage (CV) Mode

### Description

In CV mode, the output voltage is regulated to match the setpoint. Current is limited by the maximum current setting.

### Behavior

```
User sets: V_target
Output:    V_out ≈ V_target (±calibration error)
           I_out ≤ I_max

If load draws more than I_max:
  - Current limited to I_max
  - Voltage drops below setpoint
  - CC indicator shown
```

### Parameters

| Index | Parameter | Unit | Range | Description |
|-------|-----------|------|-------|-------------|
| 0 | Voltage | mV | 0-50000 | Output voltage setpoint |

### UI Elements

- Voltage setpoint (editable, large font)
- Current readout (read-only)
- Input voltage indicator
- Output enable indicator
- CC indicator (current limiting active)

### Example

```c
// Set 12.00V output in CV mode
opendps_set_parameter(0, 12000);  // param 0 = voltage
opendps_enable_output(true);
```

## Constant Current (CC) Mode

### Description

In CC mode, the output current is regulated to match the setpoint. The output voltage varies based on load impedance.

### Behavior

```
User sets: I_target
Output:    I_out ≈ I_target (±calibration error)
           V_out varies with load

Compliance voltage:
  - V_out limited by V_max setting
  - If load requires more voltage than V_max, current drops
```

### Parameters

| Index | Parameter | Unit | Range | Description |
|-------|-----------|------|-------|-------------|
| 0 | Current | mA | 0-5000 | Output current setpoint |

### Use Cases

- LED testing (constant brightness)
- Battery charging (constant current phase)
- Electrochemical processes

### Example

```c
// Set 500mA output in CC mode
opendps_set_parameter(0, 500);  // param 0 = current
opendps_enable_output(true);
```

## Current Limit (CL) Mode

### Description

CL mode combines CV and CC operation. The output regulates voltage but with a user-adjustable current limit (lower than the mode's maximum).

### Behavior

```
User sets: V_target, I_limit
Output:    V_out ≈ V_target
           I_out ≤ I_limit

When I_out reaches I_limit:
  - Transitions to current-limiting
  - Voltage drops to maintain I_limit
  - CL indicator shown
```

### Parameters

| Index | Parameter | Unit | Range | Description |
|-------|-----------|------|-------|-------------|
| 0 | Voltage | mV | 0-50000 | Output voltage setpoint |
| 1 | Current Limit | mA | 0-5000 | Current limit |

### Use Cases

- Safe charging of unknown batteries
- Testing circuits with unknown current draw
- Preventing damage during development

### Example

```c
// Set 5V with 200mA limit
opendps_set_parameter(0, 5000);   // 5.000V
opendps_set_parameter(1, 200);   // 200mA limit
opendps_enable_output(true);
```

## Function Generator Mode

### Description

The function generator mode outputs voltage waveforms. This is useful for testing, signal injection, or simple function generator applications.

### Waveforms

| Type | Description |
|------|-------------|
| Sine | Sinusoidal waveform |
| Square | Square wave (50% duty) |
| Triangle | Triangle waveform |
| Sawtooth | Sawtooth (ramp) waveform |

### Parameters

| Index | Parameter | Unit | Range | Description |
|-------|-----------|------|-------|-------------|
| 0 | Waveform | - | 0-3 | Waveform type |
| 1 | Frequency | mHz | 1-10000 | Frequency in millihertz |
| 2 | Amplitude | mV | 0-50000 | Peak-to-peak amplitude |
| 3 | Offset | mV | 0-50000 | DC offset |

### Waveform Equations

```
Sine:     V(t) = offset + (amplitude/2) * sin(2π * f * t)
Square:   V(t) = offset ± (amplitude/2)
Triangle: V(t) = offset + (amplitude/2) * triangle(f * t)
Sawtooth: V(t) = offset + amplitude * ((f * t) mod 1)
```

### Limitations

- Maximum frequency limited by DAC update rate
- Amplitude + offset must not exceed V_max
- Output capacitance affects high-frequency waveforms

### Example

```c
// Generate 1Hz sine wave, 5V p-p, centered at 10V
opendps_set_parameter(0, 0);      // Sine waveform
opendps_set_parameter(1, 1000);   // 1000 mHz = 1 Hz
opendps_set_parameter(2, 5000);   // 5V peak-to-peak
opendps_set_parameter(3, 10000);  // 10V offset
opendps_enable_output(true);
```

## Switching Modes

### Via UI

1. Press M1 button to cycle through functions
2. Or long-press SEL to access function menu
3. Select desired function with encoder
4. Press SEL to confirm

### Via Remote Command

```bash
# Switch to CV mode (index 0)
./dpsctl.py -d /dev/ttyUSB0 --function 0

# Switch to CC mode (index 1)
./dpsctl.py -d /dev/ttyUSB0 --function 1

# Switch to CL mode (index 2)
./dpsctl.py -d /dev/ttyUSB0 --function 2
```

### Programmatically

```c
// Switch to function by index
opendps_enable_function_idx(function_index);

// Get current function
uint32_t current = opendps_get_current_function();
```

## Mode Persistence

- Last used mode is saved to flash
- Mode parameters are saved when switching
- On power-up, last mode and settings are restored

## Creating a Custom Mode

### Step 1: Create Header

```c
// func_custom.h
#ifndef __FUNC_CUSTOM_H__
#define __FUNC_CUSTOM_H__

void func_custom_init(void);
ui_screen_t *func_custom_setup(void);

#endif
```

### Step 2: Implement Mode

```c
// func_custom.c
static ui_screen_t custom_screen;

void func_custom_init(void) {
    // Register with function system
}

ui_screen_t *func_custom_setup(void) {
    // Initialize UI items
    return &custom_screen;
}

static void custom_enable(bool enable) {
    if (enable) {
        // Mode activated
    } else {
        // Mode deactivated
    }
}

static void custom_tick(void) {
    // Called at 100Hz when mode is active
}
```

### Step 3: Register Mode

Add to function list in `opendps.c`:

```c
static function_t *functions[] = {
    &func_cv,
    &func_cc,
    &func_cl,
    &func_gen,
    &func_custom,  // Add here
};
```
