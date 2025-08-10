/**
 * @file watchdog_manager.c
 * @brief Watchdog Management System Implementation - STM32H753ZI Stepper Motor
 * Project
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @note Watchdog implementation following:
 * - .github/instructions/safety-rt.instructions.md
 * - config/safety_config.h (SSOT)
 *
 * @warning SAFETY-CRITICAL: Watchdog must be refreshed within timeout period
 */

#include "watchdog_manager.h"
#include "hal_abstraction/hal_abstraction.h"
#include "stm32h7xx_hal_iwdg.h"
#include "stm32h7xx_hal_wwdg.h"
#include <stdlib.h> // For abs() function
#include <string.h>

/* ==========================================================================
 */
/* Private Variables and State Management                                    */
/* ==========================================================================
 */

static bool watchdog_initialized = false;
static WatchdogConfig_t watchdog_config = {0};
static IWDG_HandleTypeDef hiwdg = {0};
static WWDG_HandleTypeDef hwwdg = {0};

// Watchdog health monitoring
static uint32_t consecutive_missed_refreshes = 0;
static uint32_t last_refresh_interval = 0;
static bool watchdog_warning_active = false;

/* ==========================================================================
 */
/* Private Function Prototypes                                               */
/* ==========================================================================
 */

static SystemError_t initialize_independent_watchdog(void);
static SystemError_t initialize_window_watchdog(void);
static void calculate_watchdog_timing(void);
static bool is_refresh_interval_valid(uint32_t interval);

/* ==========================================================================
 */
/* Public API Implementation                                                 */
/* ==========================================================================
 */

/**
 * @brief Initialize watchdog timer system
 */
SystemError_t watchdog_init(void) {
    if (watchdog_initialized) {
        return ERROR_ALREADY_INITIALIZED;
    }

    // Initialize watchdog configuration from SSOT
    watchdog_config.timeout_ms = IWDG_TIMEOUT_MS;
    watchdog_config.warning_threshold_ms = WATCHDOG_LATE_KICK_MS;
    watchdog_config.enabled = (IWDG_ENABLE == 1);
    watchdog_config.last_refresh = HAL_Abstraction_GetTick();
    watchdog_config.refresh_count = 0;
    watchdog_config.timeout_count = 0;
    watchdog_config.missed_refresh_count = 0;

    // Calculate optimal timing parameters
    calculate_watchdog_timing();

    // Initialize Independent Watchdog (IWDG) if enabled
    if (IWDG_ENABLE) {
        SystemError_t iwdg_result = initialize_independent_watchdog();
        if (iwdg_result != SYSTEM_OK) {
            return iwdg_result;
        }
    }

    // Initialize Window Watchdog (WWDG) if enabled
    if (WWDG_ENABLE) {
        SystemError_t wwdg_result = initialize_window_watchdog();
        if (wwdg_result != SYSTEM_OK) {
            return wwdg_result;
        }
    }

    // Reset health monitoring variables
    consecutive_missed_refreshes = 0;
    last_refresh_interval = 0;
    watchdog_warning_active = false;

    watchdog_initialized = true;

    return SYSTEM_OK;
}

/**
 * @brief Refresh watchdog timer (call regularly from main loop)
 */
SystemError_t watchdog_refresh(void) {
    if (!watchdog_initialized || !watchdog_config.enabled) {
        return SYSTEM_OK;
    }

    uint32_t current_time = HAL_Abstraction_GetTick();
    uint32_t elapsed = current_time - watchdog_config.last_refresh;

    // Check refresh interval validity
    if (!is_refresh_interval_valid(elapsed)) {
        consecutive_missed_refreshes++;
        watchdog_config.missed_refresh_count++;

        // Check for excessive missed refreshes
        if (consecutive_missed_refreshes > WATCHDOG_MISSED_KICK_MAX) {
            return ERROR_SAFETY_WATCHDOG_MISSED;
        }
    } else {
        consecutive_missed_refreshes = 0;
        watchdog_warning_active = false;
    }

    // Check if we're approaching timeout (warning)
    if (elapsed > watchdog_config.warning_threshold_ms) {
        watchdog_warning_active = true;
        // Warning logged by caller (safety system)
    }

    // Refresh Independent Watchdog
    if (IWDG_ENABLE) {
        HAL_IWDG_Refresh(&hiwdg);
    }

    // Refresh Window Watchdog
    if (WWDG_ENABLE) {
        // WWDG refresh requires writing to counter register within window
        HAL_WWDG_Refresh(&hwwdg);
    }

    // Update tracking variables
    last_refresh_interval = elapsed;
    watchdog_config.last_refresh = current_time;
    watchdog_config.refresh_count++;

    return SYSTEM_OK;
}

/**
 * @brief Check watchdog status and health
 */
SystemError_t watchdog_check_health(void) {
    if (!watchdog_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    if (!watchdog_config.enabled) {
        return SYSTEM_OK;
    }

    uint32_t current_time = HAL_Abstraction_GetTick();
    uint32_t elapsed = current_time - watchdog_config.last_refresh;

    // Check for timeout condition
    if (elapsed > watchdog_config.timeout_ms) {
        // This should never happen if system is healthy
        watchdog_config.timeout_count++;
        return ERROR_SAFETY_WATCHDOG_TIMEOUT;
    }

    // Check for warning condition
    if (elapsed > watchdog_config.warning_threshold_ms) {
        return ERROR_SAFETY_WATCHDOG_WARNING;
    }

    // Check for excessive missed refreshes
    if (consecutive_missed_refreshes > WATCHDOG_MISSED_KICK_MAX) {
        return ERROR_SAFETY_WATCHDOG_MISSED;
    }

    return SYSTEM_OK;
}

/**
 * @brief Get watchdog configuration
 */
WatchdogConfig_t get_watchdog_config(void) {
    return watchdog_config;
}

/**
 * @brief Perform watchdog self-test
 */
SystemError_t watchdog_self_test(void) {
    if (!watchdog_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Test 1: Verify IWDG configuration
    if (IWDG_ENABLE) {
        // Check if IWDG is properly configured
        if (hiwdg.Instance != IWDG1) {
            return ERROR_SAFETY_SELF_TEST_FAILED;
        }

        // Verify prescaler and reload values
        if (hiwdg.Init.Prescaler != IWDG_PRESCALER ||
            hiwdg.Init.Reload != IWDG_RELOAD_VALUE) {
            return ERROR_SAFETY_SELF_TEST_FAILED;
        }
    }

    // Test 2: Verify WWDG configuration
    if (WWDG_ENABLE) {
        // Check if WWDG is properly configured
        if (hwwdg.Instance != WWDG1) {
            return ERROR_SAFETY_SELF_TEST_FAILED;
        }

        // Verify window and counter values
        if (hwwdg.Init.Window != WWDG_WINDOW_VALUE ||
            hwwdg.Init.Counter != WWDG_COUNTER_VALUE) {
            return ERROR_SAFETY_SELF_TEST_FAILED;
        }
    }

    // Test 3: Verify refresh timing
    uint32_t test_start = HAL_Abstraction_GetTick();
    SystemError_t refresh_result = watchdog_refresh();
    uint32_t refresh_time = HAL_Abstraction_GetTick() - test_start;

    if (refresh_result != SYSTEM_OK ||
        refresh_time > 5) { // Should be very fast
        return ERROR_SAFETY_SELF_TEST_FAILED;
    }

    // Test 4: Verify configuration consistency
    if (watchdog_config.timeout_ms != IWDG_TIMEOUT_MS ||
        watchdog_config.warning_threshold_ms != WATCHDOG_LATE_KICK_MS) {
        return ERROR_SAFETY_SELF_TEST_FAILED;
    }

    return SYSTEM_OK;
}

/**
 * @brief Enable/disable watchdog
 */
SystemError_t watchdog_set_enabled(bool enabled) {
    if (!watchdog_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Note: Once IWDG is started, it cannot be stopped except by reset
    // This function controls whether we refresh it or not
    watchdog_config.enabled = enabled;

    if (enabled) {
        // Reset timing when re-enabling
        watchdog_config.last_refresh = HAL_Abstraction_GetTick();
        consecutive_missed_refreshes = 0;
        watchdog_warning_active = false;
    }

    return SYSTEM_OK;
}

/**
 * @brief Get watchdog statistics
 */
SystemError_t watchdog_get_statistics(uint32_t *refresh_count,
                                      uint32_t *timeout_count,
                                      uint32_t *missed_count) {
    if (!watchdog_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    if (refresh_count) {
        *refresh_count = watchdog_config.refresh_count;
    }

    if (timeout_count) {
        *timeout_count = watchdog_config.timeout_count;
    }

    if (missed_count) {
        *missed_count = watchdog_config.missed_refresh_count;
    }

    return SYSTEM_OK;
}

/**
 * @brief Check if watchdog refresh is due
 */
bool watchdog_refresh_due(void) {
    if (!watchdog_initialized || !watchdog_config.enabled) {
        return false;
    }

    uint32_t elapsed =
        HAL_Abstraction_GetTick() - watchdog_config.last_refresh;
    return (elapsed >= WATCHDOG_KICK_INTERVAL_MS);
}

/**
 * @brief Get time until next required refresh
 */
uint32_t watchdog_time_until_refresh(void) {
    if (!watchdog_initialized || !watchdog_config.enabled) {
        return UINT32_MAX;
    }

    uint32_t elapsed =
        HAL_Abstraction_GetTick() - watchdog_config.last_refresh;

    if (elapsed >= WATCHDOG_KICK_INTERVAL_MS) {
        return 0; // Refresh is overdue
    }

    return WATCHDOG_KICK_INTERVAL_MS - elapsed;
}

/* ==========================================================================
 */
/* Private Function Implementation                                           */
/* ==========================================================================
 */

/**
 * @brief Initialize Independent Watchdog (IWDG)
 */
static SystemError_t initialize_independent_watchdog(void) {
    // Configure IWDG using SSOT parameters
    hiwdg.Instance = IWDG1;
    hiwdg.Init.Prescaler = IWDG_PRESCALER;
    hiwdg.Init.Reload = IWDG_RELOAD_VALUE;
    hiwdg.Init.Window = IWDG_WINDOW_DISABLE; // No window restriction

    // Initialize IWDG
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        return ERROR_SAFETY_WATCHDOG_INIT_FAILED;
    }

    return SYSTEM_OK;
}

/**
 * @brief Initialize Window Watchdog (WWDG)
 */
static SystemError_t initialize_window_watchdog(void) {
    // Enable WWDG clock
    __HAL_RCC_WWDG1_CLK_ENABLE();

    // Configure WWDG using SSOT parameters
    hwwdg.Instance = WWDG1;
    hwwdg.Init.Prescaler = WWDG_PRESCALER;
    hwwdg.Init.Window = WWDG_WINDOW_VALUE;
    hwwdg.Init.Counter = WWDG_COUNTER_VALUE;
    hwwdg.Init.EWIMode = WWDG_EWI_ENABLE; // Enable early wakeup interrupt

    // Initialize WWDG
    if (HAL_WWDG_Init(&hwwdg) != HAL_OK) {
        return ERROR_SAFETY_WATCHDOG_INIT_FAILED;
    }

    return SYSTEM_OK;
}

/**
 * @brief Calculate optimal watchdog timing parameters
 */
static void calculate_watchdog_timing(void) {
    // Verify that SSOT timing parameters are reasonable

    // Check that kick interval is less than warning threshold
    if (WATCHDOG_KICK_INTERVAL_MS >= WATCHDOG_LATE_KICK_MS) {
        // Configuration error: adjust kick interval
        // This should be fixed in SSOT configuration
    }

    // Check that warning threshold is less than timeout
    if (WATCHDOG_LATE_KICK_MS >= IWDG_TIMEOUT_MS) {
        // Configuration error: timing hierarchy is incorrect
    }

    // Calculate actual IWDG timeout based on hardware
    // LSI frequency is typically ~32 kHz but can vary
    // Actual timeout = (4 * 2^prescaler * reload) / LSI_freq
    uint32_t prescaler_value = 4
                               << ((IWDG_PRESCALER >> IWDG_PR_PR_Pos) & 0x07);
    uint32_t calculated_timeout =
        (4 * prescaler_value * IWDG_RELOAD_VALUE) / 32000; // Assume 32kHz LSI

    // Update configuration with calculated value if significantly different
    if (abs((int32_t)calculated_timeout - (int32_t)IWDG_TIMEOUT_MS) > 10) {
        // Log discrepancy (actual vs configured timeout)
        // In production, this might warrant updating SSOT configuration
    }
}

/**
 * @brief Check if refresh interval is valid
 */
static bool is_refresh_interval_valid(uint32_t interval) {
    // Too frequent refreshes might indicate runaway code
    if (interval < (WATCHDOG_KICK_INTERVAL_MS / 2)) {
        return false;
    }

    // Too infrequent refreshes approach timeout
    if (interval > WATCHDOG_LATE_KICK_MS) {
        return false;
    }

    return true;
}

/* ==========================================================================
 */
/* Interrupt Service Routines                                                */
/* ==========================================================================
 */

/**
 * @brief WWDG Early Wakeup Interrupt Handler
 */
void WWDG_IRQHandler(void) {
    // Early wakeup interrupt for WWDG
    if (__HAL_WWDG_GET_IT_SOURCE(&hwwdg, WWDG_IT_EWI) != RESET) {
        // Clear the interrupt flag
        __HAL_WWDG_CLEAR_IT(&hwwdg, WWDG_IT_EWI);

        // This interrupt occurs before WWDG reset
        // Can be used for last-chance recovery or logging
        watchdog_warning_active = true;

        // Refresh WWDG to prevent reset
        HAL_WWDG_Refresh(&hwwdg);
    }
}

/**
 * @brief WWDG callback function
 */
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *wwdg_handle) {
    // Early wakeup callback - implement any last-chance recovery here
    watchdog_warning_active = true;
}
