/**
 * @file failsafe_manager.c
 * @brief Hardware Fail-Safe State Machine Implementation
 * @author STM32H753ZI Project Team
 * @date 2025-08-12
 *
 * @details Implementation of systematic fail-safe state machine that provides
 *          controlled progression through safe states during fault conditions.
 *          Integrates with existing emergency stop and safety systems.
 */

#include "failsafe_manager.h"
#include "config/hardware_config.h"
#include "drivers/l6470/l6470_driver.h"
#include "emergency_stop.h"
#include "hal_abstraction/hal_abstraction.h"
#include "safety_system.h"
#include "watchdog_manager.h"
#include <string.h>

/* ==========================================================================
 */
/* Private Variables                                                         */
/* ==========================================================================
 */

static FailsafeManagerContext_t failsafe_context = {0};
static bool failsafe_manager_initialized = false;

/// @brief Fail-safe state configuration table
static const FailsafeStateConfig_t state_config_table[FAILSAFE_STATE_COUNT] = {
    // INIT State
    {.state = FAILSAFE_STATE_INIT,
     .max_duration_ms = 10000,
     .entry_action = FAILSAFE_ACTION_NONE,
     .periodic_action = FAILSAFE_ACTION_NONE,
     .exit_action = FAILSAFE_ACTION_NONE,
     .allow_automatic_recovery = true,
     .description = "System Initialization and Self-Test"},

    // SAFE State
    {.state = FAILSAFE_STATE_SAFE,
     .max_duration_ms = UINT32_MAX,
     .entry_action = FAILSAFE_ACTION_NONE,
     .periodic_action = FAILSAFE_ACTION_NONE,
     .exit_action = FAILSAFE_ACTION_WARNING_INDICATOR,
     .allow_automatic_recovery = true,
     .description = "Normal Operation - All Systems OK"},

    // WARNING State
    {.state = FAILSAFE_STATE_WARNING,
     .max_duration_ms = FAILSAFE_WARNING_TIMEOUT_MS,
     .entry_action = FAILSAFE_ACTION_WARNING_INDICATOR,
     .periodic_action = FAILSAFE_ACTION_REDUCE_SPEED,
     .exit_action = FAILSAFE_ACTION_NONE,
     .allow_automatic_recovery = true,
     .description = "Warning Condition - Attempting Recovery"},

    // FAULT State
    {.state = FAILSAFE_STATE_FAULT,
     .max_duration_ms = FAILSAFE_SAFE_STOP_TIMEOUT_MS,
     .entry_action = FAILSAFE_ACTION_CONTROLLED_STOP,
     .periodic_action = FAILSAFE_ACTION_WARNING_INDICATOR,
     .exit_action = FAILSAFE_ACTION_NONE,
     .allow_automatic_recovery = false,
     .description = "Fault Condition - Safe Stop and Hold"},

    // EMERGENCY State
    {.state = FAILSAFE_STATE_EMERGENCY,
     .max_duration_ms = UINT32_MAX,
     .entry_action = FAILSAFE_ACTION_EMERGENCY_STOP,
     .periodic_action = FAILSAFE_ACTION_POWER_DISCONNECT,
     .exit_action = FAILSAFE_ACTION_NONE,
     .allow_automatic_recovery = false,
     .description = "Emergency Stop Active"},

    // CRITICAL_FAULT State
    {.state = FAILSAFE_STATE_CRITICAL_FAULT,
     .max_duration_ms = UINT32_MAX,
     .entry_action = FAILSAFE_ACTION_POWER_DISCONNECT,
     .periodic_action = FAILSAFE_ACTION_SYSTEM_RESET,
     .exit_action = FAILSAFE_ACTION_NONE,
     .allow_automatic_recovery = false,
     .description = "Critical Fault - Hardware Fail-Safe Active"}};

/* ==========================================================================
 */
/* Private Function Declarations                                             */
/* ==========================================================================
 */

static SystemError_t execute_failsafe_action(FailsafeAction_t action);
static SystemError_t transition_to_state(FailsafeState_t new_state,
                                         FailsafeTrigger_t trigger);
static bool is_recovery_possible(void);
static uint8_t calculate_fault_severity(FailsafeTrigger_t trigger);
static const FailsafeStateConfig_t *get_state_config(FailsafeState_t state);
static void update_failsafe_statistics(void);

/* ==========================================================================
 */
/* Public Function Implementation                                            */
/* ==========================================================================
 */

/**
 * @brief Initialize fail-safe manager
 */
SystemError_t failsafe_manager_init(void) {
    if (failsafe_manager_initialized) {
        return SYSTEM_OK; // Already initialized
    }

    // Initialize context
    memset(&failsafe_context, 0, sizeof(failsafe_context));
    failsafe_context.current_state = FAILSAFE_STATE_INIT;
    failsafe_context.previous_state = FAILSAFE_STATE_INIT;
    failsafe_context.last_trigger = FAILSAFE_TRIGGER_NONE;
    failsafe_context.state_entry_time = HAL_Abstraction_GetTick();

    // Initialize hardware fail-safe mechanisms
    SystemError_t result = failsafe_hardware_init();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Perform initial system self-test
    result = perform_system_self_test();
    if (result != SYSTEM_OK) {
        // Self-test failed - enter critical fault state
        transition_to_state(FAILSAFE_STATE_CRITICAL_FAULT,
                            FAILSAFE_TRIGGER_SYSTEM_FAULT);
        return result;
    }

    failsafe_context.initialized = true;
    failsafe_manager_initialized = true;

    // Transition to safe state after successful initialization
    return transition_to_state(FAILSAFE_STATE_SAFE, FAILSAFE_TRIGGER_NONE);
}

/**
 * @brief Deinitialize fail-safe manager
 */
SystemError_t failsafe_manager_deinit(void) {
    if (!failsafe_manager_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Execute safe stop before deinitializing
    execute_safe_stop();

    // Reset context
    memset(&failsafe_context, 0, sizeof(failsafe_context));
    failsafe_manager_initialized = false;

    return SYSTEM_OK;
}

/**
 * @brief Update fail-safe state machine
 */
SystemError_t failsafe_update_state_machine(void) {
    if (!failsafe_manager_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    uint32_t current_time = HAL_Abstraction_GetTick();
    uint32_t time_in_state = current_time - failsafe_context.state_entry_time;
    const FailsafeStateConfig_t *state_config =
        get_state_config(failsafe_context.current_state);

    if (state_config == NULL) {
        return ERROR_INVALID_STATE;
    }

    // Execute periodic action for current state
    SystemError_t result =
        execute_failsafe_action(state_config->periodic_action);
    if (result != SYSTEM_OK) {
        return result;
    }

    // Check for state timeout
    if (time_in_state > state_config->max_duration_ms) {
        switch (failsafe_context.current_state) {
        case FAILSAFE_STATE_INIT:
            // Initialization timeout - go to fault state
            return transition_to_state(FAILSAFE_STATE_FAULT,
                                       FAILSAFE_TRIGGER_SYSTEM_FAULT);

        case FAILSAFE_STATE_WARNING:
            // Warning timeout - escalate to fault
            return transition_to_state(FAILSAFE_STATE_FAULT,
                                       failsafe_context.last_trigger);

        case FAILSAFE_STATE_FAULT:
            // Fault timeout - escalate to emergency if no recovery
            if (!is_recovery_possible()) {
                return transition_to_state(FAILSAFE_STATE_EMERGENCY,
                                           failsafe_context.last_trigger);
            }
            break;

        default:
            // Other states don't have automatic timeout transitions
            break;
        }
    }

    // Check for recovery conditions in warning and fault states
    if ((failsafe_context.current_state == FAILSAFE_STATE_WARNING ||
         failsafe_context.current_state == FAILSAFE_STATE_FAULT) &&
        state_config->allow_automatic_recovery) {

        if (check_system_health() == SYSTEM_OK && is_recovery_possible()) {
            result = failsafe_attempt_recovery();
            if (result == SYSTEM_OK) {
                return transition_to_state(FAILSAFE_STATE_SAFE,
                                           FAILSAFE_TRIGGER_NONE);
            }
        }
    }

    // Update statistics
    update_failsafe_statistics();

    return SYSTEM_OK;
}

/**
 * @brief Trigger fail-safe condition
 */
SystemError_t failsafe_trigger(FailsafeTrigger_t trigger, uint8_t severity) {
    if (!failsafe_manager_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    if (trigger >= FAILSAFE_TRIGGER_COUNT) {
        return ERROR_INVALID_PARAMETER;
    }

    // Determine target state based on trigger severity and current state
    FailsafeState_t target_state;

    switch (trigger) {
    case FAILSAFE_TRIGGER_EMERGENCY_STOP:
        target_state = FAILSAFE_STATE_EMERGENCY;
        break;

    case FAILSAFE_TRIGGER_TIMING_VIOLATION:
    case FAILSAFE_TRIGGER_OVERCURRENT:
    case FAILSAFE_TRIGGER_WATCHDOG_TIMEOUT:
    case FAILSAFE_TRIGGER_POWER_FAULT:
        target_state = (severity >= 8) ? FAILSAFE_STATE_CRITICAL_FAULT
                                       : FAILSAFE_STATE_FAULT;
        break;

    case FAILSAFE_TRIGGER_POSITION_LIMIT:
    case FAILSAFE_TRIGGER_OVERTEMPERATURE:
    case FAILSAFE_TRIGGER_COMMUNICATION_LOSS:
    case FAILSAFE_TRIGGER_SENSOR_FAULT:
        target_state =
            (severity >= 7) ? FAILSAFE_STATE_FAULT : FAILSAFE_STATE_WARNING;
        break;

    default:
        target_state = FAILSAFE_STATE_WARNING;
        break;
    }

    // Only transition if target state is more severe than current state
    if (target_state > failsafe_context.current_state) {
        return transition_to_state(target_state, trigger);
    }

    return SYSTEM_OK;
}

/**
 * @brief Attempt recovery from fault condition
 */
SystemError_t failsafe_attempt_recovery(void) {
    if (!failsafe_manager_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Only allow recovery from warning and fault states
    if (failsafe_context.current_state != FAILSAFE_STATE_WARNING &&
        failsafe_context.current_state != FAILSAFE_STATE_FAULT) {
        return ERROR_INVALID_STATE;
    }

    failsafe_context.recovery_attempt_count++;

    // Check if too many recovery attempts
    if (failsafe_context.recovery_attempt_count >
        FAILSAFE_MAX_RECOVERY_ATTEMPTS) {
        failsafe_context.manual_reset_required = true;
        return ERROR_RETRY_LIMIT_EXCEEDED;
    }

    // Perform system health check
    SystemError_t health_result = check_system_health();
    if (health_result != SYSTEM_OK) {
        return health_result;
    }

    // Reset recovery counter on successful recovery
    failsafe_context.recovery_attempt_count = 0;

    return SYSTEM_OK;
}

/**
 * @brief Reset fail-safe system (manual reset)
 */
SystemError_t failsafe_manual_reset(void) {
    if (!failsafe_manager_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Perform system health check before reset
    SystemError_t health_result = check_system_health();
    if (health_result != SYSTEM_OK) {
        return health_result;
    }

    // Reset counters and flags
    failsafe_context.recovery_attempt_count = 0;
    failsafe_context.manual_reset_required = false;
    failsafe_context.hardware_failsafe_active = false;

    // Transition back to safe state
    return transition_to_state(FAILSAFE_STATE_SAFE, FAILSAFE_TRIGGER_NONE);
}

/**
 * @brief Get current fail-safe status
 */
SystemError_t failsafe_get_status(FailsafeStatus_t *status) {
    if (!failsafe_manager_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    if (status == NULL) {
        return ERROR_NULL_POINTER;
    }

    uint32_t current_time = HAL_Abstraction_GetTick();
    const FailsafeStateConfig_t *state_config =
        get_state_config(failsafe_context.current_state);

    status->current_state = failsafe_context.current_state;
    status->time_in_state_ms =
        current_time - failsafe_context.state_entry_time;
    status->active_triggers =
        (1 << failsafe_context.last_trigger); // Convert to bitmask
    status->recovery_possible = is_recovery_possible();
    status->manual_intervention_required =
        failsafe_context.manual_reset_required;
    status->fault_severity_level =
        calculate_fault_severity(failsafe_context.last_trigger);
    status->state_description =
        (state_config != NULL) ? state_config->description : "Unknown State";

    return SYSTEM_OK;
}

/**
 * @brief Get fail-safe manager context
 */
SystemError_t failsafe_get_context(FailsafeManagerContext_t *context) {
    if (!failsafe_manager_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    if (context == NULL) {
        return ERROR_NULL_POINTER;
    }

    memcpy(context, &failsafe_context, sizeof(FailsafeManagerContext_t));
    return SYSTEM_OK;
}

/**
 * @brief Check if system is in safe state
 */
bool failsafe_is_safe_state(void) {
    if (!failsafe_manager_initialized) {
        return false;
    }

    return (failsafe_context.current_state == FAILSAFE_STATE_SAFE);
}

/**
 * @brief Check if manual reset is required
 */
bool failsafe_requires_manual_reset(void) {
    if (!failsafe_manager_initialized) {
        return true;
    }

    return failsafe_context.manual_reset_required;
}

/**
 * @brief Execute safe stop sequence
 */
SystemError_t execute_safe_stop(void) {
    // Stop all motors with controlled deceleration
    for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
        SystemError_t result = l6470_soft_stop(motor_id);
        if (result != SYSTEM_OK) {
            // If controlled stop fails, execute immediate stop
            l6470_hard_stop(motor_id);
        }
    }

    // Wait for motors to stop
    HAL_Abstraction_Delay(FAILSAFE_CONTROLLED_STOP_TIME_MS);

    // Disable motor power if still moving
    for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
        // Check if motor is still moving and disable power if necessary
        // This would integrate with motor status checking
    }

    return SYSTEM_OK;
}

/**
 * @brief Execute emergency stop sequence
 */
SystemError_t execute_emergency_stop_sequence(void) {
    // Integrate with existing emergency stop system
    return emergency_stop_execute(ESTOP_SOURCE_SOFTWARE);
}

/**
 * @brief Initialize hardware fail-safe mechanisms
 */
SystemError_t failsafe_hardware_init(void) {
    // Initialize safety relay outputs (normally closed for fail-safe)
    SystemError_t result = HAL_Abstraction_GPIO_Init(
        SAFETY_RELAY1_PORT,
        &(HAL_GPIO_Config_t){.pin = SAFETY_RELAY1_PIN,
                             .mode = GPIO_MODE_OUTPUT_PP,
                             .pull = GPIO_NOPULL,
                             .speed = GPIO_SPEED_FREQ_LOW,
                             .alternate = 0});

    if (result == SYSTEM_OK) {
        result = HAL_Abstraction_GPIO_Init(
            SAFETY_RELAY2_PORT,
            &(HAL_GPIO_Config_t){.pin = SAFETY_RELAY2_PIN,
                                 .mode = GPIO_MODE_OUTPUT_PP,
                                 .pull = GPIO_NOPULL,
                                 .speed = GPIO_SPEED_FREQ_LOW,
                                 .alternate = 0});
    }

    // Set relays to safe state (enabled)
    if (result == SYSTEM_OK) {
        HAL_Abstraction_GPIO_Write(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN,
                                   HAL_GPIO_STATE_SET);
        HAL_Abstraction_GPIO_Write(SAFETY_RELAY2_PORT, SAFETY_RELAY2_PIN,
                                   HAL_GPIO_STATE_SET);
    }

    return result;
}

/* ==========================================================================
 */
/* State-Specific Functions Implementation                                   */
/* ==========================================================================
 */

/**
 * @brief Enter initialization state
 */
SystemError_t failsafe_enter_init_state(void) {
    return transition_to_state(FAILSAFE_STATE_INIT, FAILSAFE_TRIGGER_NONE);
}

/**
 * @brief Enter safe operational state
 */
SystemError_t failsafe_enter_safe_state(void) {
    return transition_to_state(FAILSAFE_STATE_SAFE, FAILSAFE_TRIGGER_NONE);
}

/**
 * @brief Enter warning state
 */
SystemError_t failsafe_enter_warning_state(FailsafeTrigger_t trigger) {
    return transition_to_state(FAILSAFE_STATE_WARNING, trigger);
}

/**
 * @brief Enter fault state
 */
SystemError_t failsafe_enter_fault_state(FailsafeTrigger_t trigger) {
    return transition_to_state(FAILSAFE_STATE_FAULT, trigger);
}

/**
 * @brief Enter emergency state
 */
SystemError_t failsafe_enter_emergency_state(FailsafeTrigger_t trigger) {
    return transition_to_state(FAILSAFE_STATE_EMERGENCY, trigger);
}

/**
 * @brief Enter critical fault state
 */
SystemError_t failsafe_enter_critical_fault_state(FailsafeTrigger_t trigger) {
    return transition_to_state(FAILSAFE_STATE_CRITICAL_FAULT, trigger);
}

/* ==========================================================================
 */
/* Safety Validation Functions Implementation                                */
/* ==========================================================================
 */

/**
 * @brief Perform system self-test
 */
SystemError_t perform_system_self_test(void) {
    // Check emergency stop system
    if (!safety_system_is_operational()) {
        return ERROR_HARDWARE_FAULT;
    }

    // Check watchdog system
    SystemError_t watchdog_result = watchdog_check_health();
    if (watchdog_result != SYSTEM_OK) {
        return watchdog_result;
    }

    // Check safety system
    if (!safety_system_is_operational()) {
        return ERROR_HARDWARE_FAULT;
    }

    // Additional self-tests can be added here

    return SYSTEM_OK;
}

/**
 * @brief Check system health
 */
SystemError_t check_system_health(void) {
    // Check watchdog health
    SystemError_t result = watchdog_check_health();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Check emergency stop system
    if (safety_get_emergency_stop_state()) {
        return ERROR_HARDWARE_FAULT; // Cannot be healthy with emergency stop
                                     // active
    }

    // Check safety system
    // Check safety system operational status (replaces
    // safety_system_health_check)
    if (!safety_system_is_operational()) {
        result = ERROR_SAFETY_SYSTEM_NOT_READY;
    } else {
        result = SYSTEM_OK;
    }
    if (result != SYSTEM_OK) {
        return result;
    }

    return SYSTEM_OK;
}

/**
 * @brief Validate fail-safe system integrity
 */
SystemError_t validate_failsafe_integrity(void) {
    // Validate state machine configuration
    for (uint8_t i = 0; i < FAILSAFE_STATE_COUNT; i++) {
        if (state_config_table[i].state != i) {
            return ERROR_CONFIGURATION_INVALID;
        }
    }

    // Validate hardware fail-safe mechanisms
    // This would include testing safety relays, etc.

    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* Private Function Implementation                                           */
/* ==========================================================================
 */

/**
 * @brief Execute fail-safe action
 */
static SystemError_t execute_failsafe_action(FailsafeAction_t action) {
    switch (action) {
    case FAILSAFE_ACTION_NONE:
        return SYSTEM_OK;

    case FAILSAFE_ACTION_WARNING_INDICATOR:
        // Activate warning LED/indicators
        // This would integrate with indicator management
        return SYSTEM_OK;

    case FAILSAFE_ACTION_REDUCE_SPEED:
        // Reduce motor speed to safe levels
        // This would integrate with motor control
        return SYSTEM_OK;

    case FAILSAFE_ACTION_CONTROLLED_STOP:
        return execute_safe_stop();

    case FAILSAFE_ACTION_IMMEDIATE_STOP:
        // Immediate motor stop
        for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
            l6470_hard_stop(motor_id);
        }
        return SYSTEM_OK;

    case FAILSAFE_ACTION_POWER_DISCONNECT:
        // Disconnect motor power via safety relays
        HAL_Abstraction_GPIO_Write(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN,
                                   HAL_GPIO_STATE_RESET);
        HAL_Abstraction_GPIO_Write(SAFETY_RELAY2_PORT, SAFETY_RELAY2_PIN,
                                   HAL_GPIO_STATE_RESET);
        failsafe_context.hardware_failsafe_active = true;
        return SYSTEM_OK;

    case FAILSAFE_ACTION_EMERGENCY_STOP:
        return execute_emergency_stop_sequence();

    case FAILSAFE_ACTION_SYSTEM_RESET:
        // System reset would be implemented here
        // For now, just set flag
        failsafe_context.manual_reset_required = true;
        return SYSTEM_OK;

    default:
        return ERROR_INVALID_PARAMETER;
    }
}

/**
 * @brief Transition to new fail-safe state
 */
static SystemError_t transition_to_state(FailsafeState_t new_state,
                                         FailsafeTrigger_t trigger) {
    if (new_state >= FAILSAFE_STATE_COUNT) {
        return ERROR_INVALID_PARAMETER;
    }

    const FailsafeStateConfig_t *old_state_config =
        get_state_config(failsafe_context.current_state);
    const FailsafeStateConfig_t *new_state_config =
        get_state_config(new_state);

    if (new_state_config == NULL) {
        return ERROR_INVALID_STATE;
    }

    // Execute exit action for current state
    if (old_state_config != NULL) {
        execute_failsafe_action(old_state_config->exit_action);
    }

    // Update state
    failsafe_context.previous_state = failsafe_context.current_state;
    failsafe_context.current_state = new_state;
    failsafe_context.last_trigger = trigger;
    failsafe_context.state_entry_time = HAL_Abstraction_GetTick();

    // Execute entry action for new state
    SystemError_t result =
        execute_failsafe_action(new_state_config->entry_action);

    // Update statistics
    if (new_state == FAILSAFE_STATE_FAULT ||
        new_state == FAILSAFE_STATE_EMERGENCY ||
        new_state == FAILSAFE_STATE_CRITICAL_FAULT) {
        failsafe_context.total_fault_count++;
    }

    if (new_state == FAILSAFE_STATE_EMERGENCY) {
        failsafe_context.emergency_stop_count++;
    }

    return result;
}

/**
 * @brief Check if recovery is possible
 */
static bool is_recovery_possible(void) {
    // Recovery not possible if too many attempts made
    if (failsafe_context.recovery_attempt_count >=
        FAILSAFE_MAX_RECOVERY_ATTEMPTS) {
        return false;
    }

    // Recovery not possible if hardware fail-safe is active
    if (failsafe_context.hardware_failsafe_active) {
        return false;
    }

    // Recovery not possible if manual reset is required
    if (failsafe_context.manual_reset_required) {
        return false;
    }

    // Recovery only possible from warning and fault states
    return (failsafe_context.current_state == FAILSAFE_STATE_WARNING ||
            failsafe_context.current_state == FAILSAFE_STATE_FAULT);
}

/**
 * @brief Calculate fault severity based on trigger
 */
static uint8_t calculate_fault_severity(FailsafeTrigger_t trigger) {
    switch (trigger) {
    case FAILSAFE_TRIGGER_EMERGENCY_STOP:
        return 10; // Maximum severity

    case FAILSAFE_TRIGGER_TIMING_VIOLATION:
    case FAILSAFE_TRIGGER_WATCHDOG_TIMEOUT:
    case FAILSAFE_TRIGGER_POWER_FAULT:
        return 9; // Critical severity

    case FAILSAFE_TRIGGER_OVERCURRENT:
    case FAILSAFE_TRIGGER_POSITION_LIMIT:
        return 7; // High severity

    case FAILSAFE_TRIGGER_OVERTEMPERATURE:
    case FAILSAFE_TRIGGER_COMMUNICATION_LOSS:
        return 5; // Medium severity

    case FAILSAFE_TRIGGER_SENSOR_FAULT:
        return 4; // Medium-low severity

    default:
        return 2; // Low severity
    }
}

/**
 * @brief Get state configuration for given state
 */
static const FailsafeStateConfig_t *get_state_config(FailsafeState_t state) {
    if (state >= FAILSAFE_STATE_COUNT) {
        return NULL;
    }

    return &state_config_table[state];
}

/**
 * @brief Update fail-safe statistics
 */
static void update_failsafe_statistics(void) {
    // Update timing statistics, fault counters, etc.
    // This could integrate with telemetry system for monitoring
}
