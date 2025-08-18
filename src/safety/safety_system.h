/**
 * @file safety_system.h
 * @brief Safety System Manager - STM32H753ZI Stepper Motor Project
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @note Critical safety system implementation following:
 * - .github/instructions/safety-rt.instructions.md
 * - config/safety_config.h (SSOT)
 *
 * @warning This system is SAFETY-CRITICAL. All modifications must be
 * thoroughly tested and validated before deployment.
 */

#ifndef SAFETY_SYSTEM_H
#define SAFETY_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/data_types.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Safety monitor structure (SSOT)
 */
typedef struct {
    float current_value;
    float safe_min;
    float safe_max;
    float warning_min;
    float warning_max;
    uint32_t violation_count;
    uint32_t warning_count;
    uint32_t last_violation;
    bool enabled;
} SafetyMonitor_t;

/**
 * @brief Monitor channel enumeration
 */
typedef enum {
    MONITOR_MOTOR1_CURRENT,
    MONITOR_MOTOR2_CURRENT,
    MONITOR_MOTOR1_SPEED,
    MONITOR_MOTOR2_SPEED,
    MONITOR_MOTOR1_POSITION,
    MONITOR_MOTOR2_POSITION,
    MONITOR_SYSTEM_TEMPERATURE,
    MONITOR_SUPPLY_VOLTAGE,
    MONITOR_CPU_USAGE,
    MONITOR_COMM_LATENCY,
    MONITOR_COUNT
} MonitorChannel_t;

/**
 * @brief Watchdog configuration structure (SSOT)
 */
typedef struct {
    uint32_t timeout_ms;           /**< Watchdog timeout period */
    uint32_t warning_threshold_ms; /**< Warning before timeout */
    bool enabled;                  /**< Watchdog enabled */
    uint32_t last_refresh;         /**< Last refresh timestamp */
    uint32_t refresh_count;        /**< Total refresh count */
    uint32_t timeout_count;        /**< Timeout event count */
    uint32_t missed_refresh_count; /**< Missed refresh count */
} WatchdogConfig_t;

// Function declarations
SystemError_t safety_system_init(void);
SystemError_t safety_system_task(void);
bool safety_system_is_operational(void);
bool safety_get_emergency_stop_state(void);
SystemError_t safety_monitor_update(MonitorChannel_t channel, float value);
SystemError_t log_safety_event(SafetyEventType_t event, uint32_t parameter,
                               uint32_t additional_data);
SystemError_t handle_safety_violation(MonitorChannel_t channel, float value);
void safety_log_event(SafetyEventType_t event, uint8_t motor_id,
                      uint32_t additional_data);
SafetyMonitor_t get_safety_monitor_status(MonitorChannel_t channel);
SystemError_t set_safety_monitor_enabled(MonitorChannel_t channel,
                                         bool enabled);

#ifdef __cplusplus
}
#endif

#endif /* SAFETY_SYSTEM_H */
