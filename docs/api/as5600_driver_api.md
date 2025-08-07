# AS5600 Driver API Reference

## Overview
The AS5600 Driver API provides comprehensive interface for controlling AS5600 magnetic rotary encoders in the STM32H753ZI project. This driver supports both real hardware and simulation modes with automatic hardware abstraction for closed-loop motor control.

## Core Driver Functions

### Initialization and Configuration

#### `as5600_init()`
```c
SystemError_t as5600_init(uint8_t encoder_id);
```
**Description**: Initialize AS5600 encoder driver for specified encoder.
**Parameters**:
- `encoder_id`: Encoder identifier (0 or 1 for dual encoder setup)
**Returns**: `SYSTEM_OK` on success, error code on failure
**Usage**:
```c
SystemError_t result = as5600_init(0);  // Initialize encoder 0
if (result != SYSTEM_OK) {
    // Handle initialization error
}
```

#### `as5600_deinit()`
```c
void as5600_deinit(uint8_t encoder_id);
```
**Description**: Deinitialize AS5600 driver and release resources.
**Parameters**:
- `encoder_id`: Encoder identifier
**Usage**:
```c
as5600_deinit(0);  // Clean up encoder 0 driver
```

#### `as5600_reset()`
```c
SystemError_t as5600_reset(uint8_t encoder_id);
```
**Description**: Perform software reset of AS5600 encoder.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = as5600_reset(0);
```

### Position Reading

#### `as5600_read_angle()`
```c
uint16_t as5600_read_angle(uint8_t encoder_id);
```
**Description**: Read current angular position.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: 12-bit angle value (0-4095, representing 0-360°)
**Usage**:
```c
uint16_t angle = as5600_read_angle(0);
float degrees = (float)angle * 360.0f / 4096.0f;
printf("Angle: %.2f degrees\n", degrees);
```

#### `as5600_read_angle_degrees()`
```c
float as5600_read_angle_degrees(uint8_t encoder_id);
```
**Description**: Read angular position in degrees.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: Angle in degrees (0.0 to 359.91°)
**Usage**:
```c
float angle_deg = as5600_read_angle_degrees(0);
printf("Current angle: %.2f°\n", angle_deg);
```

#### `as5600_read_angle_radians()`
```c
float as5600_read_angle_radians(uint8_t encoder_id);
```
**Description**: Read angular position in radians.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: Angle in radians (0.0 to 2π)
**Usage**:
```c
float angle_rad = as5600_read_angle_radians(0);
printf("Current angle: %.4f rad\n", angle_rad);
```

#### `as5600_read_raw_angle()`
```c
uint16_t as5600_read_raw_angle(uint8_t encoder_id);
```
**Description**: Read unfiltered raw angle data.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: 12-bit raw angle value
**Usage**:
```c
uint16_t raw_angle = as5600_read_raw_angle(0);
// Use for high-speed position feedback
```

### Multi-Turn Position Tracking

#### `as5600_get_absolute_position()`
```c
int32_t as5600_get_absolute_position(uint8_t encoder_id);
```
**Description**: Get absolute position with revolution counting.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: Absolute position in encoder counts (unlimited range)
**Usage**:
```c
int32_t abs_position = as5600_get_absolute_position(0);
printf("Absolute position: %d counts\n", abs_position);
```

#### `as5600_set_home_position()`
```c
SystemError_t as5600_set_home_position(uint8_t encoder_id);
```
**Description**: Set current position as home (zero) reference.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = as5600_set_home_position(0);  // Set current as home
```

#### `as5600_reset_position()`
```c
void as5600_reset_position(uint8_t encoder_id);
```
**Description**: Reset position counters to zero.
**Parameters**:
- `encoder_id`: Encoder identifier
**Usage**:
```c
as5600_reset_position(0);  // Reset to zero position
```

#### `as5600_get_revolution_count()`
```c
int16_t as5600_get_revolution_count(uint8_t encoder_id);
```
**Description**: Get number of complete revolutions since initialization.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: Revolution count (positive = clockwise)
**Usage**:
```c
int16_t revolutions = as5600_get_revolution_count(0);
printf("Revolutions: %d\n", revolutions);
```

### Velocity and Motion Detection

#### `as5600_get_velocity()`
```c
float as5600_get_velocity(uint8_t encoder_id);
```
**Description**: Get angular velocity in degrees per second.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: Angular velocity in degrees/second
**Usage**:
```c
float velocity = as5600_get_velocity(0);
printf("Velocity: %.2f deg/sec\n", velocity);
```

#### `as5600_get_velocity_rpm()`
```c
float as5600_get_velocity_rpm(uint8_t encoder_id);
```
**Description**: Get rotational velocity in RPM.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: Velocity in revolutions per minute
**Usage**:
```c
float rpm = as5600_get_velocity_rpm(0);
printf("Speed: %.1f RPM\n", rpm);
```

#### `as5600_is_moving()`
```c
bool as5600_is_moving(uint8_t encoder_id);
```
**Description**: Check if encoder is currently rotating.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: `true` if moving, `false` if stationary
**Usage**:
```c
if (as5600_is_moving(0)) {
    // Encoder is rotating
} else {
    // Encoder is stationary
}
```

#### `as5600_get_direction()`
```c
bool as5600_get_direction(uint8_t encoder_id);
```
**Description**: Get current rotation direction.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: `true` for clockwise, `false` for counter-clockwise
**Usage**:
```c
bool is_clockwise = as5600_get_direction(0);
printf("Direction: %s\n", is_clockwise ? "CW" : "CCW");
```

## Status and Health Monitoring

### Magnet Detection

#### `as5600_is_magnet_detected()`
```c
bool as5600_is_magnet_detected(uint8_t encoder_id);
```
**Description**: Check if magnet is within detection range.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: `true` if magnet detected, `false` otherwise
**Usage**:
```c
if (!as5600_is_magnet_detected(0)) {
    printf("Warning: No magnet detected on encoder 0\n");
}
```

#### `as5600_is_magnet_too_strong()`
```c
bool as5600_is_magnet_too_strong(uint8_t encoder_id);
```
**Description**: Check if magnet field is too strong.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: `true` if magnet too strong
**Usage**:
```c
if (as5600_is_magnet_too_strong(0)) {
    printf("Warning: Magnet too strong, move farther away\n");
}
```

#### `as5600_is_magnet_too_weak()`
```c
bool as5600_is_magnet_too_weak(uint8_t encoder_id);
```
**Description**: Check if magnet field is too weak.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: `true` if magnet too weak
**Usage**:
```c
if (as5600_is_magnet_too_weak(0)) {
    printf("Warning: Magnet too weak, move closer\n");
}
```

### AGC (Automatic Gain Control)

#### `as5600_get_agc_value()`
```c
uint8_t as5600_get_agc_value(uint8_t encoder_id);
```
**Description**: Get Automatic Gain Control value.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: AGC value (0-255)
**Usage**:
```c
uint8_t agc = as5600_get_agc_value(0);
printf("AGC value: %u\n", agc);
```

#### `as5600_get_magnitude()`
```c
uint16_t as5600_get_magnitude(uint8_t encoder_id);
```
**Description**: Get magnitude of magnetic field.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: Magnitude value (0-4095)
**Usage**:
```c
uint16_t magnitude = as5600_get_magnitude(0);
printf("Field magnitude: %u\n", magnitude);
```

## Configuration Functions

### Zero Position Configuration

#### `as5600_set_zero_position()`
```c
SystemError_t as5600_set_zero_position(uint8_t encoder_id, uint16_t zero_angle);
```
**Description**: Set zero position reference angle.
**Parameters**:
- `encoder_id`: Encoder identifier
- `zero_angle`: Zero position (0-4095)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = as5600_set_zero_position(0, 1024);  // Set 90° as zero
```

#### `as5600_get_zero_position()`
```c
uint16_t as5600_get_zero_position(uint8_t encoder_id);
```
**Description**: Get configured zero position.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: Zero position value
**Usage**:
```c
uint16_t zero_pos = as5600_get_zero_position(0);
printf("Zero position: %u\n", zero_pos);
```

### Maximum Angle Configuration

#### `as5600_set_max_angle()`
```c
SystemError_t as5600_set_max_angle(uint8_t encoder_id, uint16_t max_angle);
```
**Description**: Set maximum angle for partial rotation applications.
**Parameters**:
- `encoder_id`: Encoder identifier
- `max_angle`: Maximum angle (0-4095)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = as5600_set_max_angle(0, 2048);  // 180° range
```

#### `as5600_get_max_angle()`
```c
uint16_t as5600_get_max_angle(uint8_t encoder_id);
```
**Description**: Get configured maximum angle.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: Maximum angle value
**Usage**:
```c
uint16_t max_angle = as5600_get_max_angle(0);
printf("Max angle: %u\n", max_angle);
```

### Output Configuration

#### `as5600_set_output_stage()`
```c
SystemError_t as5600_set_output_stage(uint8_t encoder_id, AS5600_OutputStage_t output_stage);
```
**Description**: Configure analog output stage.
**Parameters**:
- `encoder_id`: Encoder identifier
- `output_stage`: Output configuration
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = as5600_set_output_stage(0, AS5600_OUTPUT_ANALOG_FULL_RANGE);
```

#### `as5600_set_pwm_frequency()`
```c
SystemError_t as5600_set_pwm_frequency(uint8_t encoder_id, AS5600_PWMFreq_t frequency);
```
**Description**: Set PWM output frequency.
**Parameters**:
- `encoder_id`: Encoder identifier
- `frequency`: PWM frequency setting
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = as5600_set_pwm_frequency(0, AS5600_PWM_115HZ);
```

### Filtering and Noise Reduction

#### `as5600_set_slow_filter()`
```c
SystemError_t as5600_set_slow_filter(uint8_t encoder_id, AS5600_SlowFilter_t filter);
```
**Description**: Configure slow filter for noise reduction.
**Parameters**:
- `encoder_id`: Encoder identifier
- `filter`: Filter setting
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = as5600_set_slow_filter(0, AS5600_SLOW_FILTER_16X);
```

#### `as5600_set_fast_filter()`
```c
SystemError_t as5600_set_fast_filter(uint8_t encoder_id, AS5600_FastFilter_t filter);
```
**Description**: Configure fast filter threshold.
**Parameters**:
- `encoder_id`: Encoder identifier
- `filter`: Filter threshold
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = as5600_set_fast_filter(0, AS5600_FAST_FILTER_10LSB);
```

## Direct Register Access

### Register Reading

#### `as5600_read_register()`
```c
uint16_t as5600_read_register(uint8_t encoder_id, uint8_t reg_address);
```
**Description**: Read value from AS5600 register.
**Parameters**:
- `encoder_id`: Encoder identifier
- `reg_address`: Register address (use AS5600_CHIP_REG_* constants)
**Returns**: Register value
**Usage**:
```c
uint16_t angle = as5600_read_register(0, AS5600_CHIP_REG_ANGLE);
uint8_t status = as5600_read_register(0, AS5600_CHIP_REG_STATUS);
```

#### `as5600_read_registers()`
```c
SystemError_t as5600_read_registers(uint8_t encoder_id, uint8_t start_reg, 
                                   uint8_t *data, uint8_t count);
```
**Description**: Read multiple consecutive registers.
**Parameters**:
- `encoder_id`: Encoder identifier
- `start_reg`: Starting register address
- `data`: Buffer to store register data
- `count`: Number of registers to read
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
uint8_t reg_data[4];
SystemError_t result = as5600_read_registers(0, AS5600_CHIP_REG_ANGLE, reg_data, 2);
```

### Register Writing

#### `as5600_write_register()`
```c
SystemError_t as5600_write_register(uint8_t encoder_id, uint8_t reg_address, uint16_t value);
```
**Description**: Write value to AS5600 register.
**Parameters**:
- `encoder_id`: Encoder identifier
- `reg_address`: Register address
- `value`: Value to write
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
// Set zero position
SystemError_t result = as5600_write_register(0, AS5600_CHIP_REG_ZPOS, 1024);
```

## Calibration and Setup

### Automatic Calibration

#### `as5600_auto_calibrate()`
```c
SystemError_t as5600_auto_calibrate(uint8_t encoder_id);
```
**Description**: Perform automatic calibration sequence.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: `SYSTEM_OK` on successful calibration
**Usage**:
```c
printf("Starting calibration...\n");
SystemError_t result = as5600_auto_calibrate(0);
if (result == SYSTEM_OK) {
    printf("Calibration completed successfully\n");
}
```

#### `as5600_calibrate_zero_max()`
```c
SystemError_t as5600_calibrate_zero_max(uint8_t encoder_id, uint16_t zero_pos, uint16_t max_pos);
```
**Description**: Calibrate with specific zero and maximum positions.
**Parameters**:
- `encoder_id`: Encoder identifier
- `zero_pos`: Zero position angle
- `max_pos`: Maximum position angle
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = as5600_calibrate_zero_max(0, 0, 2048);  // 0° to 180°
```

### Programming (One-Time)

#### `as5600_burn_angle_settings()`
```c
SystemError_t as5600_burn_angle_settings(uint8_t encoder_id);
```
**Description**: Permanently program angle settings to OTP memory.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: `SYSTEM_OK` on success
**Warning**: This is a one-time operation!
**Usage**:
```c
// WARNING: This permanently programs the chip!
SystemError_t result = as5600_burn_angle_settings(0);
```

#### `as5600_burn_config_settings()`
```c
SystemError_t as5600_burn_config_settings(uint8_t encoder_id);
```
**Description**: Permanently program configuration settings to OTP memory.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: `SYSTEM_OK` on success
**Warning**: This is a one-time operation!
**Usage**:
```c
// WARNING: This permanently programs the chip!
SystemError_t result = as5600_burn_config_settings(0);
```

## Multi-Encoder Operations

### Synchronized Reading

#### `as5600_read_all_angles()`
```c
SystemError_t as5600_read_all_angles(uint8_t encoder_count, uint8_t *encoder_ids, uint16_t *angles);
```
**Description**: Read angles from multiple encoders simultaneously.
**Parameters**:
- `encoder_count`: Number of encoders to read
- `encoder_ids`: Array of encoder identifiers
- `angles`: Array to store angle values
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
uint8_t encoders[] = {0, 1};
uint16_t angles[2];
SystemError_t result = as5600_read_all_angles(2, encoders, angles);
```

#### `as5600_sync_reset_position()`
```c
void as5600_sync_reset_position(uint8_t encoder_count, uint8_t *encoder_ids);
```
**Description**: Synchronously reset position counters for multiple encoders.
**Parameters**:
- `encoder_count`: Number of encoders
- `encoder_ids`: Array of encoder identifiers
**Usage**:
```c
uint8_t encoders[] = {0, 1};
as5600_sync_reset_position(2, encoders);
```

## Motor Integration Functions

### Motor Coupling

#### `as5600_link_to_motor()`
```c
SystemError_t as5600_link_to_motor(uint8_t encoder_id, uint8_t motor_id, float gear_ratio);
```
**Description**: Link encoder to motor for closed-loop control.
**Parameters**:
- `encoder_id`: Encoder identifier
- `motor_id`: Motor identifier to link to
- `gear_ratio`: Gear ratio (motor_steps / encoder_revolution)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
// Link encoder 0 to motor 0 with 1:1 ratio
SystemError_t result = as5600_link_to_motor(0, 0, 1.0f);

// Link with gear reduction (200:1)
result = as5600_link_to_motor(1, 1, 200.0f);
```

#### `as5600_get_motor_position_error()`
```c
float as5600_get_motor_position_error(uint8_t encoder_id);
```
**Description**: Get position error between encoder and linked motor.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: Position error in degrees
**Usage**:
```c
float error = as5600_get_motor_position_error(0);
if (fabs(error) > POSITION_ERROR_THRESHOLD) {
    // Apply correction
}
```

#### `as5600_calculate_motor_steps()`
```c
int32_t as5600_calculate_motor_steps(uint8_t encoder_id, float target_angle_degrees);
```
**Description**: Calculate motor steps needed to reach target encoder angle.
**Parameters**:
- `encoder_id`: Encoder identifier
- `target_angle_degrees`: Target angle in degrees
**Returns**: Required motor steps
**Usage**:
```c
int32_t steps = as5600_calculate_motor_steps(0, 180.0f);  // 180 degree target
l6470_move(0, steps);  // Command motor movement
```

## Utility Functions

### Data Conversion

#### `as5600_angle_to_degrees()`
```c
float as5600_angle_to_degrees(uint16_t angle_value);
```
**Description**: Convert raw angle value to degrees.
**Parameters**:
- `angle_value`: Raw angle value (0-4095)
**Returns**: Angle in degrees (0-360°)
**Usage**:
```c
uint16_t raw_angle = as5600_read_angle(0);
float degrees = as5600_angle_to_degrees(raw_angle);
```

#### `as5600_degrees_to_angle()`
```c
uint16_t as5600_degrees_to_angle(float degrees);
```
**Description**: Convert degrees to raw angle value.
**Parameters**:
- `degrees`: Angle in degrees
**Returns**: Raw angle value (0-4095)
**Usage**:
```c
uint16_t angle_val = as5600_degrees_to_angle(180.0f);  // 180° -> 2048
```

#### `as5600_angle_to_radians()`
```c
float as5600_angle_to_radians(uint16_t angle_value);
```
**Description**: Convert raw angle value to radians.
**Parameters**:
- `angle_value`: Raw angle value (0-4095)
**Returns**: Angle in radians (0-2π)
**Usage**:
```c
uint16_t raw_angle = as5600_read_angle(0);
float radians = as5600_angle_to_radians(raw_angle);
```

### Diagnostics and Testing

#### `as5600_run_self_test()`
```c
SystemError_t as5600_run_self_test(uint8_t encoder_id);
```
**Description**: Execute comprehensive self-test sequence.
**Parameters**:
- `encoder_id`: Encoder identifier
**Returns**: `SYSTEM_OK` if all tests pass
**Usage**:
```c
SystemError_t result = as5600_run_self_test(0);
if (result != SYSTEM_OK) {
    printf("Encoder self-test failed\n");
}
```

#### `as5600_dump_registers()`
```c
void as5600_dump_registers(uint8_t encoder_id);
```
**Description**: Print all register values for debugging.
**Parameters**:
- `encoder_id`: Encoder identifier
**Usage**:
```c
as5600_dump_registers(0);  // Debug output
```

#### `as5600_get_diagnostics()`
```c
void as5600_get_diagnostics(uint8_t encoder_id, AS5600_Diagnostics_t *diagnostics);
```
**Description**: Get comprehensive diagnostic information.
**Parameters**:
- `encoder_id`: Encoder identifier
- `diagnostics`: Pointer to diagnostics structure
**Usage**:
```c
AS5600_Diagnostics_t diag;
as5600_get_diagnostics(0, &diag);
printf("Magnet detected: %s\n", diag.magnet_detected ? "Yes" : "No");
printf("AGC value: %u\n", diag.agc_value);
printf("Magnitude: %u\n", diag.magnitude);
```

## Constants and Enumerations

### Register Addresses
```c
#define AS5600_CHIP_REG_ZMCO        0x00  // ZMCO register
#define AS5600_CHIP_REG_ZPOS_H      0x01  // Zero position high byte
#define AS5600_CHIP_REG_ZPOS_L      0x02  // Zero position low byte
#define AS5600_CHIP_REG_MPOS_H      0x03  // Maximum position high byte
#define AS5600_CHIP_REG_MPOS_L      0x04  // Maximum position low byte
#define AS5600_CHIP_REG_MANG_H      0x05  // Maximum angle high byte
#define AS5600_CHIP_REG_MANG_L      0x06  // Maximum angle low byte
#define AS5600_CHIP_REG_CONF_H      0x07  // Configuration high byte
#define AS5600_CHIP_REG_CONF_L      0x08  // Configuration low byte
#define AS5600_CHIP_REG_RAW_ANGLE_H 0x0C  // Raw angle high byte
#define AS5600_CHIP_REG_RAW_ANGLE_L 0x0D  // Raw angle low byte
#define AS5600_CHIP_REG_ANGLE_H     0x0E  // Filtered angle high byte
#define AS5600_CHIP_REG_ANGLE_L     0x0F  // Filtered angle low byte
#define AS5600_CHIP_REG_STATUS      0x0B  // Status register
#define AS5600_CHIP_REG_AGC         0x1A  // AGC register
#define AS5600_CHIP_REG_MAGNITUDE_H 0x1B  // Magnitude high byte
#define AS5600_CHIP_REG_MAGNITUDE_L 0x1C  // Magnitude low byte
#define AS5600_CHIP_REG_BURN        0xFF  // Burn register
```

### Status Register Bits
```c
#define AS5600_STATUS_MAGNET_DETECTED 0x20  // Magnet detected
#define AS5600_STATUS_MAGNET_TOO_STRONG 0x08  // Magnet too strong
#define AS5600_STATUS_MAGNET_TOO_WEAK 0x10   // Magnet too weak
```

### Configuration Enumerations
```c
typedef enum {
    AS5600_OUTPUT_ANALOG_FULL_RANGE = 0,
    AS5600_OUTPUT_ANALOG_REDUCED_RANGE = 1,
    AS5600_OUTPUT_PWM = 2
} AS5600_OutputStage_t;

typedef enum {
    AS5600_PWM_115HZ = 0,
    AS5600_PWM_230HZ = 1,
    AS5600_PWM_460HZ = 2,
    AS5600_PWM_920HZ = 3
} AS5600_PWMFreq_t;

typedef enum {
    AS5600_SLOW_FILTER_16X = 0,
    AS5600_SLOW_FILTER_8X = 1,
    AS5600_SLOW_FILTER_4X = 2,
    AS5600_SLOW_FILTER_2X = 3
} AS5600_SlowFilter_t;

typedef enum {
    AS5600_FAST_FILTER_10LSB = 0,
    AS5600_FAST_FILTER_6LSB = 1,
    AS5600_FAST_FILTER_7LSB = 2,
    AS5600_FAST_FILTER_9LSB = 3
} AS5600_FastFilter_t;
```

### Diagnostics Structure
```c
typedef struct {
    bool magnet_detected;
    bool magnet_too_strong;
    bool magnet_too_weak;
    uint8_t agc_value;
    uint16_t magnitude;
    uint16_t raw_angle;
    uint16_t filtered_angle;
    float velocity_deg_per_sec;
    int16_t revolution_count;
    bool is_moving;
    bool direction_clockwise;
} AS5600_Diagnostics_t;
```

## Usage Examples

### Basic Encoder Reading
```c
void basic_encoder_example(void) {
    // Initialize encoder
    SystemError_t result = as5600_init(0);
    if (result != SYSTEM_OK) {
        printf("Encoder initialization failed\n");
        return;
    }
    
    // Check magnet presence
    if (!as5600_is_magnet_detected(0)) {
        printf("Warning: No magnet detected\n");
    }
    
    // Read position continuously
    for (int i = 0; i < 100; i++) {
        float angle = as5600_read_angle_degrees(0);
        float velocity = as5600_get_velocity(0);
        
        printf("Angle: %.2f°, Velocity: %.1f°/s\n", angle, velocity);
        HAL_Delay(100);
    }
}
```

### Closed-Loop Motor Control
```c
void closed_loop_control_example(void) {
    // Initialize encoder and motor
    as5600_init(0);
    l6470_init(0);
    
    // Link encoder to motor
    as5600_link_to_motor(0, 0, 1.0f);  // 1:1 ratio
    
    // Set target position
    float target_angle = 180.0f;  // 180 degrees
    
    // Control loop
    for (int i = 0; i < 1000; i++) {
        float current_angle = as5600_read_angle_degrees(0);
        float error = target_angle - current_angle;
        
        // Simple proportional control
        if (fabs(error) > 1.0f) {  // 1 degree deadband
            int32_t correction_steps = (int32_t)(error * 10.0f);  // P gain = 10
            l6470_move(0, correction_steps);
        }
        
        HAL_Delay(10);
        
        // Check if target reached
        if (fabs(error) < 0.5f) {
            printf("Target reached: %.2f°\n", current_angle);
            break;
        }
    }
}
```

### Multi-Encoder Monitoring
```c
void multi_encoder_example(void) {
    // Initialize both encoders
    as5600_init(0);
    as5600_init(1);
    
    // Monitor both encoders
    uint8_t encoders[] = {0, 1};
    uint16_t angles[2];
    
    for (int i = 0; i < 500; i++) {
        // Read both angles simultaneously
        SystemError_t result = as5600_read_all_angles(2, encoders, angles);
        if (result == SYSTEM_OK) {
            float angle0 = as5600_angle_to_degrees(angles[0]);
            float angle1 = as5600_angle_to_degrees(angles[1]);
            
            printf("Encoder 0: %.2f°, Encoder 1: %.2f°\n", angle0, angle1);
        }
        
        HAL_Delay(20);
    }
}
```

### Calibration Example
```c
void calibration_example(void) {
    printf("Starting encoder calibration...\n");
    
    // Initialize encoder
    SystemError_t result = as5600_init(0);
    if (result != SYSTEM_OK) {
        printf("Initialization failed\n");
        return;
    }
    
    // Check magnet
    if (!as5600_is_magnet_detected(0)) {
        printf("Error: No magnet detected\n");
        return;
    }
    
    printf("Please rotate encoder to zero position and press enter...\n");
    getchar();
    
    // Set zero position
    uint16_t zero_angle = as5600_read_angle(0);
    result = as5600_set_zero_position(0, zero_angle);
    
    printf("Please rotate encoder to maximum position and press enter...\n");
    getchar();
    
    // Set maximum position
    uint16_t max_angle = as5600_read_angle(0);
    result = as5600_set_max_angle(0, max_angle);
    
    printf("Calibration complete!\n");
    printf("Zero position: %u, Max position: %u\n", zero_angle, max_angle);
}
```

## Related Documentation
- [Motor Simulation API](motor_simulation_api.md)
- [Hardware Simulation API](hardware_simulation_api.md)
- [L6470 Driver API](l6470_driver_api.md)
- [HAL Abstraction Guide](../guides/hal_abstraction_guide.md)
- [Testing Framework Guide](../guides/testing_framework_guide.md)
