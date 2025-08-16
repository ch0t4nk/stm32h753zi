/**
 * @file error_handling.h
 * @brief Common error handling definitions for STM32H753ZI project
 * @author Claude AI
 * @date 2024
 * @copyright MIT License
 */

#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief System error codes
 */
typedef enum {
    SYSTEM_OK = 0,                  ///< No error
    ERROR_INVALID_PARAMETER,        ///< Invalid parameter passed
    ERROR_TIMEOUT,                  ///< Operation timed out
    ERROR_HARDWARE_FAULT,           ///< Hardware fault detected
    ERROR_INITIALIZATION_FAILED,    ///< Initialization failed
    ERROR_OPERATION_FAILED,         ///< Generic operation failure
    ERROR_MEMORY_ALLOCATION_FAILED, ///< Memory allocation failed
    ERROR_NOT_IMPLEMENTED,          ///< Feature not implemented
    ERROR_BUSY,                     ///< Resource busy
    ERROR_INVALID_STATE,            ///< Invalid system state

    // Clock-specific errors
    ERROR_CLOCK_HSI_TIMEOUT,    ///< HSI ready timeout
    ERROR_CLOCK_HSE_TIMEOUT,    ///< HSE ready timeout
    ERROR_CLOCK_PLL_TIMEOUT,    ///< PLL lock timeout
    ERROR_CLOCK_SWITCH_TIMEOUT, ///< Clock switch timeout
    ERROR_CLOCK_VOS_TIMEOUT,    ///< Voltage scaling timeout
    ERROR_CLOCK_INVALID_CONFIG, ///< Invalid clock configuration

    // Safety-specific errors
    ERROR_SAFETY_WATCHDOG_TIMEOUT, ///< Watchdog timeout
    ERROR_SAFETY_EMERGENCY_STOP,   ///< Emergency stop triggered
    ERROR_SAFETY_LIMIT_VIOLATION,  ///< Safety limit violated
    ERROR_SAFETY_HARDWARE_FAULT,   ///< Safety hardware fault

    // Motor-specific errors
    ERROR_MOTOR_COMMUNICATION,  ///< Motor communication error
    ERROR_MOTOR_OVERCURRENT,    ///< Motor overcurrent detected
    ERROR_MOTOR_OVERSPEED,      ///< Motor overspeed detected
    ERROR_MOTOR_POSITION_LIMIT, ///< Motor position limit reached

    // Communication errors
    ERROR_COMM_TIMEOUT,         ///< Communication timeout
    ERROR_COMM_CRC_MISMATCH,    ///< CRC mismatch
    ERROR_COMM_PROTOCOL_ERROR,  ///< Protocol error
    ERROR_COMM_BUFFER_OVERFLOW, ///< Buffer overflow

    ERROR_COUNT ///< Total number of error codes
} SystemError_t;

/**
 * @brief Error severity levels
 */
typedef enum {
    ERROR_SEVERITY_INFO = 0, ///< Informational
    ERROR_SEVERITY_WARNING,  ///< Warning
    ERROR_SEVERITY_ERROR,    ///< Error
    ERROR_SEVERITY_CRITICAL, ///< Critical error
    ERROR_SEVERITY_FATAL     ///< Fatal error
} ErrorSeverity_t;

/**
 * @brief Error context structure
 */
typedef struct {
    SystemError_t error_code; ///< Error code
    ErrorSeverity_t severity; ///< Error severity
    uint32_t timestamp;       ///< Error timestamp
    const char *file;         ///< Source file
    uint32_t line;            ///< Source line
    const char *function;     ///< Function name
    uint32_t data;            ///< Additional error data
} ErrorContext_t;

/**
 * @brief Error handler callback function type
 */
typedef void (*ErrorHandler_t)(const ErrorContext_t *context);

/**
 * @brief Error handling macros
 */
#define ERROR_CHECK(expr)                                                     \
    do {                                                                      \
        SystemError_t _err = (expr);                                          \
        if (_err != SYSTEM_OK) {                                              \
            handle_error(_err, ERROR_SEVERITY_ERROR, __FILE__, __LINE__,      \
                         __func__, 0);                                        \
            return _err;                                                      \
        }                                                                     \
    } while (0)

#define ERROR_CHECK_RETURN_VOID(expr)                                         \
    do {                                                                      \
        SystemError_t _err = (expr);                                          \
        if (_err != SYSTEM_OK) {                                              \
            handle_error(_err, ERROR_SEVERITY_ERROR, __FILE__, __LINE__,      \
                         __func__, 0);                                        \
            return;                                                           \
        }                                                                     \
    } while (0)

#define ASSERT_PARAM(param)                                                   \
    do {                                                                      \
        if (!(param)) {                                                       \
            handle_error(ERROR_INVALID_PARAMETER, ERROR_SEVERITY_ERROR,       \
                         __FILE__, __LINE__, __func__, 0);                    \
            return ERROR_INVALID_PARAMETER;                                   \
        }                                                                     \
    } while (0)

/**
 * @brief Function prototypes
 */

/**
 * @brief Handle error with context
 * @param error_code Error code
 * @param severity Error severity
 * @param file Source file name
 * @param line Source line number
 * @param function Function name
 * @param data Additional error data
 */
void handle_error(SystemError_t error_code, ErrorSeverity_t severity,
                  const char *file, uint32_t line, const char *function,
                  uint32_t data);

/**
 * @brief Register error handler callback
 * @param handler Error handler callback
 * @return SystemError_t Success or error code
 */
SystemError_t register_error_handler(ErrorHandler_t handler);

/**
 * @brief Get error string description
 * @param error_code Error code
 * @return const char* Error description string
 */
const char *get_error_string(SystemError_t error_code);

/**
 * @brief Get last error context
 * @return const ErrorContext_t* Last error context
 */
const ErrorContext_t *get_last_error(void);

/**
 * @brief Clear error history
 */
void clear_error_history(void);

/**
 * @brief Check if error is critical
 * @param error_code Error code to check
 * @return bool True if critical
 */
bool is_critical_error(SystemError_t error_code);

#ifdef __cplusplus
}
#endif

#endif /* ERROR_HANDLING_H */
