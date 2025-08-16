/**
 * @file watchdog_manager.h
 * @brief Watchdog Management System - STM32H753ZI Stepper Motor Project
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @note Watchdog implementation following:
 * - .github/instructions/safety-rt.instructions.md
 * - config/safety_config.h (SSOT)
 *
 * @warning SAFETY-CRITICAL: Watchdog must be refreshed within timeout period
 */

#ifndef WATCHDOG_MANAGER_H
#define WATCHDOG_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Includes                                                                   */
/* ========================================================================== */

#ifndef UNITY_TESTING
#include "stm32h7xx_hal.h"
#endif
#include "common/error_codes.h"
#include "config/safety_config.h"
#include "safety_system.h"
#include <stdbool.h>
#include <stdint.h>

/* ========================================================================== */
/* Watchdog Manager API Functions                                            */
/* ========================================================================== */

/**
 * @brief Initialize watchdog timer system
 * @return SystemError_t Success or error code
 */
SystemError_t watchdog_init(void);

/**
 * @brief Refresh watchdog timer (call regularly from main loop)
 * @return SystemError_t Success or error code
 */
SystemError_t watchdog_refresh(void);

/**
 * @brief Check watchdog status and health
 * @return SystemError_t Success or error code
 */
SystemError_t watchdog_check_health(void);

/**
 * @brief Get watchdog configuration
 * @return WatchdogConfig_t Current configuration
 */
WatchdogConfig_t get_watchdog_config(void);

/**
 * @brief Perform watchdog self-test
 * @return SystemError_t Success or error code
 */
SystemError_t watchdog_self_test(void);

/**
 * @brief Enable/disable watchdog
 * @param enabled True to enable, false to disable
 * @return SystemError_t Success or error code
 */
SystemError_t watchdog_set_enabled(bool enabled);

/**
 * @brief Get watchdog statistics
 * @param refresh_count Pointer to store refresh count
 * @param timeout_count Pointer to store timeout count
 * @param missed_count Pointer to store missed refresh count
 * @return SystemError_t Success or error code
 */
SystemError_t watchdog_get_statistics(uint32_t *refresh_count,
                                      uint32_t *timeout_count,
                                      uint32_t *missed_count);

/**
 * @brief Check if watchdog refresh is due
 * @return bool True if refresh is needed soon
 */
bool watchdog_refresh_due(void);

/**
 * @brief Get time until next required refresh
 * @return uint32_t Time in milliseconds until refresh required
 */
uint32_t watchdog_time_until_refresh(void);

#ifdef __cplusplus
}
#endif

#endif /* WATCHDOG_MANAGER_H */
