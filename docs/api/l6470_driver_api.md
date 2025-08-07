# L6470 Driver API Reference

## Overview
The L6470 Driver API provides comprehensive interface for controlling L6470 stepper motor drivers in the STM32H753ZI project. This driver supports both real hardware and simulation modes, with automatic hardware abstraction.

## Core Driver Functions

### Initialization and Configuration

#### `l6470_init()`
```c
SystemError_t l6470_init(uint8_t motor_id);
```
**Description**: Initialize L6470 driver for specified motor.
**Parameters**:
- `motor_id`: Motor identifier (0 or 1 for dual motor setup)
**Returns**: `SYSTEM_OK` on success, error code on failure
**Usage**:
```c
SystemError_t result = l6470_init(0);  // Initialize motor 0
if (result != SYSTEM_OK) {
    // Handle initialization error
}
```

#### `l6470_deinit()`
```c
void l6470_deinit(uint8_t motor_id);
```
**Description**: Deinitialize L6470 driver and release resources.
**Parameters**:
- `motor_id`: Motor identifier
**Usage**:
```c
l6470_deinit(0);  // Clean up motor 0 driver
```

#### `l6470_reset_device()`
```c
SystemError_t l6470_reset_device(uint8_t motor_id);
```
**Description**: Perform hardware reset of L6470 device.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_reset_device(0);
```

### Basic Movement Commands

#### `l6470_move()`
```c
SystemError_t l6470_move(uint8_t motor_id, int32_t steps);
```
**Description**: Command relative movement in steps.
**Parameters**:
- `motor_id`: Motor identifier
- `steps`: Number of steps (positive = clockwise, negative = counter-clockwise)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_move(0, 1000);   // Move 1000 steps clockwise
result = l6470_move(1, -500);                 // Move 500 steps counter-clockwise
```

#### `l6470_goto()`
```c
SystemError_t l6470_goto(uint8_t motor_id, int32_t position);
```
**Description**: Move to absolute position.
**Parameters**:
- `motor_id`: Motor identifier
- `position`: Target absolute position in steps
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_goto(0, 5000);  // Go to position 5000
```

#### `l6470_goto_dir()`
```c
SystemError_t l6470_goto_dir(uint8_t motor_id, bool direction, int32_t position);
```
**Description**: Move to position in specified direction.
**Parameters**:
- `motor_id`: Motor identifier
- `direction`: `true` for clockwise, `false` for counter-clockwise
- `position`: Target position
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
// Force clockwise movement to position 1000
SystemError_t result = l6470_goto_dir(0, true, 1000);
```

#### `l6470_run()`
```c
SystemError_t l6470_run(uint8_t motor_id, float speed_steps_per_sec);
```
**Description**: Start continuous rotation at specified speed.
**Parameters**:
- `motor_id`: Motor identifier
- `speed_steps_per_sec`: Speed in steps per second (positive = clockwise)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_run(0, 1000.0f);   // Run at 1000 steps/sec clockwise
result = l6470_run(1, -800.0f);                 // Run at 800 steps/sec counter-clockwise
```

### Stop Commands

#### `l6470_soft_stop()`
```c
SystemError_t l6470_soft_stop(uint8_t motor_id);
```
**Description**: Decelerate motor to stop using configured deceleration.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_soft_stop(0);  // Gradual stop
```

#### `l6470_hard_stop()`
```c
SystemError_t l6470_hard_stop(uint8_t motor_id);
```
**Description**: Immediately stop motor without deceleration.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_hard_stop(0);  // Emergency stop
```

#### `l6470_soft_hiz()`
```c
SystemError_t l6470_soft_hiz(uint8_t motor_id);
```
**Description**: Decelerate to stop and enter high impedance state.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_soft_hiz(0);  // Stop and release motor
```

#### `l6470_hard_hiz()`
```c
SystemError_t l6470_hard_hiz(uint8_t motor_id);
```
**Description**: Immediately stop and enter high impedance state.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_hard_hiz(0);  // Emergency stop and release
```

## Status and Information

### Status Reading

#### `l6470_get_status()`
```c
uint16_t l6470_get_status(uint8_t motor_id);
```
**Description**: Read motor status register.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: 16-bit status register value
**Usage**:
```c
uint16_t status = l6470_get_status(0);
if (status & L6470_CHIP_STATUS_BUSY) {
    // Motor is executing command
}
if (status & L6470_CHIP_STATUS_UVLO) {
    // Undervoltage lockout detected
}
```

#### `l6470_is_busy()`
```c
bool l6470_is_busy(uint8_t motor_id);
```
**Description**: Check if motor is currently executing a command.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `true` if busy, `false` if idle
**Usage**:
```c
// Wait for movement completion
while (l6470_is_busy(0)) {
    HAL_Delay(10);
}
```

#### `l6470_get_position()`
```c
int32_t l6470_get_position(uint8_t motor_id);
```
**Description**: Get current motor position.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: Current position in steps
**Usage**:
```c
int32_t current_pos = l6470_get_position(0);
printf("Motor position: %d steps\n", current_pos);
```

#### `l6470_get_speed()`
```c
float l6470_get_speed(uint8_t motor_id);
```
**Description**: Get current motor speed.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: Current speed in steps per second
**Usage**:
```c
float current_speed = l6470_get_speed(0);
if (current_speed > MAX_SAFE_SPEED) {
    l6470_soft_stop(0);
}
```

### Register Access

#### `l6470_read_register()`
```c
uint32_t l6470_read_register(uint8_t motor_id, uint8_t reg_address);
```
**Description**: Read value from L6470 register.
**Parameters**:
- `motor_id`: Motor identifier
- `reg_address`: Register address (use L6470_CHIP_REG_* constants)
**Returns**: Register value
**Usage**:
```c
uint32_t abs_pos = l6470_read_register(0, L6470_CHIP_REG_ABS_POS);
uint16_t status = l6470_read_register(0, L6470_CHIP_REG_STATUS);
```

#### `l6470_write_register()`
```c
SystemError_t l6470_write_register(uint8_t motor_id, uint8_t reg_address, uint32_t value);
```
**Description**: Write value to L6470 register.
**Parameters**:
- `motor_id`: Motor identifier
- `reg_address`: Register address
- `value`: Value to write
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
// Set maximum speed
SystemError_t result = l6470_write_register(0, L6470_CHIP_REG_MAX_SPEED, 0x041);
```

#### `l6470_read_registers()`
```c
SystemError_t l6470_read_registers(uint8_t motor_id, uint8_t start_reg, 
                                  uint8_t *data, uint8_t count);
```
**Description**: Read multiple consecutive registers.
**Parameters**:
- `motor_id`: Motor identifier
- `start_reg`: Starting register address
- `data`: Buffer to store register data
- `count`: Number of registers to read
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
uint8_t reg_data[3];
SystemError_t result = l6470_read_registers(0, L6470_CHIP_REG_ABS_POS, reg_data, 3);
```

## Configuration Functions

### Motion Parameters

#### `l6470_set_acceleration()`
```c
SystemError_t l6470_set_acceleration(uint8_t motor_id, float acceleration);
```
**Description**: Set motor acceleration in steps/sec².
**Parameters**:
- `motor_id`: Motor identifier
- `acceleration`: Acceleration value (14.55 to 59590 steps/sec²)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_set_acceleration(0, 1000.0f);  // 1000 steps/sec²
```

#### `l6470_set_deceleration()`
```c
SystemError_t l6470_set_deceleration(uint8_t motor_id, float deceleration);
```
**Description**: Set motor deceleration in steps/sec².
**Parameters**:
- `motor_id`: Motor identifier
- `deceleration`: Deceleration value (14.55 to 59590 steps/sec²)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_set_deceleration(0, 2000.0f);  // 2000 steps/sec²
```

#### `l6470_set_max_speed()`
```c
SystemError_t l6470_set_max_speed(uint8_t motor_id, float max_speed);
```
**Description**: Set maximum motor speed in steps/sec.
**Parameters**:
- `motor_id`: Motor identifier
- `max_speed`: Maximum speed (15.25 to 15610 steps/sec)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_set_max_speed(0, 5000.0f);  // 5000 steps/sec max
```

#### `l6470_set_min_speed()`
```c
SystemError_t l6470_set_min_speed(uint8_t motor_id, float min_speed);
```
**Description**: Set minimum motor speed in steps/sec.
**Parameters**:
- `motor_id`: Motor identifier
- `min_speed`: Minimum speed (0 to 976.3 steps/sec)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_set_min_speed(0, 100.0f);  // 100 steps/sec min
```

#### `l6470_set_fs_speed()`
```c
SystemError_t l6470_set_fs_speed(uint8_t motor_id, float fs_speed);
```
**Description**: Set full-step speed threshold.
**Parameters**:
- `motor_id`: Motor identifier
- `fs_speed`: Full-step speed (7.63 to 15625 steps/sec)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_set_fs_speed(0, 2000.0f);  // Switch to full-step at 2000 steps/sec
```

### Current Control

#### `l6470_set_holding_current()`
```c
SystemError_t l6470_set_holding_current(uint8_t motor_id, float current_ma);
```
**Description**: Set holding current in milliamps.
**Parameters**:
- `motor_id`: Motor identifier
- `current_ma`: Holding current (31.25 to 4000 mA)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_set_holding_current(0, 500.0f);  // 500 mA holding
```

#### `l6470_set_running_current()`
```c
SystemError_t l6470_set_running_current(uint8_t motor_id, float current_ma);
```
**Description**: Set running current in milliamps.
**Parameters**:
- `motor_id`: Motor identifier
- `current_ma`: Running current (31.25 to 4000 mA)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_set_running_current(0, 1000.0f);  // 1000 mA running
```

#### `l6470_set_acceleration_current()`
```c
SystemError_t l6470_set_acceleration_current(uint8_t motor_id, float current_ma);
```
**Description**: Set acceleration current in milliamps.
**Parameters**:
- `motor_id`: Motor identifier
- `current_ma`: Acceleration current (31.25 to 4000 mA)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_set_acceleration_current(0, 1500.0f);  // 1500 mA accel
```

#### `l6470_set_deceleration_current()`
```c
SystemError_t l6470_set_deceleration_current(uint8_t motor_id, float current_ma);
```
**Description**: Set deceleration current in milliamps.
**Parameters**:
- `motor_id`: Motor identifier
- `current_ma`: Deceleration current (31.25 to 4000 mA)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_set_deceleration_current(0, 1200.0f);  // 1200 mA decel
```

### Microstepping Configuration

#### `l6470_set_step_mode()`
```c
SystemError_t l6470_set_step_mode(uint8_t motor_id, L6470_StepMode_t step_mode);
```
**Description**: Set microstepping mode.
**Parameters**:
- `motor_id`: Motor identifier
- `step_mode`: Step mode (L6470_STEP_MODE_FULL, L6470_STEP_MODE_HALF, etc.)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
// Set 1/16 microstepping
SystemError_t result = l6470_set_step_mode(0, L6470_STEP_MODE_1_16);
```

#### `l6470_get_step_mode()`
```c
L6470_StepMode_t l6470_get_step_mode(uint8_t motor_id);
```
**Description**: Get current microstepping mode.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: Current step mode
**Usage**:
```c
L6470_StepMode_t mode = l6470_get_step_mode(0);
if (mode == L6470_STEP_MODE_1_16) {
    // 1/16 microstepping active
}
```

### Advanced Configuration

#### `l6470_set_overcurrent_threshold()`
```c
SystemError_t l6470_set_overcurrent_threshold(uint8_t motor_id, float current_ma);
```
**Description**: Set overcurrent detection threshold.
**Parameters**:
- `motor_id`: Motor identifier
- `current_ma`: Threshold current (375 to 6000 mA)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_set_overcurrent_threshold(0, 3000.0f);  // 3A threshold
```

#### `l6470_set_stall_threshold()`
```c
SystemError_t l6470_set_stall_threshold(uint8_t motor_id, float current_ma);
```
**Description**: Set stall detection threshold.
**Parameters**:
- `motor_id`: Motor identifier
- `current_ma`: Stall threshold (31.25 to 4000 mA)
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_set_stall_threshold(0, 2000.0f);  // 2A stall detection
```

#### `l6470_set_alarm_config()`
```c
SystemError_t l6470_set_alarm_config(uint8_t motor_id, uint8_t alarm_flags);
```
**Description**: Configure alarm conditions.
**Parameters**:
- `motor_id`: Motor identifier
- `alarm_flags`: Alarm configuration flags
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
// Enable overcurrent and thermal alarms
uint8_t alarms = L6470_ALARM_OCD | L6470_ALARM_TH_SD;
SystemError_t result = l6470_set_alarm_config(0, alarms);
```

## Multi-Motor Operations

### Synchronized Commands

#### `l6470_sync_move()`
```c
SystemError_t l6470_sync_move(uint8_t motor_count, uint8_t *motor_ids, int32_t *steps);
```
**Description**: Synchronize movement of multiple motors.
**Parameters**:
- `motor_count`: Number of motors to coordinate
- `motor_ids`: Array of motor identifiers
- `steps`: Array of step counts for each motor
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
uint8_t motors[] = {0, 1};
int32_t steps[] = {1000, -500};
SystemError_t result = l6470_sync_move(2, motors, steps);
```

#### `l6470_sync_run()`
```c
SystemError_t l6470_sync_run(uint8_t motor_count, uint8_t *motor_ids, float *speeds);
```
**Description**: Start synchronized continuous rotation.
**Parameters**:
- `motor_count`: Number of motors
- `motor_ids`: Array of motor identifiers
- `speeds`: Array of speeds for each motor
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
uint8_t motors[] = {0, 1};
float speeds[] = {800.0f, 600.0f};
SystemError_t result = l6470_sync_run(2, motors, speeds);
```

#### `l6470_sync_stop()`
```c
SystemError_t l6470_sync_stop(uint8_t motor_count, uint8_t *motor_ids, bool hard_stop);
```
**Description**: Synchronize stop of multiple motors.
**Parameters**:
- `motor_count`: Number of motors
- `motor_ids`: Array of motor identifiers
- `hard_stop`: `true` for hard stop, `false` for soft stop
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
uint8_t motors[] = {0, 1};
SystemError_t result = l6470_sync_stop(2, motors, false);  // Soft stop both
```

## Fault and Error Handling

### Fault Detection

#### `l6470_get_fault_status()`
```c
uint16_t l6470_get_fault_status(uint8_t motor_id);
```
**Description**: Get fault status from motor.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: Fault status flags
**Usage**:
```c
uint16_t faults = l6470_get_fault_status(0);
if (faults & L6470_CHIP_STATUS_OCD) {
    handle_overcurrent_fault();
}
if (faults & L6470_CHIP_STATUS_TH_SD) {
    handle_thermal_shutdown();
}
```

#### `l6470_clear_faults()`
```c
SystemError_t l6470_clear_faults(uint8_t motor_id);
```
**Description**: Clear fault conditions and reset status.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_clear_faults(0);
```

#### `l6470_is_fault_active()`
```c
bool l6470_is_fault_active(uint8_t motor_id, uint16_t fault_mask);
```
**Description**: Check if specific fault is active.
**Parameters**:
- `motor_id`: Motor identifier
- `fault_mask`: Fault flag to check
**Returns**: `true` if fault is active
**Usage**:
```c
if (l6470_is_fault_active(0, L6470_CHIP_STATUS_UVLO)) {
    // Handle undervoltage condition
}
```

### Error Recovery

#### `l6470_recovery_sequence()`
```c
SystemError_t l6470_recovery_sequence(uint8_t motor_id);
```
**Description**: Execute complete fault recovery sequence.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `SYSTEM_OK` on successful recovery
**Usage**:
```c
if (l6470_get_fault_status(0) != 0) {
    SystemError_t result = l6470_recovery_sequence(0);
    if (result == SYSTEM_OK) {
        // Motor ready for operation
    }
}
```

## Utility Functions

### Position Management

#### `l6470_set_home_position()`
```c
SystemError_t l6470_set_home_position(uint8_t motor_id);
```
**Description**: Set current position as home (zero) position.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_set_home_position(0);  // Set current pos as home
```

#### `l6470_go_home()`
```c
SystemError_t l6470_go_home(uint8_t motor_id);
```
**Description**: Move motor to home position.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `SYSTEM_OK` on success
**Usage**:
```c
SystemError_t result = l6470_go_home(0);  // Return to home position
```

#### `l6470_calculate_steps_for_distance()`
```c
int32_t l6470_calculate_steps_for_distance(uint8_t motor_id, float distance_mm);
```
**Description**: Convert linear distance to step count.
**Parameters**:
- `motor_id`: Motor identifier
- `distance_mm`: Distance in millimeters
**Returns**: Equivalent step count
**Usage**:
```c
int32_t steps = l6470_calculate_steps_for_distance(0, 25.4f);  // 1 inch
SystemError_t result = l6470_move(0, steps);
```

### Diagnostics

#### `l6470_run_self_test()`
```c
SystemError_t l6470_run_self_test(uint8_t motor_id);
```
**Description**: Execute comprehensive self-test sequence.
**Parameters**:
- `motor_id`: Motor identifier
**Returns**: `SYSTEM_OK` if all tests pass
**Usage**:
```c
SystemError_t result = l6470_run_self_test(0);
if (result != SYSTEM_OK) {
    // Motor or driver has issues
}
```

#### `l6470_dump_registers()`
```c
void l6470_dump_registers(uint8_t motor_id);
```
**Description**: Print all register values for debugging.
**Parameters**:
- `motor_id`: Motor identifier
**Usage**:
```c
l6470_dump_registers(0);  // Debug output
```

## Constants and Enumerations

### Status Register Flags
```c
#define L6470_CHIP_STATUS_HIZ       0x0001  // High impedance state
#define L6470_CHIP_STATUS_BUSY      0x0002  // Motor busy
#define L6470_CHIP_STATUS_SW_F      0x0004  // Switch falling edge
#define L6470_CHIP_STATUS_SW_EVN    0x0008  // Switch event
#define L6470_CHIP_STATUS_DIR       0x0010  // Direction (1=CW, 0=CCW)
#define L6470_CHIP_STATUS_NOTPERF_CMD 0x0080  // Command not performed
#define L6470_CHIP_STATUS_WRONG_CMD 0x0100  // Wrong command
#define L6470_CHIP_STATUS_UVLO      0x0200  // Undervoltage lockout
#define L6470_CHIP_STATUS_TH_WRN    0x0400  // Thermal warning
#define L6470_CHIP_STATUS_TH_SD     0x0800  // Thermal shutdown
#define L6470_CHIP_STATUS_OCD       0x1000  // Overcurrent detection
#define L6470_CHIP_STATUS_STEP_LOSS_A 0x2000  // Stall A
#define L6470_CHIP_STATUS_STEP_LOSS_B 0x4000  // Stall B
#define L6470_CHIP_STATUS_SCK_MOD   0x8000  // Step clock mode
```

### Register Addresses
```c
#define L6470_CHIP_REG_ABS_POS      0x01  // Absolute position
#define L6470_CHIP_REG_EL_POS       0x02  // Electrical position
#define L6470_CHIP_REG_MARK         0x03  // Mark position
#define L6470_CHIP_REG_SPEED        0x04  // Current speed
#define L6470_CHIP_REG_ACC          0x05  // Acceleration
#define L6470_CHIP_REG_DEC          0x06  // Deceleration
#define L6470_CHIP_REG_MAX_SPEED    0x07  // Maximum speed
#define L6470_CHIP_REG_MIN_SPEED    0x08  // Minimum speed
#define L6470_CHIP_REG_KVAL_HOLD    0x09  // Holding current
#define L6470_CHIP_REG_KVAL_RUN     0x0A  // Running current
#define L6470_CHIP_REG_KVAL_ACC     0x0B  // Acceleration current
#define L6470_CHIP_REG_KVAL_DEC     0x0C  // Deceleration current
#define L6470_CHIP_REG_INT_SPD      0x0D  // Intersect speed
#define L6470_CHIP_REG_ST_SLP       0x0E  // Start slope
#define L6470_CHIP_REG_FN_SLP_ACC   0x0F  // Final slope acceleration
#define L6470_CHIP_REG_FN_SLP_DEC   0x10  // Final slope deceleration
#define L6470_CHIP_REG_K_THERM      0x11  // Thermal compensation
#define L6470_CHIP_REG_ADC_OUT      0x12  // ADC output
#define L6470_CHIP_REG_OCD_TH       0x13  // Overcurrent threshold
#define L6470_CHIP_REG_STALL_TH     0x14  // Stall threshold
#define L6470_CHIP_REG_FS_SPD       0x15  // Full-step speed
#define L6470_CHIP_REG_STEP_MODE    0x16  // Step mode
#define L6470_CHIP_REG_ALARM_EN     0x17  // Alarm enable
#define L6470_CHIP_REG_CONFIG       0x18  // Configuration
#define L6470_CHIP_REG_STATUS       0x19  // Status
```

### Step Modes
```c
typedef enum {
    L6470_STEP_MODE_FULL = 0x00,      // Full step
    L6470_STEP_MODE_HALF = 0x01,      // Half step
    L6470_STEP_MODE_1_4 = 0x02,       // 1/4 step
    L6470_STEP_MODE_1_8 = 0x03,       // 1/8 step
    L6470_STEP_MODE_1_16 = 0x04,      // 1/16 step
    L6470_STEP_MODE_1_32 = 0x05,      // 1/32 step
    L6470_STEP_MODE_1_64 = 0x06,      // 1/64 step
    L6470_STEP_MODE_1_128 = 0x07      // 1/128 step
} L6470_StepMode_t;
```

## Usage Examples

### Basic Motor Control
```c
void basic_motor_example(void) {
    // Initialize motor
    SystemError_t result = l6470_init(0);
    if (result != SYSTEM_OK) {
        // Handle error
        return;
    }
    
    // Configure motor parameters
    l6470_set_max_speed(0, 2000.0f);
    l6470_set_acceleration(0, 1000.0f);
    l6470_set_deceleration(0, 1000.0f);
    l6470_set_step_mode(0, L6470_STEP_MODE_1_16);
    
    // Move motor
    result = l6470_move(0, 1600);  // 1600 steps (100 full steps at 1/16)
    
    // Wait for completion
    while (l6470_is_busy(0)) {
        HAL_Delay(10);
    }
    
    // Check final position
    int32_t position = l6470_get_position(0);
    printf("Final position: %d steps\n", position);
}
```

### Continuous Rotation with Monitoring
```c
void continuous_rotation_example(void) {
    l6470_init(0);
    l6470_set_max_speed(0, 1500.0f);
    
    // Start continuous rotation
    SystemError_t result = l6470_run(0, 1000.0f);  // 1000 steps/sec
    
    // Monitor for 10 seconds
    for (int i = 0; i < 1000; i++) {
        HAL_Delay(10);
        
        // Check for faults
        uint16_t status = l6470_get_status(0);
        if (status & (L6470_CHIP_STATUS_OCD | L6470_CHIP_STATUS_TH_SD)) {
            l6470_hard_stop(0);
            printf("Fault detected: 0x%04X\n", status);
            break;
        }
        
        // Print speed every second
        if (i % 100 == 0) {
            float speed = l6470_get_speed(0);
            printf("Current speed: %.1f steps/sec\n", speed);
        }
    }
    
    // Stop motor
    l6470_soft_stop(0);
}
```

### Multi-Motor Coordination
```c
void multi_motor_example(void) {
    // Initialize both motors
    l6470_init(0);
    l6470_init(1);
    
    // Configure both motors
    for (int motor = 0; motor < 2; motor++) {
        l6470_set_max_speed(motor, 2000.0f);
        l6470_set_acceleration(motor, 1500.0f);
        l6470_set_step_mode(motor, L6470_STEP_MODE_1_8);
    }
    
    // Synchronized movement
    uint8_t motors[] = {0, 1};
    int32_t steps[] = {3200, -1600};  // Motor 0: +3200, Motor 1: -1600
    
    SystemError_t result = l6470_sync_move(2, motors, steps);
    
    // Wait for both to complete
    while (l6470_is_busy(0) || l6470_is_busy(1)) {
        HAL_Delay(10);
    }
    
    // Check final positions
    printf("Motor 0 position: %d\n", l6470_get_position(0));
    printf("Motor 1 position: %d\n", l6470_get_position(1));
}
```

## Related Documentation
- [Motor Simulation API](motor_simulation_api.md)
- [Hardware Simulation API](hardware_simulation_api.md)
- [AS5600 Driver API](as5600_driver_api.md)
- [HAL Abstraction Guide](../guides/hal_abstraction_guide.md)
