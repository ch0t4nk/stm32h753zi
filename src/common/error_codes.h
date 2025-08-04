/**
 * @file error_codes.h
 * @brief System Error Codes - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 * 
 * @note This file contains ALL error codes and status definitions used throughout the system.
 *       These codes are used across all communication protocols and must remain consistent.
 * 
 * TODO: See .github/instructions/error-handling.instructions.md for error handling procedures
 * TODO: See .github/instructions/safety-systems.instructions.md for error recovery implementation
 */

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#include <stdint.h>

/* ========================================================================== */
/* System Error Code Base Values (SSOT)                                      */
/* ========================================================================== */
// TODO: See .github/instructions/error-handling.instructions.md for error severity levels

#define ERROR_CODE_SUCCESS          0x0000      // Success/OK status
#define ERROR_CODE_GENERIC_BASE     0x1000      // Generic system errors
#define ERROR_CODE_MOTOR_BASE       0x2000      // Motor control errors
#define ERROR_CODE_ENCODER_BASE     0x3000      // Encoder/sensor errors
#define ERROR_CODE_COMM_BASE        0x4000      // Communication errors
#define ERROR_CODE_SAFETY_BASE      0x5000      // Safety system errors
#define ERROR_CODE_SYSTEM_BASE      0x6000      // System/hardware errors
#define ERROR_CODE_CONFIG_BASE      0x7000      // Configuration errors
#define ERROR_CODE_TEST_BASE        0x8000      // Test/debug errors

/* ========================================================================== */
/* Generic System Error Codes (SSOT)                                         */
/* ========================================================================== */

typedef enum {
    // Success codes
    SYSTEM_OK = ERROR_CODE_SUCCESS,                     // 0x0000 - Operation successful
    SYSTEM_PENDING = ERROR_CODE_SUCCESS + 1,            // 0x0001 - Operation pending
    
    // Generic error codes
    ERROR_INVALID_PARAMETER = ERROR_CODE_GENERIC_BASE,  // 0x1000 - Invalid parameter passed
    ERROR_NULL_POINTER,                                 // 0x1001 - Null pointer reference
    ERROR_BUFFER_OVERFLOW,                              // 0x1002 - Buffer overflow detected
    ERROR_BUFFER_UNDERFLOW,                             // 0x1003 - Buffer underflow detected
    ERROR_OUT_OF_RANGE,                                 // 0x1004 - Value out of valid range
    ERROR_NOT_INITIALIZED,                              // 0x1005 - Module not initialized
    ERROR_ALREADY_INITIALIZED,                          // 0x1006 - Module already initialized
    ERROR_OPERATION_FAILED,                             // 0x1007 - Generic operation failure
    ERROR_TIMEOUT,                                      // 0x1008 - Operation timeout
    ERROR_BUSY,                                         // 0x1009 - Resource busy
    ERROR_NOT_SUPPORTED,                                // 0x100A - Operation not supported
    ERROR_INVALID_STATE,                                // 0x100B - Invalid system state
    ERROR_RESOURCE_UNAVAILABLE,                         // 0x100C - Required resource unavailable
    ERROR_CHECKSUM_FAILED,                              // 0x100D - Checksum verification failed
    ERROR_VERSION_MISMATCH,                             // 0x100E - Version compatibility error
    ERROR_INSUFFICIENT_RESOURCES                        // 0x100F - Insufficient system resources
} SystemError_t;

/* ========================================================================== */
/* Motor Control Error Codes (SSOT)                                          */
/* ========================================================================== */
// TODO: See .github/instructions/error-handling.instructions.md for motor fault handling

typedef enum {
    // Motor initialization errors
    ERROR_MOTOR_INIT_FAILED = ERROR_CODE_MOTOR_BASE,    // 0x2000 - Motor initialization failed
    ERROR_MOTOR_CONFIG_INVALID,                         // 0x2001 - Invalid motor configuration
    ERROR_MOTOR_SPI_FAILED,                             // 0x2002 - SPI communication with L6470 failed
    ERROR_MOTOR_DRIVER_FAULT,                           // 0x2003 - L6470 driver reported fault
    ERROR_MOTOR_NOT_CONNECTED,                          // 0x2004 - Motor not connected/detected
    
    // Motor operation errors
    ERROR_MOTOR_OVERCURRENT,                            // 0x2010 - Motor overcurrent detected
    ERROR_MOTOR_OVERHEAT,                               // 0x2011 - Motor/driver overheating
    ERROR_MOTOR_STALL,                                  // 0x2012 - Motor stall detected
    ERROR_MOTOR_POSITION_ERROR,                         // 0x2013 - Position error too large
    ERROR_MOTOR_SPEED_ERROR,                            // 0x2014 - Speed error too large
    ERROR_MOTOR_RUNAWAY,                                // 0x2015 - Motor runaway detected
    ERROR_MOTOR_EMERGENCY_STOP,                         // 0x2016 - Emergency stop activated
    ERROR_MOTOR_LIMIT_REACHED,                          // 0x2017 - Motion limit reached
    ERROR_MOTOR_HOMING_FAILED,                          // 0x2018 - Homing sequence failed
    ERROR_MOTOR_CALIBRATION_FAILED,                     // 0x2019 - Motor calibration failed
    
    // Motor command errors
    ERROR_MOTOR_INVALID_COMMAND,                        // 0x2020 - Invalid motor command
    ERROR_MOTOR_COMMAND_TIMEOUT,                        // 0x2021 - Motor command timeout
    ERROR_MOTOR_INVALID_SPEED,                          // 0x2022 - Invalid speed parameter
    ERROR_MOTOR_INVALID_POSITION,                       // 0x2023 - Invalid position parameter
    ERROR_MOTOR_INVALID_ACCELERATION,                   // 0x2024 - Invalid acceleration parameter
    ERROR_MOTOR_MOVE_ABORTED,                           // 0x2025 - Motor move aborted
    ERROR_MOTOR_PROFILE_ERROR,                          // 0x2026 - Motion profile error
    ERROR_MOTOR_SEQUENCE_ERROR                          // 0x2027 - Motor sequence error
} MotorError_t;

/* ========================================================================== */
/* Encoder/Sensor Error Codes (SSOT)                                         */
/* ========================================================================== */
// TODO: See .github/instructions/error-handling.instructions.md for encoder fault detection

typedef enum {
    // Encoder communication errors
    ERROR_ENCODER_INIT_FAILED = ERROR_CODE_ENCODER_BASE, // 0x3000 - Encoder initialization failed
    ERROR_ENCODER_I2C_FAILED,                           // 0x3001 - I2C communication failed
    ERROR_ENCODER_NOT_RESPONDING,                       // 0x3002 - Encoder not responding
    ERROR_ENCODER_TIMEOUT,                              // 0x3003 - Encoder communication timeout
    ERROR_ENCODER_INVALID_ADDRESS,                      // 0x3004 - Invalid encoder I2C address
    
    // Encoder data errors
    ERROR_ENCODER_DATA_INVALID,                         // 0x3010 - Invalid encoder data received
    ERROR_ENCODER_ANGLE_JUMP,                           // 0x3011 - Sudden angle jump detected
    ERROR_ENCODER_STUCK,                                // 0x3012 - Encoder appears stuck
    ERROR_ENCODER_NOISE,                                // 0x3013 - Excessive encoder noise
    ERROR_ENCODER_OUT_OF_RANGE,                         // 0x3014 - Encoder reading out of range
    ERROR_ENCODER_CORRELATION_FAILED,                   // 0x3015 - Encoder-motor correlation failed
    ERROR_ENCODER_CALIBRATION_INVALID,                  // 0x3016 - Encoder calibration invalid
    ERROR_ENCODER_MAGNET_WEAK,                          // 0x3017 - Magnet field too weak (AS5600)
    ERROR_ENCODER_MAGNET_STRONG,                        // 0x3018 - Magnet field too strong (AS5600)
    ERROR_ENCODER_MAGNET_MISALIGNED,                    // 0x3019 - Magnet misaligned (AS5600)
    
    // Encoder processing errors
    ERROR_ENCODER_FILTER_FAILED,                        // 0x3020 - Encoder filter processing failed
    ERROR_ENCODER_CONVERSION_FAILED,                    // 0x3021 - Angle conversion failed
    ERROR_ENCODER_ZERO_REFERENCE_LOST,                  // 0x3022 - Zero reference position lost
    ERROR_ENCODER_RESOLUTION_ERROR,                     // 0x3023 - Encoder resolution error
    ERROR_ENCODER_LINEARIZATION_FAILED                  // 0x3024 - Encoder linearization failed
} EncoderError_t;

/* ========================================================================== */
/* Communication Error Codes (SSOT)                                          */
/* ========================================================================== */
// TODO: See .github/instructions/comm-protocols.instructions.md for communication error handling

typedef enum {
    // UART communication errors
    ERROR_UART_INIT_FAILED = ERROR_CODE_COMM_BASE,      // 0x4000 - UART initialization failed
    ERROR_UART_TX_FAILED,                               // 0x4001 - UART transmission failed
    ERROR_UART_RX_FAILED,                               // 0x4002 - UART reception failed
    ERROR_UART_TIMEOUT,                                 // 0x4003 - UART communication timeout
    ERROR_UART_FRAMING_ERROR,                           // 0x4004 - UART framing error
    ERROR_UART_PARITY_ERROR,                            // 0x4005 - UART parity error
    ERROR_UART_OVERRUN,                                 // 0x4006 - UART overrun error
    ERROR_UART_BUFFER_FULL,                             // 0x4007 - UART buffer full
    
    // CAN communication errors
    ERROR_CAN_INIT_FAILED = ERROR_CODE_COMM_BASE + 0x10, // 0x4010 - CAN initialization failed
    ERROR_CAN_TX_FAILED,                                // 0x4011 - CAN transmission failed
    ERROR_CAN_RX_FAILED,                                // 0x4012 - CAN reception failed
    ERROR_CAN_BUS_OFF,                                  // 0x4013 - CAN bus off state
    ERROR_CAN_ERROR_PASSIVE,                            // 0x4014 - CAN error passive state
    ERROR_CAN_ARBITRATION_LOST,                         // 0x4015 - CAN arbitration lost
    ERROR_CAN_MESSAGE_LOST,                             // 0x4016 - CAN message lost
    ERROR_CAN_INVALID_ID,                               // 0x4017 - Invalid CAN message ID
    ERROR_CAN_FILTER_FAILED,                            // 0x4018 - CAN filter configuration failed
    ERROR_CAN_HEARTBEAT_TIMEOUT,                        // 0x4019 - CAN heartbeat timeout
    
    // Ethernet/TCP communication errors
    ERROR_ETH_INIT_FAILED = ERROR_CODE_COMM_BASE + 0x20, // 0x4020 - Ethernet initialization failed
    ERROR_ETH_LINK_DOWN,                                // 0x4021 - Ethernet link down
    ERROR_ETH_NO_IP,                                    // 0x4022 - No IP address assigned
    ERROR_ETH_SOCKET_FAILED,                            // 0x4023 - Socket creation failed
    ERROR_ETH_BIND_FAILED,                              // 0x4024 - Socket bind failed
    ERROR_ETH_LISTEN_FAILED,                            // 0x4025 - Socket listen failed
    ERROR_ETH_ACCEPT_FAILED,                            // 0x4026 - Socket accept failed
    ERROR_ETH_SEND_FAILED,                              // 0x4027 - TCP send failed
    ERROR_ETH_RECEIVE_FAILED,                           // 0x4028 - TCP receive failed
    ERROR_ETH_CONNECTION_LOST,                          // 0x4029 - TCP connection lost
    
    // I2C communication errors
    ERROR_I2C_INIT_FAILED = ERROR_CODE_COMM_BASE + 0x30, // 0x4030 - I2C initialization failed
    ERROR_I2C_DEVICE_NOT_FOUND,                         // 0x4031 - I2C device not found
    ERROR_I2C_TIMEOUT,                                  // 0x4032 - I2C timeout
    ERROR_I2C_ARBITRATION_LOST,                         // 0x4033 - I2C arbitration lost
    ERROR_I2C_BUS_ERROR,                                // 0x4034 - I2C bus error
    ERROR_I2C_NACK_RECEIVED,                            // 0x4035 - I2C NACK received
    ERROR_I2C_OVERRUN,                                  // 0x4036 - I2C overrun error
    
    // SPI communication errors
    ERROR_SPI_INIT_FAILED = ERROR_CODE_COMM_BASE + 0x40, // 0x4040 - SPI initialization failed
    ERROR_SPI_TX_FAILED,                                // 0x4041 - SPI transmission failed
    ERROR_SPI_RX_FAILED,                                // 0x4042 - SPI reception failed
    ERROR_SPI_TIMEOUT,                                  // 0x4043 - SPI timeout
    ERROR_SPI_MODE_FAULT,                               // 0x4044 - SPI mode fault
    ERROR_SPI_OVERRUN,                                  // 0x4045 - SPI overrun error
    ERROR_SPI_CRC_ERROR,                                // 0x4046 - SPI CRC error
    ERROR_SPI_FRAME_ERROR                               // 0x4047 - SPI frame error
} CommunicationError_t;

/* ========================================================================== */
/* Safety System Error Codes (SSOT)                                          */
/* ========================================================================== */
// TODO: See .github/instructions/safety-systems.instructions.md for safety system implementation

typedef enum {
    // Safety system errors
    ERROR_SAFETY_INIT_FAILED = ERROR_CODE_SAFETY_BASE,  // 0x5000 - Safety system init failed
    ERROR_SAFETY_WATCHDOG_TIMEOUT,                      // 0x5001 - Watchdog timeout
    ERROR_SAFETY_EMERGENCY_STOP,                        // 0x5002 - Emergency stop activated
    ERROR_SAFETY_FAULT_DETECTED,                        // 0x5003 - Safety fault detected
    ERROR_SAFETY_LIMIT_EXCEEDED,                        // 0x5004 - Safety limit exceeded
    ERROR_SAFETY_INTERLOCK_OPEN,                        // 0x5005 - Safety interlock open
    ERROR_SAFETY_GUARD_OPEN,                            // 0x5006 - Safety guard open
    ERROR_SAFETY_LIGHT_CURTAIN,                         // 0x5007 - Light curtain interrupted
    ERROR_SAFETY_PRESSURE_MAT,                          // 0x5008 - Pressure mat activated
    ERROR_SAFETY_ENABLE_SWITCH,                         // 0x5009 - Enable switch not pressed
    
    // Safety monitoring errors
    ERROR_SAFETY_POWER_FAILURE,                         // 0x5010 - Power supply failure
    ERROR_SAFETY_OVERVOLTAGE,                           // 0x5011 - Overvoltage detected
    ERROR_SAFETY_UNDERVOLTAGE,                          // 0x5012 - Undervoltage detected
    ERROR_SAFETY_OVERTEMPERATURE,                       // 0x5013 - Overtemperature detected
    ERROR_SAFETY_OVERCURRENT,                           // 0x5014 - Overcurrent detected
    ERROR_SAFETY_GROUND_FAULT,                          // 0x5015 - Ground fault detected
    ERROR_SAFETY_ARC_FAULT,                             // 0x5016 - Arc fault detected
    ERROR_SAFETY_INSULATION_FAULT,                      // 0x5017 - Insulation fault detected
    
    // Safety validation errors
    ERROR_SAFETY_SELFTEST_FAILED,                       // 0x5020 - Safety self-test failed
    ERROR_SAFETY_VALIDATION_FAILED,                     // 0x5021 - Safety validation failed
    ERROR_SAFETY_REDUNDANCY_FAILED,                     // 0x5022 - Safety redundancy failed
    ERROR_SAFETY_DIAGNOSTIC_FAILED,                     // 0x5023 - Safety diagnostic failed
    ERROR_SAFETY_CONFIGURATION_ERROR,                   // 0x5024 - Safety configuration error
    ERROR_SAFETY_CALIBRATION_ERROR                      // 0x5025 - Safety calibration error
} SafetyError_t;

/* ========================================================================== */
/* System/Hardware Error Codes (SSOT)                                        */
/* ========================================================================== */
// TODO: See .github/instructions/error-handling.instructions.md for system error handling

typedef enum {
    // Hardware errors
    ERROR_HARDWARE_FAILURE = ERROR_CODE_SYSTEM_BASE,    // 0x6000 - General hardware failure
    ERROR_CPU_FAULT,                                    // 0x6001 - CPU fault detected
    ERROR_MEMORY_FAULT,                                 // 0x6002 - Memory fault detected
    ERROR_FLASH_ERROR,                                  // 0x6003 - Flash memory error
    ERROR_RAM_ERROR,                                    // 0x6004 - RAM error detected
    ERROR_CLOCK_FAILURE,                                // 0x6005 - Clock system failure
    ERROR_RESET_DETECTED,                               // 0x6006 - Unexpected reset detected
    ERROR_BROWNOUT_DETECTED,                            // 0x6007 - Brownout reset detected
    
    // Peripheral errors
    ERROR_DMA_FAILURE,                                  // 0x6010 - DMA operation failed
    ERROR_TIMER_FAILURE,                                // 0x6011 - Timer configuration failed
    ERROR_GPIO_FAILURE,                                 // 0x6012 - GPIO configuration failed
    ERROR_ADC_FAILURE,                                  // 0x6013 - ADC operation failed
    ERROR_PWM_FAILURE,                                  // 0x6014 - PWM generation failed
    ERROR_INTERRUPT_FAILURE,                            // 0x6015 - Interrupt system failure
    
    // Resource errors
    ERROR_OUT_OF_MEMORY,                                // 0x6020 - Out of memory
    ERROR_STACK_OVERFLOW,                               // 0x6021 - Stack overflow detected
    ERROR_HEAP_CORRUPTION,                              // 0x6022 - Heap corruption detected
    ERROR_RESOURCE_LEAK,                                // 0x6023 - Resource leak detected
    ERROR_DEADLOCK_DETECTED,                            // 0x6024 - Deadlock detected
    ERROR_RACE_CONDITION                                // 0x6025 - Race condition detected
} SystemHardwareError_t;

/* ========================================================================== */
/* Configuration Error Codes (SSOT)                                          */
/* ========================================================================== */

typedef enum {
    ERROR_CONFIG_INVALID = ERROR_CODE_CONFIG_BASE,      // 0x7000 - Invalid configuration
    ERROR_CONFIG_MISSING,                               // 0x7001 - Missing configuration
    ERROR_CONFIG_CORRUPT,                               // 0x7002 - Corrupt configuration
    ERROR_CONFIG_VERSION_MISMATCH,                      // 0x7003 - Configuration version mismatch
    ERROR_CONFIG_CHECKSUM_FAILED,                       // 0x7004 - Configuration checksum failed
    ERROR_CONFIG_EEPROM_FAILED,                         // 0x7005 - EEPROM access failed
    ERROR_CONFIG_PARAMETER_RANGE,                       // 0x7006 - Parameter out of range
    ERROR_CONFIG_DEPENDENCY_FAILED,                     // 0x7007 - Configuration dependency failed
    ERROR_CONFIG_VALIDATION_FAILED,                     // 0x7008 - Configuration validation failed
    ERROR_CONFIG_BACKUP_FAILED                          // 0x7009 - Configuration backup failed
} ConfigurationError_t;

/* ========================================================================== */
/* Test/Debug Error Codes (SSOT)                                             */
/* ========================================================================== */

typedef enum {
    ERROR_TEST_FRAMEWORK_FAILED = ERROR_CODE_TEST_BASE, // 0x8000 - Test framework failure
    ERROR_TEST_CASE_FAILED,                             // 0x8001 - Test case failed
    ERROR_TEST_TIMEOUT,                                 // 0x8002 - Test timeout
    ERROR_TEST_ASSERTION_FAILED,                        // 0x8003 - Test assertion failed
    ERROR_TEST_MEMORY_LEAK,                             // 0x8004 - Memory leak in test
    ERROR_TEST_INVALID_STATE,                           // 0x8005 - Invalid test state
    ERROR_DEBUG_INTERFACE_FAILED,                       // 0x8006 - Debug interface failed
    ERROR_SIMULATOR_FAILED,                             // 0x8007 - Simulator operation failed
    ERROR_MOCK_SETUP_FAILED,                            // 0x8008 - Mock setup failed
    ERROR_TEST_DATA_INVALID                             // 0x8009 - Test data invalid
} TestDebugError_t;

/* ========================================================================== */
/* Error Severity Levels (SSOT)                                              */
/* ========================================================================== */

typedef enum {
    ERROR_SEVERITY_INFO = 0,        // Informational - no action required
    ERROR_SEVERITY_WARNING = 1,     // Warning - continue with caution
    ERROR_SEVERITY_ERROR = 2,       // Error - operation failed but recoverable
    ERROR_SEVERITY_CRITICAL = 3,    // Critical - immediate action required
    ERROR_SEVERITY_FATAL = 4        // Fatal - system shutdown required
} ErrorSeverity_t;

/* ========================================================================== */
/* Error Information Structure (SSOT)                                        */
/* ========================================================================== */

typedef struct {
    uint32_t error_code;            // Error code from above enums
    ErrorSeverity_t severity;       // Error severity level
    uint32_t timestamp_ms;          // Timestamp when error occurred
    const char* description;        // Human-readable error description
    const char* file;               // Source file where error occurred
    uint32_t line;                  // Line number where error occurred
    const char* function;           // Function where error occurred
    uint32_t context_data[4];       // Additional context data
} ErrorInfo_t;

/* ========================================================================== */
/* Error Handling Macros (SSOT)                                              */
/* ========================================================================== */

// Error checking macro with file/line information
#define CHECK_ERROR(expr, error_code) \
    do { \
        if (!(expr)) { \
            log_error(error_code, __FILE__, __LINE__, __FUNCTION__); \
            return error_code; \
        } \
    } while(0)

// Error logging macro
#define LOG_ERROR(error_code, description) \
    log_error_detailed(error_code, description, __FILE__, __LINE__, __FUNCTION__)

// Error return macro
#define RETURN_ERROR(error_code) \
    do { \
        LOG_ERROR(error_code, #error_code); \
        return error_code; \
    } while(0)

/* ========================================================================== */
/* Error Utility Functions (SSOT)                                            */
/* ========================================================================== */

/**
 * @brief Convert error code to human-readable string
 * @param error_code Error code to convert
 * @return Pointer to error description string
 */
const char* error_code_to_string(uint32_t error_code);

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
void log_error(uint32_t error_code, const char* file, uint32_t line, const char* function);

/**
 * @brief Log detailed error information
 * @param error_code Error code
 * @param description Additional description
 * @param file Source file name
 * @param line Line number
 * @param function Function name
 */
void log_error_detailed(uint32_t error_code, const char* description, 
                       const char* file, uint32_t line, const char* function);

#endif /* ERROR_CODES_H */

/**
 * @note Error Code SSOT Rules:
 * 1. All error codes MUST be defined here with unique values
 * 2. Error codes must be consistent across all communication protocols
 * 3. New error codes must be added to appropriate category ranges
 * 4. Error descriptions must be clear and actionable
 * 5. Error severity levels guide automated response actions
 */
