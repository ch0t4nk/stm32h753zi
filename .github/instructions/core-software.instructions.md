---
applyTo: "src/{common,controllers,drivers,hal_abstraction}/**/*.{c,h}"
description: "Core software architecture, data types, error handling, state management, motor control algorithms, and HAL abstraction for STM32H753ZI stepper motor project"
---

# Core Software Instructions

## Overview
This instruction file provides comprehensive guidance for core software architecture in the STM32H753ZI stepper motor control system, including HAL abstraction, data types, error handling, state management, motor control algorithms, and driver implementations.

**MAJOR UPDATES**: This project uses a mature HAL abstraction layer for platform independence, comprehensive error handling with SSOT configuration, and professional embedded architecture patterns enabling hardware-free testing.

## Hardware Abstraction Layer (HAL) Guidelines

### 🎯 HAL Abstraction Architecture
**CRITICAL**: All hardware access must go through the HAL abstraction layer for platform independence and testability.

**Benefits of HAL Abstraction:**
- **Platform Independence**: Code compiles for STM32H7, mocks, and future platforms
- **Hardware-Free Testing**: Unit tests run without actual hardware using mock implementations  
- **Clean Architecture**: Application code isolated from hardware-specific details
- **Easy Debugging**: Mock layer enables controlled fault injection and state monitoring

```c
// ✅ CORRECT - Use HAL abstraction (modern pattern)
#include "hal_abstraction.h"
#include "config/hardware_config.h"

SystemError_t motor_control_function(void) {
    // Platform-independent GPIO control using SSOT configuration
    SystemError_t result = HAL_Abstraction_GPIO_Write(
        MOTOR1_CS_PORT, MOTOR1_CS_PIN, HAL_GPIO_STATE_SET);
    
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Platform-independent SPI transaction with proper error handling
    HAL_SPI_Transaction_t transaction = {
        .tx_data = command_buffer,
        .tx_size = sizeof(command_buffer),
        .rx_data = response_buffer,
        .rx_size = sizeof(response_buffer),
        .timeout_ms = SPI_TIMEOUT_MS
    };
    
    return HAL_Abstraction_SPI_Transaction(SPI_MOTOR_BUS, &transaction);
}
```

```c
// ❌ INCORRECT - Direct HAL usage bypasses abstraction (outdated pattern)
#include "stm32h7xx_hal.h"

void bad_motor_function(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);  // Not testable!
    HAL_SPI_Transmit(&hspi2, data, size, timeout);       // Platform-specific!
}
```

### 🧪 HAL Abstraction Testing Integration
**Testing Pattern for Hardware-Independent Development:**

```c
// Example: Testable motor initialization using HAL abstraction
#include "hal_abstraction.h"
#include "config/motor_config.h"

SystemError_t motor_driver_init(motor_id_t motor_id) {
    // Validate motor ID using SSOT configuration
    if (motor_id >= MAX_MOTORS) {
        return SYSTEM_ERROR_MOTOR_INVALID_ID;
    }
    
    // Initialize CS pin using HAL abstraction
    SystemError_t result = HAL_Abstraction_GPIO_Write(
        motor_cs_ports[motor_id], 
        motor_cs_pins[motor_id], 
        HAL_GPIO_STATE_SET
    );
    
    if (result != SYSTEM_OK) {
        return SYSTEM_ERROR_GPIO_INIT_FAILED;
    }
    
    // Configure SPI for motor communication
    HAL_SPI_Config_t spi_config = {
        .mode = HAL_SPI_MODE_MASTER,
        .data_width = HAL_SPI_DATA_WIDTH_8BIT,
        .clock_polarity = HAL_SPI_CPOL_HIGH,
        .clock_phase = HAL_SPI_CPHA_2EDGE,
        .baudrate_prescaler = SPI_BAUDRATE_PRESCALER_16
    };
    
    return HAL_Abstraction_SPI_Init(SPI_MOTOR_BUS, &spi_config);
}

// Unit test using mock HAL - runs without hardware
void test_motor_driver_init(void) {
    // Setup mock expectations
    MockHAL_ExpectGPIOWrite(MOTOR1_CS_PORT, MOTOR1_CS_PIN, HAL_GPIO_STATE_SET, SYSTEM_OK);
    MockHAL_ExpectSPIInit(SPI_MOTOR_BUS, SYSTEM_OK);
    
    // Test the function
    SystemError_t result = motor_driver_init(MOTOR_1);
    
    // Verify results
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_TRUE(MockHAL_AllExpectationsMet());
}
```

### HAL Abstraction Architecture
```c
// HAL abstraction provides clean, testable interfaces
typedef enum {
    HAL_GPIO_PIN_RESET = 0,
    HAL_GPIO_PIN_SET = 1
} HAL_GPIO_State_t;

typedef struct {
    uint8_t* tx_data;
    uint16_t tx_size;
    uint8_t* rx_data;
    uint16_t rx_size;
    uint32_t timeout_ms;
} HAL_SPI_Transaction_t;

// Platform-independent function signatures
SystemError_t HAL_Abstraction_GPIO_WritePin(uint32_t port, uint32_t pin, HAL_GPIO_State_t state);
SystemError_t HAL_Abstraction_SPI_Transaction(uint32_t spi_instance, HAL_SPI_Transaction_t* transaction);
uint32_t HAL_Abstraction_GetTick(void);
```

### Testing with HAL Abstraction
```c
// Hardware-independent unit testing
#ifdef UNITY_TESTING
#include "hal_abstraction_mock.h"

void test_motor_initialization(void) {
    // Configure mock behavior
    MockHAL_SetGPIOState(MOTOR1_CS_PORT, MOTOR1_CS_PIN, HAL_GPIO_PIN_SET);
    
    // Test the actual function
    SystemError_t result = motor_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Verify expected hardware interactions
    TEST_ASSERT_TRUE(MockHAL_WasGPIOPinSet(MOTOR1_CS_PORT, MOTOR1_CS_PIN));
}
#endif
```

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

### 🛡️ Error Handling Framework
**Comprehensive SystemError_t-based error management with SSOT configuration:**

```c
// Include our comprehensive error system
#include "common/error_codes.h"
#include "config/safety_config.h"

// Error context structure for debugging (enhanced)
typedef struct {
    SystemError_t error_code;          // Error code from comprehensive enum
    const char* file;                  // Source file where error occurred
    uint32_t line;                     // Line number in source file
    const char* function;              // Function name where error occurred
    timestamp_t timestamp;            // When error occurred (microsecond precision)
    uint32_t additional_info;         // Context-specific information
    motor_id_t motor_id;              // Motor ID if motor-related error
    uint8_t retry_count;              // Number of retry attempts
} ErrorContext_t;

// Professional error handling with HAL abstraction integration
SystemError_t motor_driver_safe_operation(motor_id_t motor_id, uint32_t command) {
    // Validate parameters using SSOT configuration
    if (motor_id >= MAX_MOTORS) {
        LOG_ERROR(SYSTEM_ERROR_MOTOR_INVALID_ID, motor_id);
        return SYSTEM_ERROR_MOTOR_INVALID_ID;
    }
    
    // Perform operation using HAL abstraction with error checking
    HAL_SPI_Transaction_t transaction = {
        .tx_data = &command,
        .tx_size = sizeof(command),
        .rx_data = response_buffer,
        .rx_size = sizeof(response_buffer),
        .timeout_ms = SPI_TIMEOUT_MS
    };
    
    SystemError_t result = HAL_Abstraction_SPI_Transaction(SPI_MOTOR_BUS, &transaction);
    if (result != SYSTEM_OK) {
        LOG_ERROR(result, command);
        
        // Implement intelligent retry logic
        if (result == SYSTEM_ERROR_SPI_TIMEOUT) {
            return handle_spi_timeout_error(motor_id);
        }
        
        return result;
    }
    
    // Validate response data
    if (!validate_motor_response(response_buffer, sizeof(response_buffer))) {
        LOG_ERROR(SYSTEM_ERROR_MOTOR_INVALID_RESPONSE, command);
        return SYSTEM_ERROR_MOTOR_INVALID_RESPONSE;
    }
    
    return SYSTEM_OK;
}

// Enhanced error logging with circular buffer
#define ERROR_LOG_SIZE 64
static ErrorContext_t error_log_buffer[ERROR_LOG_SIZE];
static volatile uint32_t error_log_index = 0;
static volatile uint32_t error_log_count = 0;

// Error logging macro for automatic context capture (enhanced)
#define LOG_ERROR(error_code, additional_info) \
    log_error_enhanced((error_code), __FILE__, __LINE__, __func__, (additional_info))

// Enhanced error checking with HAL abstraction integration
#define CHECK_HAL_RESULT(hal_call, error_code) \
    do { \
        SystemError_t _result = (hal_call); \
        if (_result != SYSTEM_OK) { \
            LOG_ERROR((error_code), _result); \
            return (error_code); \
        } \
    } while(0)

// Error checking with safety system integration
#define CHECK_SAFETY_CRITICAL(expression, error_code) \
    do { \
        if (!(expression)) { \
            LOG_ERROR((error_code), 0); \
            safety_trigger_emergency_stop(); \
            return (error_code); \
        } \
    } while(0)
```

**Modern Error Recovery Patterns:**
```c
// Intelligent error recovery with retry logic
SystemError_t motor_command_with_retry(motor_id_t motor_id, uint32_t command) {
    const uint8_t MAX_RETRIES = 3;
    uint8_t retry_count = 0;
    
    while (retry_count < MAX_RETRIES) {
        SystemError_t result = motor_driver_safe_operation(motor_id, command);
        
        if (result == SYSTEM_OK) {
            return SYSTEM_OK;
        }
        
        // Categorize error and decide retry strategy
        switch (result) {
            case SYSTEM_ERROR_SPI_TIMEOUT:
            case SYSTEM_ERROR_SPI_BUSY:
                // Recoverable errors - retry after delay
                HAL_Abstraction_Delay_ms(RETRY_DELAY_MS);
                retry_count++;
                break;
                
            case SYSTEM_ERROR_MOTOR_INVALID_ID:
            case SYSTEM_ERROR_MOTOR_NOT_INITIALIZED:
                // Non-recoverable parameter errors
                return result;
                
            case SYSTEM_ERROR_SAFETY_CRITICAL:
                // Safety-critical errors - immediate abort
                safety_trigger_emergency_stop();
                return result;
                
            default:
                // Unknown error - single retry then abort
                if (retry_count == 0) {
                    retry_count++;
                } else {
                    return result;
                }
                break;
        }
    }
    
    // All retries exhausted
    LOG_ERROR(SYSTEM_ERROR_MAX_RETRIES_EXCEEDED, command);
    return SYSTEM_ERROR_MAX_RETRIES_EXCEEDED;
}
```

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

## 🎛️ Motor Control Algorithms

### 🎯 Modern Position Control with HAL Abstraction
**Professional motor control using HAL abstraction and comprehensive error handling:**

```c
#include "hal_abstraction.h"
#include "config/motor_config.h"
#include "common/error_codes.h"

/**
 * @brief Enhanced PID position control with HAL abstraction
 * @param motor_id Motor identifier
 * @param target_position Target position in degrees  
 * @param dt Time step in seconds
 * @return SystemError_t Success or error code
 */
SystemError_t pid_position_control_update(motor_id_t motor_id, float target_position, float dt) {
    // Validate motor ID using SSOT configuration
    if (motor_id >= MAX_MOTORS) {
        return SYSTEM_ERROR_MOTOR_INVALID_ID;
    }
    
    // Read current position using HAL abstraction
    float current_position;
    SystemError_t encoder_result = HAL_Abstraction_Encoder_ReadPosition(
        motor_encoder_buses[motor_id], 
        &current_position
    );
    
    if (encoder_result != SYSTEM_OK) {
        LOG_ERROR(SYSTEM_ERROR_ENCODER_READ_FAILED, motor_id);
        return encoder_result;
    }
    
    // Get motor control parameters from SSOT
    const PositionControlParams_t* params = &motor_pid_params[motor_id];
    static float integral[MAX_MOTORS] = {0};
    static float previous_error[MAX_MOTORS] = {0};
    
    // Calculate position error with wraparound handling
    float error = target_position - current_position;
    if (error > 180.0f) {
        error -= 360.0f;
    } else if (error < -180.0f) {
        error += 360.0f;
    }
    
    // Apply deadband from SSOT configuration
    if (fabsf(error) < params->deadband) {
        error = 0.0f;
    }
    
    // PID calculation with anti-windup
    float proportional = params->kp * error;
    
    integral[motor_id] += error * dt;
    if (integral[motor_id] > params->integral_limit) {
        integral[motor_id] = params->integral_limit;
    } else if (integral[motor_id] < -params->integral_limit) {
        integral[motor_id] = -params->integral_limit;
    }
    float integral_term = params->ki * integral[motor_id];
    
    float derivative = (error - previous_error[motor_id]) / dt;
    float derivative_term = params->kd * derivative;
    
    float output = proportional + integral_term + derivative_term;
    
    // Apply output limits from SSOT configuration
    if (output > params->output_limit) {
        output = params->output_limit;
    } else if (output < -params->output_limit) {
        output = -params->output_limit;
    }
    
    // Send speed command to motor using HAL abstraction
    SystemError_t motor_result = HAL_Abstraction_Motor_SetSpeed(motor_id, output);
    if (motor_result != SYSTEM_OK) {
        LOG_ERROR(SYSTEM_ERROR_MOTOR_COMMAND_FAILED, motor_id);
        return motor_result;
    }
    
    previous_error[motor_id] = error;
    return SYSTEM_OK;
}
```

### 🚀 Motion Profile Generation with Safety Integration
**Smooth motion profiles with safety monitoring:**

```c
typedef struct {
    float position;        // Current position command
    float velocity;        // Current velocity command  
    float acceleration;    // Current acceleration command
    bool profile_complete; // Motion profile finished
    uint32_t elapsed_time; // Time since profile start
} MotionProfileOutput_t;

/**
 * @brief Generate safe trapezoidal motion profile
 * @param motor_id Motor identifier
 * @param start_position Starting position in degrees
 * @param end_position Target position in degrees
 * @param current_time Current time in motion (milliseconds)
 * @param output Output structure for position/velocity/acceleration
 * @return SystemError_t Success or error code
 */
SystemError_t generate_motion_profile(motor_id_t motor_id, 
                                    float start_position, 
                                    float end_position,
                                    uint32_t current_time,
                                    MotionProfileOutput_t* output) {
    // Validate parameters
    if (motor_id >= MAX_MOTORS || output == NULL) {
        return SYSTEM_ERROR_INVALID_PARAMETER;
    }
    
    // Check safety limits using SSOT configuration
    if (fabsf(end_position) > MOTOR_MAX_POSITION_DEGREES) {
        LOG_ERROR(SYSTEM_ERROR_POSITION_LIMIT_EXCEEDED, motor_id);
        return SYSTEM_ERROR_POSITION_LIMIT_EXCEEDED;
    }
    
    // Get motion profile parameters from SSOT
    const MotionProfile_t* profile = &motor_motion_profiles[motor_id];
    
    float distance = end_position - start_position;
    float max_speed = profile->max_speed;
    float acceleration = profile->acceleration;
    float deceleration = profile->deceleration;
    
    // Calculate motion timing
    float accel_time = max_speed / acceleration;
    float decel_time = max_speed / deceleration;
    float accel_distance = 0.5f * acceleration * accel_time * accel_time;
    float decel_distance = 0.5f * deceleration * decel_time * decel_time;
    
    float time_sec = current_time / 1000.0f;
    
    // Determine motion phase and calculate output
    if (fabsf(distance) <= (accel_distance + decel_distance)) {
        // Triangular profile (short move)
        return generate_triangular_profile(motor_id, start_position, end_position, 
                                         time_sec, profile, output);
    } else {
        // Trapezoidal profile (long move)
        return generate_trapezoidal_profile(motor_id, start_position, end_position,
                                          time_sec, profile, output);
    }
}

/**
 * @brief Execute motion profile with real-time safety monitoring
 * @param motor_id Motor identifier
 * @param target_position Target position in degrees
 * @return SystemError_t Success or error code
 */
SystemError_t execute_safe_motion(motor_id_t motor_id, float target_position) {
    static MotionProfileOutput_t profile_output[MAX_MOTORS];
    static uint32_t motion_start_time[MAX_MOTORS] = {0};
    static float motion_start_position[MAX_MOTORS] = {0};
    static bool motion_active[MAX_MOTORS] = {false};
    
    // Get current position using HAL abstraction
    float current_position;
    SystemError_t result = HAL_Abstraction_Encoder_ReadPosition(
        motor_encoder_buses[motor_id], &current_position);
    
    if (result != SYSTEM_OK) {
        return result;
    }
    
    // Start new motion if not active
    if (!motion_active[motor_id]) {
        motion_start_time[motor_id] = HAL_Abstraction_GetTick();
        motion_start_position[motor_id] = current_position;
        motion_active[motor_id] = true;
    }
    
    // Generate current motion profile point
    uint32_t elapsed_time = HAL_Abstraction_GetTick() - motion_start_time[motor_id];
    result = generate_motion_profile(motor_id, 
                                   motion_start_position[motor_id],
                                   target_position,
                                   elapsed_time,
                                   &profile_output[motor_id]);
    
    if (result != SYSTEM_OK) {
        motion_active[motor_id] = false;
        return result;
    }
    
    // Safety check: Verify profile output is within limits
    if (fabsf(profile_output[motor_id].velocity) > MOTOR_MAX_SPEED_RPM) {
        LOG_ERROR(SYSTEM_ERROR_SPEED_LIMIT_EXCEEDED, motor_id);
        motion_active[motor_id] = false;
        return SYSTEM_ERROR_SPEED_LIMIT_EXCEEDED;
    }
    
    // Execute position control with profile setpoint
    result = pid_position_control_update(motor_id, 
                                       profile_output[motor_id].position, 
                                       CONTROL_LOOP_PERIOD_SEC);
    
    if (result != SYSTEM_OK) {
        motion_active[motor_id] = false;
        return result;
    }
    
    // Check if motion is complete
    if (profile_output[motor_id].profile_complete) {
        motion_active[motor_id] = false;
    }
    
    return SYSTEM_OK;
}
```
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
