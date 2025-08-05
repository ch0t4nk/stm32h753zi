/**
 * @file main_application.h
 * @brief Main Application API - STM32H753ZI Stepper Motor Control Demo
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 * 
 * @note Main application API demonstrating closed-loop stepper motor control
 */

#ifndef MAIN_APPLICATION_H
#define MAIN_APPLICATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Includes                                                                   */
/* ========================================================================== */

#include "stm32h7xx_hal.h"
#include "common/error_codes.h"
#include <stdbool.h>

/* ========================================================================== */
/* Public API Function Declarations                                          */
/* ========================================================================== */

/**
 * @brief Initialize main application
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 * 
 * @note This function initializes:
 *       - HAL peripheral validation
 *       - Motor controller system
 *       - Both stepper motors
 *       - Encoder systems
 *       - Homing sequences
 *       - Demo sequence timing
 */
SystemError_t main_application_init(void);

/**
 * @brief Main application loop (call continuously)
 * @return SystemError_t - Overall system status
 * 
 * @note This function manages:
 *       - Motor control loop execution (50Hz)
 *       - Demo sequence progression
 *       - Status reporting (1Hz)
 *       - Error handling and recovery
 */
SystemError_t main_application_run(void);

/**
 * @brief Emergency stop all motors
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 * 
 * @warning This function performs immediate emergency stop of all motors
 */
SystemError_t main_application_emergency_stop(void);

/**
 * @brief Check if application is initialized
 * @return true if initialized, false otherwise
 */
bool main_application_is_initialized(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_APPLICATION_H */
