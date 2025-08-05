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
 * @warning This system is SAFETY-CRITICAL. All modifications must be thoroughly
 * tested and validated before deployment.
 */

#ifndef SAFETY_SYSTEM_H
#define SAFETY_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Includes                                                                   */
/* ========================================================================== */

#include "stm32h7xx_hal.h"
#include "config/safety_config.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#include "common/error_codes.h"
#include "common/data_types.h"
#include <stdbool.h>
#include <stdint.h>

/* ========================================================================== */
/* Safety System Types and Enums                                             */
/* ========================================================================== */

/**
 * @brief Safety Integrity Level (SIL) definitions
 */
typedef enum {
    SIL_NONE = 0,                      /**< No safety requirements */
    SIL_1 = 1,                         /**< Low safety integrity */
    SIL_2 = 2,                         /**< Medium safety integrity */
    SIL_3 = 3,                         /**< High safety integrity */
    SIL_4 = 4                          /**< Very high safety integrity */
} SafetyIntegrityLevel_t;

/**
 * @brief Safety function classifications
 */
typedef enum {
    SAFETY_FUNC_EMERGENCY_STOP,        /**< Emergency stop function */
    SAFETY_FUNC_SPEED_MONITORING,      /**< Speed limit monitoring */
    SAFETY_FUNC_POSITION_LIMITING,     /**< Position limit enforcement */
    SAFETY_FUNC_TORQUE_LIMITING,       /**< Torque/current limiting */
    SAFETY_FUNC_TEMPERATURE_MONITORING,/**< Temperature monitoring */
    SAFETY_FUNC_WATCHDOG,              /**< System watchdog */
    SAFETY_FUNC_COMMUNICATION_MONITOR, /**< Communication integrity */
    SAFETY_FUNC_SENSOR_VALIDATION,     /**< Sensor plausibility checking */
    SAFETY_FUNC_COUNT                  /**< Number of safety functions */
} SafetyFunction_t;

/**
 * @brief Emergency stop sources
 */
typedef enum {
    ESTOP_SOURCE_NONE = 0,             /**< No emergency stop */
    ESTOP_SOURCE_BUTTON,               /**< Physical button */
    ESTOP_SOURCE_SOFTWARE,             /**< Software command */
    ESTOP_SOURCE_COMM_LOSS,            /**< Communication loss */
    ESTOP_SOURCE_FAULT_CASCADE,        /**< Fault cascade */
    ESTOP_SOURCE_WATCHDOG,             /**< Watchdog timeout */
    ESTOP_SOURCE_OVERHEAT,             /**< Overtemperature */
    ESTOP_SOURCE_OVERCURRENT,          /**< Overcurrent protection */
    ESTOP_SOURCE_POSITION_LIMIT,       /**< Position limit violation */
    ESTOP_SOURCE_COUNT                 /**< Number of sources */
} EmergencyStopSource_t;

/**
 * @brief Safety event types for logging
 */
typedef enum {
    SAFETY_EVENT_EMERGENCY_STOP,       /**< Emergency stop activated */
    SAFETY_EVENT_EMERGENCY_STOP_RESET, /**< Emergency stop reset */
    SAFETY_EVENT_WATCHDOG_WARNING,     /**< Watchdog warning */
    SAFETY_EVENT_WATCHDOG_TIMEOUT,     /**< Watchdog timeout */
    SAFETY_EVENT_LIMIT_VIOLATION,      /**< Safety limit violation */
    SAFETY_EVENT_WARNING,              /**< Safety warning */
    SAFETY_EVENT_OVERCURRENT_STOP,     /**< Overcurrent stop */
    SAFETY_EVENT_OVERSPEED_STOP,       /**< Overspeed stop */
    SAFETY_EVENT_POSITION_LIMIT_STOP,  /**< Position limit stop */
    SAFETY_EVENT_TEMPERATURE_WARNING,  /**< Temperature warning */
    SAFETY_EVENT_COMMUNICATION_FAULT,  /**< Communication fault */
    SAFETY_EVENT_SYSTEM_INIT,          /**< System initialization */
    SAFETY_EVENT_MOTOR_INIT_BLOCKED,   /**< Motor init blocked by safety */
    SAFETY_EVENT_FAULT_DETECTED,       /**< Fault detected */
    SAFETY_EVENT_FAULT_CLEARED,        /**< Fault cleared */
    SAFETY_EVENT_COUNT                 /**< Number of event types */
} SafetyEvent_t;

/**
 * @brief Safety configuration structure
 */
typedef struct {
    SafetyFunction_t function;         /**< Safety function type */
    SafetyIntegrityLevel_t sil_level;  /**< Required SIL level */
    uint32_t reaction_time_ms;         /**< Maximum reaction time */
    bool enabled;                      /**< Function enabled/disabled */
    float threshold_value;             /**< Safety threshold */
    uint16_t fault_tolerance;          /**< Fault tolerance count */
    bool automatic_reset;              /**< Auto-reset capability */
} SafetyConfig_t;

/**
 * @brief Emergency stop state structure
 */
typedef struct {
    bool active;                       /**< Emergency stop is active */
    bool latched;                      /**< Emergency stop is latched */
    EmergencyStopSource_t source;      /**< Source of emergency stop */
    uint32_t timestamp;                /**< Timestamp when activated */
    uint32_t reset_attempts;           /**< Number of reset attempts */
    bool reset_pending;                /**< Reset operation pending */
} EmergencyStopState_t;

/**
 * @brief Watchdog configuration structure
 */
typedef struct {
    uint32_t timeout_ms;               /**< Watchdog timeout period */
    uint32_t warning_threshold_ms;     /**< Warning before timeout */
    bool enabled;                      /**< Watchdog enabled */
    uint32_t last_refresh;             /**< Last refresh timestamp */
    uint32_t refresh_count;            /**< Total refresh count */
    uint32_t timeout_count;            /**< Timeout event count */
    uint32_t missed_refresh_count;     /**< Missed refresh count */
} WatchdogConfig_t;

/**
 * @brief Real-time monitoring structure
 */
typedef struct {
    float current_value;               /**< Current measured value */
    float safe_min;                    /**< Minimum safe value */
    float safe_max;                    /**< Maximum safe value */
    float warning_min;                 /**< Warning threshold (min) */
    float warning_max;                 /**< Warning threshold (max) */
    uint32_t violation_count;          /**< Safety violation count */
    uint32_t warning_count;            /**< Warning count */
    uint32_t last_violation;           /**< Last violation time */
    bool enabled;                      /**< Monitoring enabled */
} SafetyMonitor_t;

/**
 * @brief Monitoring channels
 */
typedef enum {
    MONITOR_MOTOR1_CURRENT,            /**< Motor 1 current monitoring */
    MONITOR_MOTOR2_CURRENT,            /**< Motor 2 current monitoring */
    MONITOR_MOTOR1_SPEED,              /**< Motor 1 speed monitoring */
    MONITOR_MOTOR2_SPEED,              /**< Motor 2 speed monitoring */
    MONITOR_MOTOR1_POSITION,           /**< Motor 1 position monitoring */
    MONITOR_MOTOR2_POSITION,           /**< Motor 2 position monitoring */
    MONITOR_SYSTEM_TEMPERATURE,        /**< System temperature monitoring */
    MONITOR_SUPPLY_VOLTAGE,            /**< Supply voltage monitoring */
    MONITOR_CPU_USAGE,                 /**< CPU usage monitoring */
    MONITOR_COMM_LATENCY,              /**< Communication latency */
    MONITOR_COUNT                      /**< Number of monitor channels */
} MonitorChannel_t;

/**
 * @brief Safety system statistics
 */
typedef struct {
    uint32_t total_safety_events;      /**< Total safety events logged */
    uint32_t emergency_stops;          /**< Total emergency stops */
    uint32_t watchdog_timeouts;        /**< Total watchdog timeouts */
    uint32_t limit_violations;         /**< Total limit violations */
    uint32_t warnings_issued;          /**< Total warnings issued */
    uint32_t system_uptime_hours;      /**< System uptime in hours */
    uint32_t last_maintenance_hours;   /**< Hours since last maintenance */
} SafetyStatistics_t;

/* ========================================================================== */
/* Safety System API Functions                                               */
/* ========================================================================== */

/**
 * @brief Initialize the complete safety system
 * @return SystemError_t Success or error code
 */
SystemError_t safety_system_init(void);

/**
 * @brief Perform periodic safety system tasks (call from main loop)
 * @return SystemError_t Success or error code
 */
SystemError_t safety_system_task(void);

/**
 * @brief Check if safety system is operational
 * @return bool True if operational, false otherwise
 */
bool safety_system_is_operational(void);

/**
 * @brief Get emergency stop state
 * @return bool True if emergency stop is active
 */
bool safety_get_emergency_stop_state(void);

/**
 * @brief Log safety event
 * @param event Event type
 * @param motor_id Motor ID (0xFF for system events)
 * @param data Additional event data
 */
void safety_log_event(SafetyEventType_t event, uint8_t motor_id, uint32_t data);

/**
 * @brief Execute emergency stop sequence
 * @param source Source of emergency stop trigger
 * @return SystemError_t Success or error code
 */
SystemError_t execute_emergency_stop(EmergencyStopSource_t source);

/**
 * @brief Reset emergency stop (requires manual confirmation)
 * @return SystemError_t Success or error code
 */
SystemError_t reset_emergency_stop(void);

/**
 * @brief Get current emergency stop state
 * @return EmergencyStopState_t Current state
 */
EmergencyStopState_t get_emergency_stop_state(void);

/**
 * @brief Check if system is in safe state for operation
 * @return bool True if safe to operate
 */
bool is_system_safe(void);

/**
 * @brief Update safety monitor with new measurement value
 * @param channel Monitor channel
 * @param value New measurement value
 * @return SystemError_t Success or error code
 */
SystemError_t safety_monitor_update(MonitorChannel_t channel, float value);

/**
 * @brief Get safety statistics
 * @return SafetyStatistics_t Current statistics
 */
SafetyStatistics_t get_safety_statistics(void);

/**
 * @brief Log safety event
 * @param event Event type
 * @param parameter Event parameter
 * @param timestamp Event timestamp (microseconds)
 * @return SystemError_t Success or error code
 */
SystemError_t log_safety_event(SafetyEvent_t event, uint32_t parameter, uint32_t timestamp);

/* ========================================================================== */
/* Emergency Stop Functions                                                  */
/* ========================================================================== */

/**
 * @brief Initialize emergency stop system
 * @return SystemError_t Success or error code
 */
SystemError_t emergency_stop_init(void);

/**
 * @brief Emergency stop interrupt handler
 */
void emergency_stop_interrupt_handler(void);

/**
 * @brief Check emergency stop button state
 * @return bool True if button is pressed
 */
bool is_emergency_stop_pressed(void);

/* ========================================================================== */
/* Watchdog Functions                                                        */
/* ========================================================================== */

/**
 * @brief Initialize watchdog timer system
 * @return SystemError_t Success or error code
 */
SystemError_t watchdog_init(void);

/**
 * @brief Refresh watchdog timer (call regularly from main loop)
 * @return SystemError_t Success or error code
 */
SystemError_t watchdog_refresh(void);

/**
 * @brief Check watchdog status and health
 * @return SystemError_t Success or error code
 */
SystemError_t watchdog_check_health(void);

/**
 * @brief Get watchdog configuration
 * @return WatchdogConfig_t Current configuration
 */
WatchdogConfig_t get_watchdog_config(void);

/* ========================================================================== */
/* Safety Monitoring Functions                                               */
/* ========================================================================== */

/**
 * @brief Initialize safety monitoring system
 * @return SystemError_t Success or error code
 */
SystemError_t safety_monitoring_init(void);

/**
 * @brief Handle safety violation with appropriate response
 * @param channel Channel that violated safety limits
 * @param value Value that caused violation
 * @return SystemError_t Success or error code
 */
SystemError_t handle_safety_violation(MonitorChannel_t channel, float value);

/**
 * @brief Get safety monitor status
 * @param channel Monitor channel
 * @return SafetyMonitor_t Monitor status
 */
SafetyMonitor_t get_safety_monitor_status(MonitorChannel_t channel);

/**
 * @brief Enable/disable safety monitor
 * @param channel Monitor channel
 * @param enabled Enable/disable flag
 * @return SystemError_t Success or error code
 */
SystemError_t set_safety_monitor_enabled(MonitorChannel_t channel, bool enabled);

/* ========================================================================== */
/* Safety Configuration Functions                                            */
/* ========================================================================== */

/**
 * @brief Configure safety function
 * @param function Safety function to configure
 * @param config Configuration parameters
 * @return SystemError_t Success or error code
 */
SystemError_t configure_safety_function(SafetyFunction_t function, const SafetyConfig_t* config);

/**
 * @brief Get safety function configuration
 * @param function Safety function
 * @return SafetyConfig_t Current configuration
 */
SafetyConfig_t get_safety_function_config(SafetyFunction_t function);

/**
 * @brief Perform comprehensive safety self-test
 * @return SystemError_t Success or error code
 */
SystemError_t perform_safety_self_test(void);

#ifdef __cplusplus
}
#endif

#endif /* SAFETY_SYSTEM_H */
