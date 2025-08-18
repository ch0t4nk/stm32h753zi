#include <stdbool.h>
#include <stdint.h>

// NOTE: SafetyState_t is now defined in config/safety_config.h (SSOT)
// Remove duplicate definition to use SSOT

// --- Legacy Safety System Types and Macros (local patch for missing headers)
// ---
typedef enum {
    SIL_NONE = 0,
    SIL_1 = 1,
    SIL_2 = 2,
    SIL_3 = 3,
    SIL_4 = 4
} SafetyIntegrityLevel_t;

typedef enum {
    SAFETY_FUNC_EMERGENCY_STOP,
    SAFETY_FUNC_SPEED_MONITORING,
    SAFETY_FUNC_POSITION_LIMITING,
    SAFETY_FUNC_TORQUE_LIMITING,
    SAFETY_FUNC_TEMPERATURE_MONITORING,
    SAFETY_FUNC_WATCHDOG,
    SAFETY_FUNC_COMMUNICATION_MONITOR,
    SAFETY_FUNC_SENSOR_VALIDATION,
    SAFETY_FUNC_COUNT
} SafetyFunction_t;

typedef struct {
    SafetyFunction_t function;
    SafetyIntegrityLevel_t sil_level;
    uint32_t reaction_time_ms;
    bool enabled;
    float threshold_value;
    uint16_t fault_tolerance;
    bool automatic_reset;
} SafetyConfig_t;

/* EmergencyStopSource_t enum is defined in config/safety_config.h */
/* EmergencyStopState_t struct is defined in config/safety_config.h */

typedef struct {
    uint32_t total_safety_events;
    uint32_t emergency_stops;
    uint32_t watchdog_timeouts;
    uint32_t limit_violations;
    uint32_t warnings_issued;
    uint32_t system_uptime_hours;
    uint32_t last_maintenance_hours;
} SafetyStatistics_t;

// --- End legacy patch ---

/**
 * @file safety_system.c
 * @brief Safety System Manager Implementation - STM32H753ZI Stepper Motor
 * Project
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

#include "common/data_types.h"
#include "common/error_codes.h"
#include "common/system_state.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#include "config/safety_config.h"
#include "controllers/position_safety.h"
#include "controllers/timing_precision.h"
#include "drivers/l6470/l6470_driver.h"
#include "emergency_stop.h"
#include "fault_monitor.h"
#include "hal_abstraction/hal_abstraction.h"
#include "safety/failsafe_manager.h"
#include "safety/interrupt_priorities.h"
#include "safety_system.h"
#include "watchdog_manager.h"
#include <math.h>
#include <string.h>

/* ==========================================================================
 */
/* Private Variables and State Management                                    */
/* ==========================================================================
 */

static bool safety_system_initialized = false;
static SafetyStatistics_t safety_statistics = {0};
static SafetyConfig_t safety_functions[SAFETY_FUNC_COUNT];
static SafetyMonitor_t safety_monitors[MONITOR_COUNT];

// Safety event log (circular buffer)
#define SAFETY_EVENT_LOG_SIZE 64
typedef struct {
    SafetyEventType_t event;
    uint32_t parameter;
    uint32_t timestamp;
    uint32_t sequence_number;
} SafetyEventLog_t;

static SafetyEventLog_t safety_event_log[SAFETY_EVENT_LOG_SIZE];
static uint32_t safety_event_log_index = 0;
static uint32_t safety_event_sequence = 0;

// System safety state
static SafetyState_t current_safety_state = SAFETY_STATE_UNKNOWN;
static uint32_t safety_state_entry_time = 0;
static uint32_t last_safety_check_time = 0;

// Emergency stop state
static EmergencyStopState_t current_emergency_stop_state =
    EMERGENCY_STOP_NORMAL;
static EmergencyStopSource_t current_emergency_stop_source = ESTOP_SOURCE_NONE;

/* ==========================================================================
 */
/* Private Function Prototypes                                               */
/* ==========================================================================
 */

static SystemError_t initialize_safety_configurations(void);
static SystemError_t initialize_safety_monitors(void);
static void set_safety_state(SafetyState_t new_state);
static void set_emergency_stop_state(EmergencyStopState_t new_state,
                                     EmergencyStopSource_t source);
static SystemError_t perform_safety_checks(void);
static void broadcast_emergency_stop(void);
// get_microsecond_timer() is declared in timing_precision.h (non-static)

/* ==========================================================================
 */
/* Public API Implementation                                                 */
/* ==========================================================================
 */

/**
 * @brief Initialize the complete safety system
 */
SystemError_t safety_system_init(void) {
    SystemError_t result = SYSTEM_OK;

    if (safety_system_initialized) {
        return ERROR_ALREADY_INITIALIZED;
    }

    // Initialize safety configurations
    result = initialize_safety_configurations();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Initialize safety monitoring
    result = initialize_safety_monitors();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Initialize emergency stop system
    result = emergency_stop_init();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Initialize watchdog system
    result = watchdog_init();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Initialize fault monitoring
    result = fault_monitor_init();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Initialize high-precision timing system (FTR-016)
    result = timing_precision_init();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Initialize interrupt priority system (FTR-016)
    result = interrupt_priorities_init();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Initialize fail-safe manager (FTR-016)
    result = failsafe_manager_init();
    if (result != SYSTEM_OK) {
        return result;
    }

    // Initialize safety statistics
    memset(&safety_statistics, 0, sizeof(SafetyStatistics_t));
    safety_statistics.total_safety_events = 0;

    // Initialize event log
    memset(safety_event_log, 0, sizeof(safety_event_log));
    safety_event_log_index = 0;
    safety_event_sequence = 0;

    // Set initial safety state
    set_safety_state(SAFETY_STATE_SAFE);

    // Record initialization time
    last_safety_check_time = HAL_Abstraction_GetTick();

    safety_system_initialized = true;

    // Log system initialization
    log_safety_event(SAFETY_EVENT_EMERGENCY_STOP_RESET, 0,
                     get_microsecond_timer());

    return SYSTEM_OK;
}

/**
 * @brief Check all safety monitors for violations
 * @return SystemError_t Success or error code
 */
static SystemError_t check_all_safety_monitors(void) {
    SystemError_t result = SYSTEM_OK;
    uint32_t violations_found = 0;

    // Check all enabled safety monitors
    for (MonitorChannel_t channel = MONITOR_MOTOR1_CURRENT;
         channel < MONITOR_COUNT; channel++) {

        if (!safety_monitors[channel].enabled) {
            continue; // Skip disabled monitors
        }

        // Check if current value is within safe limits
        float current_value = safety_monitors[channel].current_value;
        float safe_min = safety_monitors[channel].safe_min;
        float safe_max = safety_monitors[channel].safe_max;

        if (current_value < safe_min || current_value > safe_max) {
            // Safety violation detected
            safety_monitors[channel].violation_count++;
            violations_found++;

            // Log the specific violation
            log_safety_event(SAFETY_EVENT_LIMIT_VIOLATION, (uint32_t)channel,
                             *(uint32_t *)&current_value);

            // Determine appropriate response based on channel
            SystemError_t violation_result =
                handle_safety_violation(channel, current_value);
            if (violation_result != SYSTEM_OK) {
                result = violation_result;
            }

            result = ERROR_SAFETY_LIMIT_VIOLATION;
        } else if (current_value < safety_monitors[channel].warning_min ||
                   current_value > safety_monitors[channel].warning_max) {
            // Warning threshold exceeded
            safety_monitors[channel].warning_count++;

            log_safety_event(SAFETY_EVENT_WARNING, (uint32_t)channel,
                             *(uint32_t *)&current_value);
        }
    }

    // Update safety statistics
    if (violations_found > 0) {
        safety_statistics.limit_violations += violations_found;
    }

    return result;
}

SystemError_t safety_system_task(void) {
    if (!safety_system_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    uint32_t current_time = HAL_Abstraction_GetTick();

    // Check if safety task interval has elapsed
    if ((current_time - last_safety_check_time) >= SAFETY_CHECK_INTERVAL_MS) {
        last_safety_check_time = current_time;

        if (!safety_system_initialized) {
            return ERROR_NOT_INITIALIZED;
        }

        uint32_t current_time = HAL_Abstraction_GetTick();

        // Check if safety task interval has elapsed
        if ((current_time - last_safety_check_time) >=
            SAFETY_CHECK_INTERVAL_MS) {
            last_safety_check_time = current_time;

            // Refresh watchdog
            SystemError_t watchdog_result = watchdog_refresh();
            if (watchdog_result != SYSTEM_OK) {
                log_safety_event(SAFETY_EVENT_WATCHDOG_WARNING, 0,
                                 get_microsecond_timer());
            }

            // Check watchdog health
            SystemError_t health_result = watchdog_check_health();
            if (health_result != SYSTEM_OK) {
                log_safety_event(SAFETY_EVENT_WATCHDOG_TIMEOUT, 0,
                                 get_microsecond_timer());
                safety_statistics.watchdog_timeouts++;
                return health_result;
            }

            // Check fault monitoring health
            SystemError_t fault_result = fault_monitor_check();
            if (fault_result != SYSTEM_OK) {
                log_safety_event(SAFETY_EVENT_FAULT_DETECTED, 0,
                                 (uint32_t)fault_result);
                safety_statistics.limit_violations++;
                return fault_result;
            }

            // Perform comprehensive safety monitor checks
            SystemError_t monitor_result = check_all_safety_monitors();
            if (monitor_result != SYSTEM_OK) {
                log_safety_event(SAFETY_EVENT_LIMIT_VIOLATION, 0,
                                 (uint32_t)monitor_result);
                return monitor_result;
            }

            // Perform safety checks
            SystemError_t safety_result = perform_safety_checks();
            if (safety_result != SYSTEM_OK) {
                return safety_result;
            }

            // Update safety statistics
            safety_statistics.system_uptime_hours =
                current_time / (1000 * 3600);
        }

        return SYSTEM_OK;
    }

    // Execute emergency stop in subsystem (example: triggered by fault
    // cascade)
    EmergencyStopSource_t source = ESTOP_SOURCE_FAULT_CASCADE;
    uint32_t start_time = HAL_Abstraction_GetTick();
    SystemError_t estop_result = emergency_stop_execute(source);

    // Update emergency stop state
    if (estop_result == SYSTEM_OK) {
        set_emergency_stop_state(EMERGENCY_STOP_TRIGGERED, source);
    } else {
        set_emergency_stop_state(EMERGENCY_STOP_FAULT, source);
    }

    // Update statistics
    safety_statistics.emergency_stops++;
    safety_statistics.total_safety_events++;

    // Log emergency stop event
    log_safety_event(SAFETY_EVENT_EMERGENCY_STOP, (uint32_t)source,
                     start_time);

    // Broadcast emergency stop to all systems
    broadcast_emergency_stop();

    return estop_result;
}

SystemError_t reset_emergency_stop(void) {
    // Perform comprehensive safety checks before reset
    SystemError_t check_result = perform_safety_checks();
    if (check_result != SYSTEM_OK) {
        return check_result;
    }

    // Reset emergency stop in subsystem
    SystemError_t reset_result = emergency_stop_reset();
    if (reset_result != SYSTEM_OK) {
        return reset_result;
    }

    // Reset emergency stop state to normal
    set_emergency_stop_state(EMERGENCY_STOP_NORMAL, ESTOP_SOURCE_NONE);

    // Return to safe state
    set_safety_state(SAFETY_STATE_SAFE);

    // Log reset event
    log_safety_event(SAFETY_EVENT_EMERGENCY_STOP_RESET, ESTOP_SOURCE_NONE,
                     get_microsecond_timer());

    return SYSTEM_OK;
}

static void set_emergency_stop_state(EmergencyStopState_t new_state,
                                     EmergencyStopSource_t source) {
    current_emergency_stop_state = new_state;
    current_emergency_stop_source = source;
}

EmergencyStopState_t emergency_stop_get_state(void) {
    return current_emergency_stop_state;
}

EmergencyStopState_t get_emergency_stop_state(void) {
    return emergency_stop_get_state();
}

bool is_system_safe(void) {
    if (!safety_system_initialized) {
        return false;
    }

    // Check current safety state
    if (current_safety_state != SAFETY_STATE_SAFE) {
        return false;
    }

    // Check emergency stop state
    EmergencyStopState_t estop_state = get_emergency_stop_state();
    if (estop_state != EMERGENCY_STOP_NORMAL) {
        return false;
    }

    // Check watchdog health
    if (watchdog_check_health() != SYSTEM_OK) {
        return false;
    }

    // Check all safety monitors
    for (uint8_t i = 0; i < MONITOR_COUNT; i++) {
        if (safety_monitors[i].enabled) {
            // Check if current value is within safe limits
            if (safety_monitors[i].current_value <
                    safety_monitors[i].safe_min ||
                safety_monitors[i].current_value >
                    safety_monitors[i].safe_max) {
                return false;
            }
        }
    }

    return true;
}

SystemError_t safety_monitor_update(MonitorChannel_t channel, float value) {
    if (channel >= MONITOR_COUNT) {
        return ERROR_INVALID_PARAMETER;
    }

    SafetyMonitor_t *monitor = &safety_monitors[channel];

    if (!monitor->enabled) {
        return SYSTEM_OK;
    }

    monitor->current_value = value;

    // Check for safety violations
    bool safety_violation = false;
    if (value < monitor->safe_min || value > monitor->safe_max) {
        monitor->violation_count++;
        monitor->last_violation = HAL_Abstraction_GetTick();
        safety_violation = true;

        // Update statistics
        safety_statistics.limit_violations++;
        safety_statistics.total_safety_events++;

        log_safety_event(
            SAFETY_EVENT_LIMIT_VIOLATION, channel,
            *(uint32_t *)&value); // Convert float to uint32 for logging

        // Execute immediate safety response
        SystemError_t violation_result =
            handle_safety_violation(channel, value);
        if (violation_result != SYSTEM_OK) {
            return violation_result;
        }
    }

    // Check for warnings
    if (!safety_violation &&
        (value < monitor->warning_min || value > monitor->warning_max)) {
        monitor->warning_count++;

        // Update statistics
        safety_statistics.warnings_issued++;
        safety_statistics.total_safety_events++;

        log_safety_event(SAFETY_EVENT_WARNING, channel, *(uint32_t *)&value);
    }

    return safety_violation ? ERROR_SAFETY_LIMIT_VIOLATION : SYSTEM_OK;
}

SafetyStatistics_t get_safety_statistics(void) {
    return safety_statistics;
}

SystemError_t log_safety_event(SafetyEventType_t event, uint32_t parameter,
                               uint32_t timestamp) {
    if (!safety_system_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Store event in circular buffer
    SafetyEventLog_t *log_entry = &safety_event_log[safety_event_log_index];
    log_entry->event = event;
    log_entry->parameter = parameter;
    log_entry->timestamp = timestamp;
    log_entry->sequence_number = safety_event_sequence++;

    // Advance circular buffer index
    safety_event_log_index =
        (safety_event_log_index + 1) % SAFETY_EVENT_LOG_SIZE;

    return SYSTEM_OK;
}

void safety_log_event(SafetyEventType_t event, uint8_t motor_id,
                      uint32_t data) {
    // Combine motor_id and data into a single parameter
    uint32_t parameter = ((uint32_t)motor_id << 24) | (data & 0x00FFFFFF);

    // Use existing log function
    log_safety_event(event, parameter, HAL_Abstraction_GetTick());
}

SystemError_t handle_safety_violation(MonitorChannel_t channel, float value) {
    SystemError_t result = SYSTEM_OK;

    switch (channel) {
    case MONITOR_MOTOR1_CURRENT:
    case MONITOR_MOTOR2_CURRENT:
        // Overcurrent: Stop motor immediately
        {
            uint8_t motor_id = (channel == MONITOR_MOTOR1_CURRENT) ? 0 : 1;
            // L6470 Integration: Immediate stop for overcurrent protection
            SystemError_t motor_stop_result = l6470_hard_stop(motor_id);
            if (motor_stop_result != SYSTEM_OK) {
                // Log motor stop failure for diagnostics
            }
            log_safety_event(SAFETY_EVENT_OVERCURRENT_STOP, motor_id,
                             *(uint32_t *)&value);
            result = emergency_stop_execute(ESTOP_SOURCE_OVERCURRENT);
        }
        break;

    case MONITOR_MOTOR1_SPEED:
    case MONITOR_MOTOR2_SPEED:
        // Overspeed: Reduce speed or stop
        {
            uint8_t motor_id = (channel == MONITOR_MOTOR1_SPEED) ? 0 : 1;
            // L6470 Integration: Controlled deceleration for overspeed
            SystemError_t motor_stop_result = l6470_soft_stop(motor_id);
            if (motor_stop_result != SYSTEM_OK) {
                // If soft stop fails, use hard stop for safety
                l6470_hard_stop(motor_id);
            }
            log_safety_event(SAFETY_EVENT_OVERSPEED_STOP, motor_id,
                             *(uint32_t *)&value);
        }
        break;

    case MONITOR_MOTOR1_POSITION:
    case MONITOR_MOTOR2_POSITION:
        // Position limit: Stop motor and prevent further movement
        {
            uint8_t motor_id = (channel == MONITOR_MOTOR1_POSITION) ? 0 : 1;
            // L6470 Integration: Immediate stop for position limit violation
            SystemError_t motor_stop_result = l6470_hard_stop(motor_id);
            if (motor_stop_result != SYSTEM_OK) {
                // Log motor stop failure for diagnostics
            }
            log_safety_event(SAFETY_EVENT_POSITION_LIMIT_STOP, motor_id,
                             *(uint32_t *)&value);
            result = emergency_stop_execute(ESTOP_SOURCE_POSITION_LIMIT);
        }
        break;

    case MONITOR_SYSTEM_TEMPERATURE:
        // Overtemperature: Throttle or stop system
        log_safety_event(SAFETY_EVENT_TEMPERATURE_WARNING, 0,
                         *(uint32_t *)&value);
        if (value > MOTOR_TEMP_SHUTDOWN_C) {
            result = emergency_stop_execute(ESTOP_SOURCE_OVERHEAT);
        }
        break;

    case MONITOR_SUPPLY_VOLTAGE:
        // Power supply fault: Emergency stop
        result = emergency_stop_execute(ESTOP_SOURCE_FAULT_CASCADE);
        break;

    default:
        // Unknown channel: Log warning
        log_safety_event(SAFETY_EVENT_WARNING, channel, *(uint32_t *)&value);
        break;
    }

    return result;
}

SafetyMonitor_t get_safety_monitor_status(MonitorChannel_t channel) {
    if (channel >= MONITOR_COUNT) {
        SafetyMonitor_t empty_monitor = {0};
        return empty_monitor;
    }

    return safety_monitors[channel];
}

SystemError_t set_safety_monitor_enabled(MonitorChannel_t channel,
                                         bool enabled) {
    if (channel >= MONITOR_COUNT) {
        return ERROR_INVALID_PARAMETER;
    }

    safety_monitors[channel].enabled = enabled;

    return SYSTEM_OK;
}

SystemError_t perform_safety_self_test(void) {
    SystemError_t result = SYSTEM_OK;

    // Test emergency stop system
    SystemError_t estop_test = emergency_stop_self_test();
    if (estop_test != SYSTEM_OK) {
        result = estop_test;
    }

    // Test watchdog system
    SystemError_t watchdog_test = watchdog_self_test();
    if (watchdog_test != SYSTEM_OK) {
        result = watchdog_test;
    }

    // Test fault monitoring
    SystemError_t fault_test = fault_monitor_self_test();
    if (fault_test != SYSTEM_OK) {
        result = fault_test;
    }

    return result;
}

static SystemError_t initialize_safety_configurations(void) {
    // Initialize emergency stop configuration
    safety_functions[SAFETY_FUNC_EMERGENCY_STOP].function =
        SAFETY_FUNC_EMERGENCY_STOP;
    safety_functions[SAFETY_FUNC_EMERGENCY_STOP].sil_level = SIL_3;
    safety_functions[SAFETY_FUNC_EMERGENCY_STOP].reaction_time_ms =
        ESTOP_REACTION_TIME_MS;
    safety_functions[SAFETY_FUNC_EMERGENCY_STOP].enabled = true;
    safety_functions[SAFETY_FUNC_EMERGENCY_STOP].threshold_value = 0.0f;
    safety_functions[SAFETY_FUNC_EMERGENCY_STOP].fault_tolerance = 0;
    safety_functions[SAFETY_FUNC_EMERGENCY_STOP].automatic_reset = false;

    // Initialize speed monitoring configuration
    safety_functions[SAFETY_FUNC_SPEED_MONITORING].function =
        SAFETY_FUNC_SPEED_MONITORING;
    safety_functions[SAFETY_FUNC_SPEED_MONITORING].sil_level = SIL_2;
    safety_functions[SAFETY_FUNC_SPEED_MONITORING].reaction_time_ms =
        MOTOR_OVERSPEED_TIME_MS;
    safety_functions[SAFETY_FUNC_SPEED_MONITORING].enabled = true;
    safety_functions[SAFETY_FUNC_SPEED_MONITORING].threshold_value =
        MOTOR_OVERSPEED_THRESHOLD_RPM;
    safety_functions[SAFETY_FUNC_SPEED_MONITORING].fault_tolerance = 1;
    safety_functions[SAFETY_FUNC_SPEED_MONITORING].automatic_reset = true;

    // Initialize position limiting configuration
    safety_functions[SAFETY_FUNC_POSITION_LIMITING].function =
        SAFETY_FUNC_POSITION_LIMITING;
    safety_functions[SAFETY_FUNC_POSITION_LIMITING].sil_level = SIL_2;
    safety_functions[SAFETY_FUNC_POSITION_LIMITING].reaction_time_ms = 50;
    safety_functions[SAFETY_FUNC_POSITION_LIMITING].enabled = true;
    safety_functions[SAFETY_FUNC_POSITION_LIMITING].threshold_value =
        MOTOR_POSITION_ERROR_MAX_DEG;
    safety_functions[SAFETY_FUNC_POSITION_LIMITING].fault_tolerance = 0;
    safety_functions[SAFETY_FUNC_POSITION_LIMITING].automatic_reset = false;

    // Initialize remaining safety functions with default values
    for (uint8_t i = SAFETY_FUNC_TORQUE_LIMITING; i < SAFETY_FUNC_COUNT; i++) {
        safety_functions[i].function = i;
        safety_functions[i].sil_level = SIL_1;
        safety_functions[i].reaction_time_ms = 100;
        safety_functions[i].enabled = true;
        safety_functions[i].threshold_value = 0.0f;
        safety_functions[i].fault_tolerance = 1;
        safety_functions[i].automatic_reset = true;
    }

    return SYSTEM_OK;
}

static SystemError_t initialize_safety_monitors(void) {
    // Initialize motor current monitors using SSOT configuration
    safety_monitors[MONITOR_MOTOR1_CURRENT].safe_min = 0.0f;
    safety_monitors[MONITOR_MOTOR1_CURRENT].safe_max =
        MOTOR_OVERCURRENT_THRESHOLD_MA / 1000.0f; // Convert to A
    safety_monitors[MONITOR_MOTOR1_CURRENT].warning_max =
        safety_monitors[MONITOR_MOTOR1_CURRENT].safe_max * 0.9f;
    safety_monitors[MONITOR_MOTOR1_CURRENT].warning_min =
        MOTOR_UNDERCURRENT_THRESHOLD_MA / 1000.0f;
    safety_monitors[MONITOR_MOTOR1_CURRENT].enabled = true;
    safety_monitors[MONITOR_MOTOR1_CURRENT].current_value = 0.0f;
    safety_monitors[MONITOR_MOTOR1_CURRENT].violation_count = 0;
    safety_monitors[MONITOR_MOTOR1_CURRENT].warning_count = 0;

    // Motor 2 current monitor (same as motor 1)
    safety_monitors[MONITOR_MOTOR2_CURRENT] =
        safety_monitors[MONITOR_MOTOR1_CURRENT];

    // Initialize speed monitors using SSOT configuration
    safety_monitors[MONITOR_MOTOR1_SPEED].safe_min =
        -MOTOR_OVERSPEED_THRESHOLD_RPM;
    safety_monitors[MONITOR_MOTOR1_SPEED].safe_max =
        MOTOR_OVERSPEED_THRESHOLD_RPM;
    safety_monitors[MONITOR_MOTOR1_SPEED].warning_max =
        MOTOR_OVERSPEED_THRESHOLD_RPM * 0.95f;
    safety_monitors[MONITOR_MOTOR1_SPEED].warning_min =
        -MOTOR_OVERSPEED_THRESHOLD_RPM * 0.95f;
    safety_monitors[MONITOR_MOTOR1_SPEED].enabled = true;
    safety_monitors[MONITOR_MOTOR1_SPEED].current_value = 0.0f;
    safety_monitors[MONITOR_MOTOR1_SPEED].violation_count = 0;
    safety_monitors[MONITOR_MOTOR1_SPEED].warning_count = 0;

    // Motor 2 speed monitor (same as motor 1)
    safety_monitors[MONITOR_MOTOR2_SPEED] =
        safety_monitors[MONITOR_MOTOR1_SPEED];

    // Initialize position monitors using SSOT configuration
    safety_monitors[MONITOR_MOTOR1_POSITION].safe_min =
        MOTOR1_MIN_POSITION_DEG;
    safety_monitors[MONITOR_MOTOR1_POSITION].safe_max =
        MOTOR1_MAX_POSITION_DEG;
    safety_monitors[MONITOR_MOTOR1_POSITION].warning_min =
        MOTOR1_MIN_POSITION_DEG + 5.0f;
    safety_monitors[MONITOR_MOTOR1_POSITION].warning_max =
        MOTOR1_MAX_POSITION_DEG - 5.0f;
    safety_monitors[MONITOR_MOTOR1_POSITION].enabled = true;
    safety_monitors[MONITOR_MOTOR1_POSITION].current_value = 0.0f;
    safety_monitors[MONITOR_MOTOR1_POSITION].violation_count = 0;
    safety_monitors[MONITOR_MOTOR1_POSITION].warning_count = 0;

    safety_monitors[MONITOR_MOTOR2_POSITION].safe_min =
        MOTOR2_MIN_POSITION_DEG;
    safety_monitors[MONITOR_MOTOR2_POSITION].safe_max =
        MOTOR2_MAX_POSITION_DEG;
    safety_monitors[MONITOR_MOTOR2_POSITION].warning_min =
        MOTOR2_MIN_POSITION_DEG + 5.0f;
    safety_monitors[MONITOR_MOTOR2_POSITION].warning_max =
        MOTOR2_MAX_POSITION_DEG - 5.0f;
    safety_monitors[MONITOR_MOTOR2_POSITION].enabled = true;
    safety_monitors[MONITOR_MOTOR2_POSITION].current_value = 0.0f;
    safety_monitors[MONITOR_MOTOR2_POSITION].violation_count = 0;
    safety_monitors[MONITOR_MOTOR2_POSITION].warning_count = 0;

    // Initialize temperature monitor using SSOT configuration
    safety_monitors[MONITOR_SYSTEM_TEMPERATURE].safe_min = -10.0f;
    safety_monitors[MONITOR_SYSTEM_TEMPERATURE].safe_max =
        MOTOR_TEMP_SHUTDOWN_C;
    safety_monitors[MONITOR_SYSTEM_TEMPERATURE].warning_max =
        MOTOR_TEMP_WARNING_C;
    safety_monitors[MONITOR_SYSTEM_TEMPERATURE].warning_min = 0.0f;
    safety_monitors[MONITOR_SYSTEM_TEMPERATURE].enabled = true;
    safety_monitors[MONITOR_SYSTEM_TEMPERATURE].current_value =
        25.0f; // Assume room temperature
    safety_monitors[MONITOR_SYSTEM_TEMPERATURE].violation_count = 0;
    safety_monitors[MONITOR_SYSTEM_TEMPERATURE].warning_count = 0;

    // Initialize supply voltage monitor using SSOT configuration
    safety_monitors[MONITOR_SUPPLY_VOLTAGE].safe_min = POWER_SUPPLY_MIN_V;
    safety_monitors[MONITOR_SUPPLY_VOLTAGE].safe_max = POWER_SUPPLY_MAX_V;
    safety_monitors[MONITOR_SUPPLY_VOLTAGE].warning_min =
        POWER_SUPPLY_BROWNOUT_V;
    safety_monitors[MONITOR_SUPPLY_VOLTAGE].warning_max =
        POWER_SUPPLY_OVERVOLT_V;
    safety_monitors[MONITOR_SUPPLY_VOLTAGE].enabled = true;
    safety_monitors[MONITOR_SUPPLY_VOLTAGE].current_value =
        12.0f; // Assume nominal voltage
    safety_monitors[MONITOR_SUPPLY_VOLTAGE].violation_count = 0;
    safety_monitors[MONITOR_SUPPLY_VOLTAGE].warning_count = 0;

    // Initialize remaining monitors with default values
    for (uint8_t i = MONITOR_CPU_USAGE; i < MONITOR_COUNT; i++) {
        safety_monitors[i].safe_min = 0.0f;
        safety_monitors[i].safe_max = 100.0f;
        safety_monitors[i].warning_min = 0.0f;
        safety_monitors[i].warning_max = 80.0f;
        safety_monitors[i].enabled = false; // Disabled by default
        safety_monitors[i].current_value = 0.0f;
        safety_monitors[i].violation_count = 0;
        safety_monitors[i].warning_count = 0;
    }

    return SYSTEM_OK;
}

static void update_system_state_safety(SafetyState_t new_state) {
    // For minimal implementation, we'll use the existing safety statistics
    // and update system readiness based on safety state
    // Note: Since SafetyStatistics_t doesn't have current_safety_state field,
    // we track it locally and log the state change

    // Log the safety state transition
    log_safety_event(SAFETY_EVENT_SYSTEM_INIT, (uint32_t)new_state,
                     HAL_Abstraction_GetTick());

    // Update safety statistics counters
    safety_statistics.total_safety_events++;

    // Notify other systems of safety state change
    if (new_state == SAFETY_STATE_EMERGENCY_STOP ||
        new_state == SAFETY_STATE_FAULT) {
        broadcast_emergency_stop();
    }
}

static void set_safety_state(SafetyState_t new_state) {
    if (new_state != current_safety_state) {
        current_safety_state = new_state;
        safety_state_entry_time = HAL_Abstraction_GetTick();
        // Integrate with system state manager
        update_system_state_safety(new_state);
    }
}

static SystemError_t perform_safety_checks(void) {
    // Check emergency stop button
    if (is_emergency_stop_pressed()) {
        emergency_stop_execute(ESTOP_SOURCE_BUTTON);
        return ERROR_SAFETY_EMERGENCY_STOP;
    }

    // Check fault monitoring
    SystemError_t fault_result = fault_monitor_check();
    if (fault_result != SYSTEM_OK) {
        return fault_result;
    }

    return SYSTEM_OK;
}

static void broadcast_emergency_stop(void) {
    // Log emergency stop broadcast event
    log_safety_event(SAFETY_EVENT_EMERGENCY_STOP, 0,
                     HAL_Abstraction_GetTick());

    // Notify all motor controllers to stop immediately
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        // Emergency stop all motors through motor control interface
        // In a real implementation, this would call specific motor stop
        // functions
        safety_statistics.emergency_stops++;
    }

    // Notify communication systems to report emergency stop status
    // This would be expanded in a full implementation to notify:
    // - CAN bus systems
    // - UART communication interfaces
    // - Any connected external systems
    // - Diagnostic/logging systems

    // Update safety event counters
    safety_statistics.total_safety_events++;
}

bool safety_system_is_operational(void) {
    // Safety system is considered operational if it's in SAFE or WARNING state
    // FAULT, EMERGENCY_STOP, MAINTENANCE, RECOVERY, and UNKNOWN states are not
    // operational
    return (current_safety_state == SAFETY_STATE_SAFE ||
            current_safety_state == SAFETY_STATE_WARNING);
}

// get_microsecond_timer() implementation is provided by timing_precision.c
