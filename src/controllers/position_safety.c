/**
 * @file position_safety.c
 * @brief Position Safety Enforcement Implementation
 * @author STM32H753ZI Project Team
 * @date 2025-08-12
 *
 * @details Implementation of position limit enforcement and safety monitoring
 *          for stepper motors. Integrates with existing real-time control
 *          and safety systems to prevent mechanical damage.
 */

#include "position_safety.h"
#include "config/motor_config.h"
#include "drivers/l6470/l6470_driver.h"
#include "hal_abstraction/hal_abstraction.h"
#include "position_control.h"
#include "safety/emergency_stop.h"
#include "safety/failsafe_manager.h"
#include <math.h>
#include <string.h>

/* ==========================================================================
 */
/* Private Variables                                                         */
/* ==========================================================================
 */

static PositionSafetyContext_t position_safety_context = {0};
static bool position_safety_initialized = false;

/* ==========================================================================
 */
/* Private Function Declarations                                             */
/* ==========================================================================
 */

static SystemError_t validate_motor_id(uint8_t motor_id);
static float calculate_velocity(uint8_t motor_id, float current_position,
                                uint32_t current_time);
static bool check_position_limits(float position,
                                  const PositionSafetyConfig_t *config,
                                  PositionLimitType_t *violated_limit);
static SystemError_t handle_position_violation(
    uint8_t motor_id, PositionViolationType_t violation_type, float position);
static void update_violation_statistics(uint8_t motor_id,
                                        PositionViolationType_t violation_type);
static SystemError_t apply_default_configuration(uint8_t motor_id);

/* ==========================================================================
 */
/* Public Function Implementation                                            */
/* ==========================================================================
 */

/**
 * @brief Initialize position safety system
 */
SystemError_t position_safety_init(void) {
  if (position_safety_initialized) {
    return SYSTEM_OK; // Already initialized
  }

  // Initialize context
  memset(&position_safety_context, 0, sizeof(position_safety_context));
  position_safety_context.system_start_time = HAL_Abstraction_GetTick();
  position_safety_context.global_limits_enabled = true;

  // Apply default configuration for all motors
  for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
    SystemError_t result = apply_default_configuration(motor_id);
    if (result != SYSTEM_OK) {
      return result;
    }

    // Initialize motor status
    PositionSafetyStatus_t *status =
        &position_safety_context.motor_status[motor_id];
    status->initialized = true;
    status->current_position_deg = 0.0f;
    status->last_position_deg = 0.0f;
    status->velocity_dps = 0.0f;
    status->last_update_time = HAL_Abstraction_GetTick();
    status->violation = POSITION_VIOLATION_NONE;
    status->position_valid = true;
    status->limits_active = true;
    status->runaway_detected = false;
  }

  position_safety_context.system_initialized = true;
  position_safety_initialized = true;

  return SYSTEM_OK;
}

/**
 * @brief Deinitialize position safety system
 */
SystemError_t position_safety_deinit(void) {
  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  // Reset context
  memset(&position_safety_context, 0, sizeof(position_safety_context));
  position_safety_initialized = false;

  return SYSTEM_OK;
}

/**
 * @brief Configure position safety for specific motor
 */
SystemError_t
position_safety_configure_motor(uint8_t motor_id,
                                const PositionSafetyConfig_t *config) {
  SystemError_t result = validate_motor_id(motor_id);
  if (result != SYSTEM_OK) {
    return result;
  }

  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  if (config == NULL) {
    return ERROR_NULL_POINTER;
  }

  // Validate configuration parameters
  if (config->soft_min_deg >= config->soft_max_deg ||
      config->hard_min_deg >= config->hard_max_deg ||
      config->soft_min_deg < config->hard_min_deg ||
      config->soft_max_deg > config->hard_max_deg) {
    return ERROR_INVALID_PARAMETER;
  }

  // Copy configuration
  memcpy(&position_safety_context.motor_config[motor_id], config,
         sizeof(PositionSafetyConfig_t));

  return SYSTEM_OK;
}

/**
 * @brief Update motor position and perform safety checks
 */
SystemError_t position_safety_update(uint8_t motor_id, float position_deg) {
  SystemError_t result = validate_motor_id(motor_id);
  if (result != SYSTEM_OK) {
    return result;
  }

  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  PositionSafetyStatus_t *status =
      &position_safety_context.motor_status[motor_id];
  const PositionSafetyConfig_t *config =
      &position_safety_context.motor_config[motor_id];

  if (!config->enabled || !position_safety_context.global_limits_enabled) {
    return SYSTEM_OK; // Position safety disabled
  }

  uint32_t current_time = HAL_Abstraction_GetTick();

  // Calculate velocity
  float velocity = calculate_velocity(motor_id, position_deg, current_time);

  // Update status
  status->last_position_deg = status->current_position_deg;
  status->current_position_deg = position_deg;
  status->velocity_dps = velocity;
  status->last_update_time = current_time;
  status->position_valid = true;

  // Check for position runaway
  if (position_detect_runaway(motor_id)) {
    status->runaway_detected = true;
    return handle_position_violation(motor_id, POSITION_VIOLATION_RUNAWAY,
                                     position_deg);
  }

  // Check velocity limits
  if (fabsf(velocity) > config->max_velocity_dps) {
    return handle_position_violation(motor_id, POSITION_VIOLATION_RUNAWAY,
                                     position_deg);
  }

  // Check position limits
  PositionLimitType_t violated_limit;
  if (!check_position_limits(position_deg, config, &violated_limit)) {
    PositionViolationType_t violation_type;

    switch (violated_limit) {
    case POSITION_LIMIT_SOFT_MIN:
      violation_type = POSITION_VIOLATION_SOFT_MIN;
      break;
    case POSITION_LIMIT_SOFT_MAX:
      violation_type = POSITION_VIOLATION_SOFT_MAX;
      break;
    case POSITION_LIMIT_HARD_MIN:
      violation_type = POSITION_VIOLATION_HARD_MIN;
      break;
    case POSITION_LIMIT_HARD_MAX:
      violation_type = POSITION_VIOLATION_HARD_MAX;
      break;
    default:
      violation_type = POSITION_VIOLATION_NONE;
      break;
    }

    if (violation_type != POSITION_VIOLATION_NONE) {
      return handle_position_violation(motor_id, violation_type, position_deg);
    }
  }

  // Clear violation if position is now safe
  if (status->violation != POSITION_VIOLATION_NONE) {
    status->violation = POSITION_VIOLATION_NONE;
  }

  return SYSTEM_OK;
}

/**
 * @brief Validate target position before motion command
 */
SystemError_t
position_safety_validate_target(uint8_t motor_id, float target_position_deg,
                                PositionValidationResult_t *result) {
  SystemError_t validation_result = validate_motor_id(motor_id);
  if (validation_result != SYSTEM_OK) {
    return validation_result;
  }

  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  if (result == NULL) {
    return ERROR_NULL_POINTER;
  }

  const PositionSafetyConfig_t *config =
      &position_safety_context.motor_config[motor_id];

  // Initialize result
  memset(result, 0, sizeof(PositionValidationResult_t));

  if (!config->enabled || !position_safety_context.global_limits_enabled) {
    result->position_valid = true;
    result->soft_limit_ok = true;
    result->hard_limit_ok = true;
    result->velocity_ok = true;
    result->runaway_ok = true;
    return SYSTEM_OK;
  }

  // Check hard limits
  result->hard_limit_ok = (target_position_deg >= config->hard_min_deg &&
                           target_position_deg <= config->hard_max_deg);

  // Check soft limits
  result->soft_limit_ok = (target_position_deg >= config->soft_min_deg &&
                           target_position_deg <= config->soft_max_deg);

  // Calculate distance to nearest limit
  float dist_to_soft_min = target_position_deg - config->soft_min_deg;
  float dist_to_soft_max = config->soft_max_deg - target_position_deg;
  float dist_to_hard_min = target_position_deg - config->hard_min_deg;
  float dist_to_hard_max = config->hard_max_deg - target_position_deg;

  float min_distance = fminf(fminf(dist_to_soft_min, dist_to_soft_max),
                             fminf(dist_to_hard_min, dist_to_hard_max));

  result->distance_to_limit_deg = min_distance;

  // Determine nearest limit type
  if (min_distance == dist_to_hard_min) {
    result->nearest_limit = POSITION_LIMIT_HARD_MIN;
  } else if (min_distance == dist_to_hard_max) {
    result->nearest_limit = POSITION_LIMIT_HARD_MAX;
  } else if (min_distance == dist_to_soft_min) {
    result->nearest_limit = POSITION_LIMIT_SOFT_MIN;
  } else {
    result->nearest_limit = POSITION_LIMIT_SOFT_MAX;
  }

  // Determine violation type
  if (!result->hard_limit_ok) {
    result->violation = (target_position_deg < config->hard_min_deg)
                            ? POSITION_VIOLATION_HARD_MIN
                            : POSITION_VIOLATION_HARD_MAX;
  } else if (!result->soft_limit_ok) {
    result->violation = (target_position_deg < config->soft_min_deg)
                            ? POSITION_VIOLATION_SOFT_MIN
                            : POSITION_VIOLATION_SOFT_MAX;
  } else {
    result->violation = POSITION_VIOLATION_NONE;
  }

  // Overall validation
  result->position_valid = result->hard_limit_ok && result->soft_limit_ok;
  result->velocity_ok = true; // Cannot check velocity for target position
  result->runaway_ok = true;  // Cannot check runaway for target position

  return SYSTEM_OK;
}

/**
 * @brief Enforce position limits on motion command
 */
SystemError_t position_safety_enforce_limits(uint8_t motor_id,
                                             float requested_position_deg,
                                             float *safe_position_deg) {
  SystemError_t result = validate_motor_id(motor_id);
  if (result != SYSTEM_OK) {
    return result;
  }

  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  if (safe_position_deg == NULL) {
    return ERROR_NULL_POINTER;
  }

  const PositionSafetyConfig_t *config =
      &position_safety_context.motor_config[motor_id];

  // If limits not enabled, pass through unchanged
  if (!config->enabled || !position_safety_context.global_limits_enabled) {
    *safe_position_deg = requested_position_deg;
    return SYSTEM_OK;
  }

  // Clamp to soft limits first
  float clamped_position = requested_position_deg;

  if (config->enforce_soft_limits) {
    if (clamped_position < config->soft_min_deg) {
      clamped_position = config->soft_min_deg;
    } else if (clamped_position > config->soft_max_deg) {
      clamped_position = config->soft_max_deg;
    }
  }

  // Clamp to hard limits (always enforced for safety)
  if (clamped_position < config->hard_min_deg) {
    clamped_position = config->hard_min_deg;
  } else if (clamped_position > config->hard_max_deg) {
    clamped_position = config->hard_max_deg;
  }

  *safe_position_deg = clamped_position;

  // Return error if position was modified
  if (fabsf(clamped_position - requested_position_deg) >
      POSITION_SAFETY_TOLERANCE_DEG) {
    return ERROR_POSITION_LIMIT_EXCEEDED;
  }

  return SYSTEM_OK;
}

/**
 * @brief Check if position is within safe limits
 */
bool position_is_within_limits(uint8_t motor_id, float position_deg) {
  if (validate_motor_id(motor_id) != SYSTEM_OK ||
      !position_safety_initialized) {
    return false;
  }

  const PositionSafetyConfig_t *config =
      &position_safety_context.motor_config[motor_id];

  if (!config->enabled || !position_safety_context.global_limits_enabled) {
    return true;
  }

  PositionLimitType_t violated_limit;
  return check_position_limits(position_deg, config, &violated_limit);
}

/**
 * @brief Check if motor is approaching position limits
 */
bool position_is_approaching_limits(uint8_t motor_id, float position_deg,
                                    float velocity_dps) {
  if (validate_motor_id(motor_id) != SYSTEM_OK ||
      !position_safety_initialized) {
    return false;
  }

  const PositionSafetyConfig_t *config =
      &position_safety_context.motor_config[motor_id];

  if (!config->enabled || !position_safety_context.global_limits_enabled) {
    return false;
  }

  // Check if within warning margin of limits
  float margin = config->warning_margin_deg;

  // Check approach to soft limits based on velocity direction
  if (velocity_dps > 0) { // Moving toward positive limit
    return (position_deg > (config->soft_max_deg - margin));
  } else if (velocity_dps < 0) { // Moving toward negative limit
    return (position_deg < (config->soft_min_deg + margin));
  }

  return false; // Not moving or not approaching limits
}

/**
 * @brief Set position limit violation state
 */
SystemError_t
set_position_limit_violation(uint8_t motor_id, float position_deg,
                             PositionViolationType_t violation_type) {
  SystemError_t result = validate_motor_id(motor_id);
  if (result != SYSTEM_OK) {
    return result;
  }

  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  return handle_position_violation(motor_id, violation_type, position_deg);
}

/**
 * @brief Clear position limit violations for motor
 */
SystemError_t position_safety_clear_violations(uint8_t motor_id) {
  SystemError_t result = validate_motor_id(motor_id);
  if (result != SYSTEM_OK) {
    return result;
  }

  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  PositionSafetyStatus_t *status =
      &position_safety_context.motor_status[motor_id];
  status->violation = POSITION_VIOLATION_NONE;
  status->runaway_detected = false;

  return SYSTEM_OK;
}

/**
 * @brief Get position safety status for motor
 */
SystemError_t position_safety_get_status(uint8_t motor_id,
                                         PositionSafetyStatus_t *status) {
  SystemError_t result = validate_motor_id(motor_id);
  if (result != SYSTEM_OK) {
    return result;
  }

  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  if (status == NULL) {
    return ERROR_NULL_POINTER;
  }

  memcpy(status, &position_safety_context.motor_status[motor_id],
         sizeof(PositionSafetyStatus_t));
  return SYSTEM_OK;
}

/**
 * @brief Get position safety configuration for motor
 */
SystemError_t position_safety_get_config(uint8_t motor_id,
                                         PositionSafetyConfig_t *config) {
  SystemError_t result = validate_motor_id(motor_id);
  if (result != SYSTEM_OK) {
    return result;
  }

  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  if (config == NULL) {
    return ERROR_NULL_POINTER;
  }

  memcpy(config, &position_safety_context.motor_config[motor_id],
         sizeof(PositionSafetyConfig_t));
  return SYSTEM_OK;
}

/**
 * @brief Enable/disable global position limits
 */
SystemError_t position_safety_set_global_enable(bool enabled) {
  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  position_safety_context.global_limits_enabled = enabled;
  return SYSTEM_OK;
}

/**
 * @brief Get system-wide position safety statistics
 */
SystemError_t position_safety_get_context(PositionSafetyContext_t *context) {
  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  if (context == NULL) {
    return ERROR_NULL_POINTER;
  }

  memcpy(context, &position_safety_context, sizeof(PositionSafetyContext_t));
  return SYSTEM_OK;
}

/* ==========================================================================
 */
/* Position Runaway Detection Functions                                     */
/* ==========================================================================
 */

/**
 * @brief Check for position runaway condition
 */
bool position_detect_runaway(uint8_t motor_id) {
  if (validate_motor_id(motor_id) != SYSTEM_OK ||
      !position_safety_initialized) {
    return false;
  }

  const PositionSafetyConfig_t *config =
      &position_safety_context.motor_config[motor_id];
  const PositionSafetyStatus_t *status =
      &position_safety_context.motor_status[motor_id];

  if (!config->enabled || status->runaway_detected) {
    return status->runaway_detected;
  }

  // Check for excessive position change
  float position_change =
      fabsf(status->current_position_deg - status->last_position_deg);
  if (position_change > config->runaway_threshold_deg) {
    return true;
  }

  // Check for timeout between updates
  uint32_t current_time = HAL_Abstraction_GetTick();
  uint32_t update_interval = current_time - status->last_update_time;
  if (update_interval > config->runaway_timeout_ms) {
    return true;
  }

  return false;
}

/**
 * @brief Reset runaway detection for motor
 */
SystemError_t position_reset_runaway_detection(uint8_t motor_id) {
  SystemError_t result = validate_motor_id(motor_id);
  if (result != SYSTEM_OK) {
    return result;
  }

  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  PositionSafetyStatus_t *status =
      &position_safety_context.motor_status[motor_id];
  status->runaway_detected = false;

  return SYSTEM_OK;
}

/**
 * @brief Configure runaway detection parameters
 */
SystemError_t position_configure_runaway_detection(uint8_t motor_id,
                                                   float threshold_deg,
                                                   uint32_t timeout_ms) {
  SystemError_t result = validate_motor_id(motor_id);
  if (result != SYSTEM_OK) {
    return result;
  }

  if (!position_safety_initialized) {
    return ERROR_NOT_INITIALIZED;
  }

  if (threshold_deg <= 0 || timeout_ms == 0) {
    return ERROR_INVALID_PARAMETER;
  }

  PositionSafetyConfig_t *config =
      &position_safety_context.motor_config[motor_id];
  config->runaway_threshold_deg = threshold_deg;
  config->runaway_timeout_ms = timeout_ms;

  return SYSTEM_OK;
}

/* ==========================================================================
 */
/* Emergency Response Functions                                              */
/* ==========================================================================
 */

/**
 * @brief Execute emergency stop due to position violation
 */
SystemError_t
position_safety_emergency_stop(uint8_t motor_id,
                               PositionViolationType_t violation_type) {
  SystemError_t result = validate_motor_id(motor_id);
  if (result != SYSTEM_OK) {
    return result;
  }

  // Execute immediate motor stop
  result = l6470_hard_stop(motor_id);
  if (result != SYSTEM_OK) {
    return result;
  }

  // Trigger system emergency stop
  result = emergency_stop_execute(ESTOP_SOURCE_SOFTWARE);
  if (result != SYSTEM_OK) {
    return result;
  }

  // Trigger fail-safe manager
  FailsafeTrigger_t trigger_type = FAILSAFE_TRIGGER_POSITION_LIMIT;
  uint8_t severity = (violation_type == POSITION_VIOLATION_HARD_MIN ||
                      violation_type == POSITION_VIOLATION_HARD_MAX)
                         ? 9
                         : 7;

  result = failsafe_trigger(trigger_type, severity);

  // Update statistics
  position_safety_context.emergency_stops++;

  return result;
}

/**
 * @brief Execute controlled stop for position violation
 */
SystemError_t
position_safety_controlled_stop(uint8_t motor_id,
                                PositionViolationType_t violation_type) {
  SystemError_t result = validate_motor_id(motor_id);
  if (result != SYSTEM_OK) {
    return result;
  }

  // Execute controlled deceleration
  result = l6470_soft_stop(motor_id);
  if (result != SYSTEM_OK) {
    // If controlled stop fails, execute hard stop
    result = l6470_hard_stop(motor_id);
  }

  return result;
}

/* ==========================================================================
 */
/* Private Function Implementation                                           */
/* ==========================================================================
 */

/**
 * @brief Validate motor ID parameter
 */
static SystemError_t validate_motor_id(uint8_t motor_id) {
  if (motor_id >= MAX_MOTORS) {
    return ERROR_MOTOR_INVALID_ID;
  }
  return SYSTEM_OK;
}

/**
 * @brief Calculate velocity from position and time data
 */
static float calculate_velocity(uint8_t motor_id, float current_position,
                                uint32_t current_time) {
  const PositionSafetyStatus_t *status =
      &position_safety_context.motor_status[motor_id];

  if (status->last_update_time == 0) {
    return 0.0f; // First update, no velocity data
  }

  uint32_t time_diff = current_time - status->last_update_time;
  if (time_diff == 0) {
    return status->velocity_dps; // Return last calculated velocity
  }

  float position_diff = current_position - status->last_position_deg;
  float time_diff_sec = time_diff / 1000.0f;

  return position_diff / time_diff_sec; // degrees per second
}

/**
 * @brief Check if position is within configured limits
 */
static bool check_position_limits(float position,
                                  const PositionSafetyConfig_t *config,
                                  PositionLimitType_t *violated_limit) {
  // Check hard limits first (most critical)
  if (position < config->hard_min_deg) {
    if (violated_limit)
      *violated_limit = POSITION_LIMIT_HARD_MIN;
    return false;
  }
  if (position > config->hard_max_deg) {
    if (violated_limit)
      *violated_limit = POSITION_LIMIT_HARD_MAX;
    return false;
  }

  // Check soft limits if enforced
  if (config->enforce_soft_limits) {
    if (position < config->soft_min_deg) {
      if (violated_limit)
        *violated_limit = POSITION_LIMIT_SOFT_MIN;
      return false;
    }
    if (position > config->soft_max_deg) {
      if (violated_limit)
        *violated_limit = POSITION_LIMIT_SOFT_MAX;
      return false;
    }
  }

  return true; // Position is within all limits
}

/**
 * @brief Handle position violation with appropriate response
 */
static SystemError_t handle_position_violation(
    uint8_t motor_id, PositionViolationType_t violation_type, float position) {
  PositionSafetyStatus_t *status =
      &position_safety_context.motor_status[motor_id];

  // Update violation status
  status->violation = violation_type;
  status->last_violation_time = HAL_Abstraction_GetTick();

  // Update statistics
  update_violation_statistics(motor_id, violation_type);

  // Determine response based on violation severity
  SystemError_t result = SYSTEM_OK;

  switch (violation_type) {
  case POSITION_VIOLATION_HARD_MIN:
  case POSITION_VIOLATION_HARD_MAX:
  case POSITION_VIOLATION_RUNAWAY:
    // Critical violations - emergency stop
    result = position_safety_emergency_stop(motor_id, violation_type);
    break;

  case POSITION_VIOLATION_SOFT_MIN:
  case POSITION_VIOLATION_SOFT_MAX:
    // Soft limit violations - controlled stop
    result = position_safety_controlled_stop(motor_id, violation_type);
    break;

  case POSITION_VIOLATION_ENCODER_FAULT:
    // Encoder fault - stop and flag for maintenance
    result = position_safety_controlled_stop(motor_id, violation_type);
    status->position_valid = false;
    break;

  default:
    break;
  }

  return result;
}

/**
 * @brief Update violation statistics
 */
static void
update_violation_statistics(uint8_t motor_id,
                            PositionViolationType_t violation_type) {
  PositionSafetyStatus_t *status =
      &position_safety_context.motor_status[motor_id];

  status->violation_count++;
  position_safety_context.total_violations++;

  switch (violation_type) {
  case POSITION_VIOLATION_SOFT_MIN:
  case POSITION_VIOLATION_SOFT_MAX:
    status->soft_limit_violations++;
    break;

  case POSITION_VIOLATION_HARD_MIN:
  case POSITION_VIOLATION_HARD_MAX:
    status->hard_limit_violations++;
    break;

  default:
    break;
  }
}

/**
 * @brief Apply default configuration to motor
 */
static SystemError_t apply_default_configuration(uint8_t motor_id) {
  PositionSafetyConfig_t *config =
      &position_safety_context.motor_config[motor_id];

  config->enabled = true;
  config->soft_min_deg = POSITION_SAFETY_DEFAULT_SOFT_MIN_DEG;
  config->soft_max_deg = POSITION_SAFETY_DEFAULT_SOFT_MAX_DEG;
  config->hard_min_deg = POSITION_SAFETY_DEFAULT_HARD_MIN_DEG;
  config->hard_max_deg = POSITION_SAFETY_DEFAULT_HARD_MAX_DEG;
  config->warning_margin_deg = POSITION_SAFETY_DEFAULT_WARNING_MARGIN_DEG;
  config->max_velocity_dps = POSITION_SAFETY_DEFAULT_MAX_VELOCITY_DPS;
  config->runaway_threshold_deg = POSITION_SAFETY_DEFAULT_RUNAWAY_THRESHOLD_DEG;
  config->runaway_timeout_ms = POSITION_SAFETY_DEFAULT_RUNAWAY_TIMEOUT_MS;
  config->enforce_soft_limits = true;
  config->enforce_hard_limits = true;

  return SYSTEM_OK;
}
