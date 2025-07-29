/**
 * @file data_types.h
 * @brief Common Data Types - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 * 
 * @note This file contains common data type definitions used throughout the system.
 *       These types ensure consistency and portability across all modules.
 * 
 * TODO: See .instructions/data-types.md for type usage guidelines
 * TODO: See .instructions/portability.md for cross-platform considerations
 */

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ========================================================================== */
/* Standard Type Aliases (SSOT)                                              */
/* ========================================================================== */
// TODO: See .instructions/type-conventions.md for naming standards

// Ensure consistent type definitions across the system
typedef uint8_t   u8;       // 8-bit unsigned integer
typedef int8_t    s8;       // 8-bit signed integer
typedef uint16_t  u16;      // 16-bit unsigned integer
typedef int16_t   s16;      // 16-bit signed integer
typedef uint32_t  u32;      // 32-bit unsigned integer
typedef int32_t   s32;      // 32-bit signed integer
typedef uint64_t  u64;      // 64-bit unsigned integer
typedef int64_t   s64;      // 64-bit signed integer

typedef float     f32;      // 32-bit floating point
typedef double    f64;      // 64-bit floating point

// Boolean and character types
typedef bool      boolean;  // Boolean type (true/false)
typedef char      char_t;   // Character type
typedef void      void_t;   // Void type

/* ========================================================================== */
/* Time and Timestamp Types (SSOT)                                           */
/* ========================================================================== */
// TODO: See .instructions/time-handling.md for timestamp usage patterns

typedef uint32_t timestamp_ms_t;    // Timestamp in milliseconds
typedef uint64_t timestamp_us_t;    // Timestamp in microseconds
typedef uint32_t duration_ms_t;     // Duration in milliseconds
typedef uint32_t duration_us_t;     // Duration in microseconds
typedef uint32_t frequency_hz_t;    // Frequency in Hz
typedef uint32_t period_ms_t;       // Period in milliseconds

// Time structure for human-readable time
typedef struct {
    u16 year;       // Year (e.g., 2025)
    u8  month;      // Month (1-12)
    u8  day;        // Day (1-31)
    u8  hour;       // Hour (0-23)
    u8  minute;     // Minute (0-59)
    u8  second;     // Second (0-59)
    u16 millisecond;// Millisecond (0-999)
} datetime_t;

/* ========================================================================== */
/* Physical Unit Types (SSOT)                                                */
/* ========================================================================== */
// TODO: See .instructions/units.md for unit conversion functions

// Angle and position types
typedef f32 angle_deg_t;        // Angle in degrees
typedef f32 angle_rad_t;        // Angle in radians
typedef f32 position_mm_t;      // Linear position in millimeters
typedef f32 position_m_t;       // Linear position in meters
typedef s32 encoder_counts_t;   // Encoder counts (raw)
typedef s32 motor_steps_t;      // Motor steps

// Speed and acceleration types
typedef f32 angular_velocity_rpm_t;     // Angular velocity in RPM
typedef f32 angular_velocity_rad_s_t;   // Angular velocity in rad/s
typedef f32 linear_velocity_mm_s_t;     // Linear velocity in mm/s
typedef f32 angular_accel_rpm_s_t;      // Angular acceleration in RPM/s
typedef f32 angular_accel_rad_s2_t;     // Angular acceleration in rad/s²

// Electrical types
typedef f32 voltage_v_t;        // Voltage in volts
typedef f32 current_ma_t;       // Current in milliamps
typedef f32 current_a_t;        // Current in amps
typedef f32 power_w_t;          // Power in watts
typedef f32 resistance_ohm_t;   // Resistance in ohms
typedef f32 capacitance_uf_t;   // Capacitance in microfarads
typedef f32 inductance_mh_t;    // Inductance in millihenries

// Temperature types
typedef f32 temperature_c_t;    // Temperature in Celsius
typedef f32 temperature_k_t;    // Temperature in Kelvin
typedef f32 temperature_f_t;    // Temperature in Fahrenheit

// Force and torque types
typedef f32 force_n_t;          // Force in Newtons
typedef f32 torque_nm_t;        // Torque in Newton-meters
typedef f32 pressure_pa_t;      // Pressure in Pascals

/* ========================================================================== */
/* Communication Data Types (SSOT)                                           */
/* ========================================================================== */
// TODO: See .instructions/comm-types.md for protocol-specific data structures

// Network address types
typedef u32 ip_address_t;       // IPv4 address (network byte order)
typedef u16 port_number_t;      // TCP/UDP port number
typedef u8  mac_address_t[6];   // MAC address (6 bytes)

// CAN message types
typedef u16 can_id_t;           // CAN message ID (11-bit standard)
typedef u32 can_id_extended_t;  // CAN message ID (29-bit extended)
typedef u8  can_dlc_t;          // CAN data length code (0-8 for classic, 0-15 for FD)
typedef u8  can_data_t[64];     // CAN data payload (max 64 bytes for CAN-FD)

// Serial communication types
typedef u32 baud_rate_t;        // Serial baud rate
typedef u8  uart_data_t;        // UART data byte
typedef u16 modbus_register_t;  // Modbus register value

// Network packet types
typedef struct {
    u16 length;                 // Packet length
    u8  data[1500];            // Ethernet MTU size
} ethernet_packet_t;

/* ========================================================================== */
/* Control System Data Types (SSOT)                                          */
/* ========================================================================== */
// TODO: See .instructions/control-types.md for control system implementation

// PID controller types
typedef struct {
    f32 kp;                     // Proportional gain
    f32 ki;                     // Integral gain
    f32 kd;                     // Derivative gain
    f32 integral_sum;           // Integral accumulator
    f32 last_error;             // Previous error for derivative
    f32 output_min;             // Minimum output limit
    f32 output_max;             // Maximum output limit
    boolean integral_windup_protection; // Integral windup protection enable
} pid_controller_t;

// Motion profile types
typedef struct {
    f32 start_position;         // Starting position
    f32 end_position;           // Ending position
    f32 max_velocity;           // Maximum velocity
    f32 acceleration;           // Acceleration rate
    f32 deceleration;           // Deceleration rate
    f32 jerk_limit;             // Jerk limitation
    timestamp_ms_t start_time;  // Profile start time
    duration_ms_t total_time;   // Total profile time
} motion_profile_t;

// Filter types
typedef struct {
    f32 alpha;                  // Filter coefficient (0-1)
    f32 previous_output;        // Previous filter output
    boolean initialized;        // Filter initialization flag
} low_pass_filter_t;

typedef struct {
    f32 samples[8];             // Sample buffer
    u8  index;                  // Current sample index
    u8  count;                  // Number of valid samples
    f32 sum;                    // Sum of samples
} moving_average_filter_t;

/* ========================================================================== */
/* Geometry and Mathematics Types (SSOT)                                     */
/* ========================================================================== */
// TODO: See .instructions/math-types.md for mathematical operations

// 2D and 3D vector types
typedef struct {
    f32 x;                      // X component
    f32 y;                      // Y component
} vector2d_t;

typedef struct {
    f32 x;                      // X component
    f32 y;                      // Y component
    f32 z;                      // Z component
} vector3d_t;

// Matrix types
typedef struct {
    f32 elements[2][2];         // 2x2 matrix elements
} matrix2x2_t;

typedef struct {
    f32 elements[3][3];         // 3x3 matrix elements
} matrix3x3_t;

// Quaternion type for 3D rotations
typedef struct {
    f32 w;                      // Real component
    f32 x;                      // X component
    f32 y;                      // Y component
    f32 z;                      // Z component
} quaternion_t;

// Statistical types
typedef struct {
    f32 mean;                   // Mean value
    f32 variance;               // Variance
    f32 standard_deviation;     // Standard deviation
    f32 minimum;                // Minimum value
    f32 maximum;                // Maximum value
    u32 sample_count;           // Number of samples
} statistics_t;

/* ========================================================================== */
/* Buffer and Queue Types (SSOT)                                             */
/* ========================================================================== */
// TODO: See .instructions/buffer-management.md for buffer usage patterns

// Generic circular buffer
typedef struct {
    u8* data;                   // Buffer data pointer
    u32 size;                   // Buffer size
    u32 head;                   // Head index
    u32 tail;                   // Tail index
    u32 count;                  // Number of items in buffer
    boolean overflow;           // Overflow flag
} circular_buffer_t;

// FIFO queue
typedef struct {
    void** items;               // Queue item pointers
    u32 capacity;               // Queue capacity
    u32 front;                  // Front index
    u32 rear;                   // Rear index
    u32 count;                  // Number of items
} fifo_queue_t;

// Ring buffer for specific data types
#define DECLARE_RING_BUFFER(type, name) \
    typedef struct { \
        type* buffer; \
        u32 size; \
        u32 head; \
        u32 tail; \
        u32 count; \
        boolean full; \
    } name##_ring_buffer_t

// Common ring buffer declarations
DECLARE_RING_BUFFER(f32, float);
DECLARE_RING_BUFFER(u32, uint32);
DECLARE_RING_BUFFER(timestamp_ms_t, timestamp);

/* ========================================================================== */
/* Configuration and Calibration Types (SSOT)                                */
/* ========================================================================== */
// TODO: See .instructions/config-types.md for configuration management

// Configuration parameter types
typedef union {
    u32   uint32_value;         // 32-bit unsigned integer parameter
    s32   int32_value;          // 32-bit signed integer parameter
    f32   float_value;          // 32-bit floating point parameter
    bool  bool_value;           // Boolean parameter
    char  string_value[32];     // String parameter (max 32 chars)
} config_parameter_value_t;

typedef struct {
    u16 parameter_id;           // Parameter identifier
    u8  parameter_type;         // Parameter type (uint32, float, etc.)
    config_parameter_value_t value; // Parameter value
    config_parameter_value_t min_value; // Minimum allowed value
    config_parameter_value_t max_value; // Maximum allowed value
    config_parameter_value_t default_value; // Default value
    boolean read_only;          // Read-only flag
    boolean requires_restart;   // Requires system restart flag
} config_parameter_t;

// Calibration data types
typedef struct {
    f32 offset;                 // Calibration offset
    f32 scale;                  // Calibration scale factor
    f32 linearity_correction[10]; // Linearity correction coefficients
    timestamp_ms_t calibration_time; // When calibration was performed
    u32 calibration_quality;   // Calibration quality metric (0-100)
    boolean valid;              // Calibration validity flag
    u32 checksum;               // Calibration data checksum
} calibration_data_t;

/* ========================================================================== */
/* Status and Result Types (SSOT)                                            */
/* ========================================================================== */
// TODO: See .instructions/status-types.md for status reporting patterns

// Generic result type with value and error code
#define DECLARE_RESULT_TYPE(type, name) \
    typedef struct { \
        type value; \
        u32 error_code; \
        boolean valid; \
        timestamp_ms_t timestamp; \
    } name##_result_t

// Common result type declarations
DECLARE_RESULT_TYPE(f32, float);
DECLARE_RESULT_TYPE(u32, uint32);
DECLARE_RESULT_TYPE(angle_deg_t, angle);
DECLARE_RESULT_TYPE(angular_velocity_rpm_t, velocity);

// Operation status type
typedef struct {
    boolean in_progress;        // Operation in progress
    u8 progress_percent;        // Progress percentage (0-100)
    timestamp_ms_t start_time;  // Operation start time
    duration_ms_t estimated_time; // Estimated completion time
    u32 error_code;             // Error code (0 = no error)
    char status_message[64];    // Human-readable status message
} operation_status_t;

/* ========================================================================== */
/* Memory Management Types (SSOT)                                            */
/* ========================================================================== */
// TODO: See .instructions/memory-types.md for memory management patterns

// Memory block descriptor
typedef struct {
    void* address;              // Memory block address
    u32 size;                   // Memory block size
    boolean allocated;          // Allocation status
    timestamp_ms_t alloc_time;  // Allocation timestamp
    const char* file;           // Source file that allocated memory
    u32 line;                   // Line number that allocated memory
} memory_block_t;

// Memory pool descriptor
typedef struct {
    void* pool_start;           // Pool start address
    u32 pool_size;              // Total pool size
    u32 block_size;             // Individual block size
    u32 block_count;            // Number of blocks
    u32 free_blocks;            // Number of free blocks
    u8* allocation_bitmap;      // Block allocation bitmap
} memory_pool_t;

/* ========================================================================== */
/* Utility Macros and Constants (SSOT)                                       */
/* ========================================================================== */

// Array size calculation
#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof((arr)[0]))

// Min/Max macros (safe versions)
#define MIN(a, b)               (((a) < (b)) ? (a) : (b))
#define MAX(a, b)               (((a) > (b)) ? (a) : (b))
#define CLAMP(val, min, max)    (MIN(MAX(val, min), max))

// Bit manipulation macros
#define BIT_SET(reg, bit)       ((reg) |= (1U << (bit)))
#define BIT_CLEAR(reg, bit)     ((reg) &= ~(1U << (bit)))
#define BIT_TOGGLE(reg, bit)    ((reg) ^= (1U << (bit)))
#define BIT_CHECK(reg, bit)     (((reg) >> (bit)) & 1U)

// Alignment macros
#define ALIGN_UP(val, align)    (((val) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(val, align)  ((val) & ~((align) - 1))
#define IS_ALIGNED(val, align)  (((val) & ((align) - 1)) == 0)

// Byte order conversion (for network protocols)
#define SWAP16(val)             (((val) << 8) | ((val) >> 8))
#define SWAP32(val)             (((val) << 24) | (((val) & 0xFF00) << 8) | \
                                (((val) & 0xFF0000) >> 8) | ((val) >> 24))

// Mathematical constants
#define PI                      3.14159265358979323846f
#define PI_2                    1.57079632679489661923f
#define PI_4                    0.78539816339744830962f
#define E                       2.71828182845904523536f
#define SQRT_2                  1.41421356237309504880f

// Conversion constants
#define DEG_TO_RAD              (PI / 180.0f)
#define RAD_TO_DEG              (180.0f / PI)
#define RPM_TO_RAD_S            (PI / 30.0f)
#define RAD_S_TO_RPM            (30.0f / PI)

// Time conversion constants
#define MS_PER_SECOND           1000U
#define US_PER_SECOND           1000000U
#define NS_PER_SECOND           1000000000U
#define SECONDS_PER_MINUTE      60U
#define MINUTES_PER_HOUR        60U
#define HOURS_PER_DAY           24U

// Common invalid/uninitialized values
#define INVALID_TIMESTAMP       0xFFFFFFFFU
#define INVALID_ANGLE           999.9f
#define INVALID_ID              0xFFFFU
#define UNINITIALIZED_FLOAT     -999999.9f

#endif /* DATA_TYPES_H */

/**
 * @note Data Types SSOT Rules:
 * 1. All common data types MUST be defined here
 * 2. Use these types consistently throughout the system
 * 3. Physical units are embedded in type names for clarity
 * 4. Utility macros provide common operations
 * 5. Constants are centralized to prevent magic numbers
 * 6. Result types include error codes and timestamps
 */
