/**
 * @file motor_controller.c
 * @brief Motor Controller Implementation - Integrates L6470 + AS5600
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 * 
 * @note Motor control integration layer following instruction files:
 * - .github/instructions/stm32h7-spi-l6470.instructions.md
 * - .github/instructions/stm32h7-i2c-as5600.instructions.md
 * - .github/instructions/safety-systems.instructions.md
 * - .github/instructions/units-measurements.instructions.md
 */

#include "motor_controller.h"
#include "drivers/l6470/l6470_driver.h"
#include "drivers/as5600/as5600_driver.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#include "config/safety_config.h"
#include "common/error_codes.h"
#include "common/system_state.h"
#include "safety/safety_system.h"
#include "safety/fault_monitor.h"
#include <math.h>

/* ========================================================================== */
/* Private Variables and State Management                                    */
/* ========================================================================== */

static bool motor_controller_initialized = false;
static SPI_HandleTypeDef* motor_spi_handle = NULL;
static I2C_HandleTypeDef* encoder_i2c1_handle = NULL;
static I2C_HandleTypeDef* encoder_i2c2_handle = NULL;

// Motor control state for each motor
typedef struct {
    bool enabled;
    MotorState_t state;
    float target_position_deg;
    float current_position_deg;
    float position_error_deg;
    float target_velocity_dps;
    float current_velocity_dps;
    uint32_t last_update_time;
    uint32_t fault_count;
} MotorControlState_t;

static MotorControlState_t motor_control_state[MAX_MOTORS] = {0};

/* ========================================================================== */
/* Private Function Declarations                                             */
/* ========================================================================== */

static SystemError_t motor_controller_validate_motor_id(uint8_t motor_id);
static SystemError_t motor_controller_update_position(uint8_t motor_id);
static SystemError_t motor_controller_check_limits(uint8_t motor_id, float target_position_deg);
static SystemError_t motor_controller_safety_check(uint8_t motor_id);
static float motor_controller_calculate_position_error(float target, float current);

/* ========================================================================== */
/* Public API Implementation                                                 */
/* ========================================================================== */

/**
 * @brief Initialize motor controller system
 * @param hspi SPI handle for L6470 communication
 * @param hi2c1 I2C handle for encoder 1
 * @param hi2c2 I2C handle for encoder 2
 * @return System error code
 */
SystemError_t motor_controller_init(SPI_HandleTypeDef* hspi, 
                                   I2C_HandleTypeDef* hi2c1, 
                                   I2C_HandleTypeDef* hi2c2) {
    if (hspi == NULL || hi2c1 == NULL || hi2c2 == NULL) {
        return ERROR_MOTOR_CONFIG_INVALID;
    }
    
    // SAFETY-CRITICAL: Check safety system is operational before motor init
    if (!safety_system_is_operational()) {
        safety_log_event(SAFETY_EVENT_MOTOR_INIT_BLOCKED, 0xFF, 0);
        return ERROR_SAFETY_SYSTEM_NOT_READY;
    }
    
    // Check for emergency stop condition
    if (safety_get_emergency_stop_state()) {
        safety_log_event(SAFETY_EVENT_MOTOR_INIT_BLOCKED, 0xFF, 1);
        return ERROR_SAFETY_EMERGENCY_STOP;
    }
    
    // Store handle references
    motor_spi_handle = hspi;
    encoder_i2c1_handle = hi2c1;
    encoder_i2c2_handle = hi2c2;
    
    // Initialize L6470 stepper drivers
    SystemError_t result = l6470_init(hspi);
    if (result != SYSTEM_OK) {
        fault_monitor_record_system_fault(SYSTEM_FAULT_INIT_ERROR, 
                                          FAULT_SEVERITY_CRITICAL, result);
        return result;
    }
    
    // Initialize AS5600 encoders
    result = as5600_init(hi2c1, hi2c2);
    if (result != SYSTEM_OK) {
        fault_monitor_record_system_fault(SYSTEM_FAULT_INIT_ERROR, 
                                          FAULT_SEVERITY_CRITICAL, result);
        return result;
    }
    
    // Initialize motor control states
    for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
        MotorControlState_t* state = &motor_control_state[motor_id];
        
        state->enabled = false;
        state->state = MOTOR_STATE_IDLE;
        state->target_position_deg = 0.0f;
        state->current_position_deg = 0.0f;
        state->position_error_deg = 0.0f;
        state->target_velocity_dps = 0.0f;
        state->current_velocity_dps = 0.0f;
        state->last_update_time = HAL_GetTick();
        state->fault_count = 0;
        
        // Read initial encoder position
        float initial_position;
        result = as5600_read_angle_degrees(motor_id, &initial_position);
        if (result == SYSTEM_OK) {
            state->current_position_deg = initial_position;
        }
    }
    
    motor_controller_initialized = true;
    
    return SYSTEM_OK;
}

/**
 * @brief Enable motor control
 * @param motor_id Motor identifier
 * @return System error code
 */
SystemError_t motor_controller_enable_motor(uint8_t motor_id) {
    SystemError_t result = motor_controller_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Perform safety checks before enabling
    result = motor_controller_safety_check(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Check magnet detection for encoder
    bool magnet_ok;
    result = as5600_check_magnet(motor_id, &magnet_ok);
    if (result != SYSTEM_OK || !magnet_ok) {
        return ERROR_ENCODER_MAGNET_NOT_DETECTED;
    }
    
    // Update current position from encoder
    result = motor_controller_update_position(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    MotorControlState_t* state = &motor_control_state[motor_id];
    state->enabled = true;
    state->state = MOTOR_STATE_IDLE;
    state->target_position_deg = state->current_position_deg; // Set target to current position
    
    return SYSTEM_OK;
}

/**
 * @brief Disable motor control
 * @param motor_id Motor identifier
 * @return System error code
 */
SystemError_t motor_controller_disable_motor(uint8_t motor_id) {
    SystemError_t result = motor_controller_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Stop motor and set to high impedance
    result = l6470_hard_hiz(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    MotorControlState_t* state = &motor_control_state[motor_id];
    state->enabled = false;
    state->state = MOTOR_STATE_IDLE;
    
    return SYSTEM_OK;
}

/**
 * @brief Move motor to absolute position
 * @param motor_id Motor identifier
 * @param target_position_deg Target position in degrees
 * @return System error code
 */
SystemError_t motor_controller_move_to_position(uint8_t motor_id, float target_position_deg) {
    SystemError_t result = motor_controller_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    MotorControlState_t* state = &motor_control_state[motor_id];
    
    if (!state->enabled) {
        return ERROR_MOTOR_NOT_ENABLED;
    }
    
    // Check position limits
    result = motor_controller_check_limits(motor_id, target_position_deg);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Update current position from encoder
    result = motor_controller_update_position(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Convert degrees to motor steps
    // Using SSOT motor configuration: MOTOR_TOTAL_STEPS for full revolution
    float steps_per_degree = MOTOR_TOTAL_STEPS / 360.0f;
    int32_t target_steps = (int32_t)(target_position_deg * steps_per_degree + 0.5f);
    
    // Send move command to L6470
    result = l6470_move_to_position(motor_id, target_steps);
    if (result != SYSTEM_OK) {
        state->fault_count++;
        return result;
    }
    
    // Update control state
    state->target_position_deg = target_position_deg;
    state->state = MOTOR_STATE_RUNNING;
    state->last_update_time = HAL_GetTick();
    
    return SYSTEM_OK;
}

/**
 * @brief Stop motor with controlled deceleration
 * @param motor_id Motor identifier
 * @return System error code
 */
SystemError_t motor_controller_stop_motor(uint8_t motor_id) {
    SystemError_t result = motor_controller_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    result = l6470_soft_stop(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    MotorControlState_t* state = &motor_control_state[motor_id];
    state->state = MOTOR_STATE_DECELERATING;
    state->last_update_time = HAL_GetTick();
    
    return SYSTEM_OK;
}

/**
 * @brief Emergency stop motor immediately
 * @param motor_id Motor identifier
 * @return System error code
 */
SystemError_t motor_controller_emergency_stop(uint8_t motor_id) {
    SystemError_t result = motor_controller_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    result = l6470_hard_stop(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    MotorControlState_t* state = &motor_control_state[motor_id];
    state->state = MOTOR_STATE_EMERGENCY_STOP;
    state->last_update_time = HAL_GetTick();
    
    return SYSTEM_OK;
}

/**
 * @brief Update motor control loop (call periodically)
 * @return System error code
 */
SystemError_t motor_controller_update(void) {
    if (!motor_controller_initialized) {
        return ERROR_MOTOR_INIT_FAILED;
    }
    
    SystemError_t overall_result = SYSTEM_OK;
    
    // Update all motors
    for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
        MotorControlState_t* state = &motor_control_state[motor_id];
        
        if (!state->enabled) {
            continue;
        }
        
        // Update position from encoder
        SystemError_t result = motor_controller_update_position(motor_id);
        if (result != SYSTEM_OK) {
            state->fault_count++;
            overall_result = result;
            continue;
        }
        
        // Calculate position error
        state->position_error_deg = motor_controller_calculate_position_error(
            state->target_position_deg, state->current_position_deg);
        
        // Check if movement is complete (within tolerance)
        if (state->state == MOTOR_STATE_RUNNING && 
            fabsf(state->position_error_deg) < POSITION_TOLERANCE_DEG) {
            state->state = MOTOR_STATE_IDLE;
        }
        
        // Perform safety checks
        result = motor_controller_safety_check(motor_id);
        if (result != SYSTEM_OK) {
            state->fault_count++;
            overall_result = result;
            // Emergency stop on safety fault
            motor_controller_emergency_stop(motor_id);
        }
        
        state->last_update_time = HAL_GetTick();
    }
    
    return overall_result;
}

/**
 * @brief Get motor position in degrees
 * @param motor_id Motor identifier
 * @param position_deg Pointer to store position
 * @return System error code
 */
SystemError_t motor_controller_get_position(uint8_t motor_id, float* position_deg) {
    if (position_deg == NULL) {
        return ERROR_MOTOR_CONFIG_INVALID;
    }
    
    SystemError_t result = motor_controller_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Get current position from encoder
    result = as5600_read_angle_degrees(motor_id, position_deg);
    if (result == SYSTEM_OK) {
        motor_control_state[motor_id].current_position_deg = *position_deg;
    }
    
    return result;
}

/**
 * @brief Get motor velocity in degrees per second
 * @param motor_id Motor identifier
 * @param velocity_dps Pointer to store velocity
 * @return System error code
 */
SystemError_t motor_controller_get_velocity(uint8_t motor_id, float* velocity_dps) {
    if (velocity_dps == NULL) {
        return ERROR_MOTOR_CONFIG_INVALID;
    }
    
    SystemError_t result = motor_controller_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    result = as5600_get_velocity(motor_id, velocity_dps);
    if (result == SYSTEM_OK) {
        motor_control_state[motor_id].current_velocity_dps = *velocity_dps;
    }
    
    return result;
}

/**
 * @brief Get motor control state
 * @param motor_id Motor identifier
 * @param state Pointer to store state
 * @return System error code
 */
SystemError_t motor_controller_get_state(uint8_t motor_id, MotorState_t* state) {
    if (state == NULL) {
        return ERROR_MOTOR_CONFIG_INVALID;
    }
    
    SystemError_t result = motor_controller_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    *state = motor_control_state[motor_id].state;
    return SYSTEM_OK;
}

/**
 * @brief Get comprehensive motor status information
 * @param motor_id Motor identifier
 * @param status Pointer to store status structure
 * @return System error code
 */
SystemError_t motor_controller_get_status(uint8_t motor_id, MotorStatus_t* status) {
    if (status == NULL) {
        return ERROR_MOTOR_CONFIG_INVALID;
    }
    
    SystemError_t result = motor_controller_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    MotorControlState_t* control_state = &motor_control_state[motor_id];
    
    // Update current position and velocity
    result = motor_controller_update_position(motor_id);
    if (result != SYSTEM_OK) {
        // Still provide status even if update failed
    }
    
    // Fill status structure
    status->enabled = control_state->enabled;
    status->state = control_state->state;
    status->current_position_deg = control_state->current_position_deg;
    status->target_position_deg = control_state->target_position_deg;
    status->position_error_deg = control_state->position_error_deg;
    status->current_velocity_dps = control_state->current_velocity_dps;
    status->fault_count = control_state->fault_count;
    status->last_update_time_ms = control_state->last_update_time;
    
    return result;
}

/* ========================================================================== */
/* Private Function Implementations                                          */
/* ========================================================================== */

/**
 * @brief Validate motor ID parameter
 * @param motor_id Motor identifier to validate
 * @return System error code
 */
static SystemError_t motor_controller_validate_motor_id(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }
    
    if (!motor_controller_initialized) {
        return ERROR_MOTOR_INIT_FAILED;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Update current position from encoder
 * @param motor_id Motor identifier
 * @return System error code
 */
static SystemError_t motor_controller_update_position(uint8_t motor_id) {
    float current_position;
    SystemError_t result = as5600_read_angle_degrees(motor_id, &current_position);
    if (result == SYSTEM_OK) {
        motor_control_state[motor_id].current_position_deg = current_position;
        
        // Also update velocity
        float velocity;
        result = as5600_get_velocity(motor_id, &velocity);
        if (result == SYSTEM_OK) {
            motor_control_state[motor_id].current_velocity_dps = velocity;
        }
    }
    
    return result;
}

/**
 * @brief Check position limits
 * @param motor_id Motor identifier
 * @param target_position_deg Target position to check
 * @return System error code
 */
static SystemError_t motor_controller_check_limits(uint8_t motor_id, float target_position_deg) {
    (void)motor_id; // Not used in this simple implementation
    
    // Check against SSOT motor configuration limits
    if (target_position_deg < MOTOR_MIN_ANGLE_DEG || 
        target_position_deg > MOTOR_MAX_ANGLE_DEG) {
        return ERROR_MOTOR_POSITION_OUT_OF_RANGE;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Perform safety checks on motor
 * @param motor_id Motor identifier
 * @return System error code
 */
static SystemError_t motor_controller_safety_check(uint8_t motor_id) {
    // Check L6470 status for faults
    uint16_t l6470_status;
    SystemError_t result = l6470_get_status(motor_id, &l6470_status);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Check encoder status
    bool magnet_ok;
    result = as5600_check_magnet(motor_id, &magnet_ok);
    if (result != SYSTEM_OK || !magnet_ok) {
        return ERROR_ENCODER_MAGNET_NOT_DETECTED;
    }
    
    // Additional safety checks can be added here
    // - Temperature monitoring
    // - Supply voltage monitoring
    // - Emergency stop status
    
    return SYSTEM_OK;
}

/**
 * @brief Calculate shortest angular distance between target and current position
 * @param target Target position in degrees
 * @param current Current position in degrees
 * @return Position error in degrees (-180 to +180)
 */
static float motor_controller_calculate_position_error(float target, float current) {
    float error = target - current;
    
    // Handle wraparound (shortest path)
    while (error > 180.0f) {
        error -= 360.0f;
    }
    while (error < -180.0f) {
        error += 360.0f;
    }
    
    return error;
}

/**
 * @brief Check if motor controller is initialized
 * @return true if initialized, false otherwise
 */
bool motor_controller_is_initialized(void) {
    return motor_controller_initialized;
}

/**
 * @brief Home motor to zero position
 * @param motor_id Motor identifier
 * @return System error code
 */
SystemError_t motor_controller_home_motor(uint8_t motor_id) {
    SystemError_t result = motor_controller_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    MotorControlState_t* state = &motor_control_state[motor_id];
    
    if (!state->enabled) {
        return ERROR_MOTOR_NOT_ENABLED;
    }
    
    // Read current encoder position and set as zero reference
    float current_position;
    result = as5600_read_angle_degrees(motor_id, &current_position);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Set encoder zero position
    result = as5600_set_zero_position(motor_id, current_position);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Set L6470 position to zero
    result = l6470_reset_position(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Update control state
    state->current_position_deg = 0.0f;
    state->target_position_deg = 0.0f;
    state->position_error_deg = 0.0f;
    state->last_update_time = HAL_GetTick();
    
    return SYSTEM_OK;
}

/**
 * @brief Set motor velocity for continuous motion
 * @param motor_id Motor identifier
 * @param velocity_dps Target velocity in degrees per second
 * @return System error code
 */
SystemError_t motor_controller_set_velocity(uint8_t motor_id, float velocity_dps) {
    SystemError_t result = motor_controller_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    MotorControlState_t* state = &motor_control_state[motor_id];
    
    if (!state->enabled) {
        return ERROR_MOTOR_NOT_ENABLED;
    }
    
    // Check velocity limits
    if (fabsf(velocity_dps) > MOTOR_MAX_SPEED_DPS) {
        return ERROR_MOTOR_INVALID_SPEED;
    }
    
    // Convert degrees per second to L6470 speed units
    // L6470 speed is in steps/second, need conversion from motor_config.h
    float steps_per_second = velocity_dps * MOTOR_TOTAL_STEPS / 360.0f;
    
    result = l6470_run(motor_id, velocity_dps > 0.0f, fabsf(steps_per_second));
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Update control state
    state->target_velocity_dps = velocity_dps;
    state->state = MOTOR_STATE_RUNNING;
    state->last_update_time = HAL_GetTick();
    
    return SYSTEM_OK;
}

/**
 * @brief Calibrate encoder zero position
 * @param motor_id Motor identifier
 * @param zero_position_deg Zero position in degrees
 * @return System error code
 */
SystemError_t motor_controller_calibrate_encoder(uint8_t motor_id, float zero_position_deg) {
    SystemError_t result = motor_controller_validate_motor_id(motor_id);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Validate zero position range
    if (zero_position_deg < 0.0f || zero_position_deg >= 360.0f) {
        return ERROR_MOTOR_INVALID_POSITION;
    }
    
    // Set encoder zero position
    result = as5600_set_zero_position(motor_id, zero_position_deg);
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Update control state if motor is enabled
    if (motor_control_state[motor_id].enabled) {
        motor_control_state[motor_id].last_update_time = HAL_GetTick();
    }
    
    return SYSTEM_OK;
}
