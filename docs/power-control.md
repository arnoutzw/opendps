# Power Control Module

This document describes the power control subsystem of OpenDPS, including voltage/current regulation and calibration.

## Overview

The power control module (`pwrctl.c/h`) manages:
- Output voltage regulation
- Current limiting
- ADC/DAC calibration
- Protection mechanisms

## Calibration System

### Calibration Model

OpenDPS uses linear calibration to convert between user units (mV/mA) and hardware units (ADC/DAC values):

```
DAC_value = (User_value * K) + C
User_value = (ADC_value - C) / K
```

Where:
- **K** = Slope coefficient (gain)
- **C** = Offset coefficient (zero point)

### Calibration Coefficients

| Coefficient | Description | Typical Value |
|-------------|-------------|---------------|
| V_DAC_K | Voltage DAC slope | ~0.08 |
| V_DAC_C | Voltage DAC offset | ~0 |
| V_ADC_K | Voltage ADC slope | ~12.5 |
| V_ADC_C | Voltage ADC offset | ~0 |
| A_DAC_K | Current DAC slope | ~0.8 |
| A_DAC_C | Current DAC offset | ~0 |
| A_ADC_K | Current ADC slope | ~1.25 |
| A_ADC_C | Current ADC offset | ~0 |
| VIN_ADC_K | Input voltage ADC slope | ~15.0 |
| VIN_ADC_C | Input voltage ADC offset | ~0 |

### Calibration Procedure

#### Voltage Calibration

1. Connect a calibrated voltmeter to the output
2. Set known voltage (e.g., 5000 mV)
3. Read actual output voltage from meter
4. Repeat with different voltage (e.g., 20000 mV)
5. Calculate K and C from two-point calibration:

```c
// Two-point calibration
float v1_set = 5000;   // First setpoint (mV)
float v1_actual = 4980; // Actual measured (mV)
float v2_set = 20000;  // Second setpoint (mV)
float v2_actual = 19850; // Actual measured (mV)

// Calculate slope
float K = (v2_set - v1_set) / (v2_actual - v1_actual);

// Calculate offset
float C = v1_set - (K * v1_actual);

// Store calibration
pwrctl_set_vdac_coefficient(K);
pwrctl_set_vdac_offset(C);
```

#### Current Calibration

Similar process using a calibrated ammeter in series with a load.

### Storing Calibration

Calibration coefficients are stored in PAST (Persistent Application Storage):

```c
// Save voltage DAC calibration
past_write_unit(past_V_DAC_K, &v_dac_k, sizeof(float));
past_write_unit(past_V_DAC_C, &v_dac_c, sizeof(float));

// Save current ADC calibration
past_write_unit(past_A_ADC_K, &a_adc_k, sizeof(float));
past_write_unit(past_A_ADC_C, &a_adc_c, sizeof(float));
```

## API Reference

### Setting Output

```c
/**
 * Set output voltage (with calibration)
 * @param voltage_mv Desired voltage in millivolts
 * @return true if successful
 */
bool pwrctl_set_vout(uint32_t voltage_mv);

/**
 * Set current limit (with calibration)
 * @param current_ma Current limit in milliamps
 * @return true if successful
 */
bool pwrctl_set_ilimit(uint32_t current_ma);

/**
 * Enable or disable output
 * @param enable true to enable, false to disable
 */
void pwrctl_enable_output(bool enable);
```

### Reading Measurements

```c
/**
 * Get current output voltage
 * @return Voltage in millivolts
 */
uint32_t pwrctl_get_vout(void);

/**
 * Get current output current
 * @return Current in milliamps
 */
uint32_t pwrctl_get_iout(void);

/**
 * Get input voltage
 * @return Input voltage in millivolts
 */
uint32_t pwrctl_get_vin(void);
```

### Conversion Functions

```c
/**
 * Convert user mV to DAC value
 * @param mv Voltage in millivolts
 * @return DAC value (0-4095)
 */
uint32_t pwrctl_calc_vdac(uint32_t mv);

/**
 * Convert ADC value to user mV
 * @param adc ADC reading (0-4095)
 * @return Voltage in millivolts
 */
uint32_t pwrctl_calc_vadc(uint32_t adc);

/**
 * Convert user mA to DAC value
 * @param ma Current in milliamps
 * @return DAC value (0-4095)
 */
uint32_t pwrctl_calc_idac(uint32_t ma);

/**
 * Convert ADC value to user mA
 * @param adc ADC reading (0-4095)
 * @return Current in milliamps
 */
uint32_t pwrctl_calc_iadc(uint32_t adc);
```

## Control Loop

### Voltage Regulation

```
Setpoint (mV)
     |
     v
+----+----+     +--------+     +--------+
|  V_DAC  |---->|  DAC   |---->| Output |---+
|  Calc   |     |        |     | Stage  |   |
+---------+     +--------+     +--------+   |
                                            |
                               Feedback     |
                                   |        |
                                   v        |
                             +--------+     |
                             |  ADC   |<----+
                             +----+---+
                                  |
                                  v
                             +--------+
                             | V_ADC  |---> Display (mV)
                             |  Calc  |
                             +--------+
```

### Current Limiting

The current limit creates a secondary control loop:

```
I_limit (mA)
     |
     v
+----+----+     +--------+
|  I_DAC  |---->| Current|
|  Calc   |     | Limit  |
+---------+     | Circuit|
                +---+----+
                    |
             If I > limit:
                    |
                    v
              Reduce V_out
```

## Protection Features

### Overcurrent Protection (OCP)

When output current exceeds the limit:
1. Hardware current limit circuit activates
2. OCP interrupt is generated
3. Event queue receives `event_ocp`
4. UI displays overcurrent warning
5. Output may be disabled (configurable)

```c
void handle_ocp_event(void) {
    // Flash warning on display
    tft_set_status("OCP!");

    // Optionally disable output
    if (ocp_shutdown_enabled) {
        pwrctl_enable_output(false);
    }
}
```

### Overvoltage Protection

Input voltage monitoring:

```c
uint32_t vin = pwrctl_get_vin();
if (vin > MAX_INPUT_VOLTAGE) {
    // Disable output
    pwrctl_enable_output(false);
    tft_set_status("OVP!");
}
```

### Thermal Protection

Temperature monitoring (if sensor available):

```c
if (hw_get_temperature() > TEMP_LIMIT) {
    pwrctl_enable_output(false);
    tft_set_status("OTP!");
}
```

## Model-Specific Configuration

Different DPS models have different voltage/current ranges:

```c
// DPS5005
#define V_MAX_MV  50000  // 50.00V
#define I_MAX_MA   5000  // 5.000A

// DPS3005
#define V_MAX_MV  30000  // 30.00V
#define I_MAX_MA   5000  // 5.000A

// DPS5015
#define V_MAX_MV  50000  // 50.00V
#define I_MAX_MA  15000  // 15.00A
```

These are defined in `dps-model.h` based on the build configuration.

## Example Usage

### Basic Voltage/Current Setting

```c
// Initialize power control
pwrctl_init();

// Set 12V output with 1A current limit
pwrctl_set_vout(12000);   // 12.000V
pwrctl_set_ilimit(1000);  // 1.000A

// Enable output
pwrctl_enable_output(true);

// Monitor output
uint32_t v_actual = pwrctl_get_vout();
uint32_t i_actual = pwrctl_get_iout();
printf("Output: %d.%03dV @ %d.%03dA\n",
       v_actual / 1000, v_actual % 1000,
       i_actual / 1000, i_actual % 1000);
```

### Calibration Example

```c
// Calibrate voltage output
// 1. Set to 10V
pwrctl_set_vout(10000);

// 2. User measures actual output (e.g., 9.95V)
// 3. Enter correction
float correction = 10000.0 / 9950.0;
float new_k = v_dac_k * correction;
pwrctl_set_vdac_coefficient(new_k);

// 4. Save to flash
past_write_unit(past_V_DAC_K, &new_k, sizeof(float));
```
