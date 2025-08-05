# Safety Systems Developer Guide

## Overview
This guide provides comprehensive information for implementing and maintaining safety systems in the STM32H753ZI motor control system.

## Emergency Stop Systems

### Hardware Emergency Stop
```c
#include "safety/emergency_stop.h"

// Initialize emergency stop with hardware button
SystemError_t estop_init(void) {
    // Configure GPIO pin for emergency stop button
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ESTOP_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(ESTOP_GPIO_PORT, &GPIO_InitStruct);
    
    // Enable interrupt
    HAL_NVIC_SetPriority(ESTOP_EXTI_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(ESTOP_EXTI_IRQn);
    
    return SYSTEM_OK;
}

// Emergency stop interrupt handler
void EXTI15_10_IRQHandler(void) {
    if (__HAL_GPIO_EXTI_GET_IT(ESTOP_PIN) != RESET) {
        emergency_stop_triggered();
        __HAL_GPIO_EXTI_CLEAR_IT(ESTOP_PIN);
    }
}
```

### Software Emergency Stop
```c
// Trigger software emergency stop
SystemError_t software_emergency_stop(EmergencyStopReason_t reason) {
    // Log the reason
    log_emergency_stop(reason);
    
    // Stop all motors immediately
    motor_emergency_stop_all();
    
    // Set system state to fault
    system_set_state(SYSTEM_STATE_FAULT);
    
    // Trigger safety callbacks
    safety_emergency_callback(reason);
    
    return SYSTEM_OK;
}
```

## Fault Detection and Monitoring

### Motor Fault Detection
```c
#include "safety/fault_detection.h"

// Monitor L6470 fault flags
SystemError_t monitor_motor_faults(uint8_t motor_id) {
    uint16_t status = l6470_get_status(motor_id);
    
    // Check fault conditions
    if (status & L6470_STATUS_UVLO) {
        handle_motor_fault(motor_id, FAULT_UNDERVOLTAGE);
    }
    
    if (status & L6470_STATUS_TH_WRN) {
        handle_motor_fault(motor_id, FAULT_THERMAL_WARNING);
    }
    
    if (status & L6470_STATUS_TH_SD) {
        handle_motor_fault(motor_id, FAULT_THERMAL_SHUTDOWN);
    }
    
    if (status & L6470_STATUS_OCD) {
        handle_motor_fault(motor_id, FAULT_OVERCURRENT);
    }
    
    return SYSTEM_OK;
}
```

### Encoder Fault Detection
```c
// Monitor AS5600 encoder health
SystemError_t monitor_encoder_faults(uint8_t encoder_id) {
    uint16_t magnitude = as5600_read_magnitude(encoder_id);
    uint8_t status = as5600_read_status(encoder_id);
    
    // Check magnetic field strength
    if (magnitude < ENCODER_MIN_MAGNITUDE) {
        handle_encoder_fault(encoder_id, FAULT_WEAK_MAGNETIC_FIELD);
    }
    
    // Check status flags
    if (status & AS5600_STATUS_MH) {
        handle_encoder_fault(encoder_id, FAULT_MAGNETIC_FIELD_TOO_HIGH);
    }
    
    if (status & AS5600_STATUS_ML) {
        handle_encoder_fault(encoder_id, FAULT_MAGNETIC_FIELD_TOO_LOW);
    }
    
    return SYSTEM_OK;
}
```

## Watchdog Implementation

### System Watchdog
```c
#include "safety/watchdog.h"

// Initialize hardware watchdog
SystemError_t watchdog_init(void) {
    IWDG_HandleTypeDef hiwdg;
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
    hiwdg.Init.Reload = WATCHDOG_RELOAD_VALUE;
    
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        return ERROR_WATCHDOG_INIT_FAILED;
    }
    
    return SYSTEM_OK;
}

// Watchdog refresh task (call regularly)
void watchdog_refresh_task(void) {
    // Check system health
    if (system_health_check() == SYSTEM_OK) {
        HAL_IWDG_Refresh(&hiwdg);
    }
    // If health check fails, allow watchdog reset
}
```

### Software Watchdog for Tasks
```c
// Task watchdog system
typedef struct {
    uint32_t last_checkin;
    uint32_t timeout_ms;
    bool enabled;
} TaskWatchdog_t;

static TaskWatchdog_t task_watchdogs[MAX_TASKS];

// Task check-in
void task_watchdog_checkin(TaskID_t task_id) {
    if (task_id < MAX_TASKS && task_watchdogs[task_id].enabled) {
        task_watchdogs[task_id].last_checkin = HAL_GetTick();
    }
}

// Monitor task health
SystemError_t task_watchdog_monitor(void) {
    uint32_t current_time = HAL_GetTick();
    
    for (int i = 0; i < MAX_TASKS; i++) {
        if (task_watchdogs[i].enabled) {
            uint32_t elapsed = current_time - task_watchdogs[i].last_checkin;
            if (elapsed > task_watchdogs[i].timeout_ms) {
                handle_task_timeout(i);
            }
        }
    }
    
    return SYSTEM_OK;
}
```

## Safety State Management

### System States
```c
typedef enum {
    SYSTEM_STATE_INIT,          // System initializing
    SYSTEM_STATE_READY,         // Ready for operation
    SYSTEM_STATE_RUNNING,       // Normal operation
    SYSTEM_STATE_WARNING,       // Warning conditions present
    SYSTEM_STATE_FAULT,         // Fault detected - safe state
    SYSTEM_STATE_EMERGENCY      // Emergency stop activated
} SystemState_t;

// State transition with safety checks
SystemError_t system_set_state(SystemState_t new_state) {
    SystemState_t current_state = system_get_state();
    
    // Validate state transition
    if (!is_valid_state_transition(current_state, new_state)) {
        return ERROR_INVALID_STATE_TRANSITION;
    }
    
    // Perform state-specific actions
    switch (new_state) {
        case SYSTEM_STATE_FAULT:
            motor_stop_all();
            disable_power_outputs();
            log_fault_entry();
            break;
            
        case SYSTEM_STATE_EMERGENCY:
            motor_emergency_stop_all();
            disable_all_outputs();
            activate_emergency_indicators();
            break;
            
        case SYSTEM_STATE_READY:
            if (!safety_systems_check()) {
                return ERROR_SAFETY_CHECK_FAILED;
            }
            break;
    }
    
    // Update state
    system_state = new_state;
    notify_state_change(new_state);
    
    return SYSTEM_OK;
}
```

## Safety Testing Procedures

### Unit Testing
```c
// Example safety test
void test_emergency_stop_response_time(void) {
    uint32_t start_time, stop_time;
    
    // Start motor movement
    motor_start(MOTOR_1, 1000); // 1000 steps/sec
    
    // Trigger emergency stop and measure response
    start_time = HAL_GetTick();
    emergency_stop_trigger();
    
    // Wait for motor to stop
    while (motor_is_moving(MOTOR_1)) {
        // Wait
    }
    stop_time = HAL_GetTick();
    
    // Verify response time
    uint32_t response_time = stop_time - start_time;
    TEST_ASSERT_LESS_THAN(EMERGENCY_STOP_MAX_RESPONSE_TIME_MS, response_time);
}
```

### Integration Testing
```c
// Test complete safety system
void test_safety_system_integration(void) {
    // Initialize all safety systems
    TEST_ASSERT_EQUAL(SYSTEM_OK, safety_systems_init());
    
    // Test emergency stop chain
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_emergency_stop_chain());
    
    // Test fault detection
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_fault_detection_system());
    
    // Test watchdog recovery
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_watchdog_recovery());
    
    // Test state transitions
    TEST_ASSERT_EQUAL(SYSTEM_OK, test_safety_state_transitions());
}
```

## Configuration Requirements

### Safety Configuration (SSOT)
```c
// From src/config/safety_config.h
#define EMERGENCY_STOP_MAX_RESPONSE_TIME_MS    100
#define WATCHDOG_TIMEOUT_MS                    5000
#define FAULT_DETECTION_INTERVAL_MS            50
#define MAX_MOTOR_CURRENT_MA                   3000
#define MAX_MOTOR_TEMPERATURE_C                85
#define ENCODER_MIN_MAGNITUDE                  100
#define SAFETY_CHECK_INTERVAL_MS               100
```

## Best Practices

### 1. Fail-Safe Design
- Default to safe states on power-up
- Stop motors on any fault condition
- Use hardware-enforced safety where possible

### 2. Redundancy
- Implement both hardware and software emergency stops
- Use multiple fault detection methods
- Provide backup power monitoring

### 3. Response Time
- Emergency stop response < 100ms
- Fault detection cycle < 50ms
- Watchdog timeout appropriate for system

### 4. Testing
- Test all safety functions regularly
- Validate response times under load
- Verify fault conditions trigger correctly

### 5. Documentation
- Document all safety procedures
- Maintain safety test records
- Update safety analysis for changes

## Troubleshooting

### Common Issues
1. **Emergency Stop Not Responding**
   - Check GPIO configuration
   - Verify interrupt priority
   - Test button hardware

2. **False Fault Triggers**
   - Review threshold settings
   - Check noise on sensor lines
   - Validate timing parameters

3. **Watchdog Resets**
   - Monitor task execution times
   - Check for blocking operations
   - Verify watchdog refresh frequency

## Related Documentation
- [Emergency Stop Testing](../testing/emergency_stop_testing.md)
- [Fault Recovery Procedures](../procedures/fault_recovery.md)
- [Safety Validation](../validation/safety_validation.md)
