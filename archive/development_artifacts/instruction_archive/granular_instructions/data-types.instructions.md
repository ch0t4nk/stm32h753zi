---
applyTo: "src/**"
description: "Data type definitions and type safety for STM32H753ZI stepper motor project"
---

# Data Types and Type Safety Instructions

## Overview
This instruction file provides comprehensive guidance for implementing consistent, safe, and portable data types throughout the STM32H753ZI stepper motor control project.

## Enhanced Documentation Integration
**Reference Assets**: This instruction leverages comprehensive STM32H7 data type and compiler documentation:
- **STM32H7 Data Types**: Complete data type coverage with ARM Cortex-M7 optimization guidance
- **Copilot-Optimized Type Search**:
  ```bash
  # Search data type concepts with semantic targeting
  python3 scripts/search_enhanced_docs.py concept "data_types" --scope STM32H7
  
  # Find type safety implementation patterns
  python3 scripts/search_enhanced_docs.py concept "type_safety" --scope STM32H7
  
  # Search ARM Cortex-M7 specific optimizations
  python3 scripts/search_enhanced_docs.py concept "cortex_m7_optimization" --scope STM32H7
  
  # Find HAL type definitions
  python3 scripts/search_enhanced_docs.py concept "hal_types" --scope STM32H7
  ```
- **Cross-Domain Integration**: Links data types with motor control, safety systems, and communication protocols
- **Token-Efficient Reference**: Semantic categorization for efficient Copilot assistance with type definitions

## Standard Data Types

### Base Integer Types
Use stdint.h types exclusively for guaranteed portability and size consistency:

```c
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// NEVER use these deprecated types:
// int, unsigned int, long, unsigned long, char (for numeric data)

// ALWAYS use these specific-width types:
typedef uint8_t  u8;        // 8-bit unsigned (0 to 255)
typedef int8_t   s8;        // 8-bit signed (-128 to 127)
typedef uint16_t u16;       // 16-bit unsigned (0 to 65535)
typedef int16_t  s16;       // 16-bit signed (-32768 to 32767)
typedef uint32_t u32;       // 32-bit unsigned
typedef int32_t  s32;       // 32-bit signed
typedef uint64_t u64;       // 64-bit unsigned
typedef int64_t  s64;       // 64-bit signed

// Floating point types
typedef float    f32;       // 32-bit IEEE 754
typedef double   f64;       // 64-bit IEEE 754

// Boolean type (from stdbool.h)
// bool - use exclusively for boolean values
// true/false - use instead of 1/0 for boolean constants
```

### System-Specific Types
```c
// Time and timing types
typedef uint32_t SystemTime_t;          // System tick timestamp (ms)
typedef uint32_t Duration_t;            // Duration in milliseconds
typedef uint64_t PreciseTime_t;         // High-precision timestamp (μs)

// Memory and size types
typedef size_t   MemorySize_t;          // Memory sizes and buffer lengths
typedef uintptr_t MemoryAddress_t;      // Memory addresses
typedef ptrdiff_t MemoryOffset_t;       // Memory offsets

// Status and result types
typedef enum {
    RESULT_OK = 0,                      // Operation successful
    RESULT_ERROR = 1,                   // Generic error
    RESULT_TIMEOUT = 2,                 // Operation timed out
    RESULT_INVALID_PARAMETER = 3,       // Invalid input parameter
    RESULT_OUT_OF_MEMORY = 4,           // Memory allocation failed
    RESULT_NOT_SUPPORTED = 5,           // Operation not supported
    RESULT_BUSY = 6,                    // Resource busy
    RESULT_NOT_READY = 7                // Resource not ready
} Result_t;
```

## Motor Control Data Types

### Motor Position and Motion
```c
// Motor position types - use consistent units throughout system
typedef float MotorAngle_t;             // Motor angle in degrees (0.0 to 360.0)
typedef float MotorVelocity_t;          // Motor velocity in degrees/second
typedef float MotorAcceleration_t;      // Motor acceleration in degrees/second²
typedef int32_t MotorSteps_t;           // Motor position in steps (for L6470)
typedef uint16_t MotorMicrosteps_t;     // Microstep setting (1, 2, 4, 8, 16, etc.)

// Motor limits and ranges
typedef struct {
    MotorAngle_t min_angle;             // Minimum allowed angle
    MotorAngle_t max_angle;             // Maximum allowed angle
    MotorVelocity_t max_velocity;       // Maximum velocity
    MotorAcceleration_t max_acceleration; // Maximum acceleration
    MotorAcceleration_t max_deceleration; // Maximum deceleration
} MotorLimits_t;

// Motor status and state
typedef enum {
    MOTOR_STATE_UNKNOWN = 0,            // State not determined
    MOTOR_STATE_STOPPED = 1,            // Motor stopped
    MOTOR_STATE_ACCELERATING = 2,       // Motor accelerating
    MOTOR_STATE_CONSTANT_SPEED = 3,     // Motor at constant speed
    MOTOR_STATE_DECELERATING = 4,       // Motor decelerating
    MOTOR_STATE_ERROR = 5,              // Motor in error state
    MOTOR_STATE_HOMING = 6,             // Motor performing homing sequence
    MOTOR_STATE_CALIBRATING = 7         // Motor calibrating
} MotorState_t;

// Motor configuration
typedef struct {
    uint8_t motor_id;                   // Motor identifier (0-based)
    MotorMicrosteps_t microstep_mode;   // Microstep resolution
    MotorLimits_t limits;               // Motion limits
    float gear_ratio;                   // Gear reduction ratio
    uint16_t steps_per_revolution;      // Steps per full revolution (typically 200)
    bool inverted_direction;            // Direction inversion flag
    bool enable_closed_loop;            // Closed-loop control enabled
} MotorConfig_t;
```

### L6470 Driver-Specific Types
```c
// L6470 register values - maintain exact bit widths per datasheet
typedef uint8_t  L6470Command_t;        // 8-bit command
typedef uint32_t L6470Parameter_t;      // Up to 24-bit parameter value
typedef uint16_t L6470Status_t;         // 16-bit status register
typedef uint8_t  L6470Register_t;       // Register address

// L6470 configuration parameters with proper ranges
typedef struct {
    uint16_t acc;                       // Acceleration (12-bit, 0x001-0xFFF)
    uint16_t dec;                       // Deceleration (12-bit, 0x001-0xFFF)
    uint16_t max_speed;                 // Maximum speed (10-bit, 0x000-0x3FF)
    uint16_t min_speed;                 // Minimum speed (13-bit, 0x000-0x1FFF)
    uint16_t fs_spd;                    // Full-step speed (10-bit, 0x000-0x3FF)
    uint8_t  kval_hold;                 // Holding voltage (8-bit, 0x00-0xFF)
    uint8_t  kval_run;                  // Running voltage (8-bit, 0x00-0xFF)
    uint8_t  kval_acc;                  // Acceleration voltage (8-bit, 0x00-0xFF)
    uint8_t  kval_dec;                  // Deceleration voltage (8-bit, 0x00-0xFF)
    uint8_t  ocd_th;                    // Overcurrent threshold (4-bit, 0x0-0xF)
    uint8_t  step_mode;                 // Step mode (3-bit, 0x0-0x7)
} L6470Config_t;

// L6470 status flags - bit-field representation
typedef union {
    uint16_t raw;                       // Raw 16-bit status value
    struct {
        uint16_t hiz        : 1;        // Bit 0: High impedance state
        uint16_t busy       : 1;        // Bit 1: Busy flag
        uint16_t sw_f       : 1;        // Bit 2: Switch turn-on event
        uint16_t sw_evn     : 1;        // Bit 3: Switch status
        uint16_t dir        : 1;        // Bit 4: Direction
        uint16_t mot_status : 2;        // Bits 5-6: Motor status
        uint16_t notperf_cmd: 1;        // Bit 7: Command not performed
        uint16_t wrong_cmd  : 1;        // Bit 8: Wrong command
        uint16_t uvlo       : 1;        // Bit 9: Undervoltage lockout
        uint16_t th_wrn     : 1;        // Bit 10: Thermal warning
        uint16_t th_sd      : 1;        // Bit 11: Thermal shutdown
        uint16_t ocd        : 1;        // Bit 12: Overcurrent detection
        uint16_t step_loss_a: 1;        // Bit 13: Stall detection A
        uint16_t step_loss_b: 1;        // Bit 14: Stall detection B
        uint16_t sck_mod    : 1;        // Bit 15: Step clock mode
    } bits;
} L6470StatusFlags_t;
```

## Encoder Data Types

### AS5600 Encoder-Specific Types
```c
// AS5600 raw data types - per datasheet specifications
typedef uint16_t AS5600RawAngle_t;      // 12-bit raw angle (0-4095)
typedef uint8_t  AS5600Register_t;      // Register address
typedef uint8_t  AS5600Status_t;        // Status register value

// AS5600 processed data types
typedef float EncoderAngle_t;           // Processed angle in degrees (0.0-360.0)
typedef float EncoderVelocity_t;        // Angular velocity in degrees/second
typedef uint8_t EncoderMagnetStrength_t; // Magnet field strength (0-255)

// AS5600 configuration
typedef struct {
    uint8_t encoder_id;                 // Encoder identifier
    uint16_t zero_position;             // Zero position offset (raw counts)
    bool direction_clockwise;           // Direction flag
    uint8_t hysteresis;                 // Hysteresis setting (0-3)
    uint8_t output_stage;               // Output stage selection
    uint8_t pwm_frequency;              // PWM frequency selection
    uint8_t slow_filter;                // Slow filter selection
    uint8_t fast_filter_threshold;      // Fast filter threshold
} AS5600Config_t;

// AS5600 status flags
typedef union {
    uint8_t raw;                        // Raw status byte
    struct {
        uint8_t magnet_detected : 1;    // Bit 0: Magnet detected
        uint8_t magnet_high     : 1;    // Bit 1: Magnet too strong
        uint8_t magnet_low      : 1;    // Bit 2: Magnet too weak
        uint8_t reserved        : 5;    // Bits 3-7: Reserved
    } bits;
} AS5600StatusFlags_t;

// Encoder state information
typedef struct {
    EncoderAngle_t current_angle;       // Current angle reading
    EncoderAngle_t previous_angle;      // Previous angle reading
    EncoderVelocity_t velocity;         // Calculated velocity
    AS5600StatusFlags_t status;         // Status flags
    uint32_t last_update_time;          // Last update timestamp
    uint32_t read_count;                // Total read operations
    uint32_t error_count;               // Communication error count
    bool calibrated;                    // Calibration status
} EncoderState_t;
```

## Communication Data Types

### Generic Communication Types
```c
// Communication buffer types
typedef uint8_t* CommBuffer_t;          // Communication buffer pointer
typedef uint16_t CommLength_t;          // Message length
typedef uint32_t CommTimeout_t;         // Timeout in milliseconds
typedef uint8_t  CommAddress_t;         // Device address

// Communication status
typedef enum {
    COMM_STATUS_IDLE = 0,               // Communication idle
    COMM_STATUS_TRANSMITTING = 1,       // Transmitting data
    COMM_STATUS_RECEIVING = 2,          // Receiving data
    COMM_STATUS_ERROR = 3,              // Communication error
    COMM_STATUS_TIMEOUT = 4             // Communication timeout
} CommStatus_t;

// Generic message structure
typedef struct {
    uint8_t message_id;                 // Message identifier
    CommLength_t length;                // Message length
    uint8_t* data;                      // Message data pointer
    uint32_t timestamp;                 // Message timestamp
    uint8_t checksum;                   // Message checksum
} CommMessage_t;
```

### SPI Communication Types
```c
// SPI transaction types
typedef struct {
    uint8_t* tx_buffer;                 // Transmit buffer
    uint8_t* rx_buffer;                 // Receive buffer
    uint16_t length;                    // Transaction length
    uint32_t timeout_ms;                // Transaction timeout
    GPIO_TypeDef* cs_port;              // Chip select port
    uint16_t cs_pin;                    // Chip select pin
} SPITransaction_t;

// SPI configuration
typedef struct {
    SPI_HandleTypeDef* handle;          // HAL SPI handle
    uint32_t baudrate;                  // SPI clock frequency
    uint32_t data_size;                 // Data size (8 or 16 bit)
    uint32_t clock_polarity;            // Clock polarity
    uint32_t clock_phase;               // Clock phase
    uint32_t first_bit;                 // MSB or LSB first
} SPIConfig_t;
```

### I2C Communication Types
```c
// I2C transaction types
typedef struct {
    uint8_t device_address;             // 7-bit device address
    uint16_t register_address;          // Register address
    uint8_t register_size;              // Register address size (8 or 16 bit)
    uint8_t* data;                      // Data buffer
    uint16_t length;                    // Data length
    uint32_t timeout_ms;                // Transaction timeout
} I2CTransaction_t;

// I2C configuration
typedef struct {
    I2C_HandleTypeDef* handle;          // HAL I2C handle
    uint32_t clock_speed;               // I2C clock frequency
    uint32_t duty_cycle;                // Clock duty cycle
    uint32_t own_address;               // Own address (if slave)
    uint32_t address_mode;              // 7-bit or 10-bit addressing
} I2CConfig_t;
```

### CAN Communication Types
```c
// CAN message types
typedef struct {
    uint32_t id;                        // CAN message ID (11-bit or 29-bit)
    uint8_t data[8];                    // CAN data bytes
    uint8_t length;                     // Data length (0-8)
    uint32_t timestamp;                 // Message timestamp
    bool extended_id;                   // Extended ID flag
    bool remote_frame;                  // Remote frame flag
} CANMessage_t;

// CAN filter configuration
typedef struct {
    uint32_t filter_id;                 // Filter ID
    uint32_t filter_mask;               // Filter mask
    uint32_t filter_mode;               // Filter mode
    uint32_t filter_scale;              // Filter scale
    uint32_t filter_activation;         // Filter activation
    uint32_t filter_bank;               // Filter bank number
} CANFilter_t;

// CAN configuration
typedef struct {
    FDCAN_HandleTypeDef* handle;        // HAL FDCAN handle
    uint32_t nominal_bitrate;           // Nominal bit rate
    uint32_t data_bitrate;              // Data bit rate (CAN-FD)
    bool fd_mode;                       // CAN-FD mode enabled
    CANFilter_t* filters;               // Filter configuration array
    uint8_t filter_count;               // Number of filters
} CANConfig_t;
```

### Ethernet/TCP Types
```c
// Network address types
typedef struct {
    uint8_t bytes[4];                   // IP address bytes
} IPAddress_t;

typedef struct {
    uint8_t bytes[6];                   // MAC address bytes
} MACAddress_t;

// TCP connection types
typedef struct {
    int socket_fd;                      // Socket file descriptor
    IPAddress_t remote_ip;              // Remote IP address
    uint16_t remote_port;               // Remote port
    uint16_t local_port;                // Local port
    bool connected;                     // Connection status
    uint32_t bytes_sent;                // Total bytes sent
    uint32_t bytes_received;            // Total bytes received
} TCPConnection_t;

// Network configuration
typedef struct {
    IPAddress_t ip_address;             // Static IP address
    IPAddress_t subnet_mask;            // Subnet mask
    IPAddress_t gateway;                // Default gateway
    IPAddress_t dns_server;             // DNS server
    MACAddress_t mac_address;           // MAC address
    bool dhcp_enabled;                  // DHCP client enabled
    uint16_t tcp_server_port;           // TCP server port
} NetworkConfig_t;
```

## Safety and System Data Types

### Safety System Types
```c
// Safety state enumeration
typedef enum {
    SAFETY_STATE_UNKNOWN = 0,           // Safety state unknown
    SAFETY_STATE_SAFE = 1,              // System is safe
    SAFETY_STATE_WARNING = 2,           // Warning condition
    SAFETY_STATE_UNSAFE = 3,            // Unsafe condition detected
    SAFETY_STATE_EMERGENCY = 4          // Emergency stop triggered
} SafetyState_t;

// Watchdog types
typedef struct {
    uint32_t timeout_ms;                // Watchdog timeout period
    uint32_t last_reset_time;           // Last reset timestamp
    uint32_t reset_count;               // Total reset count
    bool enabled;                       // Watchdog enabled flag
} WatchdogState_t;

// System limits and thresholds
typedef struct {
    float temperature_max;              // Maximum operating temperature
    float voltage_min;                  // Minimum supply voltage
    float voltage_max;                  // Maximum supply voltage
    float current_max;                  // Maximum current per motor
    uint32_t memory_usage_max;          // Maximum memory usage percentage
    uint32_t cpu_usage_max;             // Maximum CPU usage percentage
} SystemLimits_t;
```

### System State Types
```c
// System operating mode
typedef enum {
    SYSTEM_MODE_INIT = 0,               // System initializing
    SYSTEM_MODE_STANDBY = 1,            // System in standby
    SYSTEM_MODE_OPERATIONAL = 2,        // Normal operation
    SYSTEM_MODE_MAINTENANCE = 3,        // Maintenance mode
    SYSTEM_MODE_ERROR = 4,              // System error state
    SYSTEM_MODE_SHUTDOWN = 5            // System shutting down
} SystemMode_t;

// System health information
typedef struct {
    SystemMode_t current_mode;          // Current operating mode
    SafetyState_t safety_state;         // Current safety state
    uint32_t uptime_seconds;            // System uptime
    float cpu_temperature;              // CPU temperature
    float supply_voltage;               // Supply voltage
    uint32_t free_heap_bytes;           // Free heap memory
    uint32_t total_heap_bytes;          // Total heap memory
    uint32_t task_switch_count;         // RTOS task switches
    uint16_t error_count;               // Total error count
} SystemHealth_t;
```

## Type Safety and Validation

### Range-Checked Types
```c
// Type-safe wrappers with validation
typedef struct {
    MotorAngle_t value;
    MotorAngle_t min_value;
    MotorAngle_t max_value;
    bool valid;
} ValidatedAngle_t;

// Type validation functions
static inline bool validate_motor_angle(MotorAngle_t angle) {
    return (angle >= 0.0f && angle <= 360.0f);
}

static inline bool validate_motor_velocity(MotorVelocity_t velocity) {
    return (velocity >= -MAX_MOTOR_VELOCITY && velocity <= MAX_MOTOR_VELOCITY);
}

static inline ValidatedAngle_t create_validated_angle(MotorAngle_t angle, 
                                                     MotorAngle_t min, 
                                                     MotorAngle_t max) {
    ValidatedAngle_t result = {
        .value = angle,
        .min_value = min,
        .max_value = max,
        .valid = (angle >= min && angle <= max)
    };
    return result;
}
```

### Compile-Time Type Checking
```c
// Static assertions for type sizes
_Static_assert(sizeof(uint8_t) == 1, "uint8_t must be 1 byte");
_Static_assert(sizeof(uint16_t) == 2, "uint16_t must be 2 bytes");
_Static_assert(sizeof(uint32_t) == 4, "uint32_t must be 4 bytes");
_Static_assert(sizeof(float) == 4, "float must be 4 bytes");

// Enum size validation
_Static_assert(sizeof(MotorState_t) == sizeof(int), "MotorState_t size mismatch");
_Static_assert(sizeof(SystemMode_t) == sizeof(int), "SystemMode_t size mismatch");

// Structure alignment validation
_Static_assert(sizeof(L6470StatusFlags_t) == 2, "L6470StatusFlags_t must be 2 bytes");
_Static_assert(sizeof(AS5600StatusFlags_t) == 1, "AS5600StatusFlags_t must be 1 byte");
```

Remember: **Use explicit, well-defined data types throughout the system. Avoid implicit conversions and validate all data at boundaries between subsystems.**
