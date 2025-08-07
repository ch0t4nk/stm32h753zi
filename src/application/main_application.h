/**
 * @file main_application.h
 * @brief Main Application API - STM32H753ZI Stepper Motor Control Demo
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @note Main application API with integrated safety system and watchdog
 * management Phase 1 Step 3: Watchdog Integration - COMPLETE
 */

#ifndef MAIN_APPLICATION_H
#define MAIN_APPLICATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 */
/* Includes */
/* ==========================================================================
 */

#ifndef UNITY_TESTING
#include "stm32h7xx_hal.h"
#endif
#include "common/error_codes.h"
#include <stdbool.h>

/* ==========================================================================
 */
/* Public API Function Declarations                                          */
/* ==========================================================================
 */

/**
 * @brief Initialize main application
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 *
 * @note This function initializes:
 *       - Safety system (emergency stop, fault monitoring, watchdog)
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
 *       - Watchdog refresh (safety critical, every 50ms)
 *       - Safety system periodic checks (every 10ms)
 *       - Motor control loop execution (50Hz)
 *       - Demo sequence progression
 *       - Status reporting (every 5 seconds)
 *       - Error handling and recovery
 */
SystemError_t main_application_run(void);

/**
 * @brief Emergency stop all motors
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 *
 * @warning This function performs immediate emergency stop of all motors
 *          via the integrated safety system
 */
SystemError_t main_application_emergency_stop(void);

/**
 * @brief Check if application is initialized
 * @return true if initialized, false otherwise
 */
bool main_application_is_initialized(void);

/**
 * @brief Get application runtime statistics
 * @param uptime_ms Pointer to store uptime in milliseconds
 * @param cycles Pointer to store application cycles
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 */
SystemError_t main_application_get_stats(uint32_t *uptime_ms,
                                         uint32_t *cycles);

/**
 * @brief Perform application self-test including watchdog validation
 * @return SystemError_t - SYSTEM_OK on success, error code on failure
 *
 * @note This function tests:
 *       - Safety system functionality
 *       - Watchdog system configuration and operation
 *       - Application initialization state
 */
SystemError_t main_application_self_test(void);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_APPLICATION_H */
