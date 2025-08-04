---
applyTo: "src/**/*.c"
description: "Physical units and measurement handling for STM32H753ZI stepper motor project"
---

# Physical Units and Measurement Instructions

## Overview
This instruction file provides comprehensive guidance for implementing consistent physical units, measurements, and unit conversions throughout the STM32H753ZI stepper motor control project.

## Base Physical Units

### SI Base Units and Derived Units
Follow International System of Units (SI) with appropriate scaling for embedded systems:

```c
// Base SI units used in the system
// Length: meter (m) - typically use millimeters (mm) for mechanical dimensions
// Time: second (s) - typically use milliseconds (ms) for timing
// Mass: kilogram (kg) - typically use grams (g) for small masses
// Electric current: ampere (A) - typically use milliamperes (mA)
// Temperature: kelvin (K) - typically use Celsius (°C) for display
// Angle: radian (rad) - typically use degrees (°) for user interface

// Fundamental constants
#define MATH_PI                         3.14159265358979323846f
#define MATH_2PI                        (2.0f * MATH_PI)
#define MATH_PI_2                       (MATH_PI / 2.0f)
#define MATH_PI_4                       (MATH_PI / 4.0f)
#define MATH_DEG_TO_RAD                 (MATH_PI / 180.0f)
#define MATH_RAD_TO_DEG                 (180.0f / MATH_PI)

// Unit conversion factors
#define UNITS_MM_PER_METER              1000.0f
#define UNITS_MS_PER_SECOND             1000.0f
#define UNITS_US_PER_SECOND             1000000.0f
#define UNITS_MA_PER_AMPERE             1000.0f
#define UNITS_MV_PER_VOLT               1000.0f
#define UNITS_DEGREES_PER_REVOLUTION    360.0f
#define UNITS_STEPS_PER_REVOLUTION      200.0f      // Typical for stepper motor
#define UNITS_MICROSTEPS_PER_STEP       16.0f       // Default microstep setting
```

### Angular Units
```c
// Angular position and motion units
typedef float Angle_deg_t;              // Angle in degrees (0.0 to 360.0)
typedef float Angle_rad_t;              // Angle in radians (0.0 to 2π)
typedef float AngularVelocity_dps_t;    // Angular velocity in degrees per second
typedef float AngularVelocity_rps_t;    // Angular velocity in radians per second
typedef float AngularAccel_dps2_t;      // Angular acceleration in deg/s²
typedef float AngularAccel_rps2_t;      // Angular acceleration in rad/s²

// Angular unit conversions
static inline Angle_rad_t degrees_to_radians(Angle_deg_t degrees) {
    return degrees * MATH_DEG_TO_RAD;
}

static inline Angle_deg_t radians_to_degrees(Angle_rad_t radians) {
    return radians * MATH_RAD_TO_DEG;
}

static inline AngularVelocity_rps_t dps_to_rps(AngularVelocity_dps_t dps) {
    return dps * MATH_DEG_TO_RAD;
}

static inline AngularVelocity_dps_t rps_to_dps(AngularVelocity_rps_t rps) {
    return rps * MATH_RAD_TO_DEG;
}

// Normalize angle to 0-360 degrees range
static inline Angle_deg_t normalize_angle_deg(Angle_deg_t angle) {
    while (angle < 0.0f) angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;
    return angle;
}

// Normalize angle to 0-2π radians range
static inline Angle_rad_t normalize_angle_rad(Angle_rad_t angle) {
    while (angle < 0.0f) angle += MATH_2PI;
    while (angle >= MATH_2PI) angle -= MATH_2PI;
    return angle;
}

// Calculate shortest angular distance
static inline Angle_deg_t angular_distance_deg(Angle_deg_t from, Angle_deg_t to) {
    Angle_deg_t diff = normalize_angle_deg(to - from);
    if (diff > 180.0f) {
        diff -= 360.0f;
    }
    return diff;
}
```

### Motor Step Units
```c
// Stepper motor position units
typedef int32_t MotorSteps_t;           // Motor position in full steps
typedef int32_t MotorMicrosteps_t;      // Motor position in microsteps
typedef float MotorAngle_t;             // Motor angle in degrees

// Motor configuration for unit conversions
typedef struct {
    uint16_t steps_per_revolution;      // Full steps per revolution (typically 200)
    uint8_t microstep_setting;          // Microstep setting (1, 2, 4, 8, 16, 32, etc.)
    float gear_ratio;                   // Gear reduction ratio (output/input)
    bool direction_inverted;            // Direction inversion flag
} MotorUnitConfig_t;

// Calculate microsteps per revolution
static inline uint32_t motor_get_microsteps_per_rev(const MotorUnitConfig_t* config) {
    return config->steps_per_revolution * config->microstep_setting;
}

// Calculate degrees per microstep
static inline float motor_get_degrees_per_microstep(const MotorUnitConfig_t* config) {
    uint32_t microsteps_per_rev = motor_get_microsteps_per_rev(config);
    return (360.0f / config->gear_ratio) / (float)microsteps_per_rev;
}

// Convert motor steps to angle
static inline MotorAngle_t motor_steps_to_angle(MotorSteps_t steps, 
                                                const MotorUnitConfig_t* config) {
    float degrees_per_step = 360.0f / (config->steps_per_revolution * config->gear_ratio);
    float angle = (float)steps * degrees_per_step;
    
    if (config->direction_inverted) {
        angle = -angle;
    }
    
    return normalize_angle_deg(angle);
}

// Convert angle to motor steps
static inline MotorSteps_t motor_angle_to_steps(MotorAngle_t angle, 
                                                const MotorUnitConfig_t* config) {
    if (config->direction_inverted) {
        angle = -angle;
    }
    
    float steps_per_degree = (config->steps_per_revolution * config->gear_ratio) / 360.0f;
    return (MotorSteps_t)(angle * steps_per_degree + 0.5f); // Round to nearest step
}

// Convert microsteps to angle
static inline MotorAngle_t motor_microsteps_to_angle(MotorMicrosteps_t microsteps,
                                                     const MotorUnitConfig_t* config) {
    float degrees_per_microstep = motor_get_degrees_per_microstep(config);
    float angle = (float)microsteps * degrees_per_microstep;
    
    if (config->direction_inverted) {
        angle = -angle;
    }
    
    return normalize_angle_deg(angle);
}

// Convert angle to microsteps
static inline MotorMicrosteps_t motor_angle_to_microsteps(MotorAngle_t angle,
                                                          const MotorUnitConfig_t* config) {
    if (config->direction_inverted) {
        angle = -angle;
    }
    
    float microsteps_per_degree = 1.0f / motor_get_degrees_per_microstep(config);
    return (MotorMicrosteps_t)(angle * microsteps_per_degree + 0.5f); // Round to nearest
}
```

### Electrical Units
```c
// Electrical measurement units
typedef float Voltage_V_t;              // Voltage in volts
typedef float Voltage_mV_t;             // Voltage in millivolts
typedef float Current_A_t;              // Current in amperes
typedef float Current_mA_t;             // Current in milliamperes
typedef float Power_W_t;                // Power in watts
typedef float Power_mW_t;               // Power in milliwatts
typedef float Resistance_Ohm_t;         // Resistance in ohms
typedef float Frequency_Hz_t;           // Frequency in hertz

// Electrical unit conversions
static inline Voltage_mV_t volts_to_millivolts(Voltage_V_t volts) {
    return volts * UNITS_MV_PER_VOLT;
}

static inline Voltage_V_t millivolts_to_volts(Voltage_mV_t millivolts) {
    return millivolts / UNITS_MV_PER_VOLT;
}

static inline Current_mA_t amperes_to_milliamperes(Current_A_t amperes) {
    return amperes * UNITS_MA_PER_AMPERE;
}

static inline Current_A_t milliamperes_to_amperes(Current_mA_t milliamperes) {
    return milliamperes / UNITS_MA_PER_AMPERE;
}

static inline Power_W_t calculate_power(Voltage_V_t voltage, Current_A_t current) {
    return voltage * current;
}

static inline Resistance_Ohm_t calculate_resistance(Voltage_V_t voltage, Current_A_t current) {
    return (current > 0.0f) ? (voltage / current) : 0.0f;
}

// L6470 specific electrical parameters
typedef struct {
    Voltage_V_t supply_voltage;         // Motor driver supply voltage (7-46V)
    Current_A_t phase_current;          // Motor phase current (A)
    Voltage_V_t bemf_constant;          // Back-EMF constant (V·s/rad)
    Resistance_Ohm_t phase_resistance;  // Motor phase resistance (Ω)
    float inductance_mH;                // Motor phase inductance (mH)
} MotorElectricalParams_t;

// Calculate motor electrical constants
static inline float motor_calculate_kt(const MotorElectricalParams_t* params) {
    // Torque constant (N·m/A) approximately equals BEMF constant
    return params->bemf_constant;
}

static inline Power_W_t motor_calculate_power_consumption(const MotorElectricalParams_t* params,
                                                         AngularVelocity_rps_t velocity) {
    // P = I²R + BEMF × I (simplified model)
    Current_A_t current = params->phase_current;
    Voltage_V_t bemf = params->bemf_constant * velocity;
    
    return (current * current * params->phase_resistance) + (bemf * current);
}
```

### Temperature Units
```c
// Temperature measurement units
typedef float Temperature_C_t;          // Temperature in Celsius
typedef float Temperature_K_t;          // Temperature in Kelvin
typedef float Temperature_F_t;          // Temperature in Fahrenheit

// Temperature conversion constants
#define TEMP_KELVIN_OFFSET              273.15f
#define TEMP_FAHRENHEIT_SCALE           (9.0f / 5.0f)
#define TEMP_FAHRENHEIT_OFFSET          32.0f

// Temperature unit conversions
static inline Temperature_K_t celsius_to_kelvin(Temperature_C_t celsius) {
    return celsius + TEMP_KELVIN_OFFSET;
}

static inline Temperature_C_t kelvin_to_celsius(Temperature_K_t kelvin) {
    return kelvin - TEMP_KELVIN_OFFSET;
}

static inline Temperature_F_t celsius_to_fahrenheit(Temperature_C_t celsius) {
    return (celsius * TEMP_FAHRENHEIT_SCALE) + TEMP_FAHRENHEIT_OFFSET;
}

static inline Temperature_C_t fahrenheit_to_celsius(Temperature_F_t fahrenheit) {
    return (fahrenheit - TEMP_FAHRENHEIT_OFFSET) / TEMP_FAHRENHEIT_SCALE;
}

// Temperature safety limits for system
#define TEMP_OPERATING_MIN_C            -20.0f  // Minimum operating temperature
#define TEMP_OPERATING_MAX_C            70.0f   // Maximum operating temperature
#define TEMP_WARNING_THRESHOLD_C        60.0f   // Warning threshold
#define TEMP_SHUTDOWN_THRESHOLD_C       80.0f   // Emergency shutdown threshold

// Check temperature safety
typedef enum {
    TEMP_STATUS_NORMAL = 0,
    TEMP_STATUS_WARNING = 1,
    TEMP_STATUS_CRITICAL = 2,
    TEMP_STATUS_OUT_OF_RANGE = 3
} TemperatureStatus_t;

static inline TemperatureStatus_t temperature_check_status(Temperature_C_t temp) {
    if (temp < TEMP_OPERATING_MIN_C || temp > TEMP_SHUTDOWN_THRESHOLD_C) {
        return TEMP_STATUS_OUT_OF_RANGE;
    } else if (temp > TEMP_WARNING_THRESHOLD_C) {
        return TEMP_STATUS_WARNING;
    } else if (temp > TEMP_OPERATING_MAX_C) {
        return TEMP_STATUS_CRITICAL;
    } else {
        return TEMP_STATUS_NORMAL;
    }
}
```

### Time and Velocity Units
```c
// Time units (additional to time-handling.instructions.md)
typedef float Time_s_t;                 // Time in seconds (floating point)
typedef uint32_t Time_ms_t;             // Time in milliseconds (integer)
typedef uint32_t Time_us_t;             // Time in microseconds (integer)

// Velocity units for linear motion (if applicable)
typedef float LinearVelocity_mps_t;     // Linear velocity in meters per second
typedef float LinearVelocity_mmps_t;    // Linear velocity in millimeters per second
typedef float LinearAccel_mps2_t;       // Linear acceleration in m/s²
typedef float LinearAccel_mmps2_t;      // Linear acceleration in mm/s²

// Linear unit conversions
static inline LinearVelocity_mmps_t mps_to_mmps(LinearVelocity_mps_t mps) {
    return mps * UNITS_MM_PER_METER;
}

static inline LinearVelocity_mps_t mmps_to_mps(LinearVelocity_mmps_t mmps) {
    return mmps / UNITS_MM_PER_METER;
}

// Convert rotational to linear motion (for lead screw applications)
static inline LinearVelocity_mmps_t angular_to_linear_velocity(AngularVelocity_dps_t angular_vel,
                                                               float lead_screw_pitch_mm) {
    // Convert degrees/second to revolutions/second, then to linear velocity
    float rps = angular_vel / UNITS_DEGREES_PER_REVOLUTION;
    return rps * lead_screw_pitch_mm;
}

static inline AngularVelocity_dps_t linear_to_angular_velocity(LinearVelocity_mmps_t linear_vel,
                                                               float lead_screw_pitch_mm) {
    // Convert linear velocity to revolutions/second, then to degrees/second
    if (lead_screw_pitch_mm > 0.0f) {
        float rps = linear_vel / lead_screw_pitch_mm;
        return rps * UNITS_DEGREES_PER_REVOLUTION;
    }
    return 0.0f;
}
```

## ADC and Sensor Units

### ADC Value Conversions
```c
// ADC measurement units
typedef uint16_t ADCRaw_t;              // Raw ADC value (0-4095 for 12-bit)
typedef float ADCVoltage_t;             // ADC voltage (V)
typedef float ADCRatio_t;               // ADC ratio (0.0 to 1.0)

// ADC configuration
typedef struct {
    uint8_t resolution_bits;            // ADC resolution (12-bit typical)
    Voltage_V_t reference_voltage;      // ADC reference voltage (3.3V typical)
    float gain;                         // ADC input gain
    float offset;                       // ADC offset correction
} ADCConfig_t;

// ADC conversion functions
static inline ADCVoltage_t adc_raw_to_voltage(ADCRaw_t raw_value, 
                                              const ADCConfig_t* config) {
    uint32_t max_value = (1U << config->resolution_bits) - 1;
    float ratio = (float)raw_value / (float)max_value;
    return (ratio * config->reference_voltage / config->gain) + config->offset;
}

static inline ADCRaw_t adc_voltage_to_raw(ADCVoltage_t voltage,
                                          const ADCConfig_t* config) {
    uint32_t max_value = (1U << config->resolution_bits) - 1;
    float adjusted_voltage = (voltage - config->offset) * config->gain;
    float ratio = adjusted_voltage / config->reference_voltage;
    
    // Clamp to valid range
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;
    
    return (ADCRaw_t)(ratio * max_value + 0.5f); // Round to nearest
}

// Temperature sensor ADC conversion (STM32 internal sensor)
static inline Temperature_C_t adc_to_internal_temperature(ADCRaw_t raw_value,
                                                          const ADCConfig_t* config) {
    // STM32H7 internal temperature sensor constants
    const float V25 = 0.76f;            // Voltage at 25°C (V)
    const float AVG_SLOPE = 2.5e-3f;    // Average slope (V/°C)
    
    ADCVoltage_t voltage = adc_raw_to_voltage(raw_value, config);
    return ((V25 - voltage) / AVG_SLOPE) + 25.0f;
}

// Current sensing ADC conversion (using shunt resistor)
static inline Current_A_t adc_to_current(ADCRaw_t raw_value,
                                         const ADCConfig_t* config,
                                         Resistance_Ohm_t shunt_resistance) {
    ADCVoltage_t voltage = adc_raw_to_voltage(raw_value, config);
    return voltage / shunt_resistance;
}
```

### Encoder Units
```c
// AS5600 encoder units
typedef uint16_t EncoderRaw_t;          // Raw encoder value (0-4095 for 12-bit)
typedef float EncoderAngle_t;           // Encoder angle in degrees
typedef int16_t EncoderDelta_t;         // Encoder position change

// AS5600 encoder configuration
typedef struct {
    uint8_t resolution_bits;            // Encoder resolution (12-bit)
    EncoderAngle_t zero_offset_deg;     // Zero position offset
    bool direction_clockwise;           // Direction interpretation
    float magnetic_strength_threshold;  // Minimum magnetic field strength
} EncoderConfig_t;

// Encoder conversion functions
static inline EncoderAngle_t encoder_raw_to_angle(EncoderRaw_t raw_value,
                                                  const EncoderConfig_t* config) {
    uint32_t max_value = (1U << config->resolution_bits) - 1;
    float ratio = (float)raw_value / (float)max_value;
    float angle = ratio * 360.0f;
    
    // Apply zero offset
    angle += config->zero_offset_deg;
    
    // Apply direction
    if (!config->direction_clockwise) {
        angle = 360.0f - angle;
    }
    
    return normalize_angle_deg(angle);
}

static inline EncoderRaw_t encoder_angle_to_raw(EncoderAngle_t angle,
                                                const EncoderConfig_t* config) {
    // Apply direction
    if (!config->direction_clockwise) {
        angle = 360.0f - angle;
    }
    
    // Remove zero offset
    angle -= config->zero_offset_deg;
    angle = normalize_angle_deg(angle);
    
    uint32_t max_value = (1U << config->resolution_bits) - 1;
    float ratio = angle / 360.0f;
    
    return (EncoderRaw_t)(ratio * max_value + 0.5f); // Round to nearest
}

// Calculate encoder velocity from position difference
static inline AngularVelocity_dps_t encoder_calculate_velocity(EncoderAngle_t current_angle,
                                                               EncoderAngle_t previous_angle,
                                                               Time_ms_t time_delta_ms) {
    if (time_delta_ms == 0) return 0.0f;
    
    // Calculate shortest angular distance
    float angle_diff = angular_distance_deg(previous_angle, current_angle);
    
    // Convert to degrees per second
    float time_delta_s = (float)time_delta_ms / UNITS_MS_PER_SECOND;
    return angle_diff / time_delta_s;
}
```

## Unit Validation and Safety

### Range Checking Functions
```c
// Range validation structure
typedef struct {
    float min_value;                    // Minimum allowed value
    float max_value;                    // Maximum allowed value
    const char* unit_name;              // Unit name for error messages
} UnitRange_t;

// Common unit ranges for safety
static const UnitRange_t MOTOR_ANGLE_RANGE = {0.0f, 360.0f, "degrees"};
static const UnitRange_t MOTOR_VELOCITY_RANGE = {-1800.0f, 1800.0f, "deg/s"};
static const UnitRange_t MOTOR_ACCELERATION_RANGE = {-10000.0f, 10000.0f, "deg/s²"};
static const UnitRange_t SUPPLY_VOLTAGE_RANGE = {7.0f, 46.0f, "V"};
static const UnitRange_t MOTOR_CURRENT_RANGE = {0.0f, 3.0f, "A"};
static const UnitRange_t TEMPERATURE_RANGE = {-40.0f, 125.0f, "°C"};

// Range validation function
typedef enum {
    UNIT_VALID = 0,
    UNIT_BELOW_MIN = 1,
    UNIT_ABOVE_MAX = 2,
    UNIT_INVALID = 3
} UnitValidation_t;

static inline UnitValidation_t validate_unit_range(float value, const UnitRange_t* range) {
    if (isnan(value) || isinf(value)) {
        return UNIT_INVALID;
    }
    
    if (value < range->min_value) {
        return UNIT_BELOW_MIN;
    }
    
    if (value > range->max_value) {
        return UNIT_ABOVE_MAX;
    }
    
    return UNIT_VALID;
}

// Clamp value to valid range
static inline float clamp_to_range(float value, const UnitRange_t* range) {
    if (isnan(value) || isinf(value)) {
        return range->min_value; // Safe default
    }
    
    if (value < range->min_value) {
        return range->min_value;
    }
    
    if (value > range->max_value) {
        return range->max_value;
    }
    
    return value;
}

// Unit validation macros
#define VALIDATE_MOTOR_ANGLE(angle) \
    validate_unit_range((angle), &MOTOR_ANGLE_RANGE)

#define VALIDATE_MOTOR_VELOCITY(velocity) \
    validate_unit_range((velocity), &MOTOR_VELOCITY_RANGE)

#define VALIDATE_SUPPLY_VOLTAGE(voltage) \
    validate_unit_range((voltage), &SUPPLY_VOLTAGE_RANGE)

#define CLAMP_MOTOR_ANGLE(angle) \
    clamp_to_range((angle), &MOTOR_ANGLE_RANGE)

#define CLAMP_MOTOR_VELOCITY(velocity) \
    clamp_to_range((velocity), &MOTOR_VELOCITY_RANGE)
```

### Unit Testing and Calibration
```c
// Unit calibration structure
typedef struct {
    float scale_factor;                 // Calibration scale factor
    float offset;                       // Calibration offset
    bool calibrated;                    // Calibration status
    SystemTime_t calibration_time;      // When last calibrated
    uint32_t calibration_count;         // Number of calibrations performed
} UnitCalibration_t;

// Apply calibration to measurement
static inline float apply_calibration(float raw_value, const UnitCalibration_t* cal) {
    if (!cal->calibrated) {
        return raw_value; // No calibration applied
    }
    
    return (raw_value * cal->scale_factor) + cal->offset;
}

// Calculate calibration parameters from known references
void calculate_calibration(UnitCalibration_t* cal,
                          float raw_value1, float reference1,
                          float raw_value2, float reference2) {
    float raw_diff = raw_value2 - raw_value1;
    float ref_diff = reference2 - reference1;
    
    if (fabsf(raw_diff) > 1e-6f) { // Avoid division by zero
        cal->scale_factor = ref_diff / raw_diff;
        cal->offset = reference1 - (raw_value1 * cal->scale_factor);
        cal->calibrated = true;
        cal->calibration_time = time_get_system_ms();
        cal->calibration_count++;
    }
}

// Unit self-test functions
bool unit_test_angle_conversions(void) {
    // Test degree to radian conversion
    float deg = 180.0f;
    float rad = degrees_to_radians(deg);
    float back_to_deg = radians_to_degrees(rad);
    
    return (fabsf(back_to_deg - deg) < 1e-6f);
}

bool unit_test_motor_conversions(const MotorUnitConfig_t* config) {
    // Test motor step to angle conversion
    MotorSteps_t steps = 100;
    MotorAngle_t angle = motor_steps_to_angle(steps, config);
    MotorSteps_t back_to_steps = motor_angle_to_steps(angle, config);
    
    return (abs(back_to_steps - steps) <= 1); // Allow 1 step tolerance due to rounding
}

// Run all unit tests
bool run_unit_tests(void) {
    bool all_passed = true;
    
    all_passed &= unit_test_angle_conversions();
    
    // Test with default motor configuration
    MotorUnitConfig_t default_config = {
        .steps_per_revolution = 200,
        .microstep_setting = 16,
        .gear_ratio = 1.0f,
        .direction_inverted = false
    };
    all_passed &= unit_test_motor_conversions(&default_config);
    
    return all_passed;
}
```

## Display Formatting

### Unit Display Functions
```c
// Format units for display with appropriate precision
void format_angle_display(char* buffer, size_t buffer_size, Angle_deg_t angle) {
    snprintf(buffer, buffer_size, "%.1f°", angle);
}

void format_velocity_display(char* buffer, size_t buffer_size, AngularVelocity_dps_t velocity) {
    snprintf(buffer, buffer_size, "%.1f°/s", velocity);
}

void format_current_display(char* buffer, size_t buffer_size, Current_A_t current) {
    if (current < 1.0f) {
        snprintf(buffer, buffer_size, "%.0fmA", amperes_to_milliamperes(current));
    } else {
        snprintf(buffer, buffer_size, "%.2fA", current);
    }
}

void format_voltage_display(char* buffer, size_t buffer_size, Voltage_V_t voltage) {
    snprintf(buffer, buffer_size, "%.1fV", voltage);
}

void format_temperature_display(char* buffer, size_t buffer_size, Temperature_C_t temperature) {
    snprintf(buffer, buffer_size, "%.1f°C", temperature);
}

void format_power_display(char* buffer, size_t buffer_size, Power_W_t power) {
    if (power < 1.0f) {
        snprintf(buffer, buffer_size, "%.0fmW", power * 1000.0f);
    } else {
        snprintf(buffer, buffer_size, "%.1fW", power);
    }
}

// Generic unit formatting with automatic scaling
void format_unit_with_prefix(char* buffer, size_t buffer_size, 
                            float value, const char* base_unit) {
    const char* prefixes[] = {"p", "n", "μ", "m", "", "k", "M", "G"};
    const float scales[] = {1e-12f, 1e-9f, 1e-6f, 1e-3f, 1.0f, 1e3f, 1e6f, 1e9f};
    const int num_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);
    
    // Find appropriate prefix
    int prefix_index = 4; // Start with no prefix (1.0f scale)
    float abs_value = fabsf(value);
    
    if (abs_value > 0.0f) {
        for (int i = 0; i < num_prefixes; i++) {
            if (abs_value >= scales[i] && abs_value < scales[i] * 1000.0f) {
                prefix_index = i;
                break;
            }
        }
    }
    
    float scaled_value = value / scales[prefix_index];
    snprintf(buffer, buffer_size, "%.2f%s%s", 
             scaled_value, prefixes[prefix_index], base_unit);
}
```

Remember: **Consistent unit handling is critical for system safety and accuracy. Always validate units at system boundaries and use appropriate precision for each measurement type. Implement proper calibration procedures for all sensors and actuators.**
