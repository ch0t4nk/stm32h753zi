/**
 * @file l6470_fault_monitor.h
 * @brief L6470 Hardware Fault Monitoring - GPIO Interrupt Handlers
 * @author STM32H753ZI Project Team
 * @date 2025-08-07
 *
 * @note Hardware fault monitoring implementation for L6470 FLAG and BUSY pins
 * Provides real-time fault detection and emergency response
 */

#ifndef L6470_FAULT_MONITOR_H
#define L6470_FAULT_MONITOR_H

#include "common/error_codes.h"
#include "config/hardware_config.h"
#include "config/safety_config.h"
#ifndef UNITY_TESTING
#include "stm32h7xx_hal.h"
#endif
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 */
/* L6470 Fault Types and Status                                             */
/* ==========================================================================
 */

/**
 * @brief L6470 hardware fault types for GPIO monitoring
 */
typedef enum {
  L6470_HW_FAULT_NONE = 0x00,
  L6470_HW_FAULT_OVERCURRENT = 0x01,
  L6470_HW_FAULT_THERMAL_WARNING = 0x02,
  L6470_HW_FAULT_THERMAL_SHUTDOWN = 0x04,
  L6470_HW_FAULT_UNDERVOLTAGE = 0x08,
  L6470_HW_FAULT_STEP_LOSS = 0x10,
  L6470_HW_FAULT_STALL_DETECTED = 0x20,
  L6470_HW_FAULT_COMMAND_ERROR = 0x40,
  L6470_HW_FAULT_ALL_MASK = 0xFF
} L6470HwFaultType_t;

/**
 * @brief L6470 fault monitoring state
 */
/**
 * @brief L6470 hardware fault status structure
 */
typedef struct {
  bool fault_active;                /**< Fault currently active */
  bool monitoring_enabled;          /**< Monitoring system enabled */
  L6470HwFaultType_t active_faults; /**< Currently active faults */
  uint32_t fault_count;             /**< Total fault count */
  uint32_t last_fault_time;         /**< Last fault timestamp */
  uint32_t fault_reaction_time_us;  /**< Fault reaction time in microseconds */
} L6470HwFaultStatus_t;

/* ==========================================================================
 */
/* Public Function Declarations                                              */
/* ==========================================================================
 */

/**
 * @brief Initialize L6470 hardware fault monitoring
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_fault_monitor_init(void);

/**
 * @brief Enable L6470 fault monitoring interrupts
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_fault_monitor_enable(void);

/**
 * @brief Disable L6470 fault monitoring interrupts
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_fault_monitor_disable(void);

/**
 * @brief Get current L6470 fault status
 * @param status Pointer to store fault status
 * @return SystemError_t Success or error code
 */
/**
 * @brief Get current L6470 fault status
 */
SystemError_t l6470_fault_monitor_get_status(L6470HwFaultStatus_t *status);

/**
 * @brief Clear fault status (after resolution)
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_fault_monitor_clear_faults(void);

/**
 * @brief GPIO interrupt handler for L6470 FLAG pin
 * @note Called from EXTI interrupt handler
 */
void l6470_fault_monitor_flag_interrupt(void);

/**
 * @brief GPIO interrupt handler for L6470 BUSY pin
 * @note Called from EXTI interrupt handler for monitoring
 */
void l6470_fault_monitor_busy_interrupt(void);

/**
 * @brief Check if fault monitoring is functional
 * @return bool true if monitoring is active and functional
 */
bool l6470_fault_monitor_is_active(void);

/**
 * @brief Test fault monitoring system (for validation)
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_fault_monitor_test(void);

#endif /* L6470_FAULT_MONITOR_H */
