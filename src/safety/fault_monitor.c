/**
 * @file fault_monitor.c
 * @brief Fault Detection and Monitoring System Implementation - STM32H753ZI Stepper Motor Project
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 * 
 * @note Fault monitoring implementation following:
 * - .github/instructions/safety-rt.instructions.md
 * - config/safety_config.h (SSOT)
 * 
 * @warning SAFETY-CRITICAL: Fault detection must respond within safety reaction times
 */

#include "fault_monitor.h"
#include "safety_system.h"
#include <string.h>

/* ========================================================================== */
/* Private Variables and State Management                                    */
/* ========================================================================== */

static bool fault_monitor_initialized = false;
static FaultMonitorConfig_t fault_config = {0};

// Fault records storage
static FaultRecord_t fault_records[MAX_FAULT_RECORDS];
static uint32_t fault_record_index = 0;
static uint32_t total_fault_records = 0;

// Active fault tracking
static uint32_t active_motor_faults[MAX_MOTORS] = {0};
static uint32_t active_system_faults = 0;

// Monitoring state
static uint32_t last_motor_current[MAX_MOTORS] = {0};
static uint32_t last_motor_speed[MAX_MOTORS] = {0};
static uint32_t last_voltage_reading = 0;
static int32_t last_temperature_reading = 0;

/* ========================================================================== */
/* Private Function Prototypes                                               */
/* ========================================================================== */

static SystemError_t add_fault_record(uint32_t fault_code, FaultSeverity_t severity, 
                                       uint32_t motor_id, uint32_t additional_data);
static FaultRecord_t* find_fault_record(uint32_t fault_code);
static SystemError_t check_motor_limits(uint8_t motor_id);
static SystemError_t check_system_health(void);
static bool is_fault_critical(uint32_t fault_code);
static const char* get_fault_description(uint32_t fault_code);

/* ========================================================================== */
/* Public API Implementation                                                 */
/* ========================================================================== */

/**
 * @brief Initialize fault monitoring system
 */
SystemError_t fault_monitor_init(void) {
    if (fault_monitor_initialized) {
        return ERROR_ALREADY_INITIALIZED;
    }
    
    // Initialize configuration from SSOT
    fault_config.enabled = true;
    fault_config.check_interval_ms = FAULT_CHECK_INTERVAL_MS;
    fault_config.fault_count = 0;
    fault_config.critical_fault_count = 0;
    fault_config.last_check_time = HAL_GetTick();
    fault_config.max_fault_records = MAX_FAULT_RECORDS;
    fault_config.current_fault_count = 0;
    
    // Clear fault records
    memset(fault_records, 0, sizeof(fault_records));
    fault_record_index = 0;
    total_fault_records = 0;
    
    // Clear active fault tracking
    memset(active_motor_faults, 0, sizeof(active_motor_faults));
    active_system_faults = 0;
    
    // Initialize monitoring state
    memset(last_motor_current, 0, sizeof(last_motor_current));
    memset(last_motor_speed, 0, sizeof(last_motor_speed));
    last_voltage_reading = 0;
    last_temperature_reading = 0;
    
    fault_monitor_initialized = true;
    
    return SYSTEM_OK;
}

/**
 * @brief Perform periodic fault monitoring checks
 */
SystemError_t fault_monitor_check(void) {
    if (!fault_monitor_initialized || !fault_config.enabled) {
        return SYSTEM_OK;
    }
    
    uint32_t current_time = HAL_GetTick();
    
    // Check if it's time for monitoring cycle
    if ((current_time - fault_config.last_check_time) < fault_config.check_interval_ms) {
        return SYSTEM_OK;
    }
    
    SystemError_t result = SYSTEM_OK;
    
    // Check each motor for faults
    for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
        SystemError_t motor_result = check_motor_limits(motor_id);
        if (motor_result != SYSTEM_OK) {
            result = motor_result;  // Keep last error
        }
        
        // Check L6470 driver status
        L6470FaultFlags_t l6470_faults = fault_monitor_check_l6470_status(motor_id);
        if (l6470_faults != L6470_FAULT_NONE) {
            // Map L6470 faults to motor faults
            if (l6470_faults & L6470_FAULT_OVERCURRENT_A) {
                fault_monitor_record_motor_fault(motor_id, MOTOR_FAULT_OVERCURRENT, 
                                                  FAULT_SEVERITY_CRITICAL, l6470_faults);
            }
            if (l6470_faults & L6470_FAULT_OVERCURRENT_B) {
                fault_monitor_record_motor_fault(motor_id, MOTOR_FAULT_OVERCURRENT, 
                                                  FAULT_SEVERITY_CRITICAL, l6470_faults);
            }
            if (l6470_faults & L6470_FAULT_THERMAL_SHUTDOWN) {
                fault_monitor_record_motor_fault(motor_id, MOTOR_FAULT_OVERTEMPERATURE, 
                                                  FAULT_SEVERITY_CRITICAL, l6470_faults);
            }
            if (l6470_faults & L6470_FAULT_THERMAL_WARNING) {
                fault_monitor_record_motor_fault(motor_id, MOTOR_FAULT_TEMP_WARNING, 
                                                  FAULT_SEVERITY_WARNING, l6470_faults);
            }
            if (l6470_faults & L6470_FAULT_UVLO) {
                fault_monitor_record_motor_fault(motor_id, MOTOR_FAULT_UNDERVOLTAGE, 
                                                  FAULT_SEVERITY_CRITICAL, l6470_faults);
            }
            if (l6470_faults & L6470_FAULT_STALL_DETECTION) {
                fault_monitor_record_motor_fault(motor_id, MOTOR_FAULT_POSITION_ERROR, 
                                                  FAULT_SEVERITY_ERROR, l6470_faults);
            }
        }
    }
    
    // Check system health
    SystemError_t system_result = check_system_health();
    if (system_result != SYSTEM_OK) {
        result = system_result;
    }
    
    fault_config.last_check_time = current_time;
    
    return result;
}

/**
 * @brief Record a motor fault
 */
SystemError_t fault_monitor_record_motor_fault(uint8_t motor_id, 
                                                MotorFaultType_t fault_type,
                                                FaultSeverity_t severity,
                                                uint32_t additional_data) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    if (motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }
    
    // Update active fault tracking
    active_motor_faults[motor_id] |= fault_type;
    
    // Add to fault record
    SystemError_t result = add_fault_record(fault_type, severity, motor_id, additional_data);
    
    // Update fault counters
    fault_config.fault_count++;
    if (severity == FAULT_SEVERITY_CRITICAL) {
        fault_config.critical_fault_count++;
    }
    fault_config.current_fault_count++;
    
    // Log safety event
    safety_log_event(SAFETY_EVENT_FAULT_DETECTED, motor_id, fault_type);
    
    return result;
}

/**
 * @brief Record a system fault
 */
SystemError_t fault_monitor_record_system_fault(SystemFaultType_t fault_type,
                                                 FaultSeverity_t severity,
                                                 uint32_t additional_data) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    // Update active fault tracking
    active_system_faults |= fault_type;
    
    // Add to fault record
    SystemError_t result = add_fault_record(fault_type, severity, 0xFF, additional_data);
    
    // Update fault counters
    fault_config.fault_count++;
    if (severity == FAULT_SEVERITY_CRITICAL) {
        fault_config.critical_fault_count++;
    }
    fault_config.current_fault_count++;
    
    // Log safety event
    safety_log_event(SAFETY_EVENT_FAULT_DETECTED, 0xFF, fault_type);
    
    return result;
}

/**
 * @brief Clear a specific fault
 */
SystemError_t fault_monitor_clear_fault(uint32_t fault_code) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    FaultRecord_t* record = find_fault_record(fault_code);
    if (record != NULL && record->active) {
        record->active = false;
        fault_config.current_fault_count--;
        
        // Clear from active tracking
        if (record->motor_id < MAX_MOTORS) {
            active_motor_faults[record->motor_id] &= ~fault_code;
        } else {
            active_system_faults &= ~fault_code;
        }
        
        // Log safety event
        safety_log_event(SAFETY_EVENT_FAULT_CLEARED, record->motor_id, fault_code);
        
        return SYSTEM_OK;
    }
    
    return ERROR_FAULT_NOT_FOUND;
}

/**
 * @brief Clear all faults of specified severity or lower
 */
SystemError_t fault_monitor_clear_faults_by_severity(FaultSeverity_t max_severity) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    uint32_t cleared_count = 0;
    
    for (uint32_t i = 0; i < total_fault_records; i++) {
        if (fault_records[i].active && fault_records[i].severity <= max_severity) {
            fault_records[i].active = false;
            cleared_count++;
            
            // Clear from active tracking
            if (fault_records[i].motor_id < MAX_MOTORS) {
                active_motor_faults[fault_records[i].motor_id] &= ~fault_records[i].fault_code;
            } else {
                active_system_faults &= ~fault_records[i].fault_code;
            }
        }
    }
    
    fault_config.current_fault_count -= cleared_count;
    
    return SYSTEM_OK;
}

/**
 * @brief Acknowledge a specific fault
 */
SystemError_t fault_monitor_acknowledge_fault(uint32_t fault_code) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    FaultRecord_t* record = find_fault_record(fault_code);
    if (record != NULL) {
        record->acknowledged = true;
        return SYSTEM_OK;
    }
    
    return ERROR_FAULT_NOT_FOUND;
}

/**
 * @brief Check if any critical faults are active
 */
bool fault_monitor_has_critical_faults(void) {
    if (!fault_monitor_initialized) {
        return false;
    }
    
    // Check motor faults
    for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
        if (active_motor_faults[motor_id] & MOTOR_FAULT_CRITICAL_MASK) {
            return true;
        }
    }
    
    // Check system faults
    if (active_system_faults & SYSTEM_FAULT_CRITICAL_MASK) {
        return true;
    }
    
    return false;
}

/**
 * @brief Get count of active faults by severity
 */
uint32_t fault_monitor_get_fault_count_by_severity(FaultSeverity_t severity) {
    if (!fault_monitor_initialized) {
        return 0;
    }
    
    uint32_t count = 0;
    
    for (uint32_t i = 0; i < total_fault_records; i++) {
        if (fault_records[i].active && fault_records[i].severity == severity) {
            count++;
        }
    }
    
    return count;
}

/**
 * @brief Get active motor faults for specific motor
 */
uint32_t fault_monitor_get_motor_faults(uint8_t motor_id) {
    if (!fault_monitor_initialized || motor_id >= MAX_MOTORS) {
        return 0;
    }
    
    return active_motor_faults[motor_id];
}

/**
 * @brief Get active system faults
 */
uint32_t fault_monitor_get_system_faults(void) {
    if (!fault_monitor_initialized) {
        return 0;
    }
    
    return active_system_faults;
}

/**
 * @brief Get fault record by index
 */
SystemError_t fault_monitor_get_fault_record(uint32_t index, FaultRecord_t* fault_record) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    if (index >= total_fault_records || fault_record == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    *fault_record = fault_records[index];
    return SYSTEM_OK;
}

/**
 * @brief Get total number of fault records
 */
uint32_t fault_monitor_get_fault_record_count(void) {
    if (!fault_monitor_initialized) {
        return 0;
    }
    
    return total_fault_records;
}

/**
 * @brief Perform fault monitor self-test
 */
SystemError_t fault_monitor_self_test(void) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    // Test 1: Verify configuration
    if (fault_config.max_fault_records != MAX_FAULT_RECORDS ||
        fault_config.check_interval_ms != FAULT_CHECK_INTERVAL_MS) {
        return ERROR_SAFETY_SELF_TEST_FAILED;
    }
    
    // Test 2: Test fault recording
    uint32_t initial_count = fault_config.fault_count;
    SystemError_t result = fault_monitor_record_system_fault(SYSTEM_FAULT_SELF_TEST, 
                                                              FAULT_SEVERITY_INFO, 0);
    if (result != SYSTEM_OK || fault_config.fault_count != (initial_count + 1)) {
        return ERROR_SAFETY_SELF_TEST_FAILED;
    }
    
    // Test 3: Test fault clearing
    result = fault_monitor_clear_fault(SYSTEM_FAULT_SELF_TEST);
    if (result != SYSTEM_OK) {
        return ERROR_SAFETY_SELF_TEST_FAILED;
    }
    
    // Test 4: Verify limits are correctly configured
    if (MOTOR_MAX_CURRENT_MA <= 0 || MOTOR_MAX_SPEED_STEPS_PER_SEC <= 0 ||
        SUPPLY_VOLTAGE_MIN_MV <= 0 || SUPPLY_VOLTAGE_MAX_MV <= SUPPLY_VOLTAGE_MIN_MV) {
        return ERROR_SAFETY_SELF_TEST_FAILED;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Check L6470 driver status for faults
 */
L6470FaultFlags_t fault_monitor_check_l6470_status(uint8_t motor_id) {
    // This would normally read the L6470 status register
    // For now, return no faults as placeholder
    return L6470_FAULT_NONE;
}

/**
 * @brief Monitor motor current levels
 */
SystemError_t fault_monitor_check_motor_current(uint8_t motor_id, uint32_t current_ma) {
    if (!fault_monitor_initialized || motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }
    
    last_motor_current[motor_id] = current_ma;
    
    // Check critical overcurrent
    if (current_ma > MOTOR_MAX_CURRENT_MA) {
        return fault_monitor_record_motor_fault(motor_id, MOTOR_FAULT_OVERCURRENT, 
                                                 FAULT_SEVERITY_CRITICAL, current_ma);
    }
    
    // Check warning level (80% of maximum)
    if (current_ma > (MOTOR_MAX_CURRENT_MA * 8 / 10)) {
        return fault_monitor_record_motor_fault(motor_id, MOTOR_FAULT_CURRENT_WARNING, 
                                                 FAULT_SEVERITY_WARNING, current_ma);
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Monitor motor speed levels
 */
SystemError_t fault_monitor_check_motor_speed(uint8_t motor_id, uint32_t speed_steps_per_sec) {
    if (!fault_monitor_initialized || motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }
    
    last_motor_speed[motor_id] = speed_steps_per_sec;
    
    // Check speed limit
    if (speed_steps_per_sec > MOTOR_MAX_SPEED_STEPS_PER_SEC) {
        return fault_monitor_record_motor_fault(motor_id, MOTOR_FAULT_SPEED_WARNING, 
                                                 FAULT_SEVERITY_WARNING, speed_steps_per_sec);
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Monitor system voltage levels
 */
SystemError_t fault_monitor_check_voltage(uint32_t voltage_mv) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    last_voltage_reading = voltage_mv;
    
    // Check overvoltage
    if (voltage_mv > SUPPLY_VOLTAGE_MAX_MV) {
        return fault_monitor_record_system_fault(SYSTEM_FAULT_POWER_FAILURE, 
                                                  FAULT_SEVERITY_CRITICAL, voltage_mv);
    }
    
    // Check undervoltage
    if (voltage_mv < SUPPLY_VOLTAGE_MIN_MV) {
        return fault_monitor_record_system_fault(SYSTEM_FAULT_POWER_FAILURE, 
                                                  FAULT_SEVERITY_CRITICAL, voltage_mv);
    }
    
    // Check warning levels
    if (voltage_mv > (SUPPLY_VOLTAGE_MAX_MV * 9 / 10) || 
        voltage_mv < (SUPPLY_VOLTAGE_MIN_MV * 11 / 10)) {
        return fault_monitor_record_system_fault(SYSTEM_FAULT_POWER_FAILURE, 
                                                  FAULT_SEVERITY_WARNING, voltage_mv);
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Monitor system temperature
 */
SystemError_t fault_monitor_check_temperature(int32_t temperature_c) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    last_temperature_reading = temperature_c;
    
    // Check overtemperature
    if (temperature_c > SYSTEM_MAX_TEMP_C) {
        return fault_monitor_record_system_fault(SYSTEM_FAULT_POWER_FAILURE, 
                                                  FAULT_SEVERITY_CRITICAL, temperature_c);
    }
    
    // Check warning level (90% of maximum)
    if (temperature_c > (SYSTEM_MAX_TEMP_C * 9 / 10)) {
        return fault_monitor_record_system_fault(SYSTEM_FAULT_POWER_FAILURE, 
                                                  FAULT_SEVERITY_WARNING, temperature_c);
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Monitor encoder position accuracy
 */
SystemError_t fault_monitor_check_position_accuracy(uint8_t motor_id, 
                                                     int32_t expected_position,
                                                     int32_t actual_position) {
    if (!fault_monitor_initialized || motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }
    
    int32_t position_error = abs(expected_position - actual_position);
    
    // Check for position error exceeding threshold
    if (position_error > MAX_POSITION_ERROR_STEPS) {
        return fault_monitor_record_motor_fault(motor_id, MOTOR_FAULT_POSITION_ERROR, 
                                                 FAULT_SEVERITY_ERROR, position_error);
    }
    
    // Check warning level (50% of threshold)
    if (position_error > (MAX_POSITION_ERROR_STEPS / 2)) {
        return fault_monitor_record_motor_fault(motor_id, MOTOR_FAULT_ENCODER_WARNING, 
                                                 FAULT_SEVERITY_WARNING, position_error);
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Enable/disable fault monitoring
 */
SystemError_t fault_monitor_set_enabled(bool enabled) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    fault_config.enabled = enabled;
    
    if (enabled) {
        fault_config.last_check_time = HAL_GetTick();
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Get fault monitor configuration
 */
FaultMonitorConfig_t fault_monitor_get_config(void) {
    return fault_config;
}

/**
 * @brief Get fault monitor statistics
 */
SystemError_t fault_monitor_get_statistics(uint32_t* total_faults,
                                            uint32_t* critical_faults,
                                            uint32_t* active_faults) {
    if (!fault_monitor_initialized) {
        return ERROR_NOT_INITIALIZED;
    }
    
    if (total_faults) {
        *total_faults = fault_config.fault_count;
    }
    
    if (critical_faults) {
        *critical_faults = fault_config.critical_fault_count;
    }
    
    if (active_faults) {
        *active_faults = fault_config.current_fault_count;
    }
    
    return SYSTEM_OK;
}

/* ========================================================================== */
/* Private Function Implementation                                           */
/* ========================================================================== */

/**
 * @brief Add fault record to storage
 */
static SystemError_t add_fault_record(uint32_t fault_code, FaultSeverity_t severity, 
                                       uint32_t motor_id, uint32_t additional_data) {
    // Look for existing fault record
    FaultRecord_t* existing = find_fault_record(fault_code);
    
    if (existing != NULL) {
        // Update existing record
        existing->count++;
        existing->timestamp = HAL_GetTick();
        existing->additional_data = additional_data;
        existing->active = true;
        existing->acknowledged = false;
        return SYSTEM_OK;
    }
    
    // Add new fault record
    if (total_fault_records < MAX_FAULT_RECORDS) {
        fault_record_index = total_fault_records;
        total_fault_records++;
    } else {
        // Circular buffer - overwrite oldest
        fault_record_index = (fault_record_index + 1) % MAX_FAULT_RECORDS;
    }
    
    FaultRecord_t* record = &fault_records[fault_record_index];
    record->fault_code = fault_code;
    record->severity = severity;
    record->timestamp = HAL_GetTick();
    record->count = 1;
    record->motor_id = motor_id;
    record->additional_data = additional_data;
    record->active = true;
    record->acknowledged = false;
    
    return SYSTEM_OK;
}

/**
 * @brief Find fault record by fault code
 */
static FaultRecord_t* find_fault_record(uint32_t fault_code) {
    for (uint32_t i = 0; i < total_fault_records; i++) {
        if (fault_records[i].fault_code == fault_code) {
            return &fault_records[i];
        }
    }
    return NULL;
}

/**
 * @brief Check motor limits and parameters
 */
static SystemError_t check_motor_limits(uint8_t motor_id) {
    // This would normally check current motor parameters
    // For now, return OK as placeholder
    return SYSTEM_OK;
}

/**
 * @brief Check overall system health
 */
static SystemError_t check_system_health(void) {
    // Check stack usage (placeholder)
    // Check heap health (placeholder)
    // Check clock stability (placeholder)
    
    return SYSTEM_OK;
}

/**
 * @brief Check if fault is critical
 */
__attribute__((unused))
static bool is_fault_critical(uint32_t fault_code) {
    // Check motor fault criticality
    if ((fault_code & MOTOR_FAULT_ALL_MASK) && (fault_code & MOTOR_FAULT_CRITICAL_MASK)) {
        return true;
    }
    
    // Check system fault criticality
    if ((fault_code & SYSTEM_FAULT_ALL_MASK) && (fault_code & SYSTEM_FAULT_CRITICAL_MASK)) {
        return true;
    }
    
    return false;
}

/**
 * @brief Get human-readable fault description
 */
__attribute__((unused))
static const char* get_fault_description(uint32_t fault_code) {
    // Check motor faults first
    switch (fault_code) {
        case MOTOR_FAULT_OVERCURRENT: return "Motor Overcurrent";
        case MOTOR_FAULT_OVERVOLTAGE: return "Supply Overvoltage";
        case MOTOR_FAULT_UNDERVOLTAGE: return "Supply Undervoltage";
        case MOTOR_FAULT_OVERTEMPERATURE: return "Driver Overtemperature";
        case MOTOR_FAULT_L6470_FLAG: return "L6470 Driver Fault";
        case MOTOR_FAULT_EMERGENCY_STOP: return "Emergency Stop";
        case MOTOR_FAULT_WATCHDOG_TIMEOUT: return "Watchdog Timeout";
        case MOTOR_FAULT_POSITION_ERROR: return "Position Error";
    }
    
    // Check system faults
    switch (fault_code) {
        case SYSTEM_FAULT_CLOCK_FAILURE: return "Clock Failure";
        case SYSTEM_FAULT_POWER_FAILURE: return "Power Failure";
        case SYSTEM_FAULT_STACK_OVERFLOW: return "Stack Overflow";
        case SYSTEM_FAULT_SAFETY_VIOLATION: return "Safety Violation";
    }
    
    return "Unknown Fault";
    }
}
