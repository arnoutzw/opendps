/**
 * @file dps-model.h
 * @brief DPS Power Supply Model Configuration
 *
 * This file defines model-specific parameters for different DPS power supply
 * variants. Each model has different maximum current ratings and requires
 * different ADC/DAC calibration coefficients.
 *
 * ## Supported Models
 *
 * | Model | Max Voltage | Max Current | UI Format |
 * |-------|-------------|-------------|-----------|
 * | DPS3003 | 30V | 3A | X.XXX A |
 * | DPS3005 | 30V | 5A | X.XXX A |
 * | DPS5005 | 50V | 5A | X.XXX A |
 * | DPS5015 | 50V | 15A | XX.XX A |
 * | DPS5020 | 50V | 20A | XX.XX A |
 * | DP50V5A | 50V | 5A | X.XXX A |
 *
 * ## Calibration Coefficients
 *
 * Each model defines default calibration coefficients for:
 * - **A_ADC_K, A_ADC_C**: Current ADC (I_mA = K * ADC + C)
 * - **A_DAC_K, A_DAC_C**: Current DAC (DAC = K * I_mA + C)
 * - **V_ADC_K, V_ADC_C**: Voltage ADC (V_mV = K * ADC + C)
 * - **V_DAC_K, V_DAC_C**: Voltage DAC (DAC = K * V_mV + C)
 * - **VIN_ADC_K, VIN_ADC_C**: Input voltage ADC
 *
 * These defaults can be overridden by calibration stored in PAST.
 *
 * ## Calibration Procedure
 *
 * For ADC calibration:
 * ```
 * K = (Value1 - Value2) / (ADC1 - ADC2)
 * C = Value1 - K * ADC1
 * ```
 *
 * For DAC calibration:
 * ```
 * K = (DAC1 - DAC2) / (Value1 - Value2)
 * C = DAC1 - K * Value1
 * ```
 *
 * @see pwrctl.h for calibration usage
 * @see pastunits.h for storing custom calibration
 */

#ifndef __DPS_MODEL_H__
#define __DPS_MODEL_H__

/*
 * Calibration coefficient formulas:
 *
 * K - slope/angle factor
 * C - offset
 *
 * For ADC (reading physical value from ADC):
 *   K = (Value1 - Value2) / (ADC1 - ADC2)
 *   C = Value1 - K * ADC1
 *
 * For DAC (setting physical value via DAC):
 *   K = (DAC1 - DAC2) / (Value1 - Value2)
 *   C = DAC1 - K * Value1
 */

/* Example: Voltage ADC calibration
 * Read ADC values in CLI stat command and measure with reference voltmeter:
 *   ADC  394 =  5001 mV
 *   ADC  782 = 10030 mV
 *   ADC 1393 = 18000 mV
 *
 * Calculate coefficients:
 *   K = (18000 - 5001) / (1393 - 394) = 12.999 / 999 ≈ 13.01
 *   C = 5001 - K * 394 ≈ -125.7
 */

/* Example: Voltage DAC calibration
 * Write DAC values directly (via OpenOCD: mww 0x40007408 <value>)
 * and measure output with reference voltmeter:
 *   DAC  77 =  1004 mV
 *   DAC 872 = 12005 mV
 *
 * Calculate coefficients:
 *   K = (77 - 872) / (1004 - 12005) = -795 / -11001 ≈ 0.0723
 *   C = 77 - K * 1004 ≈ 4.44
 */

/**
 * @defgroup DPS5020_Config DPS5020 Configuration (20A model)
 * @{
 */
#if defined(DPS5020)
 #ifndef CONFIG_DPS_MAX_CURRENT
  /** @brief Maximum current in mA (20A) */
  #define CONFIG_DPS_MAX_CURRENT (20000)
 #endif
 /** @brief Number of integer digits for current display */
 #define CURRENT_DIGITS 2
 /** @brief Number of decimal digits for current display */
 #define CURRENT_DECIMALS 2
 /** @brief ADC value when output current is near zero */
 #define ADC_CHA_IOUT_GOLDEN_VALUE  (59)
 /** @brief Current ADC slope coefficient */
 #define A_ADC_K (float)6.75449f
 /** @brief Current ADC offset coefficient */
 #define A_ADC_C (float)-358.73f
 /** @brief Current DAC slope coefficient */
 #define A_DAC_K (float)0.16587f
 /** @brief Current DAC offset coefficient */
 #define A_DAC_C (float)243.793f
 /** @brief Voltage ADC slope coefficient */
 #define V_ADC_K (float)13.2930f
 /** @brief Voltage ADC offset coefficient */
 #define V_ADC_C (float)-179.91f
 /** @brief Voltage DAC slope coefficient */
 #define V_DAC_K (float)0.07528f
 /** @brief Voltage DAC offset coefficient */
 #define V_DAC_C (float)6.68949f
 /** @brief Input voltage ADC slope coefficient */
 #define VIN_ADC_K (float)16.956f
 /** @brief Input voltage ADC offset coefficient */
 #define VIN_ADC_C (float)6.6895f
/** @} */

/**
 * @defgroup DPS5015_Config DPS5015 Configuration (15A model)
 * @{
 */
#elif defined(DPS5015)
 #ifndef CONFIG_DPS_MAX_CURRENT
  /** @brief Maximum current in mA (15A) */
  #define CONFIG_DPS_MAX_CURRENT (15000)
 #endif
 #define CURRENT_DIGITS 2
 #define CURRENT_DECIMALS 2
 #define ADC_CHA_IOUT_GOLDEN_VALUE  (59)
 #define A_ADC_K (float)6.8403f
 #define A_ADC_C (float)-394.06f
 #define A_DAC_K (float)0.166666f
 #define A_DAC_C (float)261.6666f
 #define V_ADC_K (float)13.012f
 #define V_ADC_C (float)-125.732f
 #define V_DAC_K (float)0.072266f
 #define V_DAC_C (float)4.444777f
/** @} */

/**
 * @defgroup DPS5005_Config DPS5005 Configuration (5A model)
 * @{
 */
#elif defined(DPS5005)
 #ifndef CONFIG_DPS_MAX_CURRENT
  /** @brief Maximum current in mA (5A) */
  #define CONFIG_DPS_MAX_CURRENT (5000)
 #endif
 #define CURRENT_DIGITS 1
 #define CURRENT_DECIMALS 3
 #define ADC_CHA_IOUT_GOLDEN_VALUE  (0x45)
 #define A_ADC_K (float)1.713f
 #define A_ADC_C (float)-118.51f
 #define A_DAC_K (float)0.652f
 #define A_DAC_C (float)288.611f
 #define V_DAC_K (float)0.072f
 #define V_DAC_C (float)1.85f
 #define V_ADC_K (float)13.164f
 #define V_ADC_C (float)-100.751f
/** @} */

/**
 * @defgroup DP50V5A_Config DP50V5A Configuration
 * @{
 */
#elif defined(DP50V5A)
 #ifndef CONFIG_DPS_MAX_CURRENT
  #define CONFIG_DPS_MAX_CURRENT (5000)
 #endif
 #define CURRENT_DIGITS 1
 #define CURRENT_DECIMALS 3
 #define ADC_CHA_IOUT_GOLDEN_VALUE  (0x45)
 #define A_DAC_K (float)0.6402f
 #define A_DAC_C (float)299.5518f
 #define A_ADC_K (float)1.74096f
 #define A_ADC_C (float)-121.3943805f
 #define V_DAC_K (float)0.07544f
 #define V_DAC_C (float)2.1563f
 #define V_ADC_K (float)13.253f
 #define V_ADC_C (float)-103.105f
/** @} */

/**
 * @defgroup DPS3005_Config DPS3005 Configuration (30V/5A model)
 * @{
 */
#elif defined(DPS3005)
 #ifndef CONFIG_DPS_MAX_CURRENT
  #define CONFIG_DPS_MAX_CURRENT (5000)
 #endif
 #define CURRENT_DIGITS 1
 #define CURRENT_DECIMALS 3
 #define ADC_CHA_IOUT_GOLDEN_VALUE  (0x00)
 #define A_ADC_K (float)1.751f
 #define A_ADC_C (float)-1.101f
 #define A_DAC_K (float)0.653f
 #define A_DAC_C (float)262.5f
 #define V_DAC_K (float)0.0761f
 #define V_DAC_C (float)2.2857f
 #define V_ADC_K (float)13.131f
 #define V_ADC_C (float)-111.9f
/** @} */

/**
 * @defgroup DPS3003_Config DPS3003 Configuration (30V/3A model)
 * @{
 */
#elif defined(DPS3003)
 #ifndef CONFIG_DPS_MAX_CURRENT
  /** @brief Maximum current in mA (3A) */
  #define CONFIG_DPS_MAX_CURRENT (3000)
 #endif
 #define CURRENT_DIGITS 1
 #define CURRENT_DECIMALS 3
 #define ADC_CHA_IOUT_GOLDEN_VALUE  (0x00)
 #define A_ADC_K (float)0.99676f
 #define A_ADC_C (float)-44.3156f
 #define A_DAC_K (float)1.12507f
 #define A_DAC_C (float)256.302f
 #define V_DAC_K (float)0.12237f
 #define V_DAC_C (float)10.1922f
 #define V_ADC_K (float)8.16837f
 #define V_ADC_C (float)-115.582f
 #define VIN_ADC_K (float)16.7897f
 #define VIN_ADC_C (float)16.6448f
/** @} */

#else
 #error "Please set MODEL to the device you want to build for"
#endif // MODEL

/**
 * @defgroup VIN_Defaults Default Input Voltage Coefficients
 * @brief These are constant across most models but may require tuning
 * @{
 */
#ifndef VIN_ADC_K
 /** @brief Default input voltage ADC slope coefficient */
 #define VIN_ADC_K (float)16.746f
#endif

#ifndef VIN_ADC_C
 /** @brief Default input voltage ADC offset coefficient */
 #define VIN_ADC_C (float)64.112f
#endif

/**
 * @brief Input to output voltage ratio
 *
 * Maximum Vout = Vin / VIN_VOUT_RATIO
 * The power supply needs headroom between input and output voltage.
 */
#define VIN_VOUT_RATIO (float)1.1f
/** @} */

#endif // __DPS_MODEL_H__
