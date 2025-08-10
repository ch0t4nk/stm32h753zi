/**
 * @file position_control.c
 * @brief Advanced Position Control with Encoder Feedback
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @details Closed-loop position control using AS5600 magnetic encoders
 *          with PID control, feedforward, and advanced algorithms
 */

#include "position_control.h"
#include "config/motor_config.h"
#include "config/safety_config.h"
#include "drivers/as5600/as5600_driver.h"
#include "hal_abstraction/hal_abstraction.h"
#include "motion_profile.h"
#include "safety/fault_monitor.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ==========================================================================
 */
/* Forward Declarations for Static Functions                                 */
/* ==========================================================================
 */

static SystemError_t read_encoder_position(uint8_t motor_id,
                                           int32_t *position);
static void apply_position_filter(PositionControl_t *ctrl);
static void calculate_velocity(PositionControl_t *ctrl, uint32_t dt_ms);
static float calculate_pid_output(PositionControl_t *ctrl, uint32_t dt_ms);
static float calculate_feedforward_output(PositionControl_t *ctrl,
                                          uint32_t target_velocity,
                                          uint32_t dt_ms);
static float apply_output_limits(PositionControl_t *ctrl, float output);
static SystemError_t send_motor_command(uint8_t motor_id, float output);
static void update_control_history(PositionControl_t *ctrl);
static SystemError_t
perform_limit_switch_homing(uint8_t motor_id, HomingConfig_t *homing_config);
static SystemError_t
perform_encoder_index_homing(uint8_t motor_id, HomingConfig_t *homing_config);
static SystemError_t
perform_current_position_homing(uint8_t motor_id,
                                HomingConfig_t *homing_config);
static bool check_limit_switch(uint8_t motor_id);

// Position control state for each motor
static PositionControl_t position_controllers[MAX_MOTORS];
static bool controller_initialized[MAX_MOTORS] = {false};

/**
 * @brief Initialize position control system
 * @return SystemError_t Operation result
 */
SystemError_t position_control_init(void) {
    // Clear all controllers
    memset(position_controllers, 0, sizeof(position_controllers));
    memset(controller_initialized, false, sizeof(controller_initialized));

    // Initialize each motor's position controller
    for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
        SystemError_t result = position_control_init_motor(motor_id);
        if (result != SYSTEM_OK) {
            return result;
        }
    }

    return SYSTEM_OK;
}

/**
 * @brief Initialize position controller for specific motor
 * @param motor_id Motor identifier
 * @return SystemError_t Operation result
 */
SystemError_t position_control_init_motor(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }

    PositionControl_t *ctrl = &position_controllers[motor_id];

    // Initialize PID parameters from SSOT configuration
    ctrl->pid.kp = PID_KP_DEFAULT;
    ctrl->pid.ki = PID_KI_DEFAULT;
    ctrl->pid.kd = PID_KD_DEFAULT;
    ctrl->pid.integral_limit = PID_INTEGRAL_LIMIT;
    ctrl->pid.output_limit = PID_OUTPUT_LIMIT;

    // Initialize feedforward parameters
    ctrl->feedforward.velocity_gain = FEEDFORWARD_VEL_GAIN;
    ctrl->feedforward.acceleration_gain = FEEDFORWARD_ACCEL_GAIN;
    ctrl->feedforward.friction_compensation = FEEDFORWARD_FRICTION_COMP;

    // Initialize control limits
    ctrl->limits.max_position_error = MAX_POSITION_ERROR_STEPS;
    ctrl->limits.max_velocity = MOTOR_MAX_SPEED;
    ctrl->limits.max_acceleration = MOTOR_MAX_ACCELERATION;

    // Initialize state
    ctrl->state.current_position = 0;
    ctrl->state.target_position = 0;
    ctrl->state.position_error = 0;
    ctrl->state.velocity = 0;
    ctrl->state.enabled = false;
    ctrl->state.homed = false;

    // Initialize filter
    ctrl->filter.position_filter_alpha = POSITION_FILTER_ALPHA;
    ctrl->filter.velocity_filter_alpha = VELOCITY_FILTER_ALPHA;

    // Clear history
    memset(&ctrl->history, 0, sizeof(ctrl->history));

    controller_initialized[motor_id] = true;

    return SYSTEM_OK;
}

/**
 * @brief Update position control loop for specified motor
 * @param motor_id Motor identifier
 * @param dt_ms Time step in milliseconds
 * @return SystemError_t Operation result
 */
SystemError_t position_control_update(uint8_t motor_id, uint32_t dt_ms) {
    if (motor_id >= MAX_MOTORS || !controller_initialized[motor_id]) {
        return ERROR_MOTOR_INVALID_ID;
    }

    PositionControl_t *ctrl = &position_controllers[motor_id];

    if (!ctrl->state.enabled) {
        return SYSTEM_OK; // Controller not enabled
    }

    // Read current position from encoder
    SystemError_t result =
        read_encoder_position(motor_id, &ctrl->state.current_position);
    if (result != SYSTEM_OK) {
        // Handle encoder fault
        fault_monitor_report_fault(motor_id, FAULT_ENCODER_COMMUNICATION);
        return result;
    }

    // Apply position filtering
    apply_position_filter(ctrl);

    // Calculate position error
    ctrl->state.position_error =
        ctrl->state.target_position - ctrl->state.current_position;

    // Check for excessive position error
    if (abs(ctrl->state.position_error) > ctrl->limits.max_position_error) {
        fault_monitor_report_fault(motor_id, FAULT_POSITION_ERROR_EXCESSIVE);
        return ERROR_POSITION_ERROR_EXCESSIVE;
    }

    // Calculate velocity
    calculate_velocity(ctrl, dt_ms);

    // Get motion profile targets if active
    int32_t profile_target_pos = ctrl->state.target_position;
    uint32_t profile_target_vel = 0;

    if (motion_profile_is_active(motor_id)) {
        MotionProfileStatus_t profile_status;
        motion_profile_get_status(motor_id, &profile_status);
        profile_target_pos = profile_status.current_target_position;
        profile_target_vel = profile_status.current_target_velocity;

        // Update target position from profile
        ctrl->state.target_position = profile_target_pos;
        ctrl->state.position_error =
            ctrl->state.target_position - ctrl->state.current_position;
    }

    // Calculate PID output
    float pid_output = calculate_pid_output(ctrl, dt_ms);

    // Calculate feedforward output
    float feedforward_output =
        calculate_feedforward_output(ctrl, profile_target_vel, dt_ms);

    // Combine PID and feedforward
    float total_output = pid_output + feedforward_output;

    // Apply output limits
    total_output = apply_output_limits(ctrl, total_output);

    // Convert to motor command
    result = send_motor_command(motor_id, total_output);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Update history for next iteration
    update_control_history(ctrl);

    return SYSTEM_OK;
}

/**
 * @brief Set target position for motor
 * @param motor_id Motor identifier
 * @param target_position Target position in steps
 * @return SystemError_t Operation result
 */
SystemError_t position_control_set_target(uint8_t motor_id,
                                          int32_t target_position) {
    if (motor_id >= MAX_MOTORS || !controller_initialized[motor_id]) {
        return ERROR_MOTOR_INVALID_ID;
    }

    PositionControl_t *ctrl = &position_controllers[motor_id];

    // Validate target position
    if (abs(target_position) > MAX_POSITION_STEPS) {
        return ERROR_POSITION_OUT_OF_RANGE;
    }

    ctrl->state.target_position = target_position;

    return SYSTEM_OK;
}

/**
 * @brief Enable/disable position control for motor
 * @param motor_id Motor identifier
 * @param enable True to enable, false to disable
 * @return SystemError_t Operation result
 */
SystemError_t position_control_enable(uint8_t motor_id, bool enable) {
    if (motor_id >= MAX_MOTORS || !controller_initialized[motor_id]) {
        return ERROR_MOTOR_INVALID_ID;
    }

    PositionControl_t *ctrl = &position_controllers[motor_id];

    if (enable && !ctrl->state.enabled) {
        // Enabling controller - reset integral term
        ctrl->pid.integral = 0;

        // Read current position as starting point
        read_encoder_position(motor_id, &ctrl->state.current_position);
        ctrl->state.target_position = ctrl->state.current_position;
        ctrl->state.position_error = 0;
    }

    ctrl->state.enabled = enable;

    return SYSTEM_OK;
}

/**
 * @brief Perform homing sequence for motor
 * @param motor_id Motor identifier
 * @param homing_config Homing configuration
 * @return SystemError_t Operation result
 */
SystemError_t position_control_home(uint8_t motor_id,
                                    HomingConfig_t *homing_config) {
    if (motor_id >= MAX_MOTORS || !controller_initialized[motor_id] ||
        homing_config == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    PositionControl_t *ctrl = &position_controllers[motor_id];

    // Disable position control during homing
    position_control_enable(motor_id, false);

    SystemError_t result = SYSTEM_OK;

    switch (homing_config->method) {
    case HOMING_METHOD_LIMIT_SWITCH:
        result = perform_limit_switch_homing(motor_id, homing_config);
        break;

    case HOMING_METHOD_ENCODER_INDEX:
        result = perform_encoder_index_homing(motor_id, homing_config);
        break;

    case HOMING_METHOD_CURRENT_POSITION:
        result = perform_current_position_homing(motor_id, homing_config);
        break;

    default:
        result = ERROR_INVALID_PARAMETER;
        break;
    }

    if (result == SYSTEM_OK) {
        ctrl->state.homed = true;
        ctrl->state.current_position = homing_config->home_offset;
        ctrl->state.target_position = homing_config->home_offset;

        // Re-enable position control
        position_control_enable(motor_id, true);
    }

    return result;
}

/**
 * @brief Get position control status
 * @param motor_id Motor identifier
 * @param status Pointer to store status
 * @return SystemError_t Operation result
 */
SystemError_t position_control_get_status(uint8_t motor_id,
                                          PositionControlStatus_t *status) {
    if (motor_id >= MAX_MOTORS || !controller_initialized[motor_id] ||
        status == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    PositionControl_t *ctrl = &position_controllers[motor_id];

    status->enabled = ctrl->state.enabled;
    status->homed = ctrl->state.homed;
    status->current_position = ctrl->state.current_position;
    status->target_position = ctrl->state.target_position;
    status->position_error = ctrl->state.position_error;
    status->velocity = ctrl->state.velocity;
    status->pid_output = ctrl->history.last_pid_output;
    status->feedforward_output = ctrl->history.last_feedforward_output;

    // Calculate if position is settled
    status->position_settled =
        (abs(ctrl->state.position_error) <= POSITION_SETTLED_THRESHOLD) &&
        (fabsf(ctrl->state.velocity) <= VELOCITY_SETTLED_THRESHOLD);

    return SYSTEM_OK;
}

/**
 * @brief Read encoder position with error handling
 */
static SystemError_t read_encoder_position(uint8_t motor_id,
                                           int32_t *position) {
    uint16_t raw_angle;
    SystemError_t result = as5600_read_angle(motor_id, &raw_angle);

    if (result != SYSTEM_OK) {
        return result;
    }

    // Convert angle to steps (4096 counts per revolution)
    // Assuming motor has MOTOR_STEPS_PER_REV steps per revolution
    *position = (int32_t)((uint32_t)raw_angle * MOTOR_STEPS_PER_REV / 4096);

    return SYSTEM_OK;
}

/**
 * @brief Apply position filtering
 */
static void apply_position_filter(PositionControl_t *ctrl) {
    // Simple first-order low-pass filter
    ctrl->state.filtered_position =
        (ctrl->filter.position_filter_alpha * ctrl->state.current_position) +
        ((1.0f - ctrl->filter.position_filter_alpha) *
         ctrl->state.filtered_position);
}

/**
 * @brief Calculate velocity from position difference
 */
static void calculate_velocity(PositionControl_t *ctrl, uint32_t dt_ms) {
    if (dt_ms == 0)
        return;

    float dt_sec = (float)dt_ms / 1000.0f;
    int32_t position_diff =
        ctrl->state.current_position - ctrl->history.last_position;

    // Calculate raw velocity
    float raw_velocity = (float)position_diff / dt_sec;

    // Apply velocity filtering
    ctrl->state.velocity =
        (ctrl->filter.velocity_filter_alpha * raw_velocity) +
        ((1.0f - ctrl->filter.velocity_filter_alpha) * ctrl->state.velocity);
}

/**
 * @brief Calculate PID output
 */
static float calculate_pid_output(PositionControl_t *ctrl, uint32_t dt_ms) {
    if (dt_ms == 0)
        return 0.0f;

    float dt_sec = (float)dt_ms / 1000.0f;
    float error = (float)ctrl->state.position_error;

    // Proportional term
    float p_term = ctrl->pid.kp * error;

    // Integral term
    ctrl->pid.integral += error * dt_sec;

    // Apply integral windup protection
    if (ctrl->pid.integral > ctrl->pid.integral_limit) {
        ctrl->pid.integral = ctrl->pid.integral_limit;
    } else if (ctrl->pid.integral < -ctrl->pid.integral_limit) {
        ctrl->pid.integral = -ctrl->pid.integral_limit;
    }

    float i_term = ctrl->pid.ki * ctrl->pid.integral;

    // Derivative term (using filtered derivative)
    float error_diff = error - ctrl->history.last_error;
    float d_term = ctrl->pid.kd * (error_diff / dt_sec);

    // Combine PID terms
    float pid_output = p_term + i_term + d_term;

    // Store for status reporting
    ctrl->history.last_pid_output = pid_output;

    return pid_output;
}

/**
 * @brief Calculate feedforward output
 */
static float calculate_feedforward_output(PositionControl_t *ctrl,
                                          uint32_t target_velocity,
                                          uint32_t dt_ms) {
    float velocity_ff =
        ctrl->feedforward.velocity_gain * (float)target_velocity;

    // Calculate acceleration feedforward
    float accel_ff = 0.0f;
    if (dt_ms > 0) {
        float dt_sec = (float)dt_ms / 1000.0f;
        float velocity_diff =
            (float)target_velocity - ctrl->history.last_target_velocity;
        float acceleration = velocity_diff / dt_sec;
        accel_ff = ctrl->feedforward.acceleration_gain * acceleration;
    }

    // Friction compensation (simple model)
    float friction_ff = 0.0f;
    if (target_velocity != 0) {
        friction_ff = (target_velocity > 0)
                          ? ctrl->feedforward.friction_compensation
                          : -ctrl->feedforward.friction_compensation;
    }

    float feedforward_output = velocity_ff + accel_ff + friction_ff;

    // Store for status reporting
    ctrl->history.last_feedforward_output = feedforward_output;
    ctrl->history.last_target_velocity = (float)target_velocity;

    return feedforward_output;
}

/**
 * @brief Apply output limits
 */
static float apply_output_limits(PositionControl_t *ctrl, float output) {
    if (output > ctrl->pid.output_limit) {
        return ctrl->pid.output_limit;
    } else if (output < -ctrl->pid.output_limit) {
        return -ctrl->pid.output_limit;
    }

    return output;
}

/**
 * @brief Send motor command
 */
static SystemError_t send_motor_command(uint8_t motor_id, float output) {
    // Convert output to motor velocity command
    int32_t motor_velocity = (int32_t)output;

    // Apply safety limits
    if (abs(motor_velocity) > MOTOR_MAX_SPEED) {
        motor_velocity =
            (motor_velocity > 0) ? MOTOR_MAX_SPEED : -MOTOR_MAX_SPEED;
    }

    // Send command to L6470 driver
    return motor_set_velocity(motor_id, motor_velocity);
}

/**
 * @brief Update control history
 */
static void update_control_history(PositionControl_t *ctrl) {
    ctrl->history.last_position = ctrl->state.current_position;
    ctrl->history.last_error = (float)ctrl->state.position_error;
}

/**
 * @brief Perform limit switch homing
 */
static SystemError_t perform_limit_switch_homing(uint8_t motor_id,
                                                 HomingConfig_t *config) {
    // Implementation would depend on limit switch hardware
    // This is a simplified version

    // Move towards home switch at homing speed
    SystemError_t result =
        motor_run(motor_id, config->search_direction, config->search_speed);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Wait for limit switch activation (would need actual hardware interface)
    uint32_t timeout = HAL_Abstraction_GetTick() + config->timeout_ms;
    bool switch_found = false;

    while (HAL_Abstraction_GetTick() < timeout && !switch_found) {
        // Check limit switch state (placeholder)
        switch_found = check_limit_switch(motor_id);
        HAL_Abstraction_Delay(10);
    }

    if (!switch_found) {
        motor_stop(motor_id);
        return ERROR_HOMING_TIMEOUT;
    }

    // Stop motor
    motor_stop(motor_id);

    // Back off from switch
    result =
        motor_run(motor_id, -config->search_direction, config->backoff_speed);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Wait for switch to deactivate
    HAL_Abstraction_Delay(config->backoff_distance);
    motor_stop(motor_id);

    return SYSTEM_OK;
}

/**
 * @brief Perform encoder index homing
 */
static SystemError_t perform_encoder_index_homing(uint8_t motor_id,
                                                  HomingConfig_t *config) {
    // Implementation would use encoder index pulse
    // This is a placeholder implementation

    // Reset encoder position to zero at index
    return as5600_set_zero_position(motor_id, 0.0f);
}

/**
 * @brief Perform current position homing
 */
static SystemError_t perform_current_position_homing(uint8_t motor_id,
                                                     HomingConfig_t *config) {
    // Simply set current position as home
    int32_t current_pos;
    SystemError_t result = read_encoder_position(motor_id, &current_pos);

    if (result == SYSTEM_OK) {
        // Set encoder offset so current position becomes home_offset
        // This would require encoder calibration
    }

    return result;
}

/**
 * @brief Check limit switch state (placeholder)
 */
static bool check_limit_switch(uint8_t motor_id) {
    // Placeholder - would read actual limit switch GPIO
    return false;
}
