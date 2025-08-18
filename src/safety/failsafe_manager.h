/**
 * @file failsafe_manager.h
 * @brief Hardware Fail-Safe State Machine Manager
 * @author STM32H753ZI Project Team
 * @date 2025-08-12
 *
 * @details Systematic fail-safe state machine providing controlled progression
 *          through safe states during fault conditions. Integrates with
 * existing emergency stop and safety monitoring systems following
 *          .github/instructions/safety-rt.instructions.md
 *
 * @note SAFETY-CRITICAL: This system ensures predictable safe state
 * progression during complex fault scenarios with hardware-level fail-safe
 * actions
 */

#ifndef FAILSAFE_MANAGER_H
#define FAILSAFE_MANAGER_H

#include "common/system_state.h"
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
 * @brief Fail-safe state enumeration
 */
typedef enum {
    FAILSAFE_STATE_INIT = 0,       ///< System initialization and self-test
    FAILSAFE_STATE_SAFE,           ///< Normal operation - all systems OK
    FAILSAFE_STATE_WARNING,        ///< Warning condition - attempt recovery
    FAILSAFE_STATE_FAULT,          ///< Fault condition - safe stop and hold
    FAILSAFE_STATE_EMERGENCY,      ///< Emergency stop active
    FAILSAFE_STATE_CRITICAL_FAULT, ///< Critical fault - hardware fail-safe
    FAILSAFE_STATE_COUNT
} FailsafeState_t;

/**
 * @brief Fail-safe trigger source
 */
typedef enum {
    FAILSAFE_TRIGGER_NONE = 0,           ///< No trigger
    FAILSAFE_TRIGGER_EMERGENCY_STOP,     ///< Emergency stop triggered
    FAILSAFE_TRIGGER_TIMING_VIOLATION,   ///< Real-time timing violation
    FAILSAFE_TRIGGER_POSITION_LIMIT,     ///< Position limit exceeded
    FAILSAFE_TRIGGER_OVERCURRENT,        ///< Motor overcurrent detected
    FAILSAFE_TRIGGER_OVERTEMPERATURE,    ///< System overtemperature
    FAILSAFE_TRIGGER_COMMUNICATION_LOSS, ///< Communication timeout
    FAILSAFE_TRIGGER_SENSOR_FAULT,       ///< Sensor validation failure
    FAILSAFE_TRIGGER_WATCHDOG_TIMEOUT,   ///< Watchdog timeout
    FAILSAFE_TRIGGER_POWER_FAULT,        ///< Power supply fault
    FAILSAFE_TRIGGER_SYSTEM_FAULT,       ///< General system fault
    FAILSAFE_TRIGGER_COUNT
} FailsafeTrigger_t;

/**
 * @brief Fail-safe action type
 */
typedef enum {
    FAILSAFE_ACTION_NONE = 0,          ///< No action required
    FAILSAFE_ACTION_WARNING_INDICATOR, ///< Activate warning indicators
    FAILSAFE_ACTION_REDUCE_SPEED,      ///< Reduce motor speed
    FAILSAFE_ACTION_CONTROLLED_STOP,   ///< Controlled deceleration stop
    FAILSAFE_ACTION_IMMEDIATE_STOP,    ///< Immediate motor stop
    FAILSAFE_ACTION_POWER_DISCONNECT,  ///< Disconnect motor power
    FAILSAFE_ACTION_EMERGENCY_STOP,    ///< Execute emergency stop sequence
    FAILSAFE_ACTION_SYSTEM_RESET,      ///< System reset required
    FAILSAFE_ACTION_COUNT
} FailsafeAction_t;

/**
 * @brief Fail-safe state configuration
 */
typedef struct {
    FailsafeState_t state;            ///< State identifier
    uint32_t max_duration_ms;         ///< Maximum time in this state
    FailsafeAction_t entry_action;    ///< Action on entering state
    FailsafeAction_t periodic_action; ///< Periodic action while in state
    FailsafeAction_t exit_action;     ///< Action on exiting state
    bool allow_automatic_recovery;    ///< Allow automatic recovery from state
    const char *description;          ///< State description
} FailsafeStateConfig_t;

/**
 * @brief Fail-safe manager context
 */
typedef struct {
    bool initialized;                ///< Manager initialized
    FailsafeState_t current_state;   ///< Current fail-safe state
    FailsafeState_t previous_state;  ///< Previous fail-safe state
    FailsafeTrigger_t last_trigger;  ///< Last trigger that caused state change
    uint32_t state_entry_time;       ///< Time when current state was entered
    uint32_t total_fault_count;      ///< Total number of faults
    uint32_t recovery_attempt_count; ///< Number of recovery attempts
    uint32_t emergency_stop_count;   ///< Number of emergency stops
    bool manual_reset_required;      ///< Manual reset required to continue
    bool hardware_failsafe_active;   ///< Hardware fail-safe is active
} FailsafeManagerContext_t;

/**
 * @brief Fail-safe system status
 */
typedef struct {
    FailsafeState_t current_state;     ///< Current state
    uint32_t time_in_state_ms;         ///< Time in current state
    FailsafeTrigger_t active_triggers; ///< Currently active triggers (bitmask)
    bool recovery_possible;            ///< Recovery is possible
    bool manual_intervention_required; ///< Manual intervention required
    uint8_t fault_severity_level;      ///< Fault severity (0-10)
    const char *state_description;     ///< Human-readable state description
} FailsafeStatus_t;

/* ==========================================================================
 */
/* Public Function Declarations                                              */
/* ==========================================================================
 */

/**
 * @brief Initialize fail-safe manager
 * @return SystemError_t Success or error code
 *
 * @details Initializes fail-safe state machine, configures hardware fail-safe
 *          mechanisms, and integrates with existing safety systems
 */
SystemError_t failsafe_manager_init(void);

/**
 * @brief Deinitialize fail-safe manager
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_manager_deinit(void);

/**
 * @brief Update fail-safe state machine (call periodically)
 * @return SystemError_t Success or error code
 *
 * @details Must be called regularly to process state transitions, execute
 *          fail-safe actions, and monitor for recovery conditions
 */
SystemError_t failsafe_update_state_machine(void);

/**
 * @brief Trigger fail-safe condition
 * @param trigger Trigger source
 * @param severity Fault severity level (0-10)
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_trigger(FailsafeTrigger_t trigger, uint8_t severity);

/**
 * @brief Attempt recovery from fault condition
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_attempt_recovery(void);

/**
 * @brief Reset fail-safe system (manual reset)
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_manual_reset(void);

/**
 * @brief Get current fail-safe status
 * @param status Pointer to status structure
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_get_status(FailsafeStatus_t *status);

/**
 * @brief Get fail-safe manager context
 * @param context Pointer to context structure
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_get_context(FailsafeManagerContext_t *context);

/**
 * @brief Check if system is in safe state
 * @return bool True if in safe operational state
 */
bool failsafe_is_safe_state(void);

/**
 * @brief Check if manual reset is required
 * @return bool True if manual reset required
 */
bool failsafe_requires_manual_reset(void);

/**
 * @brief Execute safe stop sequence
 * @return SystemError_t Success or error code
 *
 * @details Executes controlled safe stop with proper deceleration profiles
 *          and hardware fail-safe activation as required
 */
SystemError_t execute_safe_stop(void);

/**
 * @brief Execute emergency stop sequence
 * @return SystemError_t Success or error code
 *
 * @details Integrates with existing emergency stop system for immediate
 *          safety response
 */
SystemError_t execute_emergency_stop_sequence(void);

/**
 * @brief Initialize hardware fail-safe mechanisms
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_hardware_init(void);

/* ==========================================================================
 */
/* State-Specific Functions                                                  */
/* ==========================================================================
 */

/**
 * @brief Enter initialization state
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_enter_init_state(void);

/**
 * @brief Enter safe operational state
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_enter_safe_state(void);

/**
 * @brief Enter warning state
 * @param trigger Warning trigger source
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_enter_warning_state(FailsafeTrigger_t trigger);

/**
 * @brief Enter fault state
 * @param trigger Fault trigger source
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_enter_fault_state(FailsafeTrigger_t trigger);

/**
 * @brief Enter emergency state
 * @param trigger Emergency trigger source
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_enter_emergency_state(FailsafeTrigger_t trigger);

/**
 * @brief Enter critical fault state
 * @param trigger Critical fault trigger source
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_enter_critical_fault_state(FailsafeTrigger_t trigger);

/* ==========================================================================
 */
/* Safety Validation Functions                                               */
/* ==========================================================================
 */

/**
 * @brief Perform system self-test
 * @return SystemError_t Success or error code
 */
SystemError_t perform_system_self_test(void);

/**
 * @brief Check system health
 * @return SystemError_t Success or error code
 */
SystemError_t check_system_health(void);

/**
 * @brief Validate fail-safe system integrity
 * @return SystemError_t Success or error code
 */
SystemError_t validate_failsafe_integrity(void);

/* ==========================================================================
 */
/* Configuration Constants                                                   */
/* ==========================================================================
 */

/// @brief Maximum time in warning state before escalation (ms)
#define FAILSAFE_WARNING_TIMEOUT_MS 5000

/// @brief Maximum recovery attempts before manual reset required
#define FAILSAFE_MAX_RECOVERY_ATTEMPTS 3

/// @brief Time delay for controlled stop sequence (ms)
#define FAILSAFE_CONTROLLED_STOP_TIME_MS 2000

/// @brief Time delay for safe stop sequence (ms)
#define FAILSAFE_SAFE_STOP_TIMEOUT_MS 5000

/// @brief Emergency stop integration timeout (ms)
#define FAILSAFE_EMERGENCY_TIMEOUT_MS 100

/// @brief Critical fault immediate action timeout (ms)
#define FAILSAFE_CRITICAL_ACTION_TIMEOUT_MS 50

#ifdef __cplusplus
}
#endif

#endif /* FAILSAFE_MANAGER_H */
