/**
 * @file error_codes.h
 * @brief System Error Codes - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note This file contains ALL error codes and status definitions used
 * throughout the system. These codes are used across all communication
 * protocols and must remain consistent.
 *
 * TODO: See .github/instructions/error-handling.instructions.md for error
 * handling procedures
 * TODO: See .github/instructions/safety-systems.instructions.md for error
 * recovery implementation
 */

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 */
/* System Error Code Base Values (SSOT)                                      */
/* ==========================================================================
 */
// TODO: See .github/instructions/error-handling.instructions.md for error
// severity levels

#define ERROR_CODE_SUCCESS 0x0000      // Success/OK status
#define ERROR_CODE_BASE 0x1000         // Base for error codes
#define ERROR_CODE_GENERIC_BASE 0x1000 // Generic system errors
#define ERROR_CODE_MOTOR_BASE 0x2000   // Motor control errors
#define ERROR_CODE_ENCODER_BASE 0x3000 // Encoder/sensor errors
#define ERROR_CODE_COMM_BASE 0x4000    // Communication errors
#define ERROR_CODE_SAFETY_BASE 0x5000  // Safety system errors
#define ERROR_CODE_SYSTEM_BASE 0x6000  // System/hardware errors
#define ERROR_CODE_CONFIG_BASE 0x7000  // Configuration errors
#define ERROR_CODE_TEST_BASE 0x8000    // Test/debug errors

/* ==========================================================================
 */
/* Generic System Error Codes (SSOT)                                         */
/* ==========================================================================
 */

typedef enum {
  // Success codes
  SYSTEM_OK = ERROR_CODE_SUCCESS,          // 0x0000 - Operation successful
  SYSTEM_PENDING = ERROR_CODE_SUCCESS + 1, // 0x0001 - Operation pending

  // Generic error codes
  ERROR_INVALID_PARAMETER =
      ERROR_CODE_GENERIC_BASE,  // 0x1000 - Invalid parameter passed
  ERROR_NULL_POINTER,           // 0x1001 - Null pointer reference
  ERROR_BUFFER_OVERFLOW,        // 0x1002 - Buffer overflow detected
  ERROR_BUFFER_UNDERFLOW,       // 0x1003 - Buffer underflow detected
  ERROR_OUT_OF_RANGE,           // 0x1004 - Value out of valid range
  ERROR_NOT_INITIALIZED,        // 0x1005 - Module not initialized
  ERROR_ALREADY_INITIALIZED,    // 0x1006 - Module already initialized
  ERROR_OPERATION_FAILED,       // 0x1007 - Generic operation failure
  ERROR_TIMEOUT,                // 0x1008 - Operation timeout
  ERROR_BUSY,                   // 0x1009 - Resource busy
  ERROR_NOT_SUPPORTED,          // 0x100A - Operation not supported
  ERROR_INVALID_STATE,          // 0x100B - Invalid system state
  ERROR_RESOURCE_UNAVAILABLE,   // 0x100C - Required resource unavailable
  ERROR_CHECKSUM_FAILED,        // 0x100D - Checksum verification failed
  ERROR_VERSION_MISMATCH,       // 0x100E - Version compatibility error
  ERROR_INSUFFICIENT_RESOURCES, // 0x100F - Insufficient system resources
  ERROR_MEMORY_ALLOCATION,      // 0x1010 - Memory allocation failed
  ERROR_RETRY_LIMIT_EXCEEDED,   // 0x1011 - Maximum retry attempts exceeded
  ERROR_CONFIGURATION_INVALID,  // 0x1012 - Invalid configuration detected

  // RTOS Error Codes
  ERROR_QUEUE_FULL,     // 0x1013 - Queue is full, cannot add item
  ERROR_STACK_OVERFLOW, // 0x1014 - Stack overflow detected

  // Timer Error Codes
  ERROR_TIMER_INIT_FAILED,  // 0x1015 - Timer initialization failed
  ERROR_TIMER_START_FAILED, // 0x1016 - Timer start operation failed

  // Motor Control Error Codes - integrated into SystemError_t
  ERROR_MOTOR_INIT_FAILED =
      ERROR_CODE_MOTOR_BASE,  // 0x2000 - Motor initialization failed
  ERROR_MOTOR_CONFIG_INVALID, // 0x2001 - Invalid motor configuration
  ERROR_MOTOR_SPI_FAILED,     // 0x2002 - SPI communication with L6470 failed
  ERROR_MOTOR_DRIVER_FAULT,   // 0x2003 - L6470 driver reported fault
  ERROR_MOTOR_NOT_CONNECTED,  // 0x2004 - Motor not connected/detected
  ERROR_MOTOR_OVERCURRENT,    // 0x2005 - Motor overcurrent detected
  ERROR_MOTOR_OVERHEAT,       // 0x2006 - Motor/driver overheating
  ERROR_MOTOR_STALL,          // 0x2007 - Motor stall detected
  ERROR_MOTOR_POSITION_ERROR, // 0x2008 - Position error too large
  ERROR_MOTOR_SPEED_ERROR,    // 0x2009 - Speed error too large
  ERROR_MOTOR_RUNAWAY,        // 0x200A - Motor runaway detected
  ERROR_MOTOR_EMERGENCY_STOP, // 0x200B - Emergency stop activated
  ERROR_MOTOR_LIMIT_REACHED,  // 0x200C - Motion limit reached
  ERROR_MOTOR_HOMING_FAILED,  // 0x200D - Homing sequence failed
  ERROR_MOTOR_CALIBRATION_FAILED,     // 0x200E - Motor calibration failed
  ERROR_MOTOR_INVALID_COMMAND,        // 0x200F - Invalid motor command
  ERROR_MOTOR_COMMAND_TIMEOUT,        // 0x2010 - Motor command timeout
  ERROR_MOTOR_INVALID_SPEED,          // 0x2011 - Invalid speed parameter
  ERROR_MOTOR_INVALID_POSITION,       // 0x2012 - Invalid position parameter
  ERROR_MOTOR_INVALID_ACCELERATION,   // 0x2013 - Invalid acceleration
                                      // parameter
  ERROR_MOTOR_MOVE_ABORTED,           // 0x2014 - Motor move aborted
  ERROR_MOTOR_INVALID_ID,             // 0x2015 - Invalid motor ID
  ERROR_MOTOR_POSITION_LIMIT,         // 0x2016 - Motor position limit exceeded
  ERROR_MOTOR_SPEED_LIMIT,            // 0x2017 - Motor speed limit exceeded
  ERROR_MOTOR_PARAMETER_INVALID,      // 0x2018 - Invalid motor parameter
  ERROR_MOTOR_PARAMETER_OUT_OF_RANGE, // 0x2019 - Motor parameter out of
                                      // range
  ERROR_MOTOR_UNDERVOLTAGE,           // 0x201A - Motor supply undervoltage
  ERROR_MOTOR_POSITION_OUT_OF_RANGE,  // 0x201B - Position parameter out of
                                      // range
  ERROR_MOTOR_NOT_ENABLED,            // 0x201C - Motor not enabled
  ERROR_MOTOR_PROFILE_ERROR,          // 0x201D - Motion profile error
  ERROR_MOTOR_SEQUENCE_ERROR,         // 0x201E - Motor sequence error
  ERROR_MOTOR_COMMUNICATION_FAILED,   // 0x201F - Motor communication failed

  // Additional optimization-specific motor errors
  ERROR_OPERATION_IN_PROGRESS,       // 0x2020 - Operation already in progress
  ERROR_INVALID_DATA,                // 0x2021 - Invalid data provided
  ERROR_SIGNAL_QUALITY_LOW,          // 0x2022 - Signal quality too low
  ERROR_PARAMETER_VALIDATION_FAILED, // 0x2023 - Parameter validation failed

  // Encoder/Sensor Error Codes - integrated into SystemError_t
  ERROR_ENCODER_INIT_FAILED =
      ERROR_CODE_ENCODER_BASE,       // 0x3000 - Encoder initialization failed
  ERROR_ENCODER_I2C_FAILED,          // 0x3001 - I2C communication failed
  ERROR_ENCODER_NOT_RESPONDING,      // 0x3002 - Encoder not responding
  ERROR_ENCODER_TIMEOUT,             // 0x3003 - Encoder communication timeout
  ERROR_ENCODER_INVALID_ADDRESS,     // 0x3004 - Invalid encoder I2C address
  ERROR_ENCODER_CONFIG_INVALID,      // 0x3005 - Invalid encoder configuration
  ERROR_ENCODER_COMMUNICATION,       // 0x3006 - General communication error
  ERROR_ENCODER_DATA_INVALID,        // 0x3007 - Invalid encoder data received
  ERROR_ENCODER_ANGLE_JUMP,          // 0x3008 - Sudden angle jump detected
  ERROR_ENCODER_STUCK,               // 0x3009 - Encoder appears stuck
  ERROR_ENCODER_NOISE,               // 0x300A - Excessive encoder noise
  ERROR_ENCODER_OUT_OF_RANGE,        // 0x300B - Encoder reading out of range
  ERROR_ENCODER_CORRELATION_FAILED,  // 0x300C - Encoder-motor correlation
                                     // failed
  ERROR_ENCODER_CALIBRATION_INVALID, // 0x300D - Encoder calibration invalid
  ERROR_ENCODER_MAGNET_WEAK,         // 0x300E - Magnet field too weak (AS5600)
  ERROR_ENCODER_MAGNET_STRONG,     // 0x300F - Magnet field too strong (AS5600)
  ERROR_ENCODER_MAGNET_MISALIGNED, // 0x3010 - Magnet misaligned (AS5600)
  ERROR_ENCODER_INVALID_ID,        // 0x3011 - Invalid encoder ID
  ERROR_ENCODER_MAGNET_NOT_DETECTED, // 0x3012 - Magnet not detected
  ERROR_ENCODER_MAGNET_TOO_STRONG,   // 0x3013 - Magnet field too strong
  ERROR_ENCODER_MAGNET_TOO_WEAK,     // 0x3014 - Magnet field too weak
  ERROR_ENCODER_FILTER_FAILED,     // 0x3015 - Encoder filter processing failed
  ERROR_ENCODER_CONVERSION_FAILED, // 0x3016 - Angle conversion failed
  ERROR_ENCODER_ZERO_REFERENCE_LOST,  // 0x3017 - Zero reference position lost
  ERROR_ENCODER_RESOLUTION_ERROR,     // 0x3018 - Encoder resolution error
  ERROR_ENCODER_LINEARIZATION_FAILED, // 0x3019 - Encoder linearization
                                      // failed

  // Communication Error Codes - integrated into SystemError_t
  ERROR_UART_INIT_FAILED =
      ERROR_CODE_COMM_BASE,      // 0x4000 - UART initialization failed
  ERROR_UART_TX_FAILED,          // 0x4001 - UART transmission failed
  ERROR_UART_RX_FAILED,          // 0x4002 - UART reception failed
  ERROR_UART_TIMEOUT,            // 0x4003 - UART communication timeout
  ERROR_UART_FRAMING_ERROR,      // 0x4004 - UART framing error
  ERROR_UART_PARITY_ERROR,       // 0x4005 - UART parity error
  ERROR_UART_OVERRUN,            // 0x4006 - UART overrun error
  ERROR_UART_BUFFER_FULL,        // 0x4007 - UART buffer full
  ERROR_CAN_INIT_FAILED,         // 0x4008 - CAN initialization failed
  ERROR_CAN_TX_FAILED,           // 0x4009 - CAN transmission failed
  ERROR_CAN_RX_FAILED,           // 0x400A - CAN reception failed
  ERROR_CAN_BUS_OFF,             // 0x400B - CAN bus off state
  ERROR_CAN_ERROR_PASSIVE,       // 0x400C - CAN error passive state
  ERROR_CAN_ARBITRATION_LOST,    // 0x400D - CAN arbitration lost
  ERROR_CAN_MESSAGE_LOST,        // 0x400E - CAN message lost
  ERROR_CAN_INVALID_ID,          // 0x400F - Invalid CAN message ID
  ERROR_CAN_FILTER_FAILED,       // 0x4010 - CAN filter configuration failed
  ERROR_CAN_HEARTBEAT_TIMEOUT,   // 0x4011 - CAN heartbeat timeout
  ERROR_COMM_INIT_FAILED,        // 0x4012 - Communication initialization failed
  ERROR_ETH_INIT_FAILED,         // 0x4013 - Ethernet initialization failed
  ERROR_ETH_LINK_DOWN,           // 0x4014 - Ethernet link down
  ERROR_ETH_NO_IP,               // 0x4014 - No IP address assigned
  ERROR_ETH_SOCKET_FAILED,       // 0x4015 - Socket creation failed
  ERROR_ETH_BIND_FAILED,         // 0x4016 - Socket bind failed
  ERROR_ETH_LISTEN_FAILED,       // 0x4017 - Socket listen failed
  ERROR_ETH_ACCEPT_FAILED,       // 0x4018 - Socket accept failed
  ERROR_ETH_SEND_FAILED,         // 0x4019 - TCP send failed
  ERROR_ETH_RECEIVE_FAILED,      // 0x401A - TCP receive failed
  ERROR_ETH_CONNECTION_LOST,     // 0x401B - TCP connection lost
  ERROR_I2C_INIT_FAILED,         // 0x401C - I2C initialization failed
  ERROR_I2C_DEVICE_NOT_FOUND,    // 0x401D - I2C device not found
  ERROR_I2C_TIMEOUT,             // 0x401E - I2C timeout
  ERROR_I2C_ARBITRATION_LOST,    // 0x401F - I2C arbitration lost
  ERROR_I2C_BUS_ERROR,           // 0x4020 - I2C bus error
  ERROR_I2C_NACK_RECEIVED,       // 0x4021 - I2C NACK received
  ERROR_I2C_OVERRUN,             // 0x4022 - I2C overrun error
  ERROR_SPI_INIT_FAILED,         // 0x4023 - SPI initialization failed
  ERROR_SPI_TX_FAILED,           // 0x4024 - SPI transmission failed
  ERROR_SPI_RX_FAILED,           // 0x4025 - SPI reception failed
  ERROR_SPI_TIMEOUT,             // 0x4026 - SPI timeout
  ERROR_SPI_MODE_FAULT,          // 0x4027 - SPI mode fault
  ERROR_SPI_OVERRUN,             // 0x4028 - SPI overrun error
  ERROR_SPI_CRC_ERROR,           // 0x4029 - SPI CRC error
  ERROR_SPI_FRAME_ERROR,         // 0x402A - SPI frame error
  ERROR_SPI_TRANSMISSION_FAILED, // 0x402B - SPI transmission failed

  // Safety System Error Codes
  ERROR_SAFETY_SYSTEM_NOT_READY =
      ERROR_CODE_SAFETY_BASE,         // 0x5000 - Safety system not operational
  ERROR_SAFETY_EMERGENCY_STOP,        // 0x5001 - Emergency stop activated
  ERROR_SAFETY_WATCHDOG_TIMEOUT,      // 0x5002 - Watchdog timeout
  ERROR_SAFETY_WATCHDOG_WARNING,      // 0x5003 - Watchdog warning
  ERROR_SAFETY_WATCHDOG_MISSED,       // 0x5004 - Watchdog refresh missed
  ERROR_SAFETY_WATCHDOG_INIT_FAILED,  // 0x5005 - Watchdog initialization
                                      // failed
  ERROR_SAFETY_CRITICAL_FAULT,        // 0x5006 - Critical fault detected
  ERROR_SAFETY_SELF_TEST_FAILED,      // 0x5007 - Safety self-test failed
  ERROR_SAFETY_VIOLATION,             // 0x5008 - Safety rule violation
  ERROR_SAFETY_REDUNDANCY_LOST,       // 0x5009 - Safety redundancy lost
  ERROR_SAFETY_SENSOR_FAULT,          // 0x500A - Safety sensor fault
  ERROR_SAFETY_ACTUATOR_FAULT,        // 0x500B - Safety actuator fault
  ERROR_SAFETY_ESTOP_BUTTON_FAULT,    // 0x500C - Emergency stop button fault
  ERROR_SAFETY_RELAY_FAULT,           // 0x500D - Safety relay fault
  ERROR_SAFETY_MONITOR_FAULT,         // 0x500E - Safety monitor fault
  ERROR_SAFETY_PARAMETER_INVALID,     // 0x500F - Safety parameter invalid
  ERROR_SAFETY_REACTION_TIME,         // 0x5010 - Safety reaction time exceeded
  ERROR_SAFETY_INVALID_RESET,         // 0x5011 - Invalid safety reset attempt
  ERROR_SAFETY_BUTTON_STILL_PRESSED,  // 0x5012 - Emergency button still
                                      // pressed
  ERROR_SAFETY_RESET_TOO_SOON,        // 0x5013 - Reset attempted too soon
  ERROR_SAFETY_BUTTON_STUCK,          // 0x5014 - Emergency button stuck
  ERROR_SAFETY_EXCESSIVE_ACTIVATIONS, // 0x5015 - Excessive emergency stop
                                      // activations
  ERROR_SAFETY_LIMIT_VIOLATION,       // 0x5016 - Safety limit violation

  // System/Hardware Error Codes
  ERROR_SYSTEM_INIT_FAILED =
      ERROR_CODE_SYSTEM_BASE,      // 0x6000 - System initialization failed
  ERROR_FAULT_NOT_FOUND,           // 0x6001 - Fault record not found
  ERROR_COMM_INVALID_MESSAGE,      // 0x6002 - Invalid communication message
  ERROR_COMM_MESSAGE_TOO_LARGE,    // 0x6003 - Message too large
  ERROR_COMM_UNSUPPORTED_PROTOCOL, // 0x6004 - Unsupported protocol
  ERROR_COMM_UNSUPPORTED_COMMAND,  // 0x6005 - Unsupported command
  ERROR_COMM_INVALID_COMMAND,      // 0x6006 - Invalid command format
  ERROR_COMM_CHECKSUM_FAILED,      // 0x6007 - Message checksum failed
  ERROR_COMM_BUSY,                 // 0x6008 - Communication busy
  ERROR_COMM_SEND_FAILED,          // 0x6009 - Send operation failed
  ERROR_COMM_DMA_FAILED,           // 0x600A - DMA operation failed
  ERROR_COMM_SELF_TEST_FAILED,     // 0x600B - Communication self-test failed
  ERROR_COMM_TIMEOUT,              // 0x600C - Communication timeout

  // 0x7000 range: System and Hardware Faults
  ERROR_SYSTEM_FAULT =
      ERROR_CODE_BASE | 0x7000, // 0x7000 - General system fault
  ERROR_HARDWARE_FAULT,         // 0x7001 - Hardware fault detected
  ERROR_POWER_SUPPLY_FAULT,     // 0x7002 - Power supply fault
  ERROR_TEMPERATURE_FAULT,      // 0x7003 - Temperature out of range
  ERROR_THERMAL_THROTTLE,       // 0x7004 - Thermal throttling activated
  ERROR_VOLTAGE_FAULT,          // 0x7005 - Voltage out of range
  ERROR_CLOCK_FAULT,            // 0x7006 - System clock fault
  ERROR_HARDWARE_FAILURE,       // 0x7007 - Hardware failure
  ERROR_UNKNOWN,                // 0x7008 - Unknown error
  ERROR_GPIO_WRITE_FAILED,      // 0x7009 - GPIO write operation failed
  ERROR_CONFIG_OUT_OF_RANGE,    // 0x700A - Configuration parameter out of range
} SystemError_t;

/* ==========================================================================
 */
/* Error Severity Levels (SSOT)                                              */
/* ==========================================================================
 */

typedef enum {
  ERROR_SEVERITY_INFO = 0,     // Informational - no action required
  ERROR_SEVERITY_WARNING = 1,  // Warning - continue with caution
  ERROR_SEVERITY_ERROR = 2,    // Error - operation failed but recoverable
  ERROR_SEVERITY_CRITICAL = 3, // Critical - immediate action required
  ERROR_SEVERITY_FATAL = 4     // Fatal - system shutdown required
} ErrorSeverity_t;

/* ==========================================================================
 */
/* Error Information Structure (SSOT)                                        */
/* ==========================================================================
 */

typedef struct {
  uint32_t error_code;      // Error code from above enums
  ErrorSeverity_t severity; // Error severity level
  uint32_t timestamp_ms;    // Timestamp when error occurred
  const char *description;  // Human-readable error description
  const char *file;         // Source file where error occurred
  uint32_t line;            // Line number where error occurred
  const char *function;     // Function where error occurred
  uint32_t context_data[4]; // Additional context data
} ErrorInfo_t;

/* ==========================================================================
 */
/* Error Handling Macros (SSOT)                                              */
/* ==========================================================================
 */

// Error checking macro with file/line information
#define CHECK_ERROR(expr, error_code)                                          \
  do {                                                                         \
    if (!(expr)) {                                                             \
      log_error(error_code, __FILE__, __LINE__, __FUNCTION__);                 \
      return error_code;                                                       \
    }                                                                          \
  } while (0)

// Error logging macro
#define LOG_ERROR(error_code, description)                                     \
  log_error_detailed(error_code, description, __FILE__, __LINE__, __FUNCTION__)

// Error return macro
#define RETURN_ERROR(error_code)                                               \
  do {                                                                         \
    LOG_ERROR(error_code, #error_code);                                        \
    return error_code;                                                         \
  } while (0)

/* ==========================================================================
 */
/* Error Utility Functions (SSOT)                                            */
/* ==========================================================================
 */

/**
 * @brief Convert error code to human-readable string
 * @param error_code Error code to convert
 * @return Pointer to error description string
 */
const char *error_code_to_string(uint32_t error_code);

/**
 * @brief Get error severity from error code
 * @param error_code Error code
 * @return Error severity level
 */
ErrorSeverity_t get_error_severity(uint32_t error_code);

/**
 * @brief Check if error code indicates a critical condition
 * @param error_code Error code to check
 * @return true if critical, false otherwise
 */
bool is_critical_error(uint32_t error_code);

/**
 * @brief Log error with file/line information
 * @param error_code Error code
 * @param file Source file name
 * @param line Line number
 * @param function Function name
 */
void log_error(uint32_t error_code, const char *file, uint32_t line,
               const char *function);

/**
 * @brief Log detailed error information
 * @param error_code Error code
 * @param description Additional description
 * @param file Source file name
 * @param line Line number
 * @param function Function name
 */
void log_error_detailed(uint32_t error_code, const char *description,
                        const char *file, uint32_t line, const char *function);

#endif /* ERROR_CODES_H */

/**
 * @note Error Code SSOT Rules:
 * 1. All error codes MUST be defined here with unique values
 * 2. Error codes must be consistent across all communication protocols
 * 3. New error codes must be added to appropriate category ranges
 * 4. Error descriptions must be clear and actionable
 * 5. Error severity levels guide automated response actions
 */
