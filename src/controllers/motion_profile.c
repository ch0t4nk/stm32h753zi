/**
 * @file motion_profile.c
 * @brief Advanced Motion Profiling for STM32H753ZI Motor Control
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @details Advanced motion profiling algorithms including trapezoidal,
 *          S-curve, and multi-axis synchronized motion control
 */

#include "motion_profile.h"
#include "common/error_codes.h"
#include "config/motor_config.h"
#include "config/safety_config.h"
#include "stm32h7xx_hal.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Motion profile storage
static MotionProfile_t active_profiles[MAX_MOTORS];
static bool profile_active[MAX_MOTORS] = {false};

/**
 * @brief Initialize motion profiling system
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_init(void) {
    // Clear all motion profiles
    memset(active_profiles, 0, sizeof(active_profiles));
    memset(profile_active, false, sizeof(profile_active));

    return SYSTEM_OK;
}

/**
 * @brief Generate trapezoidal motion profile
 * @param profile Pointer to motion profile structure
 * @param start_pos Starting position (steps)
 * @param end_pos Target position (steps)
 * @param max_vel Maximum velocity (steps/sec)
 * @param acceleration Acceleration (steps/sec²)
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_generate_trapezoidal(MotionProfile_t *profile,
                                                  int32_t start_pos,
                                                  int32_t end_pos,
                                                  uint32_t max_vel,
                                                  uint32_t acceleration) {
    if (profile == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Validate parameters
    if (max_vel > MOTOR_MAX_SPEED || acceleration > MOTOR_MAX_ACCELERATION) {
        return ERROR_MOTOR_PARAMETER_OUT_OF_RANGE;
    }

    // Initialize profile
    profile->type = PROFILE_TRAPEZOIDAL;
    profile->start_position = start_pos;
    profile->end_position = end_pos;
    profile->max_velocity = max_vel;
    profile->acceleration = acceleration;
    profile->deceleration = acceleration; // Use same for deceleration
    profile->current_phase = PROFILE_PHASE_ACCEL;
    profile->start_time_ms = 0;

    // Calculate distances and times
    int32_t total_distance = abs(end_pos - start_pos);
    profile->direction = (end_pos > start_pos) ? 1 : -1;

    // Time to reach maximum velocity
    float accel_time = (float)max_vel / acceleration;

    // Distance covered during acceleration
    int32_t accel_distance = (acceleration * accel_time * accel_time) / 2;

    // Check if we can reach maximum velocity
    if (2 * accel_distance <= total_distance) {
        // Trapezoidal profile - we reach max velocity
        profile->accel_time_ms = (uint32_t)(accel_time * 1000);
        profile->accel_distance = accel_distance;
        profile->decel_distance = accel_distance; // Symmetric
        profile->const_vel_distance = total_distance - 2 * accel_distance;
        profile->const_vel_time_ms =
            (uint32_t)((float)profile->const_vel_distance / max_vel * 1000);
        profile->decel_time_ms = profile->accel_time_ms;
        profile->total_time_ms = profile->accel_time_ms +
                                 profile->const_vel_time_ms +
                                 profile->decel_time_ms;
        profile->peak_velocity = max_vel;
    } else {
        // Triangular profile - never reach max velocity
        float triangle_time = sqrtf((float)total_distance / acceleration);
        profile->peak_velocity = (uint32_t)(acceleration * triangle_time);
        profile->accel_time_ms = (uint32_t)(triangle_time * 1000);
        profile->accel_distance = total_distance / 2;
        profile->decel_distance = total_distance / 2;
        profile->const_vel_distance = 0;
        profile->const_vel_time_ms = 0;
        profile->decel_time_ms = profile->accel_time_ms;
        profile->total_time_ms = 2 * profile->accel_time_ms;
    }

    return SYSTEM_OK;
}

/**
 * @brief Generate S-curve motion profile for smooth acceleration
 * @param profile Pointer to motion profile structure
 * @param start_pos Starting position (steps)
 * @param end_pos Target position (steps)
 * @param max_vel Maximum velocity (steps/sec)
 * @param max_accel Maximum acceleration (steps/sec²)
 * @param jerk_time Jerk time constant (ms)
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_generate_scurve(MotionProfile_t *profile,
                                             int32_t start_pos,
                                             int32_t end_pos, uint32_t max_vel,
                                             uint32_t max_accel,
                                             uint32_t jerk_time) {
    if (profile == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Validate parameters
    if (max_vel > MOTOR_MAX_SPEED || max_accel > MOTOR_MAX_ACCELERATION) {
        return ERROR_MOTOR_PARAMETER_OUT_OF_RANGE;
    }

    // Initialize S-curve profile
    profile->type = PROFILE_SCURVE;
    profile->start_position = start_pos;
    profile->end_position = end_pos;
    profile->max_velocity = max_vel;
    profile->acceleration = max_accel;
    profile->jerk_time_ms = jerk_time;
    profile->current_phase = PROFILE_PHASE_JERK_ACCEL;

    int32_t total_distance = abs(end_pos - start_pos);
    profile->direction = (end_pos > start_pos) ? 1 : -1;

    // Calculate jerk value
    float jerk_time_sec = (float)jerk_time / 1000.0f;
    profile->jerk = max_accel / jerk_time_sec;

    // Calculate phase times (simplified S-curve)
    profile->jerk_accel_time_ms = jerk_time;
    profile->linear_accel_time_ms =
        (uint32_t)((float)max_vel / max_accel * 1000) - jerk_time;
    profile->jerk_decel_accel_time_ms = jerk_time;

    // Mirror for deceleration
    profile->jerk_accel_decel_time_ms = jerk_time;
    profile->linear_decel_time_ms = profile->linear_accel_time_ms;
    profile->jerk_decel_decel_time_ms = jerk_time;

    // Calculate total time and distances (simplified calculation)
    profile->total_time_ms =
        2 * (profile->jerk_accel_time_ms + profile->linear_accel_time_ms +
             profile->jerk_decel_accel_time_ms);

    return SYSTEM_OK;
}

/**
 * @brief Execute motion profile at given time
 * @param motor_id Motor identifier
 * @param profile Pointer to motion profile
 * @param elapsed_time_ms Time since profile start
 * @param target_pos Pointer to store target position
 * @param target_vel Pointer to store target velocity
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_execute(uint8_t motor_id,
                                     MotionProfile_t *profile,
                                     uint32_t elapsed_time_ms,
                                     int32_t *target_pos,
                                     uint32_t *target_vel) {
    if (motor_id >= MAX_MOTORS || profile == NULL || target_pos == NULL ||
        target_vel == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Check if profile is complete
    if (elapsed_time_ms >= profile->total_time_ms) {
        *target_pos = profile->end_position;
        *target_vel = 0;
        profile->current_phase = PROFILE_PHASE_COMPLETE;
        return SYSTEM_OK;
    }

    switch (profile->type) {
    case PROFILE_TRAPEZOIDAL:
        return execute_trapezoidal_profile(profile, elapsed_time_ms,
                                           target_pos, target_vel);

    case PROFILE_SCURVE:
        return execute_scurve_profile(profile, elapsed_time_ms, target_pos,
                                      target_vel);

    default:
        return ERROR_INVALID_PARAMETER;
    }
}

/**
 * @brief Execute trapezoidal profile calculation
 */
static SystemError_t execute_trapezoidal_profile(MotionProfile_t *profile,
                                                 uint32_t elapsed_time_ms,
                                                 int32_t *target_pos,
                                                 uint32_t *target_vel) {
    float elapsed_sec = (float)elapsed_time_ms / 1000.0f;
    float accel_time_sec = (float)profile->accel_time_ms / 1000.0f;
    float const_vel_time_sec = (float)profile->const_vel_time_ms / 1000.0f;

    if (elapsed_time_ms <= profile->accel_time_ms) {
        // Acceleration phase
        profile->current_phase = PROFILE_PHASE_ACCEL;
        *target_vel = (uint32_t)(profile->acceleration * elapsed_sec);
        float distance =
            0.5f * profile->acceleration * elapsed_sec * elapsed_sec;
        *target_pos =
            profile->start_position + (int32_t)(distance * profile->direction);

    } else if (elapsed_time_ms <=
               (profile->accel_time_ms + profile->const_vel_time_ms)) {
        // Constant velocity phase
        profile->current_phase = PROFILE_PHASE_CONST_VEL;
        *target_vel = profile->peak_velocity;
        float const_vel_elapsed = elapsed_sec - accel_time_sec;
        float distance = profile->accel_distance +
                         (profile->peak_velocity * const_vel_elapsed);
        *target_pos =
            profile->start_position + (int32_t)(distance * profile->direction);

    } else {
        // Deceleration phase
        profile->current_phase = PROFILE_PHASE_DECEL;
        float decel_elapsed =
            elapsed_sec - accel_time_sec - const_vel_time_sec;
        *target_vel = (uint32_t)(profile->peak_velocity -
                                 (profile->deceleration * decel_elapsed));

        if (*target_vel > profile->peak_velocity) {
            *target_vel = 0; // Handle underflow
        }

        float decel_distance =
            profile->peak_velocity * decel_elapsed -
            0.5f * profile->deceleration * decel_elapsed * decel_elapsed;
        float total_distance = profile->accel_distance +
                               profile->const_vel_distance + decel_distance;
        *target_pos = profile->start_position +
                      (int32_t)(total_distance * profile->direction);
    }

    return SYSTEM_OK;
}

/**
 * @brief Execute S-curve profile calculation
 */
static SystemError_t execute_scurve_profile(MotionProfile_t *profile,
                                            uint32_t elapsed_time_ms,
                                            int32_t *target_pos,
                                            uint32_t *target_vel) {
    // Simplified S-curve implementation
    // For full implementation, would need to handle all 7 phases of S-curve

    float elapsed_sec = (float)elapsed_time_ms / 1000.0f;
    float jerk_time_sec = (float)profile->jerk_time_ms / 1000.0f;

    if (elapsed_time_ms <= profile->jerk_accel_time_ms) {
        // Jerk acceleration phase
        profile->current_phase = PROFILE_PHASE_JERK_ACCEL;
        float jerk_accel = 0.5f * profile->jerk * elapsed_sec * elapsed_sec;
        *target_vel = (uint32_t)(jerk_accel);
        float distance = (1.0f / 6.0f) * profile->jerk * elapsed_sec *
                         elapsed_sec * elapsed_sec;
        *target_pos =
            profile->start_position + (int32_t)(distance * profile->direction);

    } else {
        // For simplicity, fall back to linear acceleration after jerk phase
        // Full S-curve would have 7 distinct phases
        float remaining_accel_time = elapsed_sec - jerk_time_sec;
        float jerk_velocity =
            0.5f * profile->jerk * jerk_time_sec * jerk_time_sec;
        *target_vel = (uint32_t)(jerk_velocity +
                                 profile->acceleration * remaining_accel_time);

        // Simplified position calculation
        float jerk_distance = (1.0f / 6.0f) * profile->jerk * jerk_time_sec *
                              jerk_time_sec * jerk_time_sec;
        float linear_distance = jerk_velocity * remaining_accel_time +
                                0.5f * profile->acceleration *
                                    remaining_accel_time *
                                    remaining_accel_time;
        *target_pos =
            profile->start_position +
            (int32_t)((jerk_distance + linear_distance) * profile->direction);
    }

    return SYSTEM_OK;
}

/**
 * @brief Start motion profile for specified motor
 * @param motor_id Motor identifier
 * @param profile Pointer to motion profile
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_start(uint8_t motor_id,
                                   MotionProfile_t *profile) {
    if (motor_id >= MAX_MOTORS || profile == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Copy profile to active storage
    memcpy(&active_profiles[motor_id], profile, sizeof(MotionProfile_t));
    profile_active[motor_id] = true;

    // Record start time
    active_profiles[motor_id].start_time_ms = HAL_GetTick();

    return SYSTEM_OK;
}

/**
 * @brief Stop motion profile for specified motor
 * @param motor_id Motor identifier
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_stop(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }

    profile_active[motor_id] = false;
    return SYSTEM_OK;
}

/**
 * @brief Check if motion profile is active for motor
 * @param motor_id Motor identifier
 * @return bool True if profile is active
 */
bool motion_profile_is_active(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS) {
        return false;
    }

    return profile_active[motor_id];
}

/**
 * @brief Get current motion profile status
 * @param motor_id Motor identifier
 * @param status Pointer to store profile status
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_get_status(uint8_t motor_id,
                                        MotionProfileStatus_t *status) {
    if (motor_id >= MAX_MOTORS || status == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    if (!profile_active[motor_id]) {
        status->is_active = false;
        return SYSTEM_OK;
    }

    MotionProfile_t *profile = &active_profiles[motor_id];
    uint32_t current_time = HAL_GetTick();
    uint32_t elapsed_time = current_time - profile->start_time_ms;

    status->is_active = true;
    status->current_phase = profile->current_phase;
    status->elapsed_time_ms = elapsed_time;
    status->total_time_ms = profile->total_time_ms;
    status->progress_percent = (elapsed_time * 100) / profile->total_time_ms;

    if (status->progress_percent > 100) {
        status->progress_percent = 100;
    }

    // Calculate current targets
    motion_profile_execute(motor_id, profile, elapsed_time,
                           &status->current_target_position,
                           &status->current_target_velocity);

    return SYSTEM_OK;
}

/**
 * @brief Synchronize multiple motor profiles for coordinated motion
 * @param motor_ids Array of motor identifiers
 * @param profiles Array of motion profiles
 * @param motor_count Number of motors to synchronize
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_synchronize(uint8_t *motor_ids,
                                         MotionProfile_t *profiles,
                                         uint8_t motor_count) {
    if (motor_ids == NULL || profiles == NULL || motor_count == 0 ||
        motor_count > MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }

    // Find the longest profile time
    uint32_t max_time = 0;
    for (uint8_t i = 0; i < motor_count; i++) {
        if (profiles[i].total_time_ms > max_time) {
            max_time = profiles[i].total_time_ms;
        }
    }

    // Scale all profiles to match the longest time
    for (uint8_t i = 0; i < motor_count; i++) {
        if (profiles[i].total_time_ms < max_time) {
            // Reduce velocity to extend time
            float scale_factor = (float)profiles[i].total_time_ms / max_time;
            profiles[i].max_velocity =
                (uint32_t)(profiles[i].max_velocity * scale_factor);
            profiles[i].peak_velocity =
                (uint32_t)(profiles[i].peak_velocity * scale_factor);

            // Recalculate timing with new velocity
            motion_profile_generate_trapezoidal(
                &profiles[i], profiles[i].start_position,
                profiles[i].end_position, profiles[i].max_velocity,
                profiles[i].acceleration);
        }
    }

    // Start all profiles simultaneously
    uint32_t sync_start_time = HAL_GetTick();
    for (uint8_t i = 0; i < motor_count; i++) {
        profiles[i].start_time_ms = sync_start_time;
        motion_profile_start(motor_ids[i], &profiles[i]);
    }

    return SYSTEM_OK;
}
