---
applyTo: "src/{safety,controllers}/**/*.{c,h}"
description: "Safety systems, real-time control, timing management, and fail-safe mechanisms for STM32H753ZI stepper motor project"
---

# Safety & Real-Time Instructions

## Overview
This instruction file provides comprehensive guidance for safety-critical systems and real-time control in the STM32H753ZI stepper motor project, including watchdog systems, fault detection, timing management, and fail-safe mechanisms.

## Safety System Architecture

### Safety Requirements and Compliance
```c
// Safety Integrity Level (SIL) definitions
typedef enum {
    SIL_NONE = 0,                      // No safety requirements
    SIL_1 = 1,                         // Low safety integrity
    SIL_2 = 2,                         // Medium safety integrity  
    SIL_3 = 3,                         // High safety integrity
    SIL_4 = 4                          // Very high safety integrity
} SafetyIntegrityLevel_t;

// Safety function classifications
typedef enum {
    SAFETY_FUNC_EMERGENCY_STOP,        // Emergency stop function
    SAFETY_FUNC_SPEED_MONITORING,      // Speed limit monitoring
    SAFETY_FUNC_POSITION_LIMITING,     // Position limit enforcement
    SAFETY_FUNC_TORQUE_LIMITING,       // Torque/current limiting
    SAFETY_FUNC_TEMPERATURE_MONITORING, // Temperature monitoring
    SAFETY_FUNC_WATCHDOG,              // System watchdog
    SAFETY_FUNC_COMMUNICATION_MONITOR, // Communication integrity
    SAFETY_FUNC_SENSOR_VALIDATION      // Sensor plausibility checking
} SafetyFunction_t;

// Safety configuration structure
typedef struct {
    SafetyFunction_t function;         // Safety function type
    SafetyIntegrityLevel_t sil_level;  // Required SIL level
    uint32_t reaction_time_ms;         // Maximum reaction time
    bool enabled;                      // Function enabled/disabled
    float threshold_value;             // Safety threshold
    uint16_t fault_tolerance;          // Fault tolerance count
    bool automatic_reset;             // Auto-reset capability
} SafetyConfig_t;
```

### Emergency Stop System
```c
/**
 * @brief Initialize emergency stop system
 * @return SystemError_t Success or error code
 */
SystemError_t emergency_stop_init(void) {
    // Configure emergency stop input with interrupt
    gpio_init_interrupt(ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN, 
                       GPIO_MODE_IT_FALLING, GPIO_PULLUP, 
                       EMERGENCY_STOP_PRIORITY);
    
    // Initialize emergency stop state
    emergency_stop_state.active = false;
    emergency_stop_state.latched = false;
    emergency_stop_state.source = ESTOP_SOURCE_NONE;
    emergency_stop_state.timestamp = 0;
    
    // Configure safety relay outputs if available
    gpio_init_output(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN, GPIO_PIN_SET);
    gpio_init_output(SAFETY_RELAY2_PORT, SAFETY_RELAY2_PIN, GPIO_PIN_SET);
    
    return SYSTEM_OK;
}

/**
 * @brief Execute emergency stop sequence
 * @param source Source of emergency stop trigger
 * @return SystemError_t Success or error code
 */
SystemError_t execute_emergency_stop(EmergencyStopSource_t source) {
    uint32_t start_time = get_microsecond_timer();
    
    // Immediately disable motor power (hardware level)
    HAL_GPIO_WritePin(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SAFETY_RELAY2_PORT, SAFETY_RELAY2_PIN, GPIO_PIN_RESET);
    
    // Stop all motor motion commands
    for (uint8_t i = 0; i < MAX_MOTORS; i++) {
        l6470_immediate_stop(i);
    }
    
    // Update emergency stop state
    emergency_stop_state.active = true;
    emergency_stop_state.latched = true;
    emergency_stop_state.source = source;
    emergency_stop_state.timestamp = HAL_GetTick();
    
    // Set system to safe state
    set_system_state(SYSTEM_STATE_FAULT);
    
    // Log emergency stop event
    log_safety_event(SAFETY_EVENT_EMERGENCY_STOP, source, 
                    get_microsecond_timer() - start_time);
    
    // Notify all systems of emergency stop
    broadcast_emergency_stop();
    
    return SYSTEM_OK;
}

/**
 * @brief Emergency stop interrupt handler
 */
void emergency_stop_interrupt_handler(void) {
    // Critical: Execute emergency stop immediately
    execute_emergency_stop(ESTOP_SOURCE_BUTTON);
    
    // Clear interrupt flag
    HAL_GPIO_EXTI_IRQHandler(ESTOP_BUTTON_PIN);
}

/**
 * @brief Reset emergency stop (requires manual action)
 * @return SystemError_t Success or error code
 */
SystemError_t reset_emergency_stop(void) {
    // Check if reset is allowed
    if (!emergency_stop_state.latched) {
        return ERROR_SAFETY_INVALID_RESET;
    }
    
    // Verify physical button is released
    if (HAL_GPIO_ReadPin(ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN) == GPIO_PIN_RESET) {
        return ERROR_SAFETY_BUTTON_STILL_PRESSED;
    }
    
    // Perform system checks before reset
    SystemError_t check_result = perform_safety_checks();
    if (check_result != SYSTEM_OK) {
        return check_result;
    }
    
    // Reset emergency stop state
    emergency_stop_state.active = false;
    emergency_stop_state.latched = false;
    emergency_stop_state.source = ESTOP_SOURCE_NONE;
    
    // Re-enable safety relays
    HAL_GPIO_WritePin(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SAFETY_RELAY2_PORT, SAFETY_RELAY2_PIN, GPIO_PIN_SET);
    
    // Return to ready state
    set_system_state(SYSTEM_STATE_READY);
    
    log_safety_event(SAFETY_EVENT_EMERGENCY_STOP_RESET, ESTOP_SOURCE_MANUAL, 0);
    
    return SYSTEM_OK;
}
```

### Watchdog System Implementation
```c
// Watchdog configuration
typedef struct {
    uint32_t timeout_ms;               // Watchdog timeout period
    uint32_t warning_threshold_ms;     // Warning before timeout
    bool enabled;                      // Watchdog enabled
    uint32_t last_refresh;            // Last refresh timestamp
    uint32_t refresh_count;           // Total refresh count
    uint32_t timeout_count;           // Timeout event count
} WatchdogConfig_t;

/**
 * @brief Initialize independent watchdog timer
 * @return SystemError_t Success or error code
 */
SystemError_t watchdog_init(void) {
    IWDG_HandleTypeDef* hiwdg = &hiwdg;
    
    // Configure IWDG for ~1 second timeout
    hiwdg->Instance = IWDG;
    hiwdg->Init.Prescaler = IWDG_PRESCALER_32;    // 32 kHz LSI / 32 = 1 kHz
    hiwdg->Init.Reload = 1000;                    // 1000 ms timeout
    
    if (HAL_IWDG_Init(hiwdg) != HAL_OK) {
        return ERROR_SAFETY_WATCHDOG_INIT_FAILED;
    }
    
    // Initialize software watchdog tracking
    watchdog_config.timeout_ms = WATCHDOG_TIMEOUT_MS;
    watchdog_config.warning_threshold_ms = WATCHDOG_WARNING_THRESHOLD_MS;
    watchdog_config.enabled = true;
    watchdog_config.last_refresh = HAL_GetTick();
    watchdog_config.refresh_count = 0;
    watchdog_config.timeout_count = 0;
    
    return SYSTEM_OK;
}

/**
 * @brief Refresh watchdog timer (call regularly from main loop)
 * @return SystemError_t Success or error code
 */
SystemError_t watchdog_refresh(void) {
    if (!watchdog_config.enabled) {
        return SYSTEM_OK;
    }
    
    uint32_t current_time = HAL_GetTick();
    uint32_t elapsed = current_time - watchdog_config.last_refresh;
    
    // Check if we're approaching timeout (warning)
    if (elapsed > watchdog_config.warning_threshold_ms) {
        log_safety_event(SAFETY_EVENT_WATCHDOG_WARNING, 0, elapsed);
    }
    
    // Refresh hardware watchdog
    HAL_IWDG_Refresh(&hiwdg);
    
    // Update software tracking
    watchdog_config.last_refresh = current_time;
    watchdog_config.refresh_count++;
    
    return SYSTEM_OK;
}

/**
 * @brief Check watchdog status and health
 * @return SystemError_t Success or error code
 */
SystemError_t watchdog_check_health(void) {
    uint32_t current_time = HAL_GetTick();
    uint32_t elapsed = current_time - watchdog_config.last_refresh;
    
    if (elapsed > watchdog_config.timeout_ms) {
        // Watchdog timeout detected (should not happen if system is healthy)
        watchdog_config.timeout_count++;
        log_safety_event(SAFETY_EVENT_WATCHDOG_TIMEOUT, 0, elapsed);
        
        // Execute emergency procedures
        execute_emergency_stop(ESTOP_SOURCE_WATCHDOG);
        
        return ERROR_SAFETY_WATCHDOG_TIMEOUT;
    }
    
    return SYSTEM_OK;
}
```

### Real-Time Monitoring Systems
```c
// Real-time monitoring structure
typedef struct {
    float current_value;               // Current measured value
    float safe_min;                   // Minimum safe value
    float safe_max;                   // Maximum safe value
    float warning_min;                // Warning threshold (min)
    float warning_max;                // Warning threshold (max)
    uint32_t violation_count;         // Safety violation count
    uint32_t warning_count;           // Warning count
    timestamp_t last_violation;       // Last violation time
    bool enabled;                     // Monitoring enabled
} SafetyMonitor_t;

// Monitoring channels
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

static SafetyMonitor_t safety_monitors[MONITOR_COUNT];

/**
 * @brief Initialize safety monitoring system
 * @return SystemError_t Success or error code
 */
SystemError_t safety_monitoring_init(void) {
    // Initialize motor current monitors
    safety_monitors[MONITOR_MOTOR1_CURRENT].safe_min = 0.0f;
    safety_monitors[MONITOR_MOTOR1_CURRENT].safe_max = MOTOR_MAX_CURRENT_A;
    safety_monitors[MONITOR_MOTOR1_CURRENT].warning_max = MOTOR_MAX_CURRENT_A * 0.9f;
    safety_monitors[MONITOR_MOTOR1_CURRENT].enabled = true;
    
    safety_monitors[MONITOR_MOTOR2_CURRENT] = safety_monitors[MONITOR_MOTOR1_CURRENT];
    
    // Initialize speed monitors
    safety_monitors[MONITOR_MOTOR1_SPEED].safe_min = -MOTOR_MAX_SPEED_RPM;
    safety_monitors[MONITOR_MOTOR1_SPEED].safe_max = MOTOR_MAX_SPEED_RPM;
    safety_monitors[MONITOR_MOTOR1_SPEED].warning_max = MOTOR_MAX_SPEED_RPM * 0.95f;
    safety_monitors[MONITOR_MOTOR1_SPEED].warning_min = -MOTOR_MAX_SPEED_RPM * 0.95f;
    safety_monitors[MONITOR_MOTOR1_SPEED].enabled = true;
    
    safety_monitors[MONITOR_MOTOR2_SPEED] = safety_monitors[MONITOR_MOTOR1_SPEED];
    
    // Initialize position monitors
    safety_monitors[MONITOR_MOTOR1_POSITION].safe_min = MOTOR1_MIN_POSITION_DEG;
    safety_monitors[MONITOR_MOTOR1_POSITION].safe_max = MOTOR1_MAX_POSITION_DEG;
    safety_monitors[MONITOR_MOTOR1_POSITION].warning_min = MOTOR1_MIN_POSITION_DEG + 5.0f;
    safety_monitors[MONITOR_MOTOR1_POSITION].warning_max = MOTOR1_MAX_POSITION_DEG - 5.0f;
    safety_monitors[MONITOR_MOTOR1_POSITION].enabled = true;
    
    safety_monitors[MONITOR_MOTOR2_POSITION].safe_min = MOTOR2_MIN_POSITION_DEG;
    safety_monitors[MONITOR_MOTOR2_POSITION].safe_max = MOTOR2_MAX_POSITION_DEG;
    safety_monitors[MONITOR_MOTOR2_POSITION].warning_min = MOTOR2_MIN_POSITION_DEG + 5.0f;
    safety_monitors[MONITOR_MOTOR2_POSITION].warning_max = MOTOR2_MAX_POSITION_DEG - 5.0f;
    safety_monitors[MONITOR_MOTOR2_POSITION].enabled = true;
    
    // Initialize temperature monitor
    safety_monitors[MONITOR_SYSTEM_TEMPERATURE].safe_min = -10.0f;
    safety_monitors[MONITOR_SYSTEM_TEMPERATURE].safe_max = 85.0f;
    safety_monitors[MONITOR_SYSTEM_TEMPERATURE].warning_max = 75.0f;
    safety_monitors[MONITOR_SYSTEM_TEMPERATURE].enabled = true;
    
    return SYSTEM_OK;
}

/**
 * @brief Update safety monitor with new value
 * @param channel Monitor channel
 * @param value New measurement value
 * @return SystemError_t Success or error code
 */
SystemError_t safety_monitor_update(MonitorChannel_t channel, float value) {
    if (channel >= MONITOR_COUNT) {
        return ERROR_INVALID_PARAMETER;
    }
    
    SafetyMonitor_t* monitor = &safety_monitors[channel];
    
    if (!monitor->enabled) {
        return SYSTEM_OK;
    }
    
    monitor->current_value = value;
    
    // Check for safety violations
    bool safety_violation = false;
    if (value < monitor->safe_min || value > monitor->safe_max) {
        monitor->violation_count++;
        monitor->last_violation = HAL_GetTick();
        safety_violation = true;
        
        log_safety_event(SAFETY_EVENT_LIMIT_VIOLATION, channel, 
                        *(uint32_t*)&value);  // Convert float to uint32 for logging
        
        // Execute immediate safety response
        handle_safety_violation(channel, value);
    }
    
    // Check for warnings
    if (!safety_violation && 
        (value < monitor->warning_min || value > monitor->warning_max)) {
        monitor->warning_count++;
        
        log_safety_event(SAFETY_EVENT_WARNING, channel, 
                        *(uint32_t*)&value);
    }
    
    return safety_violation ? ERROR_SAFETY_LIMIT_VIOLATION : SYSTEM_OK;
}

/**
 * @brief Handle safety violation with appropriate response
 * @param channel Channel that violated safety limits
 * @param value Value that caused violation
 * @return SystemError_t Success or error code
 */
SystemError_t handle_safety_violation(MonitorChannel_t channel, float value) {
    switch (channel) {
        case MONITOR_MOTOR1_CURRENT:
        case MONITOR_MOTOR2_CURRENT:
            // Overcurrent: Stop motor immediately
            {
                uint8_t motor_id = (channel == MONITOR_MOTOR1_CURRENT) ? 0 : 1;
                l6470_immediate_stop(motor_id);
                log_safety_event(SAFETY_EVENT_OVERCURRENT_STOP, motor_id, 
                                *(uint32_t*)&value);
            }
            break;
            
        case MONITOR_MOTOR1_SPEED:
        case MONITOR_MOTOR2_SPEED:
            // Overspeed: Reduce speed or stop
            {
                uint8_t motor_id = (channel == MONITOR_MOTOR1_SPEED) ? 0 : 1;
                l6470_decelerate_to_stop(motor_id);
                log_safety_event(SAFETY_EVENT_OVERSPEED_STOP, motor_id,
                                *(uint32_t*)&value);
            }
            break;
            
        case MONITOR_MOTOR1_POSITION:
        case MONITOR_MOTOR2_POSITION:
            // Position limit: Stop and prevent further motion in violation direction
            {
                uint8_t motor_id = (channel == MONITOR_MOTOR1_POSITION) ? 0 : 1;
                l6470_immediate_stop(motor_id);
                set_position_limit_violation(motor_id, value);
                log_safety_event(SAFETY_EVENT_POSITION_LIMIT_STOP, motor_id,
                                *(uint32_t*)&value);
            }
            break;
            
        case MONITOR_SYSTEM_TEMPERATURE:
            // Overtemperature: Stop all motors
            execute_emergency_stop(ESTOP_SOURCE_OVERTEMPERATURE);
            break;
            
        default:
            // Generic safety violation: Emergency stop
            execute_emergency_stop(ESTOP_SOURCE_SAFETY_VIOLATION);
            break;
    }
    
    return SYSTEM_OK;
}
```

## Real-Time Control System

### High-Precision Timing
```c
/**
 * @brief Initialize high-resolution timer for real-time control
 * @return SystemError_t Success or error code
 */
SystemError_t realtime_timer_init(void) {
    TIM_HandleTypeDef* htim = &htim2;  // Use TIM2 for microsecond timing
    
    // Configure timer for 1 MHz (1 μs resolution)
    htim->Instance = TIM2;
    htim->Init.Prescaler = (SystemCoreClock / 1000000) - 1;  // 1 MHz
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = 0xFFFFFFFF;  // 32-bit counter for long intervals
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_Base_Init(htim) != HAL_OK) {
        return ERROR_TIMER_INIT_FAILED;
    }
    
    // Start timer
    if (HAL_TIM_Base_Start(htim) != HAL_OK) {
        return ERROR_TIMER_START_FAILED;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Get high-resolution timestamp in microseconds
 * @return uint32_t Timestamp in microseconds
 */
uint32_t get_microsecond_timer(void) {
    return __HAL_TIM_GET_COUNTER(&htim2);
}

/**
 * @brief Precise delay in microseconds
 * @param microseconds Delay time in microseconds
 */
void delay_microseconds(uint32_t microseconds) {
    uint32_t start = get_microsecond_timer();
    while ((get_microsecond_timer() - start) < microseconds) {
        // Busy wait for precise timing
        __NOP();
    }
}
```

### Real-Time Control Loop
```c
// Control loop timing configuration
typedef struct {
    uint32_t period_us;                // Control loop period in microseconds
    uint32_t max_jitter_us;           // Maximum allowed timing jitter
    uint32_t actual_period_us;        // Measured actual period
    uint32_t max_execution_time_us;   // Maximum execution time seen
    uint32_t overrun_count;           // Number of timing overruns
    bool enabled;                     // Control loop enabled
} ControlLoopTiming_t;

static ControlLoopTiming_t control_timing = {
    .period_us = CONTROL_LOOP_PERIOD_US,
    .max_jitter_us = CONTROL_LOOP_MAX_JITTER_US,
    .enabled = false
};

/**
 * @brief Real-time motor control loop (called from timer interrupt)
 */
void realtime_control_loop(void) {
    static uint32_t last_execution_time = 0;
    static uint32_t loop_counter = 0;
    
    uint32_t start_time = get_microsecond_timer();
    
    // Measure actual period
    if (last_execution_time != 0) {
        control_timing.actual_period_us = start_time - last_execution_time;
        
        // Check for timing violations
        uint32_t jitter = abs((int32_t)control_timing.actual_period_us - 
                             (int32_t)control_timing.period_us);
        if (jitter > control_timing.max_jitter_us) {
            control_timing.overrun_count++;
            log_timing_violation(jitter, control_timing.actual_period_us);
        }
    }
    
    // Execute control algorithms
    if (control_timing.enabled) {
        // Update motor positions and velocities
        update_motor_feedback();
        
        // Execute position control for each motor
        for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
            if (is_motor_enabled(motor_id)) {
                execute_motor_control_step(motor_id);
            }
        }
        
        // Update safety monitoring
        update_safety_monitoring();
        
        // Execute motion profiles
        update_motion_profiles();
    }
    
    // Measure execution time
    uint32_t execution_time = get_microsecond_timer() - start_time;
    if (execution_time > control_timing.max_execution_time_us) {
        control_timing.max_execution_time_us = execution_time;
    }
    
    // Check for control loop overrun
    if (execution_time > (control_timing.period_us * 0.8f)) {
        log_control_loop_overrun(execution_time, control_timing.period_us);
    }
    
    last_execution_time = start_time;
    loop_counter++;
    
    // Periodic diagnostics (every 1000 loops)
    if ((loop_counter % 1000) == 0) {
        update_control_loop_diagnostics();
    }
}

/**
 * @brief Execute single motor control step
 * @param motor_id Motor identifier
 * @return SystemError_t Success or error code
 */
SystemError_t execute_motor_control_step(uint8_t motor_id) {
    MotorStateInfo_t* motor_state = &motor_states[motor_id];
    
    // Read current position from encoder
    float current_position;
    SystemError_t result = read_encoder_position(motor_id, &current_position);
    if (result != SYSTEM_OK) {
        handle_encoder_error(motor_id, result);
        return result;
    }
    
    // Update motor state
    motor_state->current_position = current_position;
    motor_state->last_update_time = HAL_GetTick();
    
    // Execute position control if target is set
    if (motor_state->state == MOTOR_STATE_MOVING) {
        float dt = control_timing.period_us / 1000000.0f;  // Convert to seconds
        
        float speed_command = pid_position_control(motor_id, 
                                                  motor_state->target_position,
                                                  current_position, dt);
        
        // Apply speed command to motor
        result = set_motor_speed(motor_id, speed_command);
        if (result != SYSTEM_OK) {
            handle_motor_error(motor_id, result);
            return result;
        }
        
        motor_state->current_speed = speed_command;
        
        // Check if target reached
        float position_error = fabsf(motor_state->target_position - current_position);
        if (position_error < POSITION_TOLERANCE_DEG) {
            motor_state->state = MOTOR_STATE_HOLDING;
            result = set_motor_speed(motor_id, 0.0f);
        }
    }
    
    // Update safety monitoring
    safety_monitor_update(MONITOR_MOTOR1_POSITION + motor_id, current_position);
    safety_monitor_update(MONITOR_MOTOR1_SPEED + motor_id, motor_state->current_speed);
    
    return SYSTEM_OK;
}
```

### Interrupt Priority Management
```c
/**
 * @brief Configure interrupt priorities for real-time system
 */
void configure_interrupt_priorities(void) {
    // Safety-critical interrupts (highest priority)
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);  // Emergency stop button
    
    // Real-time control interrupts
    HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 1, 0);  // Control loop timer
    
    // Motor driver interrupts
    HAL_NVIC_SetPriority(SPI2_IRQn, 2, 0);  // L6470 SPI communication
    
    // Sensor interrupts
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 3, 0);  // AS5600 encoder I2C
    HAL_NVIC_SetPriority(I2C2_EV_IRQn, 3, 1);  // AS5600 encoder I2C
    
    // Communication interrupts (lower priority)
    HAL_NVIC_SetPriority(USART3_IRQn, 4, 0);  // Debug UART
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 4, 1);  // CAN communication
    
    // System management interrupts (lowest priority)
    HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);  // System tick
}

/**
 * @brief Critical section management for thread safety
 */
typedef struct {
    uint32_t interrupt_state;          // Saved interrupt state
    bool in_critical_section;          // Critical section flag
    uint32_t nesting_level;           // Nesting level counter
} CriticalSection_t;

static CriticalSection_t critical_section = {0};

/**
 * @brief Enter critical section (disable interrupts)
 */
void enter_critical_section(void) {
    uint32_t interrupt_state = __get_PRIMASK();
    __disable_irq();
    
    if (critical_section.nesting_level == 0) {
        critical_section.interrupt_state = interrupt_state;
    }
    critical_section.nesting_level++;
    critical_section.in_critical_section = true;
}

/**
 * @brief Exit critical section (restore interrupts)
 */
void exit_critical_section(void) {
    if (critical_section.nesting_level > 0) {
        critical_section.nesting_level--;
        
        if (critical_section.nesting_level == 0) {
            critical_section.in_critical_section = false;
            __set_PRIMASK(critical_section.interrupt_state);
        }
    }
}
```

## Fail-Safe Mechanisms

### Hardware Fail-Safe Design
```c
/**
 * @brief Initialize hardware fail-safe mechanisms
 * @return SystemError_t Success or error code
 */
SystemError_t failsafe_hardware_init(void) {
    // Configure safety relay outputs (normally closed)
    gpio_init_output(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN, GPIO_PIN_SET);
    gpio_init_output(SAFETY_RELAY2_PORT, SAFETY_RELAY2_PIN, GPIO_PIN_SET);
    
    // Configure motor enable outputs (active high)
    gpio_init_output(MOTOR1_ENABLE_PORT, MOTOR1_ENABLE_PIN, GPIO_PIN_RESET);
    gpio_init_output(MOTOR2_ENABLE_PORT, MOTOR2_ENABLE_PIN, GPIO_PIN_RESET);
    
    // Configure hardware watchdog
    watchdog_init();
    
    // Set up brown-out detection
    configure_brown_out_detection();
    
    return SYSTEM_OK;
}

/**
 * @brief Software fail-safe state machine
 * @return SystemError_t Success or error code
 */
SystemError_t update_failsafe_state_machine(void) {
    static FailsafeState_t current_state = FAILSAFE_STATE_INIT;
    static uint32_t state_entry_time = 0;
    
    uint32_t current_time = HAL_GetTick();
    
    switch (current_state) {
        case FAILSAFE_STATE_INIT:
            // Perform system initialization checks
            if (perform_system_self_test() == SYSTEM_OK) {
                current_state = FAILSAFE_STATE_SAFE;
                state_entry_time = current_time;
            } else {
                current_state = FAILSAFE_STATE_FAULT;
                state_entry_time = current_time;
            }
            break;
            
        case FAILSAFE_STATE_SAFE:
            // Normal operation - monitor for faults
            if (check_system_health() != SYSTEM_OK) {
                current_state = FAILSAFE_STATE_WARNING;
                state_entry_time = current_time;
            }
            break;
            
        case FAILSAFE_STATE_WARNING:
            // Warning state - attempt recovery
            if (check_system_health() == SYSTEM_OK) {
                current_state = FAILSAFE_STATE_SAFE;
                state_entry_time = current_time;
            } else if ((current_time - state_entry_time) > WARNING_TIMEOUT_MS) {
                current_state = FAILSAFE_STATE_FAULT;
                state_entry_time = current_time;
            }
            break;
            
        case FAILSAFE_STATE_FAULT:
            // Fault state - safe stop and wait for reset
            execute_safe_stop();
            // Remain in fault state until manual reset
            break;
    }
    
    return SYSTEM_OK;
}

/**
 * @brief Execute safe stop sequence
 * @return SystemError_t Success or error code
 */
SystemError_t execute_safe_stop(void) {
    // Stop all motors with controlled deceleration
    for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
        l6470_decelerate_to_stop(motor_id);
    }
    
    // Wait for motors to stop
    HAL_Delay(SAFE_STOP_TIMEOUT_MS);
    
    // Disable motor power
    HAL_GPIO_WritePin(MOTOR1_ENABLE_PORT, MOTOR1_ENABLE_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(MOTOR2_ENABLE_PORT, MOTOR2_ENABLE_PIN, GPIO_PIN_RESET);
    
    // Activate safety relays (power off)
    HAL_GPIO_WritePin(SAFETY_RELAY1_PORT, SAFETY_RELAY1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SAFETY_RELAY2_PORT, SAFETY_RELAY2_PIN, GPIO_PIN_RESET);
    
    // Set system to safe state
    set_system_state(SYSTEM_STATE_FAULT);
    
    return SYSTEM_OK;
}
```

Remember: **Safety systems must always fail to a safe state, real-time constraints are critical for stable control, redundant monitoring prevents single points of failure, proper interrupt priority ensures deterministic behavior.**
