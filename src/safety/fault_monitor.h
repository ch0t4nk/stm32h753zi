/**
 * @file fault_monitor.h
 * @brief Fault Detection and Monitoring System Interface - STM32H753ZI Stepper Motor Project
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 * 
 * @note Fault monitoring implementation following:
 * - .github/instructions/safety-rt.instructions.md
 * - config/safety_config.h (SSOT)
 * 
 * @warning SAFETY-CRITICAL: Fault detection must respond within safety reaction times
 */

#ifndef FAULT_MONITOR_H
#define FAULT_MONITOR_H

#include "config/safety_config.h"
#include "common/data_types.h"
#include "stm32h7xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* ========================================================================== */
/* Fault Type Definitions                                                    */
/* ========================================================================== */

/**
 * @brief Motor fault types with severity levels
 */
typedef enum {
    MOTOR_FAULT_NONE                = 0x00000000,
    
    // Critical faults (immediate stop required)
    MOTOR_FAULT_OVERCURRENT        = 0x00000001,  ///< Motor overcurrent detected
    MOTOR_FAULT_OVERVOLTAGE        = 0x00000002,  ///< Supply overvoltage
    MOTOR_FAULT_UNDERVOLTAGE       = 0x00000004,  ///< Supply undervoltage
    MOTOR_FAULT_OVERTEMPERATURE    = 0x00000008,  ///< Driver overtemperature
    MOTOR_FAULT_L6470_FLAG         = 0x00000010,  ///< L6470 driver fault flag
    MOTOR_FAULT_EMERGENCY_STOP     = 0x00000020,  ///< Emergency stop activated
    MOTOR_FAULT_WATCHDOG_TIMEOUT   = 0x00000040,  ///< System watchdog timeout
    MOTOR_FAULT_POSITION_ERROR     = 0x00000080,  ///< Position feedback error
    
    // Warning faults (degraded operation)
    MOTOR_FAULT_SPEED_WARNING      = 0x00000100,  ///< Speed approaching limit
    MOTOR_FAULT_CURRENT_WARNING    = 0x00000200,  ///< Current approaching limit
    MOTOR_FAULT_TEMP_WARNING       = 0x00000400,  ///< Temperature warning
    MOTOR_FAULT_VOLTAGE_WARNING    = 0x00000800,  ///< Voltage warning
    MOTOR_FAULT_ENCODER_WARNING    = 0x00001000,  ///< Encoder signal degraded
    MOTOR_FAULT_COMM_WARNING       = 0x00002000,  ///< Communication issues
    
    // System faults
    MOTOR_FAULT_MEMORY_ERROR       = 0x00010000,  ///< Memory corruption
    MOTOR_FAULT_CONFIG_ERROR       = 0x00020000,  ///< Configuration invalid
    MOTOR_FAULT_INIT_ERROR         = 0x00040000,  ///< Initialization failed
    MOTOR_FAULT_SELF_TEST_FAILED   = 0x00080000,  ///< Self-test failure
    
    // Critical fault mask
    MOTOR_FAULT_CRITICAL_MASK      = 0x000000FF,
    
    // All faults mask
    MOTOR_FAULT_ALL_MASK           = 0xFFFFFFFF
} MotorFaultType_t;

/**
 * @brief System fault types
 */
typedef enum {
    SYSTEM_FAULT_NONE              = 0x00000000,
    
    // Hardware faults
    SYSTEM_FAULT_CLOCK_FAILURE     = 0x00000001,  ///< System clock failure
    SYSTEM_FAULT_POWER_FAILURE     = 0x00000002,  ///< Power supply failure
    SYSTEM_FAULT_GPIO_FAULT        = 0x00000004,  ///< GPIO configuration fault
    SYSTEM_FAULT_SPI_FAULT         = 0x00000008,  ///< SPI communication fault
    SYSTEM_FAULT_UART_FAULT        = 0x00000010,  ///< UART communication fault
    SYSTEM_FAULT_CAN_FAULT         = 0x00000020,  ///< CAN communication fault
    SYSTEM_FAULT_COMM_ERROR        = 0x00000040,  ///< General communication error
    SYSTEM_FAULT_COMM_TIMEOUT      = 0x00000080,  ///< Communication timeout
    
    // Software faults
    SYSTEM_FAULT_STACK_OVERFLOW    = 0x00000100,  ///< Stack overflow detected
    SYSTEM_FAULT_HEAP_CORRUPTION   = 0x00000200,  ///< Heap corruption
    SYSTEM_FAULT_ASSERT_FAILED     = 0x00000400,  ///< Assertion failure
    SYSTEM_FAULT_RTOS_ERROR        = 0x00000800,  ///< RTOS error
    
    // Safety system faults
    SYSTEM_FAULT_SAFETY_VIOLATION  = 0x00001000,  ///< Safety rule violation
    SYSTEM_FAULT_REDUNDANCY_LOST   = 0x00002000,  ///< Redundancy system failure
    
    // Critical system fault mask
    SYSTEM_FAULT_CRITICAL_MASK     = 0x0000FFFF,
    
    // All system faults mask
    SYSTEM_FAULT_ALL_MASK          = 0xFFFFFFFF
} SystemFaultType_t;

/**
 * @brief Fault severity levels
 */
typedef enum {
    FAULT_SEVERITY_INFO            = 0,  ///< Informational only
    FAULT_SEVERITY_WARNING         = 1,  ///< Warning - monitor closely
    FAULT_SEVERITY_ERROR           = 2,  ///< Error - action required
    FAULT_SEVERITY_CRITICAL        = 3   ///< Critical - immediate stop
} FaultSeverity_t;

/**
 * @brief Fault record structure
 */
typedef struct {
    uint32_t fault_code;                    ///< Fault type code
    FaultSeverity_t severity;               ///< Fault severity level
    uint32_t timestamp;                     ///< Time when fault occurred
    uint32_t count;                         ///< Number of occurrences
    uint32_t motor_id;                      ///< Motor ID (if motor-specific)
    uint32_t additional_data;               ///< Additional fault-specific data
    bool active;                            ///< Whether fault is currently active
    bool acknowledged;                      ///< Whether fault has been acknowledged
} FaultRecord_t;

/**
 * @brief Fault monitor configuration
 */
typedef struct {
    bool enabled;                           ///< Monitor enabled/disabled
    uint32_t check_interval_ms;             ///< Monitoring interval
    uint32_t fault_count;                   ///< Total fault count
    uint32_t critical_fault_count;          ///< Critical fault count
    uint32_t last_check_time;               ///< Last monitoring cycle time
    uint32_t max_fault_records;             ///< Maximum fault records to keep
    uint32_t current_fault_count;           ///< Current number of active faults
} FaultMonitorConfig_t;

/* ========================================================================== */
/* L6470 Fault Status Definitions                                            */
/* ========================================================================== */

/**
 * @brief L6470 status register fault flags
 */
typedef enum {
    L6470_FAULT_NONE               = 0x0000,
    L6470_FAULT_OVERCURRENT_B      = 0x0001,  ///< Bridge B overcurrent
    L6470_FAULT_OVERCURRENT_A      = 0x0002,  ///< Bridge A overcurrent
    L6470_FAULT_THERMAL_SHUTDOWN   = 0x0004,  ///< Thermal shutdown
    L6470_FAULT_THERMAL_WARNING    = 0x0008,  ///< Thermal warning
    L6470_FAULT_UVLO               = 0x0010,  ///< Undervoltage lockout
    L6470_FAULT_STALL_DETECTION    = 0x0020,  ///< Stall detection
    L6470_FAULT_SWITCH_EVENT       = 0x0040,  ///< Switch turn-on event
    L6470_FAULT_COMMAND_ERROR      = 0x0080,  ///< Wrong command
    L6470_FAULT_ALL_MASK           = 0x00FF
} L6470FaultFlags_t;

/* ========================================================================== */
/* Public Function Prototypes                                                */
/* ========================================================================== */

/**
 * @brief Initialize fault monitoring system
 * @return System error code
 */
SystemError_t fault_monitor_init(void);

/**
 * @brief Perform periodic fault monitoring checks
 * @return System error code
 */
SystemError_t fault_monitor_check(void);

/**
 * @brief Record a motor fault
 * @param motor_id Motor identifier (0-based)
 * @param fault_type Type of fault detected
 * @param severity Severity level of fault
 * @param additional_data Optional additional fault data
 * @return System error code
 */
SystemError_t fault_monitor_record_motor_fault(uint8_t motor_id, 
                                                MotorFaultType_t fault_type,
                                                FaultSeverity_t severity,
                                                uint32_t additional_data);

/**
 * @brief Record a system fault
 * @param fault_type Type of system fault
 * @param severity Severity level of fault
 * @param additional_data Optional additional fault data
 * @return System error code
 */
SystemError_t fault_monitor_record_system_fault(SystemFaultType_t fault_type,
                                                 FaultSeverity_t severity,
                                                 uint32_t additional_data);

/**
 * @brief Clear a specific fault
 * @param fault_code Fault code to clear
 * @return System error code
 */
SystemError_t fault_monitor_clear_fault(uint32_t fault_code);

/**
 * @brief Clear all faults of specified severity or lower
 * @param max_severity Maximum severity to clear (inclusive)
 * @return System error code
 */
SystemError_t fault_monitor_clear_faults_by_severity(FaultSeverity_t max_severity);

/**
 * @brief Acknowledge a specific fault
 * @param fault_code Fault code to acknowledge
 * @return System error code
 */
SystemError_t fault_monitor_acknowledge_fault(uint32_t fault_code);

/**
 * @brief Check if any critical faults are active
 * @return true if critical faults exist, false otherwise
 */
bool fault_monitor_has_critical_faults(void);

/**
 * @brief Get count of active faults by severity
 * @param severity Severity level to count
 * @return Number of active faults at specified severity
 */
uint32_t fault_monitor_get_fault_count_by_severity(FaultSeverity_t severity);

/**
 * @brief Get active motor faults for specific motor
 * @param motor_id Motor identifier
 * @return Bitmask of active motor faults
 */
uint32_t fault_monitor_get_motor_faults(uint8_t motor_id);

/**
 * @brief Get active system faults
 * @return Bitmask of active system faults
 */
uint32_t fault_monitor_get_system_faults(void);

/**
 * @brief Get fault record by index
 * @param index Fault record index
 * @param fault_record Pointer to fault record structure to fill
 * @return System error code
 */
SystemError_t fault_monitor_get_fault_record(uint32_t index, FaultRecord_t* fault_record);

/**
 * @brief Get total number of fault records
 * @return Number of fault records
 */
uint32_t fault_monitor_get_fault_record_count(void);

/**
 * @brief Perform fault monitor self-test
 * @return System error code
 */
SystemError_t fault_monitor_self_test(void);

/**
 * @brief Check L6470 driver status for faults
 * @param motor_id Motor identifier
 * @return L6470 fault flags
 */
L6470FaultFlags_t fault_monitor_check_l6470_status(uint8_t motor_id);

/**
 * @brief Monitor motor current levels
 * @param motor_id Motor identifier
 * @param current_ma Current reading in milliamps
 * @return System error code
 */
SystemError_t fault_monitor_check_motor_current(uint8_t motor_id, uint32_t current_ma);

/**
 * @brief Monitor motor speed levels
 * @param motor_id Motor identifier
 * @param speed_steps_per_sec Speed in steps per second
 * @return System error code
 */
SystemError_t fault_monitor_check_motor_speed(uint8_t motor_id, uint32_t speed_steps_per_sec);

/**
 * @brief Monitor system voltage levels
 * @param voltage_mv Supply voltage in millivolts
 * @return System error code
 */
SystemError_t fault_monitor_check_voltage(uint32_t voltage_mv);

/**
 * @brief Monitor system temperature
 * @param temperature_c Temperature in degrees Celsius
 * @return System error code
 */
SystemError_t fault_monitor_check_temperature(int32_t temperature_c);

/**
 * @brief Monitor encoder position accuracy
 * @param motor_id Motor identifier
 * @param expected_position Expected position
 * @param actual_position Actual encoder position
 * @return System error code
 */
SystemError_t fault_monitor_check_position_accuracy(uint8_t motor_id, 
                                                     int32_t expected_position,
                                                     int32_t actual_position);

/**
 * @brief Enable/disable fault monitoring
 * @param enabled True to enable, false to disable
 * @return System error code
 */
SystemError_t fault_monitor_set_enabled(bool enabled);

/**
 * @brief Get fault monitor configuration
 * @return Fault monitor configuration structure
 */
FaultMonitorConfig_t fault_monitor_get_config(void);

/**
 * @brief Get fault monitor statistics
 * @param total_faults Pointer to store total fault count
 * @param critical_faults Pointer to store critical fault count
 * @param active_faults Pointer to store active fault count
 * @return System error code
 */
SystemError_t fault_monitor_get_statistics(uint32_t* total_faults,
                                            uint32_t* critical_faults,
                                            uint32_t* active_faults);

/* ========================================================================== */
/* Safety Integration Macros                                                 */
/* ========================================================================== */

/**
 * @brief Quick fault check macro for critical operations
 */
#define FAULT_CHECK_CRITICAL() \
    do { \
        if (fault_monitor_has_critical_faults()) { \
            return ERROR_SAFETY_CRITICAL_FAULT; \
        } \
    } while(0)

/**
 * @brief Record motor fault with automatic severity determination
 */
#define RECORD_MOTOR_FAULT(motor_id, fault_type, data) \
    do { \
        FaultSeverity_t severity = ((fault_type) & MOTOR_FAULT_CRITICAL_MASK) ? \
                                   FAULT_SEVERITY_CRITICAL : FAULT_SEVERITY_ERROR; \
        fault_monitor_record_motor_fault((motor_id), (fault_type), severity, (data)); \
    } while(0)

/**
 * @brief Record system fault with automatic severity determination
 */
#define RECORD_SYSTEM_FAULT(fault_type, data) \
    do { \
        FaultSeverity_t severity = ((fault_type) & SYSTEM_FAULT_CRITICAL_MASK) ? \
                                   FAULT_SEVERITY_CRITICAL : FAULT_SEVERITY_ERROR; \
        fault_monitor_record_system_fault((fault_type), severity, (data)); \
    } while(0)

#endif /* FAULT_MONITOR_H */
