---
applyTo: "src/**"
description: "Safety system configuration including watchdog, fault detection, and fail-safe mechanisms"
---

# Safety System Configuration Instructions

## Overview
This instruction file provides comprehensive guidance for implementing safety systems in the STM32H753ZI stepper motor control project, emphasizing fail-safe operation and fault recovery.

## Enhanced Documentation Integration
**Reference Assets**: This instruction leverages comprehensive safety and watchdog documentation:
- **STM32H7 Safety Documentation**: Complete coverage across 3,997 files with semantic safety categorization
- **Copilot-Optimized Safety Search**:
  ```bash
  # Search safety system concepts with token efficiency
  python3 scripts/search_enhanced_docs.py concept "safety_systems" --scope STM32H7
  
  # Find watchdog peripheral documentation
  python3 scripts/search_enhanced_docs.py peripheral IWDG --scope STM32H7
  
  # Search fault detection mechanisms
  python3 scripts/search_enhanced_docs.py concept "fault_detection" --scope all
  
  # Find safety-critical HAL functions
  python3 scripts/search_enhanced_docs.py function HAL_IWDG_Init --scope STM32H7
  ```
- **Industrial Safety Standards**: IEC 61508 functional safety integration with STM32H7 capabilities
- **Cross-Domain Safety**: Unified search across motor control, communication, and system safety documentation

## Safety-First Design Principles

### Fundamental Safety Requirements
Based on industrial motor control best practices and IEC 61508 functional safety standards:

```c
// Safety Configuration Priorities (SSOT Reference)
#define SAFETY_PRIORITY_EMERGENCY_STOP  0       // Highest priority
#define SAFETY_PRIORITY_MOTOR_FAULT     1       // Motor driver faults  
#define SAFETY_PRIORITY_SENSOR_FAULT    2       // Encoder/sensor faults
#define SAFETY_PRIORITY_COMM_FAULT      3       // Communication timeouts
#define SAFETY_PRIORITY_SYSTEM_FAULT    4       // General system faults

// Safety Response Times (Maximum allowed response times)
#define SAFETY_EMERGENCY_RESPONSE_MS    10      // Emergency stop response
#define SAFETY_FAULT_DETECTION_MS       50      // Fault detection latency
#define SAFETY_RECOVERY_TIMEOUT_MS      5000    // Recovery attempt timeout
```

### Fail-Safe State Definition
```c
// System Fail-Safe State Configuration
typedef enum {
    FAILSAFE_MOTOR_STOP = 0,        // Stop all motors immediately
    FAILSAFE_MOTOR_HIZ,             // High impedance (power off)
    FAILSAFE_MOTOR_HOLD,            // Hold position with reduced current
    FAILSAFE_SYSTEM_RESET           // Complete system reset
} FailSafeAction_t;

// Default fail-safe actions for different fault types
#define FAILSAFE_DEFAULT_ACTION         FAILSAFE_MOTOR_HIZ
#define FAILSAFE_OVERCURRENT_ACTION     FAILSAFE_MOTOR_HIZ
#define FAILSAFE_THERMAL_ACTION         FAILSAFE_MOTOR_HIZ
#define FAILSAFE_ENCODER_FAULT_ACTION   FAILSAFE_MOTOR_STOP
#define FAILSAFE_COMM_TIMEOUT_ACTION    FAILSAFE_MOTOR_STOP
```

## Watchdog Timer Configuration

### Independent Watchdog (IWDG) Setup
```c
// IWDG Configuration (SSOT Reference)
#define IWDG_PRESCALER              IWDG_PRESCALER_32   // 32kHz / 32 = 1kHz
#define IWDG_RELOAD_VALUE           1000                // 1 second timeout
#define IWDG_WINDOW_VALUE           0                   // No window (full period)

// Watchdog Service Intervals
#define IWDG_SERVICE_INTERVAL_MS    500                 // Service every 500ms
#define IWDG_GRACE_PERIOD_MS        100                 // Grace period for critical operations

// Watchdog Service Strategy
void safety_service_watchdog(void) {
    static uint32_t last_service_time = 0;
    uint32_t current_time = HAL_GetTick();
    
    // Only service if minimum interval has passed
    if ((current_time - last_service_time) >= IWDG_SERVICE_INTERVAL_MS) {
        // Perform safety checks before servicing watchdog
        if (safety_system_health_check() == SYSTEM_OK) {
            HAL_IWDG_Refresh(&hiwdg);
            last_service_time = current_time;
        }
        // If health check fails, let watchdog reset the system
    }
}
```

### Window Watchdog (WWDG) for Critical Operations
```c
// WWDG Configuration for time-critical safety operations
#define WWDG_PRESCALER              WWDG_PRESCALER_8    // 32MHz / 4096 / 8
#define WWDG_WINDOW                 80                  // Window value
#define WWDG_COUNTER                127                 // Counter value

// Use WWDG for critical control loop timing
void safety_control_loop_watchdog(void) {
    // Service WWDG within specific time window
    HAL_WWDG_Refresh(&hwwdg);
}
```

## Motor Driver Fault Detection

### L6470 Fault Monitoring Configuration
```c
// L6470 Fault Detection Thresholds (SSOT Reference)
#define MOTOR_OVERCURRENT_THRESHOLD     3000            // mA
#define MOTOR_STALL_THRESHOLD           2000            // mA  
#define MOTOR_THERMAL_WARNING_TEMP      125             // °C (typical)
#define MOTOR_THERMAL_SHUTDOWN_TEMP     150             // °C (typical)
#define MOTOR_UNDERVOLTAGE_THRESHOLD    8000            // mV

// Fault Status Monitoring
typedef struct {
    uint16_t status_register;       // Latest L6470 status
    uint32_t fault_count;           // Number of faults detected
    uint32_t last_fault_time;       // Timestamp of last fault
    FailSafeAction_t last_action;   // Last fail-safe action taken
} MotorFaultStatus_t;

// Fault Detection Implementation
SystemError_t safety_check_motor_faults(uint8_t motor_id) {
    uint16_t status = l6470_get_status(motor_id);
    MotorFaultStatus_t *fault_status = &motor_fault_status[motor_id];
    
    fault_status->status_register = status;
    
    // Check for critical faults
    if (status & L6470_STATUS_OCD) {
        system_log_fault(ERROR_MOTOR_OVERCURRENT, motor_id);
        safety_execute_failsafe(motor_id, FAILSAFE_OVERCURRENT_ACTION);
        return ERROR_MOTOR_OVERCURRENT;
    }
    
    if (status & L6470_STATUS_TH_SD) {
        system_log_fault(ERROR_MOTOR_THERMAL_SHUTDOWN, motor_id);
        safety_execute_failsafe(motor_id, FAILSAFE_THERMAL_ACTION);
        return ERROR_MOTOR_THERMAL_SHUTDOWN;
    }
    
    if (status & L6470_STATUS_TH_WRN) {
        system_log_warning(WARNING_MOTOR_THERMAL_WARNING, motor_id);
        // Reduce current but continue operation
        l6470_reduce_current(motor_id, 0.8f);
    }
    
    if (status & L6470_STATUS_UVLO) {
        system_log_fault(ERROR_MOTOR_UNDERVOLTAGE, motor_id);
        safety_execute_failsafe(motor_id, FAILSAFE_DEFAULT_ACTION);
        return ERROR_MOTOR_UNDERVOLTAGE;
    }
    
    if (status & (L6470_STATUS_STEP_LOSS_A | L6470_STATUS_STEP_LOSS_B)) {
        system_log_fault(ERROR_MOTOR_STEP_LOSS, motor_id);
        safety_execute_failsafe(motor_id, FAILSAFE_MOTOR_STOP);
        return ERROR_MOTOR_STEP_LOSS;
    }
    
    return SYSTEM_OK;
}
```

## Encoder Fault Detection

### AS5600 Encoder Validation
```c
// Encoder Validation Thresholds (SSOT Reference)
#define ENCODER_MIN_MAGNITUDE           100             // Minimum magnetic field strength
#define ENCODER_MAX_MAGNITUDE           4000            // Maximum magnetic field strength
#define ENCODER_POSITION_JUMP_THRESHOLD 180.0f         // Maximum position jump (degrees)
#define ENCODER_VELOCITY_LIMIT          1800.0f        // Maximum velocity (deg/s)
#define ENCODER_TIMEOUT_MS              100             // I2C communication timeout

// Encoder Health Monitoring
typedef struct {
    float last_position;            // Last valid position reading
    float last_velocity;            // Last computed velocity
    uint32_t last_update_time;      // Timestamp of last update
    uint16_t magnitude;             // Magnetic field magnitude
    uint8_t status_flags;           // AS5600 status flags
    uint32_t error_count;           // Communication error count
} EncoderHealth_t;

SystemError_t safety_validate_encoder_reading(uint8_t encoder_id, float new_position) {
    EncoderHealth_t *health = &encoder_health[encoder_id];
    uint32_t current_time = HAL_GetTick();
    
    // Check for communication timeout
    if ((current_time - health->last_update_time) > ENCODER_TIMEOUT_MS) {
        system_log_fault(ERROR_ENCODER_TIMEOUT, encoder_id);
        return ERROR_ENCODER_TIMEOUT;
    }
    
    // Check for unrealistic position jump
    float position_delta = fabs(new_position - health->last_position);
    if (position_delta > ENCODER_POSITION_JUMP_THRESHOLD) {
        // Handle 0/360 degree wraparound
        float wrapped_delta = 360.0f - position_delta;
        if (wrapped_delta > ENCODER_POSITION_JUMP_THRESHOLD) {
            system_log_fault(ERROR_ENCODER_POSITION_JUMP, encoder_id);
            return ERROR_ENCODER_POSITION_JUMP;
        }
    }
    
    // Check magnetic field strength
    if (health->magnitude < ENCODER_MIN_MAGNITUDE || 
        health->magnitude > ENCODER_MAX_MAGNITUDE) {
        system_log_fault(ERROR_ENCODER_MAGNETIC_FIELD, encoder_id);
        return ERROR_ENCODER_MAGNETIC_FIELD;
    }
    
    // Update health status
    health->last_position = new_position;
    health->last_update_time = current_time;
    
    return SYSTEM_OK;
}
```

## Communication Fault Detection

### Communication Timeout Monitoring
```c
// Communication Timeout Configuration (SSOT Reference)
#define COMM_UART_TIMEOUT_MS            1000            // UART command timeout
#define COMM_CAN_HEARTBEAT_TIMEOUT_MS   500             // CAN heartbeat timeout
#define COMM_TCP_CONNECTION_TIMEOUT_MS  5000            // TCP connection timeout
#define COMM_I2C_TRANSACTION_TIMEOUT_MS 100             // I2C transaction timeout
#define COMM_SPI_TRANSACTION_TIMEOUT_MS 10              // SPI transaction timeout

// Communication Health Monitoring
typedef struct {
    uint32_t last_activity_time;    // Last successful communication
    uint32_t timeout_count;         // Number of timeouts
    uint32_t error_count;           // Number of errors
    bool is_critical;               // Is this interface critical for safety?
} CommHealth_t;

SystemError_t safety_check_communication_health(void) {
    uint32_t current_time = HAL_GetTick();
    SystemError_t result = SYSTEM_OK;
    
    // Check critical communication interfaces
    for (int i = 0; i < COMM_INTERFACE_COUNT; i++) {
        CommHealth_t *health = &comm_health[i];
        
        if (health->is_critical) {
            uint32_t timeout_threshold = comm_timeout_thresholds[i];
            
            if ((current_time - health->last_activity_time) > timeout_threshold) {
                system_log_fault(ERROR_COMM_TIMEOUT, i);
                
                // Execute fail-safe action for communication timeout
                safety_execute_failsafe(0xFF, FAILSAFE_COMM_TIMEOUT_ACTION);
                result = ERROR_COMM_TIMEOUT;
            }
        }
    }
    
    return result;
}
```

## Emergency Stop Implementation

### Hardware Emergency Stop
```c
// Emergency Stop Configuration (SSOT Reference)
#define ESTOP_INPUT_PIN                 GPIO_PIN_0      // Emergency stop input
#define ESTOP_INPUT_PORT                GPIOC           // Emergency stop port
#define ESTOP_ACTIVE_STATE              GPIO_PIN_RESET  // Active low
#define ESTOP_DEBOUNCE_TIME_MS          10              // Debounce time

// Emergency Stop Handler (Interrupt-driven)
void safety_emergency_stop_handler(void) {
    // Disable all PWM outputs immediately
    __HAL_TIM_DISABLE(&htim1);
    
    // Set all motor drivers to high impedance
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        l6470_emergency_stop(i);
    }
    
    // Set system state to emergency stop
    g_system_state.safety.emergency_stop_active = true;
    g_system_state.safety.last_emergency_stop_time = HAL_GetTick();
    
    // Log emergency stop event
    system_log_emergency(ERROR_SAFETY_EMERGENCY_STOP);
    
    // Disable all non-critical interrupts
    safety_disable_non_critical_operations();
}
```

### Software Emergency Stop
```c
// Software-triggered emergency stop
void safety_software_emergency_stop(SystemError_t reason) {
    // Call hardware emergency stop routine
    safety_emergency_stop_handler();
    
    // Additional software cleanup
    safety_clear_command_queues();
    safety_notify_external_systems(reason);
    
    // Set recovery requirements
    g_system_state.safety.recovery_required = true;
    g_system_state.safety.stop_reason = reason;
}
```

## Safety System Health Monitoring

### Comprehensive System Health Check
```c
// System Health Check (called before watchdog service)
SystemError_t safety_system_health_check(void) {
    SystemError_t health_status = SYSTEM_OK;
    
    // Check motor driver health
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        SystemError_t motor_status = safety_check_motor_faults(i);
        if (motor_status != SYSTEM_OK) {
            health_status = motor_status;
        }
    }
    
    // Check encoder health
    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        SystemError_t encoder_status = safety_check_encoder_health(i);
        if (encoder_status != SYSTEM_OK) {
            health_status = encoder_status;
        }
    }
    
    // Check communication health
    SystemError_t comm_status = safety_check_communication_health();
    if (comm_status != SYSTEM_OK) {
        health_status = comm_status;
    }
    
    // Check system resources
    SystemError_t resource_status = safety_check_system_resources();
    if (resource_status != SYSTEM_OK) {
        health_status = resource_status;
    }
    
    return health_status;
}
```

### System Resource Monitoring
```c
// System Resource Thresholds (SSOT Reference)
#define SAFETY_MAX_CPU_USAGE_PERCENT    85              // Maximum CPU usage
#define SAFETY_MIN_FREE_MEMORY_BYTES    1024            // Minimum free memory
#define SAFETY_MAX_STACK_USAGE_PERCENT  80              // Maximum stack usage

SystemError_t safety_check_system_resources(void) {
    // Check CPU usage (if RTOS is used)
    #ifdef USE_FREERTOS
    if (osGetCPUUsage() > SAFETY_MAX_CPU_USAGE_PERCENT) {
        system_log_warning(WARNING_HIGH_CPU_USAGE);
        return WARNING_HIGH_CPU_USAGE;
    }
    #endif
    
    // Check stack usage
    uint32_t stack_usage = safety_get_stack_usage();
    if (stack_usage > SAFETY_MAX_STACK_USAGE_PERCENT) {
        system_log_warning(WARNING_HIGH_STACK_USAGE);
        return WARNING_HIGH_STACK_USAGE;
    }
    
    return SYSTEM_OK;
}
```

## Fault Recovery Procedures

### Automatic Recovery Strategy
```c
// Recovery Configuration (SSOT Reference)
#define RECOVERY_MAX_ATTEMPTS           3               // Maximum recovery attempts
#define RECOVERY_BACKOFF_BASE_MS        1000            // Base backoff time
#define RECOVERY_BACKOFF_MULTIPLIER     2               // Exponential backoff

typedef enum {
    RECOVERY_NONE = 0,
    RECOVERY_SOFT_RESET,            // Reset affected subsystem
    RECOVERY_HARD_RESET,            // Reset entire system
    RECOVERY_MANUAL_INTERVENTION    // Requires manual intervention
} RecoveryStrategy_t;

void safety_attempt_recovery(SystemError_t fault_code) {
    static uint32_t recovery_attempts[MAX_FAULT_TYPES] = {0};
    static uint32_t last_recovery_time[MAX_FAULT_TYPES] = {0};
    
    uint8_t fault_index = (fault_code >> 12) & 0xF;  // Extract fault category
    uint32_t current_time = HAL_GetTick();
    
    // Check if maximum recovery attempts exceeded
    if (recovery_attempts[fault_index] >= RECOVERY_MAX_ATTEMPTS) {
        system_log_fault(ERROR_SAFETY_RECOVERY_FAILED, fault_code);
        safety_require_manual_intervention(fault_code);
        return;
    }
    
    // Implement exponential backoff
    uint32_t backoff_time = RECOVERY_BACKOFF_BASE_MS * 
                           (1 << recovery_attempts[fault_index]);
    
    if ((current_time - last_recovery_time[fault_index]) < backoff_time) {
        return;  // Too soon to retry
    }
    
    // Attempt recovery based on fault type
    RecoveryStrategy_t strategy = safety_get_recovery_strategy(fault_code);
    bool recovery_success = false;
    
    switch (strategy) {
        case RECOVERY_SOFT_RESET:
            recovery_success = safety_soft_reset_subsystem(fault_code);
            break;
            
        case RECOVERY_HARD_RESET:
            safety_schedule_hard_reset(fault_code);
            recovery_success = true;  // Will be validated after reset
            break;
            
        default:
            recovery_success = false;
            break;
    }
    
    // Update recovery tracking
    recovery_attempts[fault_index]++;
    last_recovery_time[fault_index] = current_time;
    
    if (recovery_success) {
        system_log_info(INFO_RECOVERY_SUCCESSFUL, fault_code);
        recovery_attempts[fault_index] = 0;  // Reset counter on success
    }
}
```

## Safety System Integration

### Main Safety Loop Implementation
```c
// Safety system main loop (highest priority task)
void safety_main_loop(void) {
    static uint32_t last_health_check = 0;
    static uint32_t last_watchdog_service = 0;
    uint32_t current_time = HAL_GetTick();
    
    // Service watchdog at regular intervals
    if ((current_time - last_watchdog_service) >= IWDG_SERVICE_INTERVAL_MS) {
        safety_service_watchdog();
        last_watchdog_service = current_time;
    }
    
    // Perform comprehensive health check
    if ((current_time - last_health_check) >= SAFETY_FAULT_DETECTION_MS) {
        SystemError_t health_status = safety_system_health_check();
        
        if (health_status != SYSTEM_OK) {
            safety_attempt_recovery(health_status);
        }
        
        last_health_check = current_time;
    }
    
    // Check for emergency stop conditions
    safety_check_emergency_conditions();
    
    // Update safety system state
    safety_update_system_state();
}
```

Remember: **Safety is paramount - when in doubt, fail safe. It's better to stop the system than to risk damage or injury.**
