---
applyTo: "src/**"
description: "Error handling procedures and fault recovery implementation for STM32H753ZI stepper motor project"
---

# Error Handling and Fault Recovery Instructions

## Overview
This instruction file provides comprehensive guidance for implementing robust error handling and fault recovery mechanisms in the STM32H753ZI stepper motor control project.

## Enhanced Documentation Integration
**Reference Assets**: This instruction leverages comprehensive error handling and safety documentation:
- **STM32H7 Error Handling**: Complete error handling coverage with safety-critical implementation patterns
- **Copilot-Optimized Error Search**:
  ```bash
  # Search error handling concepts with semantic targeting
  python3 scripts/search_enhanced_docs.py concept "error_handling" --scope STM32H7
  
  # Find fault detection and recovery patterns
  python3 scripts/search_enhanced_docs.py concept "fault_detection" --scope all
  
  # Search safety system integration
  python3 scripts/search_enhanced_docs.py concept "safety_systems" --scope STM32H7
  
  # Find HAL error code definitions
  python3 scripts/search_enhanced_docs.py concept "hal_error_codes" --scope STM32H7
  ```
- **Cross-System Integration**: Links error handling with motor control, communication, and safety systems
- **Token-Efficient Reference**: Semantic categorization optimized for Copilot assistance with fault recovery

## Error Classification System

### Hierarchical Error Code Structure
Based on IEC 61508 functional safety classification and embedded systems best practices:

```c
// Error Code Format: 0xXXXX
// Bits [15:12] - Error Category (4 bits)
// Bits [11:8]  - Subsystem ID (4 bits)  
// Bits [7:0]   - Specific Error Code (8 bits)

// Error Categories (SSOT Reference)
#define ERROR_CATEGORY_SYSTEM           0x0000  // System-level errors
#define ERROR_CATEGORY_MOTOR            0x1000  // Motor control errors
#define ERROR_CATEGORY_ENCODER          0x2000  // Encoder/sensor errors
#define ERROR_CATEGORY_COMM             0x3000  // Communication errors
#define ERROR_CATEGORY_SAFETY           0x4000  // Safety system errors
#define ERROR_CATEGORY_MEMORY           0x5000  // Memory management errors
#define ERROR_CATEGORY_TIMING           0x6000  // Real-time constraint errors
#define ERROR_CATEGORY_CONFIG           0x7000  // Configuration errors

// Severity Levels
typedef enum {
    ERROR_SEVERITY_INFO = 0,        // Informational (no action required)
    ERROR_SEVERITY_WARNING,         // Warning (continue with caution)
    ERROR_SEVERITY_ERROR,           // Error (operation may fail)
    ERROR_SEVERITY_CRITICAL,        // Critical (system safety at risk)
    ERROR_SEVERITY_FATAL            // Fatal (immediate system shutdown)
} ErrorSeverity_t;
```

### Motor Control Error Codes
```c
// Motor Driver Errors (L6470 specific)
#define ERROR_MOTOR_OVERCURRENT         (ERROR_CATEGORY_MOTOR | 0x001)
#define ERROR_MOTOR_THERMAL_WARNING     (ERROR_CATEGORY_MOTOR | 0x002)
#define ERROR_MOTOR_THERMAL_SHUTDOWN    (ERROR_CATEGORY_MOTOR | 0x003)
#define ERROR_MOTOR_UNDERVOLTAGE        (ERROR_CATEGORY_MOTOR | 0x004)
#define ERROR_MOTOR_STEP_LOSS           (ERROR_CATEGORY_MOTOR | 0x005)
#define ERROR_MOTOR_POSITION_ERROR      (ERROR_CATEGORY_MOTOR | 0x006)
#define ERROR_MOTOR_VELOCITY_LIMIT      (ERROR_CATEGORY_MOTOR | 0x007)
#define ERROR_MOTOR_ACCELERATION_LIMIT  (ERROR_CATEGORY_MOTOR | 0x008)
#define ERROR_MOTOR_COMMUNICATION       (ERROR_CATEGORY_MOTOR | 0x009)
#define ERROR_MOTOR_INVALID_COMMAND     (ERROR_CATEGORY_MOTOR | 0x00A)
#define ERROR_MOTOR_NOT_HOMED           (ERROR_CATEGORY_MOTOR | 0x00B)
#define ERROR_MOTOR_LIMIT_SWITCH        (ERROR_CATEGORY_MOTOR | 0x00C)

// Error Severity Mapping for Motor Errors
const ErrorSeverity_t motor_error_severity[] = {
    [0x001] = ERROR_SEVERITY_CRITICAL,    // Overcurrent
    [0x002] = ERROR_SEVERITY_WARNING,     // Thermal warning
    [0x003] = ERROR_SEVERITY_CRITICAL,    // Thermal shutdown
    [0x004] = ERROR_SEVERITY_CRITICAL,    // Undervoltage
    [0x005] = ERROR_SEVERITY_ERROR,       // Step loss
    [0x006] = ERROR_SEVERITY_ERROR,       // Position error
    [0x007] = ERROR_SEVERITY_WARNING,     // Velocity limit
    [0x008] = ERROR_SEVERITY_WARNING,     // Acceleration limit
    [0x009] = ERROR_SEVERITY_ERROR,       // Communication
    [0x00A] = ERROR_SEVERITY_WARNING,     // Invalid command
    [0x00B] = ERROR_SEVERITY_WARNING,     // Not homed
    [0x00C] = ERROR_SEVERITY_CRITICAL     // Limit switch
};
```

### Encoder Error Codes
```c
// Encoder Errors (AS5600 specific)
#define ERROR_ENCODER_COMMUNICATION     (ERROR_CATEGORY_ENCODER | 0x001)
#define ERROR_ENCODER_MAGNETIC_FIELD    (ERROR_CATEGORY_ENCODER | 0x002)
#define ERROR_ENCODER_POSITION_JUMP     (ERROR_CATEGORY_ENCODER | 0x003)
#define ERROR_ENCODER_VELOCITY_ERROR    (ERROR_CATEGORY_ENCODER | 0x004)
#define ERROR_ENCODER_CALIBRATION       (ERROR_CATEGORY_ENCODER | 0x005)
#define ERROR_ENCODER_TIMEOUT           (ERROR_CATEGORY_ENCODER | 0x006)
#define ERROR_ENCODER_DATA_CORRUPT      (ERROR_CATEGORY_ENCODER | 0x007)
#define ERROR_ENCODER_OUT_OF_RANGE      (ERROR_CATEGORY_ENCODER | 0x008)

// Recovery Actions for Encoder Errors
typedef enum {
    ENCODER_RECOVERY_RETRY = 0,         // Retry reading
    ENCODER_RECOVERY_RECALIBRATE,       // Recalibrate encoder
    ENCODER_RECOVERY_USE_BACKUP,        // Use backup sensor
    ENCODER_RECOVERY_OPEN_LOOP,         // Switch to open-loop
    ENCODER_RECOVERY_STOP               // Stop affected motor
} EncoderRecoveryAction_t;
```

## Error Handling Framework

### Error Context Structure
```c
// Error Context for Debugging and Recovery
typedef struct {
    SystemError_t error_code;           // Primary error code
    uint32_t timestamp;                 // When error occurred
    uint8_t subsystem_id;               // Which subsystem reported error
    uint32_t context_data[4];           // Additional context information
    const char* file_name;              // Source file where error occurred
    uint32_t line_number;               // Line number where error occurred
    uint32_t stack_trace[8];            // Stack trace (if available)
} ErrorContext_t;

// Error reporting macro with context
#define REPORT_ERROR(code, ...) \
    do { \
        ErrorContext_t ctx = { \
            .error_code = (code), \
            .timestamp = HAL_GetTick(), \
            .file_name = __FILE__, \
            .line_number = __LINE__, \
            .context_data = {__VA_ARGS__} \
        }; \
        error_handler_report_error(&ctx); \
    } while(0)
```

### Error Handler Implementation
```c
// Central Error Handler
void error_handler_report_error(ErrorContext_t* context) {
    // Determine error severity
    ErrorSeverity_t severity = error_get_severity(context->error_code);
    
    // Log error with appropriate level
    switch (severity) {
        case ERROR_SEVERITY_FATAL:
        case ERROR_SEVERITY_CRITICAL:
            system_log_critical(context->error_code, context);
            break;
        case ERROR_SEVERITY_ERROR:
            system_log_error(context->error_code, context);
            break;
        case ERROR_SEVERITY_WARNING:
            system_log_warning(context->error_code, context);
            break;
        case ERROR_SEVERITY_INFO:
            system_log_info(context->error_code, context);
            break;
    }
    
    // Execute immediate response
    error_execute_immediate_response(context);
    
    // Schedule recovery action
    error_schedule_recovery(context);
    
    // Update error statistics
    error_update_statistics(context->error_code);
}
```

## Fault Recovery Strategies

### Recovery Action Matrix
```c
// Recovery Strategy Configuration (SSOT Reference)
typedef struct {
    SystemError_t error_code;           // Error code this applies to
    uint32_t max_attempts;              // Maximum recovery attempts
    uint32_t retry_delay_ms;            // Delay between attempts
    bool requires_manual_intervention;  // Needs manual intervention
    RecoveryFunction_t recovery_func;   // Recovery function pointer
} RecoveryStrategy_t;

// Recovery Strategy Database
const RecoveryStrategy_t recovery_strategies[] = {
    // Motor errors
    {ERROR_MOTOR_OVERCURRENT, 2, 1000, true, motor_recovery_overcurrent},
    {ERROR_MOTOR_THERMAL_SHUTDOWN, 1, 30000, true, motor_recovery_thermal},
    {ERROR_MOTOR_STEP_LOSS, 3, 500, false, motor_recovery_step_loss},
    {ERROR_MOTOR_COMMUNICATION, 5, 100, false, motor_recovery_communication},
    
    // Encoder errors
    {ERROR_ENCODER_COMMUNICATION, 3, 100, false, encoder_recovery_communication},
    {ERROR_ENCODER_MAGNETIC_FIELD, 1, 5000, true, encoder_recovery_magnet},
    {ERROR_ENCODER_POSITION_JUMP, 2, 1000, false, encoder_recovery_position},
    
    // Communication errors
    {ERROR_COMM_TIMEOUT, 3, 1000, false, comm_recovery_timeout},
    {ERROR_COMM_OVERRUN, 1, 100, false, comm_recovery_overrun},
    
    // Sentinel
    {0, 0, 0, false, NULL}
};
```

### Motor Recovery Functions
```c
// Motor Overcurrent Recovery
SystemError_t motor_recovery_overcurrent(uint8_t motor_id, ErrorContext_t* context) {
    // Step 1: Immediately disable motor output
    l6470_send_command(motor_id, L6470_CMD_HARDHIZ);
    
    // Step 2: Check motor wiring and load
    // (This would involve external diagnostics in real system)
    
    // Step 3: Reduce current limit and retry
    uint16_t current_limit = l6470_get_param(motor_id, L6470_REG_OCD_TH);
    if (current_limit > L6470_MIN_CURRENT_LIMIT) {
        current_limit = (current_limit * 80) / 100;  // Reduce by 20%
        l6470_set_param(motor_id, L6470_REG_OCD_TH, current_limit);
        
        // Re-enable motor with reduced current
        l6470_send_command(motor_id, L6470_CMD_RELEASESW);
        
        // Log recovery attempt
        system_log_info(INFO_MOTOR_RECOVERY_CURRENT_REDUCED, motor_id);
        return SYSTEM_OK;
    }
    
    // Recovery failed - requires manual intervention
    system_log_error(ERROR_MOTOR_RECOVERY_FAILED, motor_id);
    return ERROR_MOTOR_RECOVERY_FAILED;
}

// Motor Step Loss Recovery  
SystemError_t motor_recovery_step_loss(uint8_t motor_id, ErrorContext_t* context) {
    // Step 1: Stop motor and clear position error
    l6470_send_command(motor_id, L6470_CMD_SOFTSTOP);
    
    // Step 2: Read actual position from encoder
    float actual_position = as5600_read_angle(motor_id);
    
    // Step 3: Update motor position to match encoder
    int32_t encoder_steps = (int32_t)(actual_position * STEPS_PER_DEGREE);
    l6470_set_param(motor_id, L6470_REG_ABS_POS, encoder_steps);
    
    // Step 4: Reduce acceleration to prevent future step loss
    uint16_t acceleration = l6470_get_param(motor_id, L6470_REG_ACC);
    acceleration = (acceleration * 80) / 100;  // Reduce by 20%
    l6470_set_param(motor_id, L6470_REG_ACC, acceleration);
    
    system_log_info(INFO_MOTOR_RECOVERY_STEP_LOSS, motor_id);
    return SYSTEM_OK;
}
```

### Encoder Recovery Functions
```c
// Encoder Communication Recovery
SystemError_t encoder_recovery_communication(uint8_t encoder_id, ErrorContext_t* context) {
    I2C_HandleTypeDef* hi2c = encoder_get_i2c_handle(encoder_id);
    
    // Step 1: Reset I2C peripheral
    HAL_I2C_DeInit(hi2c);
    HAL_Delay(10);
    HAL_I2C_Init(hi2c);
    
    // Step 2: Attempt to read encoder ID register
    uint8_t test_data;
    if (HAL_I2C_Mem_Read(hi2c, AS5600_ADDRESS, AS5600_REG_STATUS,
                         I2C_MEMADD_SIZE_8BIT, &test_data, 1, 100) == HAL_OK) {
        system_log_info(INFO_ENCODER_RECOVERY_SUCCESS, encoder_id);
        return SYSTEM_OK;
    }
    
    // Step 3: Check if encoder is physically connected
    // (Would involve additional diagnostics in real system)
    
    system_log_warning(WARNING_ENCODER_RECOVERY_FAILED, encoder_id);
    return ERROR_ENCODER_RECOVERY_FAILED;
}

// Encoder Position Jump Recovery
SystemError_t encoder_recovery_position(uint8_t encoder_id, ErrorContext_t* context) {
    // Step 1: Take multiple position readings
    float positions[5];
    int valid_readings = 0;
    
    for (int i = 0; i < 5; i++) {
        HAL_Delay(10);  // Small delay between readings
        float pos = as5600_read_angle(encoder_id);
        if (pos >= 0.0f) {  // Valid reading
            positions[valid_readings++] = pos;
        }
    }
    
    if (valid_readings < 3) {
        return ERROR_ENCODER_RECOVERY_FAILED;
    }
    
    // Step 2: Calculate median position
    // (Simple sorting for median calculation)
    for (int i = 0; i < valid_readings - 1; i++) {
        for (int j = i + 1; j < valid_readings; j++) {
            if (positions[i] > positions[j]) {
                float temp = positions[i];
                positions[i] = positions[j];
                positions[j] = temp;
            }
        }
    }
    
    float median_position = positions[valid_readings / 2];
    
    // Step 3: Update encoder position filter
    encoder_update_position_filter(encoder_id, median_position);
    
    system_log_info(INFO_ENCODER_RECOVERY_POSITION, encoder_id);
    return SYSTEM_OK;
}
```

## Error Statistics and Monitoring

### Error Statistics Tracking
```c
// Error Statistics Structure
typedef struct {
    uint32_t total_errors;              // Total error count
    uint32_t error_count_by_type[256];  // Count per error type
    uint32_t last_error_time;           // Last error timestamp
    uint32_t mean_time_between_errors;  // MTBE calculation
    uint32_t recovery_success_count;    // Successful recoveries
    uint32_t recovery_failure_count;    // Failed recoveries
} ErrorStatistics_t;

// Update error statistics
void error_update_statistics(SystemError_t error_code) {
    ErrorStatistics_t* stats = &g_error_statistics;
    uint32_t current_time = HAL_GetTick();
    
    // Update total count
    stats->total_errors++;
    
    // Update count by type
    uint8_t error_type = error_code & 0xFF;
    stats->error_count_by_type[error_type]++;
    
    // Update MTBE calculation
    if (stats->last_error_time > 0) {
        uint32_t time_between = current_time - stats->last_error_time;
        stats->mean_time_between_errors = 
            (stats->mean_time_between_errors + time_between) / 2;
    }
    
    stats->last_error_time = current_time;
}
```

### Error Reporting and Diagnostics
```c
// Error Report Generation
void error_generate_report(char* buffer, size_t buffer_size) {
    ErrorStatistics_t* stats = &g_error_statistics;
    
    snprintf(buffer, buffer_size,
        "Error Report:\n"
        "Total Errors: %lu\n"
        "MTBE: %lu ms\n"
        "Recovery Success Rate: %.1f%%\n"
        "Top Error Types:\n",
        stats->total_errors,
        stats->mean_time_between_errors,
        (float)stats->recovery_success_count * 100.0f / 
        (stats->recovery_success_count + stats->recovery_failure_count));
    
    // Add most frequent error types
    error_add_top_errors_to_report(buffer, buffer_size);
}

// Error Log Entry Structure
typedef struct {
    uint32_t timestamp;                 // When error occurred
    SystemError_t error_code;           // Error code
    uint8_t subsystem_id;               // Affected subsystem
    uint32_t context[2];                // Additional context
} ErrorLogEntry_t;

// Circular error log buffer
#define ERROR_LOG_SIZE 100
static ErrorLogEntry_t error_log[ERROR_LOG_SIZE];
static uint32_t error_log_index = 0;

void error_log_add_entry(SystemError_t error_code, uint8_t subsystem_id, 
                        uint32_t context1, uint32_t context2) {
    ErrorLogEntry_t* entry = &error_log[error_log_index];
    
    entry->timestamp = HAL_GetTick();
    entry->error_code = error_code;
    entry->subsystem_id = subsystem_id;
    entry->context[0] = context1;
    entry->context[1] = context2;
    
    error_log_index = (error_log_index + 1) % ERROR_LOG_SIZE;
}
```

## Integration with Safety System

### Error-Triggered Safety Actions
```c
// Safety Action Configuration
typedef struct {
    SystemError_t error_code;           // Triggering error
    FailSafeAction_t safety_action;     // Required safety action
    uint32_t action_timeout_ms;         // Maximum time to execute action
    bool requires_confirmation;         // Needs external confirmation
} SafetyActionMapping_t;

// Safety action mappings
const SafetyActionMapping_t safety_actions[] = {
    {ERROR_MOTOR_OVERCURRENT, FAILSAFE_MOTOR_HIZ, 10, false},
    {ERROR_MOTOR_THERMAL_SHUTDOWN, FAILSAFE_MOTOR_HIZ, 10, false},
    {ERROR_ENCODER_COMMUNICATION, FAILSAFE_MOTOR_STOP, 50, false},
    {ERROR_SAFETY_EMERGENCY_STOP, FAILSAFE_SYSTEM_RESET, 5, true},
    // ... more mappings
};

// Execute safety action based on error
void error_execute_safety_action(SystemError_t error_code) {
    for (int i = 0; safety_actions[i].error_code != 0; i++) {
        if (safety_actions[i].error_code == error_code) {
            safety_execute_failsafe_action(safety_actions[i].safety_action,
                                         safety_actions[i].action_timeout_ms);
            break;
        }
    }
}
```

Remember: **Robust error handling is essential for safe operation. Always assume errors can occur and design for graceful degradation and recovery.**
