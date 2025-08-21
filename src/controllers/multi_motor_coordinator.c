/**
 * @file multi_motor_coordinator.c
 * @brief Multi-Motor Coordination and Synchronization System
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @details Coordinates multiple stepper motors for synchronized motion,
 *          load balancing, and complex multi-axis movements
 */

#include "multi_motor_coordinator.h"
#include "config/comm_config.h" // for INVALID_DEVICE_ID
#include "config/motor_config.h"
#include "config/safety_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include "motion_profile.h"
#include "position_control.h"
#include "safety/fault_monitor.h"
#include "safety/safety_system.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ==========================================================================
 */
/* Forward Declarations for Static Functions                                 */
/* ==========================================================================
 */

static SystemError_t
validate_coordinated_move(CoordinatedMoveCommand_t *move_cmd);
static SystemError_t
execute_synchronized_move(CoordinatedMoveCommand_t *move_cmd);
static SystemError_t
execute_load_sharing_move(CoordinatedMoveCommand_t *move_cmd);
static SystemError_t
execute_master_slave_move(CoordinatedMoveCommand_t *move_cmd);
static SystemError_t
execute_independent_move(CoordinatedMoveCommand_t *move_cmd);
static void update_motor_states(void);
static void update_synchronized_motion(uint32_t dt_ms);
static void update_load_sharing(uint32_t dt_ms);
static void update_master_slave_motion(uint32_t dt_ms);
static void check_motion_completion(void);
static void calculate_sync_error(CoordinationStatus_t *status);
static uint32_t calculate_move_time(int32_t distance, uint32_t max_velocity,
                                    uint32_t acceleration);

/* ==========================================================================
 */
/* Private Variables                                                         */
/* ==========================================================================
 */

// Multi-motor coordination state
static MultiMotorCoordinator_t coordinator;
static bool coordinator_initialized = false;

/**
 * @brief Initialize multi-motor coordination system
 * @return SystemError_t Operation result
 */
SystemError_t multi_motor_init(void) {
#if defined(SAFE_NO_MOTOR_POWER) && SAFE_NO_MOTOR_POWER == 1
    // Safe-mode: do not initialize motor coordination that could enable
    // outputs
    memset(&coordinator, 0, sizeof(coordinator));
    coordinator_initialized = false;
    safety_log_event(SAFETY_EVENT_MOTOR_INIT_BLOCKED, INVALID_DEVICE_ID, 0);
    return ERROR_SAFETY_SYSTEM_NOT_READY;
#endif
    // Clear coordinator state
    memset(&coordinator, 0, sizeof(coordinator));

    // Initialize motor states
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        coordinator.motor_states[i].motor_id = i;
        coordinator.motor_states[i].enabled = false;
        coordinator.motor_states[i].synchronized = false;
        coordinator.motor_states[i].load_share_enabled = false;
        coordinator.motor_states[i].priority = MOTOR_PRIORITY_NORMAL;
    }

    // Initialize synchronization parameters
    coordinator.sync_config.sync_tolerance = SYNC_TOLERANCE_DEFAULT;
    coordinator.sync_config.max_sync_wait_ms = MAX_SYNC_WAIT_MS;
    coordinator.sync_config.sync_master = 0; // Motor 0 as default master

    // Initialize load sharing parameters
    coordinator.load_config.load_balance_enabled = false;
    coordinator.load_config.load_threshold = LOAD_BALANCE_THRESHOLD;
    coordinator.load_config.max_load_difference = MAX_LOAD_DIFFERENCE;

    // Initialize coordinated motion state
    coordinator.motion_state.mode = COORDINATION_MODE_INDEPENDENT;
    coordinator.motion_state.active = false;
    coordinator.motion_state.sequence_step = 0;

    coordinator_initialized = true;

    return SYSTEM_OK;
}

/**
 * @brief Enable motor in coordination system
 * @param motor_id Motor identifier
 * @param enable True to enable, false to disable
 * @return SystemError_t Operation result
 */
SystemError_t multi_motor_enable_motor(uint8_t motor_id, bool enable) {
#if defined(SAFE_NO_MOTOR_POWER) && SAFE_NO_MOTOR_POWER == 1
    // In safe-mode, reject enable requests
    (void)motor_id;
    (void)enable;
    safety_log_event(SAFETY_EVENT_MOTOR_INIT_BLOCKED, motor_id, 0);
    return ERROR_SAFETY_SYSTEM_NOT_READY;
#endif
    if (motor_id >= MAX_MOTORS || !coordinator_initialized) {
        return ERROR_MOTOR_INVALID_ID;
    }

    coordinator.motor_states[motor_id].enabled = enable;

    if (!enable) {
        // Disable synchronization and load sharing for disabled motor
        coordinator.motor_states[motor_id].synchronized = false;
        coordinator.motor_states[motor_id].load_share_enabled = false;
    }

    return SYSTEM_OK;
}

/**
 * @brief Set coordination mode
 * @param mode Coordination mode
 * @return SystemError_t Operation result
 */
SystemError_t multi_motor_set_mode(CoordinationMode_t mode) {
    if (!coordinator_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Stop any active coordinated motion before changing mode
    if (coordinator.motion_state.active) {
        multi_motor_stop_coordinated_motion();
    }

    coordinator.motion_state.mode = mode;

    // Configure motors based on mode
    switch (mode) {
    case COORDINATION_MODE_INDEPENDENT:
        // Disable synchronization for all motors
        for (uint8_t i = 0; i < MAX_MOTORS; i++) {
            coordinator.motor_states[i].synchronized = false;
            coordinator.motor_states[i].load_share_enabled = false;
        }
        break;

    case COORDINATION_MODE_SYNCHRONIZED:
        // Enable synchronization for all enabled motors
        for (uint8_t i = 0; i < MAX_MOTORS; i++) {
            if (coordinator.motor_states[i].enabled) {
                coordinator.motor_states[i].synchronized = true;
            }
        }
        break;

    case COORDINATION_MODE_LOAD_SHARING:
        // Enable load sharing for all enabled motors
        for (uint8_t i = 0; i < MAX_MOTORS; i++) {
            if (coordinator.motor_states[i].enabled) {
                coordinator.motor_states[i].load_share_enabled = true;
            }
        }
        coordinator.load_config.load_balance_enabled = true;
        break;

    case COORDINATION_MODE_MASTER_SLAVE: {
        // Set first enabled motor as master, others as slaves
        bool master_set = false;
        for (uint8_t i = 0; i < MAX_MOTORS; i++) {
            if (coordinator.motor_states[i].enabled) {
                if (!master_set) {
                    coordinator.sync_config.sync_master = i;
                    coordinator.motor_states[i].synchronized =
                        false; // Master not synchronized to itself
                    master_set = true;
                } else {
                    coordinator.motor_states[i].synchronized = true;
                }
            }
        }
        break;
    }
    }

    return SYSTEM_OK;
}

/**
 * @brief Execute coordinated move command
 * @param move_cmd Coordinated move command
 * @return SystemError_t Operation result
 */
SystemError_t
multi_motor_coordinated_move(CoordinatedMoveCommand_t *move_cmd) {
    if (!coordinator_initialized || move_cmd == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (coordinator.motion_state.active) {
        return ERROR_MOTION_ACTIVE;
    }

    // Validate move command
    SystemError_t result = validate_coordinated_move(move_cmd);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Store move command
    memcpy(&coordinator.current_move, move_cmd,
           sizeof(CoordinatedMoveCommand_t));

    // Execute based on coordination mode
    switch (coordinator.motion_state.mode) {
    case COORDINATION_MODE_SYNCHRONIZED:
        result = execute_synchronized_move(move_cmd);
        break;

    case COORDINATION_MODE_LOAD_SHARING:
        result = execute_load_sharing_move(move_cmd);
        break;

    case COORDINATION_MODE_MASTER_SLAVE:
        result = execute_master_slave_move(move_cmd);
        break;

    case COORDINATION_MODE_INDEPENDENT:
    default:
        result = execute_independent_move(move_cmd);
        break;
    }

    if (result == SYSTEM_OK) {
        coordinator.motion_state.active = true;
        coordinator.motion_state.start_time = HAL_Abstraction_GetTick();
    }

    return result;
}

/**
 * @brief Update coordination system (call from main control loop)
 * @param dt_ms Time step in milliseconds
 * @return SystemError_t Operation result
 */
SystemError_t multi_motor_update(uint32_t dt_ms) {
    if (!coordinator_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Update motor states
    update_motor_states();

    // Handle coordination based on current mode
    switch (coordinator.motion_state.mode) {
    case COORDINATION_MODE_SYNCHRONIZED:
        update_synchronized_motion(dt_ms);
        break;

    case COORDINATION_MODE_LOAD_SHARING:
        update_load_sharing(dt_ms);
        break;

    case COORDINATION_MODE_MASTER_SLAVE:
        update_master_slave_motion(dt_ms);
        break;

    case COORDINATION_MODE_INDEPENDENT:
    default:
        // No special coordination needed
        break;
    }

    // Check for motion completion
    if (coordinator.motion_state.active) {
        check_motion_completion();
    }

    return SYSTEM_OK;
}

/**
 * @brief Stop all coordinated motion
 * @return SystemError_t Operation result
 */
SystemError_t multi_motor_stop_coordinated_motion(void) {
    if (!coordinator_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Stop all enabled motors
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (coordinator.motor_states[i].enabled) {
            motion_profile_stop(i);
        }
    }

    coordinator.motion_state.active = false;
    coordinator.motion_state.sequence_step = 0;

    return SYSTEM_OK;
}

/**
 * @brief Get coordination system status
 * @param status Pointer to store status
 * @return SystemError_t Operation result
 */
SystemError_t multi_motor_get_status(CoordinationStatus_t *status) {
    if (!coordinator_initialized || status == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    status->mode = coordinator.motion_state.mode;
    status->active = coordinator.motion_state.active;
    status->sync_master = coordinator.sync_config.sync_master;
    status->load_balance_enabled =
        coordinator.load_config.load_balance_enabled;

    // Copy motor states
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        status->motor_enabled[i] = coordinator.motor_states[i].enabled;
        status->motor_synchronized[i] =
            coordinator.motor_states[i].synchronized;
        status->motor_load_share[i] =
            coordinator.motor_states[i].load_share_enabled;

        // Get position control status
        PositionControlStatus_t pos_status;
        if (position_control_get_status(i, &pos_status) == SYSTEM_OK) {
            status->motor_positions[i] = pos_status.current_position;
            status->motor_settled[i] = pos_status.position_settled;
        }
    }

    // Calculate synchronization error
    calculate_sync_error(status);

    return SYSTEM_OK;
}

/**
 * @brief Configure synchronization parameters
 * @param config Synchronization configuration
 * @return SystemError_t Operation result
 */
SystemError_t multi_motor_configure_sync(SynchronizationConfig_t *config) {
    if (!coordinator_initialized || config == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (config->sync_master >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }

    memcpy(&coordinator.sync_config, config, sizeof(SynchronizationConfig_t));

    return SYSTEM_OK;
}

/**
 * @brief Configure load sharing parameters
 * @param config Load sharing configuration
 * @return SystemError_t Operation result
 */
SystemError_t multi_motor_configure_load_sharing(LoadSharingConfig_t *config) {
    if (!coordinator_initialized || config == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    memcpy(&coordinator.load_config, config, sizeof(LoadSharingConfig_t));

    return SYSTEM_OK;
}

/**
 * @brief Validate coordinated move command
 */
static SystemError_t
validate_coordinated_move(CoordinatedMoveCommand_t *move_cmd) {
    // Check if any motor is enabled for the move
    bool any_motor_enabled = false;
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (move_cmd->motor_targets[i].enabled) {
            any_motor_enabled = true;

            // Validate individual motor parameters
            if (abs(move_cmd->motor_targets[i].target_position) >
                MAX_POSITION_STEPS) {
                return ERROR_POSITION_OUT_OF_RANGE;
            }

            if (move_cmd->motor_targets[i].max_velocity > MOTOR_MAX_SPEED) {
                return ERROR_VELOCITY_OUT_OF_RANGE;
            }
        }
    }

    if (!any_motor_enabled) {
        return ERROR_NO_MOTORS_ENABLED;
    }

    return SYSTEM_OK;
}

/**
 * @brief Execute synchronized move
 */
static SystemError_t
execute_synchronized_move(CoordinatedMoveCommand_t *move_cmd) {
    // Calculate move time based on slowest motor
    uint32_t max_move_time = 0;

    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (move_cmd->motor_targets[i].enabled &&
            coordinator.motor_states[i].enabled) {
            // Get current position
            PositionControlStatus_t pos_status;
            position_control_get_status(i, &pos_status);

            // Calculate distance and time needed
            int32_t distance = abs(move_cmd->motor_targets[i].target_position -
                                   pos_status.current_position);
            uint32_t move_time = calculate_move_time(
                distance, move_cmd->motor_targets[i].max_velocity,
                move_cmd->motor_targets[i].acceleration);

            if (move_time > max_move_time) {
                max_move_time = move_time;
            }
        }
    }

    // Create synchronized motion profiles
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (move_cmd->motor_targets[i].enabled &&
            coordinator.motor_states[i].enabled) {
            MotionProfile_t profile_config = {
                .type = PROFILE_TRAPEZOIDAL,
                .start_position = coordinator.motor_states[i].current_position,
                .end_position = move_cmd->motor_targets[i].target_position,
                .max_velocity = move_cmd->motor_targets[i].max_velocity,
                .acceleration = move_cmd->motor_targets[i].acceleration,
                .direction = (move_cmd->motor_targets[i].target_position >
                              coordinator.motor_states[i].current_position)
                                 ? DIRECTION_FORWARD
                                 : DIRECTION_BACKWARD};

            SystemError_t result = motion_profile_start(i, &profile_config);
            if (result != SYSTEM_OK) {
                // Stop all motors on failure
                multi_motor_stop_coordinated_motion();
                return result;
            }
        }
    }

    return SYSTEM_OK;
}

/**
 * @brief Execute load sharing move
 */
static SystemError_t
execute_load_sharing_move(CoordinatedMoveCommand_t *move_cmd) {
    // Calculate total load and distribute among enabled motors
    int32_t total_distance = 0;
    uint8_t active_motors = 0;

    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (move_cmd->motor_targets[i].enabled &&
            coordinator.motor_states[i].enabled) {
            PositionControlStatus_t pos_status;
            position_control_get_status(i, &pos_status);

            total_distance += abs(move_cmd->motor_targets[i].target_position -
                                  pos_status.current_position);
            active_motors++;
        }
    }

    if (active_motors == 0) {
        return ERROR_NO_MOTORS_ENABLED;
    }

    // Distribute load evenly among motors
    int32_t load_per_motor = total_distance / active_motors;

    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (move_cmd->motor_targets[i].enabled &&
            coordinator.motor_states[i].enabled) {
            MotionProfile_t profile_config;

            // Adjust target based on load sharing
            PositionControlStatus_t pos_status;
            position_control_get_status(i, &pos_status);

            // Simple load sharing: each motor moves its share
            int32_t target_pos =
                pos_status.current_position +
                (load_per_motor *
                 ((move_cmd->motor_targets[i].target_position >
                   pos_status.current_position)
                      ? 1
                      : -1));

            profile_config.type = PROFILE_TRAPEZOIDAL;
            profile_config.start_position = pos_status.current_position;
            profile_config.end_position = target_pos;
            profile_config.max_velocity =
                move_cmd->motor_targets[i].max_velocity;
            profile_config.acceleration =
                move_cmd->motor_targets[i].acceleration;
            profile_config.direction =
                (target_pos > pos_status.current_position)
                    ? DIRECTION_FORWARD
                    : DIRECTION_BACKWARD;

            SystemError_t result = motion_profile_start(i, &profile_config);
            if (result != SYSTEM_OK) {
                multi_motor_stop_coordinated_motion();
                return result;
            }
        }
    }

    return SYSTEM_OK;
}

/**
 * @brief Execute master-slave move
 */
static SystemError_t
execute_master_slave_move(CoordinatedMoveCommand_t *move_cmd) {
    uint8_t master_id = coordinator.sync_config.sync_master;

    // Start master motor first
    if (move_cmd->motor_targets[master_id].enabled &&
        coordinator.motor_states[master_id].enabled) {
        MotionProfile_t master_config = {
            .type = PROFILE_TRAPEZOIDAL,
            .start_position =
                coordinator.motor_states[master_id].current_position,
            .end_position = move_cmd->motor_targets[master_id].target_position,
            .max_velocity = move_cmd->motor_targets[master_id].max_velocity,
            .acceleration = move_cmd->motor_targets[master_id].acceleration,
            .direction = (move_cmd->motor_targets[master_id].target_position >
                          coordinator.motor_states[master_id].current_position)
                             ? DIRECTION_FORWARD
                             : DIRECTION_BACKWARD};

        SystemError_t result = motion_profile_start(master_id, &master_config);
        if (result != SYSTEM_OK) {
            return result;
        }
    }

    // Configure slave motors to follow master
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (i != master_id && move_cmd->motor_targets[i].enabled &&
            coordinator.motor_states[i].enabled) {
            // Slave motors will be updated in update_master_slave_motion()
            coordinator.motor_states[i].slave_offset =
                move_cmd->motor_targets[i].target_position -
                move_cmd->motor_targets[master_id].target_position;
        }
    }

    return SYSTEM_OK;
}

/**
 * @brief Execute independent move
 */
static SystemError_t
execute_independent_move(CoordinatedMoveCommand_t *move_cmd) {
    // Start each motor independently
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (move_cmd->motor_targets[i].enabled &&
            coordinator.motor_states[i].enabled) {
            MotionProfile_t profile_config = {
                .type = PROFILE_TRAPEZOIDAL,
                .start_position = coordinator.motor_states[i].current_position,
                .end_position = move_cmd->motor_targets[i].target_position,
                .max_velocity = move_cmd->motor_targets[i].max_velocity,
                .acceleration = move_cmd->motor_targets[i].acceleration,
                .direction = (move_cmd->motor_targets[i].target_position >
                              coordinator.motor_states[i].current_position)
                                 ? DIRECTION_FORWARD
                                 : DIRECTION_BACKWARD};

            SystemError_t result = motion_profile_start(i, &profile_config);
            if (result != SYSTEM_OK) {
                // Continue with other motors even if one fails
                fault_monitor_report_fault(i, FAULT_MOTION_PROFILE_FAILED);
            }
        }
    }

    return SYSTEM_OK;
}

/**
 * @brief Update motor states
 */
static void update_motor_states(void) {
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (coordinator.motor_states[i].enabled) {
            // Update position and status
            PositionControlStatus_t pos_status;
            if (position_control_get_status(i, &pos_status) == SYSTEM_OK) {
                coordinator.motor_states[i].current_position =
                    pos_status.current_position;
                coordinator.motor_states[i].settled =
                    pos_status.position_settled;
            }

            // Update motion profile status
            coordinator.motor_states[i].profile_active =
                motion_profile_is_active(i);
        }
    }
}

/**
 * @brief Update synchronized motion
 */
static void update_synchronized_motion(uint32_t dt_ms) {
    // Check synchronization between motors
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (coordinator.motor_states[i].synchronized &&
            coordinator.motor_states[i].enabled) {
            // Calculate synchronization error relative to master
            uint8_t master_id = coordinator.sync_config.sync_master;
            int32_t sync_error =
                coordinator.motor_states[i].current_position -
                coordinator.motor_states[master_id].current_position;

            if (abs(sync_error) > coordinator.sync_config.sync_tolerance) {
                // Report synchronization fault
                fault_monitor_report_fault(i, FAULT_SYNCHRONIZATION_ERROR);
            }
        }
    }
}

/**
 * @brief Update load sharing
 */
static void update_load_sharing(uint32_t dt_ms) {
    if (!coordinator.load_config.load_balance_enabled) {
        return;
    }

    // Calculate average position of all enabled motors
    int32_t total_position = 0;
    uint8_t active_motors = 0;

    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (coordinator.motor_states[i].load_share_enabled &&
            coordinator.motor_states[i].enabled) {
            total_position += coordinator.motor_states[i].current_position;
            active_motors++;
        }
    }

    if (active_motors < 2) {
        return; // Need at least 2 motors for load sharing
    }

    int32_t average_position = total_position / active_motors;

    // Adjust motors that are too far from average
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (coordinator.motor_states[i].load_share_enabled &&
            coordinator.motor_states[i].enabled) {
            int32_t position_diff =
                coordinator.motor_states[i].current_position -
                average_position;

            if (abs(position_diff) >
                coordinator.load_config.max_load_difference) {
                // Adjust this motor's target to balance load
                int32_t adjustment =
                    position_diff / 2; // Move halfway back to average
                int32_t new_target =
                    coordinator.motor_states[i].current_position - adjustment;

                position_control_set_target(i, new_target);
            }
        }
    }
}

/**
 * @brief Update master-slave motion
 */
static void update_master_slave_motion(uint32_t dt_ms) {
    uint8_t master_id = coordinator.sync_config.sync_master;

    if (!coordinator.motor_states[master_id].enabled) {
        return;
    }

    int32_t master_position =
        coordinator.motor_states[master_id].current_position;

    // Update slave motors to follow master
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (i != master_id && coordinator.motor_states[i].synchronized &&
            coordinator.motor_states[i].enabled) {
            int32_t slave_target =
                master_position + coordinator.motor_states[i].slave_offset;
            position_control_set_target(i, slave_target);
        }
    }
}

/**
 * @brief Check for motion completion
 */
static void check_motion_completion(void) {
    bool all_settled = true;

    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (coordinator.motor_states[i].enabled &&
            coordinator.current_move.motor_targets[i].enabled) {
            if (!coordinator.motor_states[i].settled ||
                coordinator.motor_states[i].profile_active) {
                all_settled = false;
                break;
            }
        }
    }

    if (all_settled) {
        coordinator.motion_state.active = false;
        coordinator.motion_state.sequence_step = 0;
    }
}

/**
 * @brief Calculate synchronization error
 */
static void calculate_sync_error(CoordinationStatus_t *status) {
    if (coordinator.motion_state.mode != COORDINATION_MODE_SYNCHRONIZED) {
        status->max_sync_error = 0;
        return;
    }

    int32_t min_pos = INT32_MAX;
    int32_t max_pos = INT32_MIN;

    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        if (coordinator.motor_states[i].enabled) {
            if (status->motor_positions[i] < min_pos) {
                min_pos = status->motor_positions[i];
            }
            if (status->motor_positions[i] > max_pos) {
                max_pos = status->motor_positions[i];
            }
        }
    }

    status->max_sync_error = max_pos - min_pos;
}

/**
 * @brief Calculate move time for given parameters
 */
static uint32_t calculate_move_time(int32_t distance, uint32_t max_velocity,
                                    uint32_t acceleration) {
    // Simple trapezoidal profile time calculation
    float accel_time = (float)max_velocity / (float)acceleration;
    float accel_distance =
        0.5f * (float)acceleration * accel_time * accel_time;

    if (2.0f * accel_distance >= (float)distance) {
        // Triangular profile
        return (uint32_t)(2.0f * sqrtf((float)distance / (float)acceleration) *
                          1000.0f);
    } else {
        // Trapezoidal profile
        float constant_distance = (float)distance - 2.0f * accel_distance;
        float constant_time = constant_distance / (float)max_velocity;
        return (uint32_t)((2.0f * accel_time + constant_time) * 1000.0f);
    }
}
