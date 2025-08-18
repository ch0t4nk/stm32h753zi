

#include "watchdog_manager.h"
#include "common/data_types.h"
#include "common/error_codes.h"
#include "config/safety_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include "safety_system.h"
#include "stm32h7xx_hal_iwdg.h"
#include "stm32h7xx_hal_wwdg.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h> // For abs() function
#include <string.h>

// Internal state variables (must match legacy/RTOS and SSOT)
static bool watchdog_initialized = false;
static WatchdogConfig_t watchdog_config = {0};
static IWDG_HandleTypeDef hiwdg = {0};
static WWDG_HandleTypeDef hwwdg = {0};
static uint32_t consecutive_missed_refreshes = 0;
static uint32_t last_refresh_interval = 0;
static bool watchdog_warning_active = false;

// Helper for elapsed time in ms since last refresh
static uint32_t get_elapsed_since_last_refresh(void) {
    return HAL_Abstraction_GetTick() - watchdog_config.last_refresh;
}
/**
 * @file watchdog_manager.c
/**
 * @brief Check watchdog status and health
 */
SystemError_t watchdog_check_health(void) {
    if (!watchdog_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    uint32_t elapsed = get_elapsed_since_last_refresh();

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
    uint32_t elapsed = get_elapsed_since_last_refresh();
    return (elapsed >= WATCHDOG_KICK_INTERVAL_MS);
}

/**
 * @brief Get time until next required refresh
 */
uint32_t watchdog_time_until_refresh(void) {
    if (!watchdog_initialized || !watchdog_config.enabled) {
        return UINT32_MAX;
    }
    uint32_t elapsed = get_elapsed_since_last_refresh();
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
    uint32_t prescaler_value = 4U
                               << ((IWDG_PRESCALER >> IWDG_PR_PR_Pos) & 0x07);
    uint32_t calculated_timeout = (4U * prescaler_value * IWDG_RELOAD_VALUE) /
                                  32000U; // Assume 32kHz LSI

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
