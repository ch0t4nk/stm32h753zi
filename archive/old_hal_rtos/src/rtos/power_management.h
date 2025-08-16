/**
 * @file power_management.h
 * @brief Advanced Power Management for STM32H753ZI Motor Control System
 * @author STM32H753ZI Project Team
 * @date August 2025
 * @version 1.0
 *
 * This file provides advanced power management capabilities including:
 * - Dynamic CPU frequency scaling
 * - Task period adjustment for different power modes
 * - Low-power sleep modes with wake-on-interrupt
 * - Thermal management and energy optimization
 *
 * @note Industrial Enhancement: Power Management
 * Supports SIL-2 safety requirements with power-aware fault detection
 * and graceful degradation under thermal or power constraints.
 */

#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 */
/* Includes */
/* ============================================================================
 */

#include "common/data_types.h"
#include "common/error_codes.h"
#include "config/freertos_config_ssot.h"

/* ============================================================================
 */
/* Power Management Types and Structures                                     */
/* ============================================================================
 */

/**
 * @brief Power management state information
 */
typedef struct {
  PowerMode_t current_mode;        ///< Current power mode
  PowerMode_t requested_mode;      ///< Requested power mode
  uint32_t mode_entry_time;        ///< Time when current mode was entered
  uint32_t last_activity_time;     ///< Last system activity timestamp
  uint32_t idle_duration_ms;       ///< Current idle duration
  uint32_t wake_events_count;      ///< Number of wake events
  uint32_t mode_transitions_count; ///< Number of mode transitions
  bool mode_transition_pending;    ///< Mode transition in progress
} PowerManagementState_t;

/**
 * @brief Power mode configuration
 */
typedef struct {
  PowerMode_t mode;                  ///< Power mode identifier
  uint32_t cpu_frequency_mhz;        ///< CPU frequency for this mode
  uint16_t motor_period_multiplier;  ///< Motor task period multiplier
  uint16_t safety_period_multiplier; ///< Safety task period multiplier
  uint16_t comm_period_multiplier;   ///< Communication task period multiplier
  uint16_t telemetry_period_multiplier; ///< Telemetry task period multiplier
  bool allow_sleep;                     ///< Allow CPU sleep in this mode
  bool allow_stop;                      ///< Allow peripheral stop in this mode
} PowerModeConfig_t;

/**
 * @brief Power statistics for monitoring and optimization
 */
typedef struct {
  uint32_t time_in_active_ms;         ///< Time spent in active mode
  uint32_t time_in_quiet_ms;          ///< Time spent in quiet mode
  uint32_t time_in_idle_ms;           ///< Time spent in idle mode
  uint32_t time_in_sleep_ms;          ///< Time spent in sleep mode
  uint32_t total_transitions;         ///< Total mode transitions
  uint32_t wake_from_motion;          ///< Wake events from motion
  uint32_t wake_from_communication;   ///< Wake events from communication
  uint32_t wake_from_safety;          ///< Wake events from safety
  uint32_t power_savings_estimate_mw; ///< Estimated power savings (mW)
} PowerStatistics_t;

/* ============================================================================
 */
/* Power Management API */
/* ============================================================================
 */

/**
 * @brief Initialize power management system
 *
 * Sets up power management infrastructure, configures CPU frequency scaling,
 * and establishes baseline power mode configurations.
 *
 * @return SYSTEM_OK on success, error code on failure
 * @note MISRA-C: Essential function for power management initialization
 * @note IEC 61508: SIL-2 - Initialize with safe defaults
 */
SystemError_t power_management_init(void);

/**
 * @brief Update power management state
 *
 * Called periodically to evaluate system activity and determine
 * appropriate power mode transitions based on activity patterns.
 *
 * @return SYSTEM_OK on success, error code on failure
 * @note Called from telemetry task every 100ms
 * @note IEC 61508: SIL-2 - Continuous power state monitoring
 */
SystemError_t power_management_update(void);

/**
 * @brief Request power mode change
 *
 * Request transition to a specific power mode. The transition will
 * be evaluated and executed if safe and appropriate.
 *
 * @param mode Requested power mode
 * @param force Force transition even if not recommended
 * @return SYSTEM_OK on success, error code on failure
 * @note IEC 61508: SIL-2 - Safe mode transitions with validation
 */
SystemError_t power_management_request_mode(PowerMode_t mode, bool force);

/**
 * @brief Signal system activity
 *
 * Called whenever system activity occurs (motion, communication, etc.)
 * to reset idle timers and potentially wake from low-power modes.
 *
 * @param activity_source Source of activity (for statistics)
 * @return SYSTEM_OK on success, error code on failure
 * @note Thread-safe, can be called from any task or ISR
 */
SystemError_t power_management_signal_activity(uint32_t activity_source);

/**
 * @brief Get current power management state
 *
 * Retrieves current power mode, timing information, and statistics
 * for monitoring and debugging purposes.
 *
 * @param state Pointer to state structure to fill
 * @return SYSTEM_OK on success, error code on failure
 */
SystemError_t power_management_get_state(PowerManagementState_t *state);

/**
 * @brief Get power management statistics
 *
 * Retrieves detailed power usage statistics including time spent
 * in each mode and estimated power savings.
 *
 * @param stats Pointer to statistics structure to fill
 * @return SYSTEM_OK on success, error code on failure
 */
SystemError_t power_management_get_statistics(PowerStatistics_t *stats);

/**
 * @brief Enter sleep mode with wake conditions
 *
 * Safely enters sleep mode with specified wake conditions.
 * All critical safety monitoring continues during sleep.
 *
 * @param wake_sources Bitmask of wake sources to enable
 * @param max_sleep_ms Maximum sleep duration
 * @return SYSTEM_OK on successful wake, error code on failure
 * @note IEC 61508: SIL-2 - Safety monitoring continues in sleep
 */
SystemError_t power_management_sleep(uint32_t wake_sources,
                                     uint32_t max_sleep_ms);

/**
 * @brief Configure CPU frequency
 *
 * Dynamically adjusts CPU frequency for power optimization while
 * maintaining real-time performance requirements.
 *
 * @param frequency_mhz Target CPU frequency in MHz
 * @return SYSTEM_OK on success, error code on failure
 * @note MISRA-C: Validates frequency range before application
 * @note IEC 61508: SIL-2 - Maintains safety timing requirements
 */
SystemError_t power_management_set_cpu_frequency(uint32_t frequency_mhz);

/* ============================================================================
 */
/* Advanced Power Features */
/* ============================================================================
 */

/**
 * @brief Thermal throttling management
 *
 * Monitors system temperature and adjusts power mode to prevent
 * overheating while maintaining essential functions.
 *
 * @param temperature_celsius Current system temperature
 * @return SYSTEM_OK if normal, ERROR_THERMAL_THROTTLE if throttling applied
 * @note IEC 61508: SIL-2 - Thermal protection for safety-critical systems
 */
SystemError_t power_management_thermal_check(float temperature_celsius);

/**
 * @brief Predictive power management
 *
 * Uses activity patterns to predict optimal power mode transitions
 * before they are strictly necessary, improving responsiveness.
 *
 * @return SYSTEM_OK on success, error code on failure
 * @note AI-Driven: Uses pattern analysis for optimization
 */
SystemError_t power_management_predictive_optimization(void);

/**
 * @brief Emergency power conservation
 *
 * Enters emergency power conservation mode when low power conditions
 * are detected, maintaining only essential safety functions.
 *
 * @return SYSTEM_OK on success, error code on failure
 * @note IEC 61508: SIL-2 - Emergency power conservation with safety priority
 */
SystemError_t power_management_emergency_conservation(void);

#ifdef __cplusplus
}
#endif

#endif // POWER_MANAGEMENT_H
