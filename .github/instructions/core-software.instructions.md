---
applyTo: "src/{common,controllers,drivers}/**/*.{c,h}"
description: "Core software architecture, data types, error handling, state management, and motor control algorithms for STM32H753ZI stepper motor project"
---

# Core Software Instructions

## Overview
This instruction file provides comprehensive guidance for core software architecture in the STM32H753ZI stepper motor control system, including data types, error handling, state management, motor control algorithms, and driver implementations.

## Data Types and Type Safety

### Fundamental Type Definitions
```c
// Ensure consistent type definitions across project
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Fixed-width integer types (required for embedded systems)
typedef uint8_t   u8;      // 8-bit unsigned
typedef int8_t    s8;      // 8-bit signed
typedef uint16_t  u16;     // 16-bit unsigned
typedef int16_t   s16;     // 16-bit signed
typedef uint32_t  u32;     // 32-bit unsigned
typedef int32_t   s32;     // 32-bit signed
typedef uint64_t  u64;     // 64-bit unsigned
typedef int64_t   s64;     // 64-bit signed

// Floating-point types
typedef float     f32;     // 32-bit IEEE 754
typedef double    f64;     // 64-bit IEEE 754 (limited use in embedded)

// System-specific types
typedef uint32_t  timestamp_t;    // System timestamp in milliseconds
typedef uint16_t  sequence_t;     // Message sequence numbers
typedef uint8_t   motor_id_t;     // Motor identifier type
typedef uint8_t   encoder_id_t;   // Encoder identifier type
```

### Motor Control Data Types
```c
// Motor state enumeration (values only, not information containers)
typedef enum {
    MOTOR_STATE_UNINITIALIZED = 0x00,   // Power-on state
    MOTOR_STATE_INITIALIZING = 0x01,    // During initialization
    MOTOR_STATE_IDLE = 0x02,            // Ready but not moving
    MOTOR_STATE_MOVING = 0x03,          // In motion
    MOTOR_STATE_HOLDING = 0x04,         // Holding position
    MOTOR_STATE_HOMING = 0x05,          // Finding home position
    MOTOR_STATE_ERROR = 0x06,           // Error condition
    MOTOR_STATE_EMERGENCY_STOP = 0x07   // Emergency stop active
} MotorState_t;

// Motor information structure (detailed state container)
typedef struct {
    MotorState_t state;                 // Current motor state
    float current_position;             // Position in degrees
    float target_position;              // Target position in degrees
    float current_speed;                // Speed in RPM
    float target_speed;                 // Target speed in RPM
    float acceleration;                 // Acceleration in deg/s²
    float deceleration;                 // Deceleration in deg/s²
    uint32_t step_count;               // Total step count
    uint32_t last_update_time;         // Last update timestamp
    bool position_valid;               // Position measurement validity
    bool homed;                        // Home position established
    SystemError_t last_error;          // Last error encountered
    uint16_t error_count;              // Total error count
} MotorStateInfo_t;

// Position control parameters
typedef struct {
    float kp;                          // Proportional gain
    float ki;                          // Integral gain  
    float kd;                          // Derivative gain
    float integral_limit;              // Anti-windup limit
    float output_limit;                // Output saturation limit
    float deadband;                    // Position deadband in degrees
    uint32_t update_period_ms;         // Control loop period
} PositionControlParams_t;

// Motion profile structure
typedef struct {
    float max_speed;                   // Maximum speed in RPM
    float acceleration;                // Acceleration in deg/s²
    float deceleration;                // Deceleration in deg/s²
    float jerk_limit;                  // Jerk limitation in deg/s³
    bool smooth_motion;                // Enable S-curve profiles
    uint32_t settle_time_ms;           // Time to settle at target
} MotionProfile_t;
```

### Safety System Data Types
```c
// Safety state enumeration
typedef enum {
    SAFETY_STATE_NORMAL = 0x00,        // Normal operation
    SAFETY_STATE_WARNING = 0x01,       // Warning condition
    SAFETY_STATE_FAULT = 0x02,         // Fault condition
    SAFETY_STATE_EMERGENCY = 0x03,     // Emergency stop
    SAFETY_STATE_MAINTENANCE = 0x04    // Maintenance mode
} SafetyState_t;

// Safety information structure
typedef struct {
    SafetyState_t state;               // Current safety state
    bool watchdog_active;              // Watchdog timer status
    bool emergency_stop_active;        // E-stop button status
    bool over_temperature;             // Temperature warning
    bool over_current;                 // Current limit exceeded
    bool position_limit_exceeded;      // Soft limit violation
    bool encoder_fault;                // Encoder failure detected
    bool communication_timeout;        // Communication watchdog
    uint32_t fault_code;              // Detailed fault information
    uint32_t fault_timestamp;         // When fault occurred
    uint16_t safety_violations;       // Total violation count
} SafetyStateInfo_t;

// Sensor data structure with validation
typedef struct {
    float value;                       // Sensor reading
    PhysicalUnit_t unit;              // Physical unit
    float min_valid;                  // Minimum valid value
    float max_valid;                  // Maximum valid value
    timestamp_t timestamp;            // When measured
    bool valid;                       // Data validity flag
    uint16_t error_count;             // Consecutive error count
} SensorData_t;
```

## Error Handling System

### Unified Error Code System (SSOT)
```c
// Single Source of Truth for all error codes
typedef enum {
    // Success
    SYSTEM_OK = 0x0000,

    // General system errors (0x1000-0x1FFF)
    ERROR_NULL_POINTER = 0x1001,
    ERROR_INVALID_PARAMETER = 0x1002,
    ERROR_BUFFER_OVERFLOW = 0x1003,
    ERROR_BUFFER_UNDERFLOW = 0x1004,
    ERROR_TIMEOUT = 0x1005,
    ERROR_NOT_INITIALIZED = 0x1006,
    ERROR_ALREADY_INITIALIZED = 0x1007,
    ERROR_RESOURCE_BUSY = 0x1008,
    ERROR_INSUFFICIENT_MEMORY = 0x1009,
    ERROR_CHECKSUM_FAILED = 0x100A,
    ERROR_HARDWARE_FAILURE = 0x100B,
    ERROR_SOFTWARE_FAULT = 0x100C,
    ERROR_STACK_OVERFLOW = 0x100D,
    ERROR_HEAP_CORRUPTION = 0x100E,
    ERROR_INVALID_STATE = 0x100F,

    // Motor control errors (0x2000-0x2FFF)
    ERROR_MOTOR_BASE = 0x2000,
    ERROR_MOTOR_INVALID_ID = 0x2001,
    ERROR_MOTOR_NOT_INITIALIZED = 0x2002,
    ERROR_MOTOR_INIT_FAILED = 0x2003,
    ERROR_MOTOR_MOVE_FAILED = 0x2004,
    ERROR_MOTOR_POSITION_INVALID = 0x2005,
    ERROR_MOTOR_SPEED_OUT_OF_RANGE = 0x2006,
    ERROR_MOTOR_ACCELERATION_INVALID = 0x2007,
    ERROR_MOTOR_NOT_HOMED = 0x2008,
    ERROR_MOTOR_HOMING_FAILED = 0x2009,
    ERROR_MOTOR_STALL_DETECTED = 0x200A,
    ERROR_MOTOR_OVERCURRENT = 0x200B,
    ERROR_MOTOR_OVERTEMPERATURE = 0x200C,
    ERROR_MOTOR_POSITION_LIMIT = 0x200D,
    ERROR_MOTOR_EMERGENCY_STOP = 0x200E,
    ERROR_MOTOR_L6470_FAULT = 0x200F,

    // Encoder errors (0x3000-0x3FFF)
    ERROR_ENCODER_BASE = 0x3000,
    ERROR_ENCODER_INVALID_ID = 0x3001,
    ERROR_ENCODER_NOT_INITIALIZED = 0x3002,
    ERROR_ENCODER_INIT_FAILED = 0x3003,
    ERROR_ENCODER_READ_FAILED = 0x3004,
    ERROR_ENCODER_MAGNET_NOT_DETECTED = 0x3005,
    ERROR_ENCODER_MAGNET_TOO_WEAK = 0x3006,
    ERROR_ENCODER_MAGNET_TOO_STRONG = 0x3007,
    ERROR_ENCODER_CALIBRATION_FAILED = 0x3008,
    ERROR_ENCODER_POSITION_INVALID = 0x3009,
    ERROR_ENCODER_COMMUNICATION_FAILED = 0x300A,
    ERROR_ENCODER_TIMEOUT = 0x300B,
    ERROR_ENCODER_CHECKSUM_ERROR = 0x300C,

    // Communication errors (0x4000-0x4FFF)
    ERROR_COMM_BASE = 0x4000,
    ERROR_COMM_INIT_FAILED = 0x4001,
    ERROR_COMM_CONFIG_FAILED = 0x4002,
    ERROR_COMM_START_FAILED = 0x4003,
    ERROR_COMM_TX_FAILED = 0x4004,
    ERROR_COMM_RX_FAILED = 0x4005,
    ERROR_COMM_TIMEOUT = 0x4006,
    ERROR_COMM_BUFFER_FULL = 0x4007,
    ERROR_COMM_BUFFER_EMPTY = 0x4008,
    ERROR_COMM_PROTOCOL_ERROR = 0x4009,
    ERROR_COMM_CHECKSUM_ERROR = 0x400A,
    ERROR_COMM_PARSE_FAILED = 0x400B,
    ERROR_COMM_INVALID_PARAMS = 0x400C,
    ERROR_COMM_SPI_FAILED = 0x400D,
    ERROR_COMM_I2C_FAILED = 0x400E,
    ERROR_COMM_UART_FAILED = 0x400F,
    ERROR_COMM_CAN_FAILED = 0x4010,
    ERROR_COMM_ETHERNET_FAILED = 0x4011,
    ERROR_COMM_UDP_FAILED = 0x4012,
    ERROR_COMM_TCP_FAILED = 0x4013,
    ERROR_COMM_DMA_FAILED = 0x4014,
    ERROR_COMM_INTERRUPT_FAILED = 0x4015,
    ERROR_COMM_PROTOCOL_DISABLED = 0x4016,
    ERROR_COMM_MEMORY_FAILED = 0x4017,

    // Safety system errors (0x5000-0x5FFF)
    ERROR_SAFETY_BASE = 0x5000,
    ERROR_SAFETY_EMERGENCY_STOP = 0x5001,
    ERROR_SAFETY_WATCHDOG_TIMEOUT = 0x5002,
    ERROR_SAFETY_OVER_TEMPERATURE = 0x5003,
    ERROR_SAFETY_OVER_CURRENT = 0x5004,
    ERROR_SAFETY_POSITION_LIMIT = 0x5005,
    ERROR_SAFETY_SPEED_LIMIT = 0x5006,
    ERROR_SAFETY_ACCELERATION_LIMIT = 0x5007,
    ERROR_SAFETY_SENSOR_FAULT = 0x5008,
    ERROR_SAFETY_SYSTEM_FAULT = 0x5009,
    ERROR_SAFETY_INTERLOCK_OPEN = 0x500A,
    ERROR_SAFETY_CONFIGURATION_ERROR = 0x500B,

    // Configuration errors (0x6000-0x6FFF)
    ERROR_CONFIG_BASE = 0x6000,
    ERROR_CONFIG_INVALID_VALUE = 0x6001,
    ERROR_CONFIG_OUT_OF_RANGE = 0x6002,
    ERROR_CONFIG_INCOMPATIBLE = 0x6003,
    ERROR_CONFIG_MISSING = 0x6004,
    ERROR_CONFIG_CORRUPTED = 0x6005,
    ERROR_CONFIG_WRITE_FAILED = 0x6006,
    ERROR_CONFIG_READ_FAILED = 0x6007
} SystemError_t;
```

### Error Handling Framework
```c
// Error context structure for debugging
typedef struct {
    SystemError_t error_code;          // Error code
    const char* file;                  // Source file where error occurred
    uint32_t line;                     // Line number in source file
    const char* function;              // Function name where error occurred
    timestamp_t timestamp;            // When error occurred
    uint32_t additional_info;         // Context-specific information
} ErrorContext_t;

// Error logging function
void log_error(SystemError_t error_code, const char* file, uint32_t line, 
              const char* function, uint32_t additional_info);

// Error logging macro for automatic context capture
#define LOG_ERROR(error_code, additional_info) \
    log_error((error_code), __FILE__, __LINE__, __func__, (additional_info))

// Error checking macro with logging
#define CHECK_ERROR(expression, error_code) \
    do { \
        if (!(expression)) { \
            LOG_ERROR((error_code), 0); \
            return (error_code); \
        } \
    } while(0)

// Error checking macro with cleanup
#define CHECK_ERROR_CLEANUP(expression, error_code, cleanup_code) \
    do { \
        if (!(expression)) { \
            LOG_ERROR((error_code), 0); \
            cleanup_code; \
            return (error_code); \
        } \
    } while(0)

/**
 * @brief Initialize error handling system
 * @return SystemError_t Success or error code
 */
SystemError_t error_system_init(void) {
    // Initialize error logging buffer
    memset(error_log_buffer, 0, sizeof(error_log_buffer));
    error_log_index = 0;
    error_log_count = 0;
    
    // Enable error interrupts if needed
    // Configure fault handlers
    
    return SYSTEM_OK;
}

/**
 * @brief Get human-readable error description
 * @param error_code Error code to describe
 * @return const char* Error description string
 */
const char* get_error_description(SystemError_t error_code) {
    switch (error_code) {
        case SYSTEM_OK:
            return "Operation successful";
        case ERROR_NULL_POINTER:
            return "Null pointer dereference";
        case ERROR_INVALID_PARAMETER:
            return "Invalid parameter value";
        case ERROR_MOTOR_NOT_HOMED:
            return "Motor not homed to reference position";
        case ERROR_ENCODER_MAGNET_NOT_DETECTED:
            return "Encoder magnet not detected";
        case ERROR_COMM_TIMEOUT:
            return "Communication timeout";
        case ERROR_SAFETY_EMERGENCY_STOP:
            return "Emergency stop activated";
        // Add more descriptions as needed
        default:
            return "Unknown error code";
    }
}
```

### Error Recovery Strategies
```c
// Error recovery action enumeration
typedef enum {
    RECOVERY_NONE,                     // No recovery possible
    RECOVERY_RETRY,                    // Retry operation
    RECOVERY_RESET_COMPONENT,          // Reset specific component
    RECOVERY_RESTART_SYSTEM,           // Full system restart
    RECOVERY_SAFE_STOP,               // Stop all motion safely
    RECOVERY_USER_INTERVENTION        // Requires user action
} RecoveryAction_t;

/**
 * @brief Determine recovery action for error
 * @param error_code Error that occurred
 * @return RecoveryAction_t Recommended recovery action
 */
RecoveryAction_t get_recovery_action(SystemError_t error_code) {
    switch (error_code) {
        case ERROR_COMM_TIMEOUT:
        case ERROR_COMM_TX_FAILED:
        case ERROR_COMM_RX_FAILED:
            return RECOVERY_RETRY;
            
        case ERROR_MOTOR_L6470_FAULT:
        case ERROR_ENCODER_COMMUNICATION_FAILED:
            return RECOVERY_RESET_COMPONENT;
            
        case ERROR_SAFETY_EMERGENCY_STOP:
        case ERROR_SAFETY_OVER_TEMPERATURE:
        case ERROR_SAFETY_OVER_CURRENT:
            return RECOVERY_SAFE_STOP;
            
        case ERROR_MOTOR_NOT_HOMED:
        case ERROR_ENCODER_MAGNET_NOT_DETECTED:
            return RECOVERY_USER_INTERVENTION;
            
        case ERROR_STACK_OVERFLOW:
        case ERROR_HEAP_CORRUPTION:
            return RECOVERY_RESTART_SYSTEM;
            
        default:
            return RECOVERY_NONE;
    }
}

/**
 * @brief Execute error recovery action
 * @param error_code Error that triggered recovery
 * @param recovery_action Recovery action to execute
 * @return SystemError_t Recovery result
 */
SystemError_t execute_recovery_action(SystemError_t error_code, RecoveryAction_t recovery_action) {
    switch (recovery_action) {
        case RECOVERY_RETRY:
            // Increment retry counter and try again
            break;
            
        case RECOVERY_RESET_COMPONENT:
            // Reset specific hardware component
            if (IS_MOTOR_ERROR(error_code)) {
                return motor_reset_and_reinit();
            } else if (IS_ENCODER_ERROR(error_code)) {
                return encoder_reset_and_reinit();
            } else if (IS_COMM_ERROR(error_code)) {
                return comm_reset_and_reinit();
            }
            break;
            
        case RECOVERY_SAFE_STOP:
            return safety_emergency_stop();
            
        case RECOVERY_RESTART_SYSTEM:
            HAL_NVIC_SystemReset();  // Does not return
            break;
            
        case RECOVERY_USER_INTERVENTION:
            // Set system to maintenance mode
            set_system_state(SYSTEM_STATE_MAINTENANCE);
            break;
            
        default:
            break;
    }
    
    return SYSTEM_OK;
}
```

## State Management

### System State Architecture
```c
// System-wide state enumeration
typedef enum {
    SYSTEM_STATE_BOOT = 0x00,          // System booting
    SYSTEM_STATE_INIT = 0x01,          // Initializing components
    SYSTEM_STATE_READY = 0x02,         // Ready for operation
    SYSTEM_STATE_RUNNING = 0x03,       // Normal operation
    SYSTEM_STATE_FAULT = 0x04,         // Fault condition
    SYSTEM_STATE_MAINTENANCE = 0x05,   // Maintenance mode
    SYSTEM_STATE_SHUTDOWN = 0x06       // Shutting down
} SystemState_t;

// System status information
typedef struct {
    SystemState_t state;               // Current system state
    uint8_t active_motors;             // Number of active motors
    uint8_t active_encoders;           // Number of active encoders
    uint32_t uptime_ms;               // System uptime
    uint32_t error_count;             // Total error count
    uint32_t command_count;           // Commands processed
    SafetyStateInfo_t safety_status;  // Safety system status
    MotorStateInfo_t motor_states[MAX_MOTORS];  // Motor status array
    float cpu_usage_percent;          // CPU utilization
    uint32_t free_heap_bytes;         // Available heap memory
    uint32_t min_stack_bytes;         // Minimum stack space seen
} SystemStatus_t;

// State machine context
typedef struct {
    SystemState_t current_state;       // Current state
    SystemState_t previous_state;      // Previous state for rollback
    timestamp_t state_entry_time;     // When current state was entered
    uint32_t state_duration_ms;       // How long in current state
    bool state_changed;               // Flag for state transitions
    SystemError_t transition_error;   // Error during last transition
} StateMachine_t;
```

### State Transition Management
```c
/**
 * @brief Change system state with validation
 * @param new_state Target state
 * @return SystemError_t Success or error code
 */
SystemError_t set_system_state(SystemState_t new_state) {
    StateMachine_t* sm = &system_state_machine;
    
    // Validate state transition
    if (!is_valid_state_transition(sm->current_state, new_state)) {
        LOG_ERROR(ERROR_INVALID_STATE, (uint32_t)new_state);
        return ERROR_INVALID_STATE;
    }
    
    // Execute exit actions for current state
    SystemError_t exit_result = execute_state_exit_actions(sm->current_state);
    if (exit_result != SYSTEM_OK) {
        LOG_ERROR(exit_result, 0);
        return exit_result;
    }
    
    // Update state machine
    sm->previous_state = sm->current_state;
    sm->current_state = new_state;
    sm->state_entry_time = HAL_GetTick();
    sm->state_duration_ms = 0;
    sm->state_changed = true;
    
    // Execute entry actions for new state
    SystemError_t entry_result = execute_state_entry_actions(new_state);
    if (entry_result != SYSTEM_OK) {
        // Rollback state change
        sm->current_state = sm->previous_state;
        sm->transition_error = entry_result;
        LOG_ERROR(entry_result, 0);
        return entry_result;
    }
    
    sm->transition_error = SYSTEM_OK;
    return SYSTEM_OK;
}

/**
 * @brief Check if state transition is valid
 * @param current_state Current system state
 * @param new_state Proposed new state
 * @return bool true if transition is valid
 */
bool is_valid_state_transition(SystemState_t current_state, SystemState_t new_state) {
    // State transition matrix
    switch (current_state) {
        case SYSTEM_STATE_BOOT:
            return (new_state == SYSTEM_STATE_INIT);
            
        case SYSTEM_STATE_INIT:
            return (new_state == SYSTEM_STATE_READY || 
                   new_state == SYSTEM_STATE_FAULT);
            
        case SYSTEM_STATE_READY:
            return (new_state == SYSTEM_STATE_RUNNING ||
                   new_state == SYSTEM_STATE_MAINTENANCE ||
                   new_state == SYSTEM_STATE_FAULT ||
                   new_state == SYSTEM_STATE_SHUTDOWN);
            
        case SYSTEM_STATE_RUNNING:
            return (new_state == SYSTEM_STATE_READY ||
                   new_state == SYSTEM_STATE_FAULT ||
                   new_state == SYSTEM_STATE_SHUTDOWN);
            
        case SYSTEM_STATE_FAULT:
            return (new_state == SYSTEM_STATE_READY ||
                   new_state == SYSTEM_STATE_MAINTENANCE ||
                   new_state == SYSTEM_STATE_SHUTDOWN);
            
        case SYSTEM_STATE_MAINTENANCE:
            return (new_state == SYSTEM_STATE_READY ||
                   new_state == SYSTEM_STATE_SHUTDOWN);
            
        case SYSTEM_STATE_SHUTDOWN:
            return false;  // No transitions from shutdown
            
        default:
            return false;
    }
}
```

## Motor Control Algorithms

### Position Control Algorithm
```c
/**
 * @brief PID position control algorithm
 * @param motor_id Motor identifier
 * @param target_position Target position in degrees
 * @param current_position Current position in degrees
 * @param dt Time step in seconds
 * @return float Control output (speed command in RPM)
 */
float pid_position_control(uint8_t motor_id, float target_position, 
                          float current_position, float dt) {
    static float integral[MAX_MOTORS] = {0};
    static float previous_error[MAX_MOTORS] = {0};
    
    PositionControlParams_t* params = &position_control_params[motor_id];
    
    // Calculate position error
    float error = target_position - current_position;
    
    // Handle angle wraparound if needed
    if (error > 180.0f) {
        error -= 360.0f;
    } else if (error < -180.0f) {
        error += 360.0f;
    }
    
    // Check deadband
    if (fabsf(error) < params->deadband) {
        error = 0.0f;
    }
    
    // Proportional term
    float proportional = params->kp * error;
    
    // Integral term with anti-windup
    integral[motor_id] += error * dt;
    if (integral[motor_id] > params->integral_limit) {
        integral[motor_id] = params->integral_limit;
    } else if (integral[motor_id] < -params->integral_limit) {
        integral[motor_id] = -params->integral_limit;
    }
    float integral_term = params->ki * integral[motor_id];
    
    // Derivative term
    float derivative = (error - previous_error[motor_id]) / dt;
    float derivative_term = params->kd * derivative;
    
    // Calculate total output
    float output = proportional + integral_term + derivative_term;
    
    // Apply output limits
    if (output > params->output_limit) {
        output = params->output_limit;
    } else if (output < -params->output_limit) {
        output = -params->output_limit;
    }
    
    // Update previous error for next iteration
    previous_error[motor_id] = error;
    
    return output;  // Speed command in RPM
}
```

### Motion Profile Generation
```c
/**
 * @brief Generate trapezoidal motion profile
 * @param profile Motion profile parameters
 * @param start_position Starting position in degrees
 * @param end_position Target position in degrees
 * @param current_time Current time in motion
 * @param motion_output Output structure for position/velocity/acceleration
 * @return SystemError_t Success or error code
 */
SystemError_t generate_motion_profile(const MotionProfile_t* profile,
                                    float start_position, float end_position,
                                    float current_time, MotionOutput_t* motion_output) {
    CHECK_ERROR(profile != NULL, ERROR_NULL_POINTER);
    CHECK_ERROR(motion_output != NULL, ERROR_NULL_POINTER);
    
    float distance = end_position - start_position;
    float max_speed_deg_per_sec = profile->max_speed * 6.0f;  // RPM to deg/s
    
    // Calculate motion times
    float accel_time = max_speed_deg_per_sec / profile->acceleration;
    float decel_time = max_speed_deg_per_sec / profile->deceleration;
    float accel_distance = 0.5f * profile->acceleration * accel_time * accel_time;
    float decel_distance = 0.5f * profile->deceleration * decel_time * decel_time;
    
    // Check if triangular profile is needed (cannot reach max speed)
    if ((accel_distance + decel_distance) > fabsf(distance)) {
        // Triangular profile
        float peak_speed = sqrtf(fabsf(distance) * profile->acceleration * profile->deceleration / 
                               (profile->acceleration + profile->deceleration));
        accel_time = peak_speed / profile->acceleration;
        decel_time = peak_speed / profile->deceleration;
        accel_distance = 0.5f * profile->acceleration * accel_time * accel_time;
        decel_distance = 0.5f * profile->deceleration * decel_time * decel_time;
    }
    
    float const_speed_distance = fabsf(distance) - accel_distance - decel_distance;
    float const_speed_time = (const_speed_distance > 0) ? 
                            const_speed_distance / max_speed_deg_per_sec : 0;
    
    float total_time = accel_time + const_speed_time + decel_time;
    
    // Calculate position, velocity, acceleration at current time
    float sign = (distance >= 0) ? 1.0f : -1.0f;
    
    if (current_time <= accel_time) {
        // Acceleration phase
        motion_output->position = start_position + sign * 0.5f * profile->acceleration * current_time * current_time;
        motion_output->velocity = sign * profile->acceleration * current_time;
        motion_output->acceleration = sign * profile->acceleration;
    } else if (current_time <= (accel_time + const_speed_time)) {
        // Constant speed phase
        float const_time = current_time - accel_time;
        motion_output->position = start_position + sign * (accel_distance + max_speed_deg_per_sec * const_time);
        motion_output->velocity = sign * max_speed_deg_per_sec;
        motion_output->acceleration = 0.0f;
    } else if (current_time <= total_time) {
        // Deceleration phase
        float decel_time_remaining = total_time - current_time;
        motion_output->position = end_position - sign * 0.5f * profile->deceleration * decel_time_remaining * decel_time_remaining;
        motion_output->velocity = sign * profile->deceleration * decel_time_remaining;
        motion_output->acceleration = -sign * profile->deceleration;
    } else {
        // Motion complete
        motion_output->position = end_position;
        motion_output->velocity = 0.0f;
        motion_output->acceleration = 0.0f;
    }
    
    motion_output->time_remaining = (current_time < total_time) ? (total_time - current_time) : 0.0f;
    motion_output->motion_complete = (current_time >= total_time);
    
    return SYSTEM_OK;
}
```

### Driver Interface Abstraction
```c
// Generic motor driver interface
typedef struct {
    SystemError_t (*init)(uint8_t motor_id);
    SystemError_t (*deinit)(uint8_t motor_id);
    SystemError_t (*move_to_position)(uint8_t motor_id, float position);
    SystemError_t (*set_speed)(uint8_t motor_id, float speed);
    SystemError_t (*stop)(uint8_t motor_id);
    SystemError_t (*get_position)(uint8_t motor_id, float* position);
    SystemError_t (*get_status)(uint8_t motor_id, MotorStateInfo_t* status);
    SystemError_t (*reset)(uint8_t motor_id);
} MotorDriverInterface_t;

// Generic encoder driver interface
typedef struct {
    SystemError_t (*init)(uint8_t encoder_id);
    SystemError_t (*deinit)(uint8_t encoder_id);
    SystemError_t (*read_angle)(uint8_t encoder_id, float* angle);
    SystemError_t (*read_velocity)(uint8_t encoder_id, float* velocity);
    SystemError_t (*calibrate)(uint8_t encoder_id);
    SystemError_t (*get_status)(uint8_t encoder_id, SensorData_t* status);
    SystemError_t (*reset)(uint8_t encoder_id);
} EncoderDriverInterface_t;

// Driver registration
extern const MotorDriverInterface_t l6470_driver_interface;
extern const EncoderDriverInterface_t as5600_driver_interface;

// Driver accessor functions
const MotorDriverInterface_t* get_motor_driver(uint8_t motor_id) {
    // For this project, all motors use L6470
    return &l6470_driver_interface;
}

const EncoderDriverInterface_t* get_encoder_driver(uint8_t encoder_id) {
    // For this project, all encoders use AS5600
    return &as5600_driver_interface;
}
```

Remember: **Consistent data types ensure system reliability, comprehensive error handling prevents failures, proper state management enables safe operation, abstracted interfaces support maintainability.**
