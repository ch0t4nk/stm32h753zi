/**
 * @file motion_profile.h
 * @brief Advanced Motion Profiling Header for STM32H753ZI Motor Control
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @details Defines motion profiling algorithms for smooth motor movement
 */

#ifndef MOTION_PROFILE_H
#define MOTION_PROFILE_H

#include "common/error_codes.h"
#include "config/motor_config.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Motion profile types
 */
typedef enum {
  PROFILE_TRAPEZOIDAL = 0, ///< Standard trapezoidal velocity profile
  PROFILE_SCURVE,          ///< S-curve profile for smooth acceleration
  PROFILE_POINT_TO_POINT,  ///< Simple point-to-point movement
  PROFILE_CUSTOM           ///< Custom user-defined profile
} MotionProfileType_t;

/**
 * @brief Motion profile phases
 */
typedef enum {
  PROFILE_PHASE_IDLE = 0,         ///< Profile not active
  PROFILE_PHASE_ACCEL,            ///< Acceleration phase
  PROFILE_PHASE_CONST_VEL,        ///< Constant velocity phase
  PROFILE_PHASE_DECEL,            ///< Deceleration phase
  PROFILE_PHASE_COMPLETE,         ///< Profile completed
  PROFILE_PHASE_JERK_ACCEL,       ///< S-curve: Jerk acceleration
  PROFILE_PHASE_LINEAR_ACCEL,     ///< S-curve: Linear acceleration
  PROFILE_PHASE_JERK_DECEL_ACCEL, ///< S-curve: Jerk deceleration (accel)
  PROFILE_PHASE_CONST_VEL_SCURVE, ///< S-curve: Constant velocity
  PROFILE_PHASE_JERK_ACCEL_DECEL, ///< S-curve: Jerk acceleration (decel)
  PROFILE_PHASE_LINEAR_DECEL,     ///< S-curve: Linear deceleration
  PROFILE_PHASE_JERK_DECEL        ///< S-curve: Jerk deceleration
} MotionProfilePhase_t;

/**
 * @brief Motion direction constants
 */
#define DIRECTION_FORWARD 1   ///< Forward direction
#define DIRECTION_BACKWARD -1 ///< Backward direction

/**
 * @brief Motion profile structure
 */
typedef struct {
  MotionProfileType_t type;           ///< Profile type
  MotionProfilePhase_t current_phase; ///< Current execution phase

  // Position parameters
  int32_t start_position; ///< Starting position (steps)
  int32_t end_position;   ///< Target position (steps)
  int32_t direction;      ///< Movement direction (+1 or -1)

  // Velocity parameters
  uint32_t max_velocity;  ///< Maximum velocity (steps/sec)
  uint32_t peak_velocity; ///< Actual peak velocity achieved
  uint32_t acceleration;  ///< Acceleration (steps/sec²)
  uint32_t deceleration;  ///< Deceleration (steps/sec²)

  // S-curve specific parameters
  uint32_t jerk;         ///< Jerk value for S-curve (steps/sec³)
  uint32_t jerk_time_ms; ///< Jerk time constant (ms)

  // Timing parameters
  uint32_t start_time_ms;     ///< Profile start time
  uint32_t total_time_ms;     ///< Total profile execution time
  uint32_t accel_time_ms;     ///< Acceleration phase time
  uint32_t const_vel_time_ms; ///< Constant velocity phase time
  uint32_t decel_time_ms;     ///< Deceleration phase time

  // S-curve timing (all phases)
  uint32_t jerk_accel_time_ms;       ///< Jerk acceleration time
  uint32_t linear_accel_time_ms;     ///< Linear acceleration time
  uint32_t jerk_decel_accel_time_ms; ///< Jerk deceleration (accel) time
  uint32_t jerk_accel_decel_time_ms; ///< Jerk acceleration (decel) time
  uint32_t linear_decel_time_ms;     ///< Linear deceleration time
  uint32_t jerk_decel_decel_time_ms; ///< Jerk deceleration time

  // Distance parameters
  int32_t accel_distance;     ///< Distance during acceleration
  int32_t const_vel_distance; ///< Distance during constant velocity
  int32_t decel_distance;     ///< Distance during deceleration
} MotionProfile_t;

/**
 * @brief Motion profile status
 */
typedef struct {
  bool is_active;                     ///< True if profile is executing
  MotionProfilePhase_t current_phase; ///< Current execution phase
  uint32_t elapsed_time_ms;           ///< Time since profile start
  uint32_t total_time_ms;             ///< Total profile time
  uint8_t progress_percent;           ///< Progress percentage (0-100)
  int32_t current_target_position;    ///< Current target position
  uint32_t current_target_velocity;   ///< Current target velocity
} MotionProfileStatus_t;

/**
 * @brief Motion profile configuration
 */
typedef struct {
  uint32_t max_acceleration;       ///< Maximum allowed acceleration
  uint32_t max_jerk;               ///< Maximum allowed jerk
  uint32_t min_velocity;           ///< Minimum velocity for movement
  uint32_t profile_update_rate_ms; ///< Profile update frequency
} MotionProfileConfig_t;

// Function prototypes

/**
 * @brief Initialize motion profiling system
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_init(void);

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
                                                  uint32_t acceleration);

/**
 * @brief Generate S-curve motion profile
 * @param profile Pointer to motion profile structure
 * @param start_pos Starting position (steps)
 * @param end_pos Target position (steps)
 * @param max_vel Maximum velocity (steps/sec)
 * @param max_accel Maximum acceleration (steps/sec²)
 * @param jerk_time Jerk time constant (ms)
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_generate_scurve(MotionProfile_t *profile,
                                             int32_t start_pos, int32_t end_pos,
                                             uint32_t max_vel,
                                             uint32_t max_accel,
                                             uint32_t jerk_time);

/**
 * @brief Execute motion profile at given time
 * @param motor_id Motor identifier
 * @param profile Pointer to motion profile
 * @param elapsed_time_ms Time since profile start
 * @param target_pos Pointer to store target position
 * @param target_vel Pointer to store target velocity
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_execute(uint8_t motor_id, MotionProfile_t *profile,
                                     uint32_t elapsed_time_ms,
                                     int32_t *target_pos, uint32_t *target_vel);

/**
 * @brief Start motion profile for specified motor
 * @param motor_id Motor identifier
 * @param profile Pointer to motion profile
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_start(uint8_t motor_id, MotionProfile_t *profile);

/**
 * @brief Stop motion profile for specified motor
 * @param motor_id Motor identifier
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_stop(uint8_t motor_id);

/**
 * @brief Check if motion profile is active for motor
 * @param motor_id Motor identifier
 * @return bool True if profile is active
 */
bool motion_profile_is_active(uint8_t motor_id);

/**
 * @brief Get current motion profile status
 * @param motor_id Motor identifier
 * @param status Pointer to store profile status
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_get_status(uint8_t motor_id,
                                        MotionProfileStatus_t *status);

/**
 * @brief Synchronize multiple motor profiles for coordinated motion
 * @param motor_ids Array of motor identifiers
 * @param profiles Array of motion profiles
 * @param motor_count Number of motors to synchronize
 * @return SystemError_t Operation result
 */
SystemError_t motion_profile_synchronize(uint8_t *motor_ids,
                                         MotionProfile_t *profiles,
                                         uint8_t motor_count);

// Internal functions are implemented in motion_profile.c
// (No public declarations needed for static internal functions)

// Motion profile constants
#define MOTION_PROFILE_UPDATE_RATE_MS 1      ///< Default update rate (1ms)
#define MOTION_PROFILE_MIN_TIME_MS 10        ///< Minimum profile time
#define MOTION_PROFILE_MAX_JERK_TIME_MS 1000 ///< Maximum jerk time

#ifdef __cplusplus
}
#endif

#endif // MOTION_PROFILE_H
