/**
 * @file position_safety.h
 * @brief Position Safety Enforcement for Motor Control
 * @author STM32H753ZI Project Team
 * @date 2025-08-12
 *
 * @details Position limit enforcement and safety monitoring for stepper motors
 *          to prevent mechanical damage and ensure safe operation within
 *          defined position boundaries following
 *          .github/instructions/safety-rt.instructions.md
 *
 * @note SAFETY-CRITICAL: This system prevents motors from exceeding safe
 *       position limits that could cause mechanical damage or unsafe
 * conditions
 */

#ifndef POSITION_SAFETY_H
#define POSITION_SAFETY_H

#include "common/system_state.h"
#include "config/motor_config.h"
#include "config/safety_config.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 */
/* Type Definitions                                                          */
/* ==========================================================================
 */

/**
 * @brief Position limit types
 */
typedef enum {
  POSITION_LIMIT_SOFT_MIN = 0, ///< Soft minimum position limit
  POSITION_LIMIT_SOFT_MAX,     ///< Soft maximum position limit
  POSITION_LIMIT_HARD_MIN,     ///< Hard minimum position limit
  POSITION_LIMIT_HARD_MAX,     ///< Hard maximum position limit
  POSITION_LIMIT_COUNT
} PositionLimitType_t;

/**
 * @brief Position safety violation type
 */
typedef enum {
  POSITION_VIOLATION_NONE = 0,      ///< No violation
  POSITION_VIOLATION_SOFT_MIN,      ///< Soft minimum limit exceeded
  POSITION_VIOLATION_SOFT_MAX,      ///< Soft maximum limit exceeded
  POSITION_VIOLATION_HARD_MIN,      ///< Hard minimum limit exceeded
  POSITION_VIOLATION_HARD_MAX,      ///< Hard maximum limit exceeded
  POSITION_VIOLATION_RUNAWAY,       ///< Position runaway detected
  POSITION_VIOLATION_ENCODER_FAULT, ///< Encoder fault/validation failure
  POSITION_VIOLATION_COUNT
} PositionViolationType_t;

/**
 * @brief Position safety configuration for a single motor
 */
typedef struct {
  bool enabled;                ///< Position safety enabled for this motor
  float soft_min_deg;          ///< Soft minimum position (degrees)
  float soft_max_deg;          ///< Soft maximum position (degrees)
  float hard_min_deg;          ///< Hard minimum position (degrees)
  float hard_max_deg;          ///< Hard maximum position (degrees)
  float warning_margin_deg;    ///< Warning margin before limit (degrees)
  float max_velocity_dps;      ///< Maximum allowed velocity (deg/sec)
  float runaway_threshold_deg; ///< Position runaway threshold (degrees)
  uint32_t runaway_timeout_ms; ///< Time threshold for runaway detection
  bool enforce_soft_limits;    ///< Enforce soft limits with controlled stop
  bool enforce_hard_limits;    ///< Enforce hard limits with immediate stop
} PositionSafetyConfig_t;

/**
 * @brief Position safety status for a single motor
 */
typedef struct {
  bool initialized;                  ///< Position safety initialized
  float current_position_deg;        ///< Current position (degrees)
  float last_position_deg;           ///< Last recorded position (degrees)
  float velocity_dps;                ///< Current velocity (deg/sec)
  uint32_t last_update_time;         ///< Last position update timestamp
  PositionViolationType_t violation; ///< Current violation type
  uint32_t violation_count;          ///< Total violation count
  uint32_t soft_limit_violations;    ///< Soft limit violation count
  uint32_t hard_limit_violations;    ///< Hard limit violation count
  bool position_valid;               ///< Position reading is valid
  bool limits_active;                ///< Position limits are active
  bool runaway_detected;             ///< Position runaway detected
  uint32_t last_violation_time;      ///< Last violation timestamp
} PositionSafetyStatus_t;

/**
 * @brief Position safety system context
 */
typedef struct {
  bool system_initialized;                         ///< System initialized
  PositionSafetyConfig_t motor_config[MAX_MOTORS]; ///< Per-motor configuration
  PositionSafetyStatus_t motor_status[MAX_MOTORS]; ///< Per-motor status
  uint32_t total_violations;                       ///< Total system violations
  uint32_t emergency_stops;   ///< Emergency stops due to position
  bool global_limits_enabled; ///< Global position limits enabled
  uint32_t system_start_time; ///< System start timestamp
} PositionSafetyContext_t;

/**
 * @brief Position validation result
 */
typedef struct {
  bool position_valid;               ///< Position is within all limits
  bool soft_limit_ok;                ///< Within soft limits
  bool hard_limit_ok;                ///< Within hard limits
  bool velocity_ok;                  ///< Velocity within limits
  bool runaway_ok;                   ///< No runaway detected
  PositionViolationType_t violation; ///< Violation type if any
  float distance_to_limit_deg;       ///< Distance to nearest limit
  PositionLimitType_t nearest_limit; ///< Type of nearest limit
} PositionValidationResult_t;

/* ==========================================================================
 */
/* Public Function Declarations                                              */
/* ==========================================================================
 */

/**
 * @brief Initialize position safety system
 * @return SystemError_t Success or error code
 *
 * @details Initializes position safety enforcement for all motors with
 *          default safety limits from configuration
 */
SystemError_t position_safety_init(void);

/**
 * @brief Deinitialize position safety system
 * @return SystemError_t Success or error code
 */
SystemError_t position_safety_deinit(void);

/**
 * @brief Configure position safety for specific motor
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @param config Position safety configuration
 * @return SystemError_t Success or error code
 */
SystemError_t
position_safety_configure_motor(uint8_t motor_id,
                                const PositionSafetyConfig_t *config);

/**
 * @brief Update motor position and perform safety checks
 * @param motor_id Motor identifier
 * @param position_deg Current position in degrees
 * @return SystemError_t Success or error code
 *
 * @details Must be called regularly with updated position data to maintain
 *          real-time position safety monitoring
 */
SystemError_t position_safety_update(uint8_t motor_id, float position_deg);

/**
 * @brief Validate target position before motion command
 * @param motor_id Motor identifier
 * @param target_position_deg Target position in degrees
 * @param result Pointer to validation result
 * @return SystemError_t Success or error code
 */
SystemError_t
position_safety_validate_target(uint8_t motor_id, float target_position_deg,
                                PositionValidationResult_t *result);

/**
 * @brief Enforce position limits on motion command
 * @param motor_id Motor identifier
 * @param requested_position_deg Requested position
 * @param safe_position_deg Pointer to safe position output
 * @return SystemError_t Success or error code
 */
SystemError_t position_safety_enforce_limits(uint8_t motor_id,
                                             float requested_position_deg,
                                             float *safe_position_deg);

/**
 * @brief Check if position is within safe limits
 * @param motor_id Motor identifier
 * @param position_deg Position to check
 * @return bool True if position is safe, false otherwise
 */
bool position_is_within_limits(uint8_t motor_id, float position_deg);

/**
 * @brief Check if motor is approaching position limits
 * @param motor_id Motor identifier
 * @param position_deg Current position
 * @param velocity_dps Current velocity
 * @return bool True if approaching limits, false otherwise
 */
bool position_is_approaching_limits(uint8_t motor_id, float position_deg,
                                    float velocity_dps);

/**
 * @brief Set position limit violation state
 * @param motor_id Motor identifier
 * @param position_deg Position that caused violation
 * @param violation_type Type of violation
 * @return SystemError_t Success or error code
 */
SystemError_t
set_position_limit_violation(uint8_t motor_id, float position_deg,
                             PositionViolationType_t violation_type);

/**
 * @brief Clear position limit violations for motor
 * @param motor_id Motor identifier
 * @return SystemError_t Success or error code
 */
SystemError_t position_safety_clear_violations(uint8_t motor_id);

/**
 * @brief Get position safety status for motor
 * @param motor_id Motor identifier
 * @param status Pointer to status structure
 * @return SystemError_t Success or error code
 */
SystemError_t position_safety_get_status(uint8_t motor_id,
                                         PositionSafetyStatus_t *status);

/**
 * @brief Get position safety configuration for motor
 * @param motor_id Motor identifier
 * @param config Pointer to configuration structure
 * @return SystemError_t Success or error code
 */
SystemError_t position_safety_get_config(uint8_t motor_id,
                                         PositionSafetyConfig_t *config);

/**
 * @brief Enable/disable global position limits
 * @param enabled True to enable, false to disable
 * @return SystemError_t Success or error code
 */
SystemError_t position_safety_set_global_enable(bool enabled);

/**
 * @brief Get system-wide position safety statistics
 * @param context Pointer to context structure
 * @return SystemError_t Success or error code
 */
SystemError_t position_safety_get_context(PositionSafetyContext_t *context);

/* ==========================================================================
 */
/* Position Runaway Detection Functions                                     */
/* ==========================================================================
 */

/**
 * @brief Check for position runaway condition
 * @param motor_id Motor identifier
 * @return bool True if runaway detected, false otherwise
 */
bool position_detect_runaway(uint8_t motor_id);

/**
 * @brief Reset runaway detection for motor
 * @param motor_id Motor identifier
 * @return SystemError_t Success or error code
 */
SystemError_t position_reset_runaway_detection(uint8_t motor_id);

/**
 * @brief Configure runaway detection parameters
 * @param motor_id Motor identifier
 * @param threshold_deg Runaway threshold in degrees
 * @param timeout_ms Timeout for runaway detection
 * @return SystemError_t Success or error code
 */
SystemError_t position_configure_runaway_detection(uint8_t motor_id,
                                                   float threshold_deg,
                                                   uint32_t timeout_ms);

/* ==========================================================================
 */
/* Emergency Response Functions                                              */
/* ==========================================================================
 */

/**
 * @brief Execute emergency stop due to position violation
 * @param motor_id Motor identifier
 * @param violation_type Type of violation
 * @return SystemError_t Success or error code
 */
SystemError_t
position_safety_emergency_stop(uint8_t motor_id,
                               PositionViolationType_t violation_type);

/**
 * @brief Execute controlled stop for position violation
 * @param motor_id Motor identifier
 * @param violation_type Type of violation
 * @return SystemError_t Success or error code
 */
SystemError_t
position_safety_controlled_stop(uint8_t motor_id,
                                PositionViolationType_t violation_type);

/* ==========================================================================
 */
/* Configuration Constants                                                   */
/* ==========================================================================
 */

/// @brief Default position safety configuration values
#define POSITION_SAFETY_DEFAULT_SOFT_MIN_DEG (-180.0f)
#define POSITION_SAFETY_DEFAULT_SOFT_MAX_DEG (180.0f)
#define POSITION_SAFETY_DEFAULT_HARD_MIN_DEG (-200.0f)
#define POSITION_SAFETY_DEFAULT_HARD_MAX_DEG (200.0f)
#define POSITION_SAFETY_DEFAULT_WARNING_MARGIN_DEG (10.0f)
#define POSITION_SAFETY_DEFAULT_MAX_VELOCITY_DPS (360.0f)
#define POSITION_SAFETY_DEFAULT_RUNAWAY_THRESHOLD_DEG (720.0f)
#define POSITION_SAFETY_DEFAULT_RUNAWAY_TIMEOUT_MS (5000)

/// @brief Position update frequency requirements
#define POSITION_SAFETY_MIN_UPDATE_RATE_HZ (100)
#define POSITION_SAFETY_MAX_UPDATE_INTERVAL_MS                                 \
  (1000 / POSITION_SAFETY_MIN_UPDATE_RATE_HZ)

/// @brief Position validation tolerances
#define POSITION_SAFETY_TOLERANCE_DEG (0.1f)
#define POSITION_SAFETY_VELOCITY_TOLERANCE_DPS (1.0f)

/// @brief Emergency response timeouts
#define POSITION_SAFETY_EMERGENCY_RESPONSE_MS (50)
#define POSITION_SAFETY_CONTROLLED_STOP_MS (1000)

#ifdef __cplusplus
}
#endif

#endif /* POSITION_SAFETY_H */
