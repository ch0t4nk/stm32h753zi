# Documentation Owner: GitHub Copilot (auto-assigned)
# Motor Control Developer Guide

## Overview
This guide provides comprehensive information for implementing motor control algorithms for the STM32H753ZI with X-NUCLEO-IHM02A1 stepper motor control shield.

## L6470 Stepper Motor Driver

### Initialization
```c
#include "controllers/motor_control.h"
#include "drivers/l6470_driver.h"

// Initialize L6470 driver
SystemError_t motor_init(uint8_t motor_id) {
    // Validate motor ID
    if (motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }
    
    // Reset L6470
    l6470_reset(motor_id);
    HAL_Delay(1); // Wait for reset
    
    // Configure L6470 parameters (from SSOT)
    l6470_set_acceleration(motor_id, MOTOR_ACCELERATION);
    l6470_set_deceleration(motor_id, MOTOR_DECELERATION);
    l6470_set_max_speed(motor_id, MOTOR_MAX_SPEED);
    l6470_set_min_speed(motor_id, MOTOR_MIN_SPEED);
    
    // Configure current settings
    l6470_set_kval_hold(motor_id, MOTOR_KVAL_HOLD);
    l6470_set_kval_run(motor_id, MOTOR_KVAL_RUN);
    l6470_set_kval_acc(motor_id, MOTOR_KVAL_ACC);
    l6470_set_kval_dec(motor_id, MOTOR_KVAL_DEC);
    
    // Configure overcurrent protection
    l6470_set_ocd_threshold(motor_id, MOTOR_OCD_THRESHOLD);
    
    // Configure thermal protection
    l6470_set_thermal_compensation(motor_id, MOTOR_THERMAL_COMP);
    
    return SYSTEM_OK;
}
```

### Basic Motion Control
```c
// Move motor to absolute position
SystemError_t motor_move_to_position(uint8_t motor_id, int32_t position) {
    // Validate parameters
    if (motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }
    
    if (abs(position) > MOTOR_MAX_POSITION) {
        return ERROR_MOTOR_POSITION_OUT_OF_RANGE;
    }
    
    // Check motor status
    if (!motor_is_ready(motor_id)) {
        return ERROR_MOTOR_NOT_READY;
    }
    
    // Execute move command
    l6470_goto(motor_id, position);
    
    return SYSTEM_OK;
}

// Run motor at constant speed
SystemError_t motor_run(uint8_t motor_id, Direction_t direction, uint32_t speed) {
    // Validate parameters
    if (motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }
    
    if (speed > MOTOR_MAX_SPEED) {
        return ERROR_MOTOR_SPEED_OUT_OF_RANGE;
    }
    
    // Execute run command
    if (direction == DIRECTION_FORWARD) {
        l6470_run(motor_id, L6470_DIR_FWD, speed);
    } else {
        l6470_run(motor_id, L6470_DIR_REV, speed);
    }
    
    return SYSTEM_OK;
}

// Stop motor with deceleration
SystemError_t motor_soft_stop(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }
    
    l6470_soft_stop(motor_id);
    return SYSTEM_OK;
}

// Emergency stop (immediate)
SystemError_t motor_hard_stop(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }
    
    l6470_hard_stop(motor_id);
    return SYSTEM_OK;
}
```

## Closed-Loop Position Control

### AS5600 Encoder Integration
```c
#include "drivers/as5600_driver.h"

// Read encoder position
SystemError_t encoder_read_position(uint8_t encoder_id, uint16_t* position) {
    if (encoder_id >= MAX_ENCODERS || position == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    // Read raw angle from AS5600
    uint16_t raw_angle = as5600_read_angle(encoder_id);
    
    // Apply calibration if needed
    *position = apply_encoder_calibration(encoder_id, raw_angle);
    
    return SYSTEM_OK;
}

// Position control with encoder feedback
SystemError_t position_control_update(uint8_t motor_id) {
    static int32_t previous_error[MAX_MOTORS] = {0};
    static int32_t integral_error[MAX_MOTORS] = {0};
    
    // Read current position from encoder
    uint16_t encoder_position;
    if (encoder_read_position(motor_id, &encoder_position) != SYSTEM_OK) {
        return ERROR_ENCODER_READ_FAILED;
    }
    
    // Get target position
    int32_t target_position = motor_get_target_position(motor_id);
    
    // Calculate error
    int32_t error = target_position - (int32_t)encoder_position;
    
    // PID control calculation
    integral_error[motor_id] += error;
    
    // Anti-windup
    if (integral_error[motor_id] > PID_INTEGRAL_MAX) {
        integral_error[motor_id] = PID_INTEGRAL_MAX;
    } else if (integral_error[motor_id] < -PID_INTEGRAL_MAX) {
        integral_error[motor_id] = -PID_INTEGRAL_MAX;
    }
    
    int32_t derivative_error = error - previous_error[motor_id];
    
    // PID output
    int32_t output = (PID_KP * error) + 
                     (PID_KI * integral_error[motor_id]) + 
                     (PID_KD * derivative_error);
    
    // Scale output to motor speed
    int32_t motor_speed = output / PID_OUTPUT_SCALE;
    
    // Limit motor speed
    if (motor_speed > MOTOR_MAX_SPEED) {
        motor_speed = MOTOR_MAX_SPEED;
    } else if (motor_speed < -MOTOR_MAX_SPEED) {
        motor_speed = -MOTOR_MAX_SPEED;
    }
    
    // Apply motor command
    if (motor_speed > 0) {
        motor_run(motor_id, DIRECTION_FORWARD, motor_speed);
    } else if (motor_speed < 0) {
        motor_run(motor_id, DIRECTION_REVERSE, -motor_speed);
    } else {
        motor_soft_stop(motor_id);
    }
    
    previous_error[motor_id] = error;
    
    return SYSTEM_OK;
}
```

## Motion Profiling

### Trapezoidal Profile Generation
```c
typedef struct {
    int32_t start_position;
    int32_t end_position;
    uint32_t max_velocity;
    uint32_t acceleration;
    uint32_t deceleration;
    uint32_t total_time_ms;
    ProfilePhase_t current_phase;
} MotionProfile_t;

// Generate trapezoidal motion profile
SystemError_t motion_profile_generate(MotionProfile_t* profile, 
                                      int32_t start, int32_t end, 
                                      uint32_t max_vel, uint32_t accel) {
    if (profile == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    profile->start_position = start;
    profile->end_position = end;
    profile->max_velocity = max_vel;
    profile->acceleration = accel;
    profile->deceleration = accel; // Use same for decel
    
    int32_t distance = abs(end - start);
    
    // Calculate acceleration time
    uint32_t accel_time = max_vel / accel;
    int32_t accel_distance = (accel * accel_time * accel_time) / 2;
    
    // Check if we reach max velocity
    if (2 * accel_distance < distance) {
        // Trapezoidal profile
        uint32_t const_vel_distance = distance - 2 * accel_distance;
        uint32_t const_vel_time = const_vel_distance / max_vel;
        profile->total_time_ms = 2 * accel_time + const_vel_time;
    } else {
        // Triangular profile
        accel_time = sqrt(distance / accel);
        profile->max_velocity = accel * accel_time;
        profile->total_time_ms = 2 * accel_time;
    }
    
    profile->current_phase = PROFILE_PHASE_ACCEL;
    
    return SYSTEM_OK;
}

// Execute motion profile
SystemError_t motion_profile_execute(uint8_t motor_id, MotionProfile_t* profile, 
                                     uint32_t elapsed_time_ms) {
    if (profile == NULL || motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }
    
    int32_t target_position;
    uint32_t target_velocity;
    
    // Calculate target based on profile phase
    switch (profile->current_phase) {
        case PROFILE_PHASE_ACCEL:
            target_velocity = profile->acceleration * elapsed_time_ms / 1000;
            target_position = profile->start_position + 
                             (profile->acceleration * elapsed_time_ms * elapsed_time_ms) / 2000000;
            
            if (target_velocity >= profile->max_velocity) {
                profile->current_phase = PROFILE_PHASE_CONST_VEL;
            }
            break;
            
        case PROFILE_PHASE_CONST_VEL:
            target_velocity = profile->max_velocity;
            // Calculate position during constant velocity phase
            // [Implementation details...]
            break;
            
        case PROFILE_PHASE_DECEL:
            // Calculate deceleration phase
            // [Implementation details...]
            break;
            
        case PROFILE_PHASE_COMPLETE:
            target_position = profile->end_position;
            target_velocity = 0;
            break;
    }
    
    // Set motor target
    motor_set_target_position(motor_id, target_position);
    motor_set_target_velocity(motor_id, target_velocity);
    
    return SYSTEM_OK;
}
```

## Synchronization and Coordination

### Multi-Motor Coordination
```c
// Synchronize multiple motors
SystemError_t motors_sync_move(uint8_t* motor_ids, int32_t* positions, 
                               uint8_t motor_count) {
    if (motor_ids == NULL || positions == NULL || motor_count == 0) {
        return ERROR_INVALID_PARAMETER;
    }
    
    // Calculate timing for synchronized move
    uint32_t max_time = 0;
    for (uint8_t i = 0; i < motor_count; i++) {
        uint32_t move_time = calculate_move_time(motor_ids[i], positions[i]);
        if (move_time > max_time) {
            max_time = move_time;
        }
    }
    
    // Generate profiles for all motors
    MotionProfile_t profiles[MAX_MOTORS];
    for (uint8_t i = 0; i < motor_count; i++) {
        int32_t current_pos = motor_get_position(motor_ids[i]);
        uint32_t adjusted_speed = calculate_sync_speed(current_pos, positions[i], max_time);
        
        motion_profile_generate(&profiles[motor_ids[i]], 
                               current_pos, positions[i], 
                               adjusted_speed, MOTOR_ACCELERATION);
    }
    
    // Start synchronized execution
    uint32_t start_time = HAL_GetTick();
    for (uint8_t i = 0; i < motor_count; i++) {
        motor_start_profile(motor_ids[i], &profiles[motor_ids[i]]);
    }
    
    return SYSTEM_OK;
}
```

## Motor Diagnostics and Status

### Status Monitoring
```c
// Get comprehensive motor status
SystemError_t motor_get_status(uint8_t motor_id, MotorStatus_t* status) {
    if (motor_id >= MAX_MOTORS || status == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    // Read L6470 status register
    uint16_t l6470_status = l6470_get_status(motor_id);
    
    // Parse status flags
    status->is_moving = !(l6470_status & L6470_STATUS_MOT_STATUS);
    status->direction = (l6470_status & L6470_STATUS_DIR) ? DIRECTION_FORWARD : DIRECTION_REVERSE;
    status->step_clock_mode = (l6470_status & L6470_STATUS_SCK_MOD) ? true : false;
    
    // Fault flags
    status->faults.undervoltage = (l6470_status & L6470_STATUS_UVLO) ? true : false;
    status->faults.thermal_warning = (l6470_status & L6470_STATUS_TH_WRN) ? true : false;
    status->faults.thermal_shutdown = (l6470_status & L6470_STATUS_TH_SD) ? true : false;
    status->faults.overcurrent = (l6470_status & L6470_STATUS_OCD) ? true : false;
    
    // Read current position
    status->current_position = l6470_get_position(motor_id);
    
    // Read encoder position if available
    if (encoder_read_position(motor_id, &status->encoder_position) != SYSTEM_OK) {
        status->encoder_available = false;
    } else {
        status->encoder_available = true;
        status->position_error = status->current_position - status->encoder_position;
    }
    
    return SYSTEM_OK;
}
```

## Configuration and Tuning

### Motor Parameters (SSOT)
```c
// From src/config/motor_config.h
#define MOTOR_ACCELERATION              0x1000    // steps/s²
#define MOTOR_DECELERATION              0x1000    // steps/s²
#define MOTOR_MAX_SPEED                 0x1000    // steps/s
#define MOTOR_MIN_SPEED                 0x0040    // steps/s
#define MOTOR_KVAL_HOLD                 0x50      // Hold current
#define MOTOR_KVAL_RUN                  0x50      // Run current
#define MOTOR_KVAL_ACC                  0x50      // Acceleration current
#define MOTOR_KVAL_DEC                  0x50      // Deceleration current
#define MOTOR_OCD_THRESHOLD             0x08      // Overcurrent detection
#define MOTOR_THERMAL_COMP              0x00      // Thermal compensation

// PID Parameters
#define PID_KP                          100       // Proportional gain
#define PID_KI                          10        // Integral gain
#define PID_KD                          5         // Derivative gain
#define PID_INTEGRAL_MAX                10000     // Anti-windup limit
#define PID_OUTPUT_SCALE                1000      // Output scaling
```

### Tuning Guidelines
1. **Start with conservative values**
2. **Increase gains gradually**
3. **Monitor stability**
4. **Test under load conditions**
5. **Validate safety limits**

## Error Handling

### Common Motor Errors
```c
typedef enum {
    MOTOR_ERROR_NONE = 0,
    MOTOR_ERROR_INVALID_ID,
    MOTOR_ERROR_NOT_INITIALIZED,
    MOTOR_ERROR_COMMUNICATION_FAILURE,
    MOTOR_ERROR_POSITION_OUT_OF_RANGE,
    MOTOR_ERROR_SPEED_OUT_OF_RANGE,
    MOTOR_ERROR_OVERCURRENT,
    MOTOR_ERROR_THERMAL_FAULT,
    MOTOR_ERROR_ENCODER_FAILURE,
    MOTOR_ERROR_POSITION_LOST
} MotorError_t;

// Error recovery procedures
SystemError_t motor_error_recovery(uint8_t motor_id, MotorError_t error) {
    switch (error) {
        case MOTOR_ERROR_OVERCURRENT:
            // Stop motor and reduce current
            motor_hard_stop(motor_id);
            l6470_set_kval_run(motor_id, MOTOR_KVAL_RUN * 0.8);
            break;
            
        case MOTOR_ERROR_THERMAL_FAULT:
            // Stop motor and wait for cooling
            motor_hard_stop(motor_id);
            motor_set_state(motor_id, MOTOR_STATE_THERMAL_FAULT);
            break;
            
        case MOTOR_ERROR_POSITION_LOST:
            // Re-home motor
            motor_home(motor_id);
            break;
            
        default:
            return ERROR_UNKNOWN_MOTOR_ERROR;
    }
    
    return SYSTEM_OK;
}
```

## Performance Optimization

### Timing Considerations
- Control loop frequency: 1kHz (1ms period)
- Position update rate: 100Hz (10ms period)
- Status monitoring: 50Hz (20ms period)

### Memory Usage
- Minimize dynamic allocation
- Use fixed-size buffers
- Pre-allocate motion profiles

## Testing and Validation

### Unit Tests
```c
// Test motor initialization
void test_motor_init(void) {
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_init(MOTOR_1));
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_init(MOTOR_2));
    TEST_ASSERT_EQUAL(ERROR_MOTOR_INVALID_ID, motor_init(99));
}

// Test position control
void test_position_control(void) {
    // Initialize motor
    motor_init(MOTOR_1);
    
    // Move to position
    TEST_ASSERT_EQUAL(SYSTEM_OK, motor_move_to_position(MOTOR_1, 1000));
    
    // Wait for completion
    while (motor_is_moving(MOTOR_1)) {
        HAL_Delay(10);
    }
    
    // Verify position
    int32_t final_position = motor_get_position(MOTOR_1);
    TEST_ASSERT_INT32_WITHIN(10, 1000, final_position);
}
```

## Related Documentation
- [L6470 Register Reference](../reference/l6470_registers.md)
- [AS5600 Encoder Guide](../reference/as5600_encoder.md)
- [Motor Tuning Procedures](../procedures/motor_tuning.md)
