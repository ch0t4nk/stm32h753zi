/**
 * @file clock_config_480mhz.h
 * @brief STM32H753ZI 480MHz clock configuration header using SSOT error
 * handling
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 */

#ifndef CLOCK_CONFIG_480MHZ_H
#define CLOCK_CONFIG_480MHZ_H

#include "common/error_handling.h" // Use existing SSOT error handling
#include "stm32h7xx_hal.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize 480MHz clock configuration for STM32H753ZI Revision V
 *
 * This function configures the STM32H753ZI for 480MHz operation using:
 * - HSI oscillator as source (no external crystal dependency)
 * - VOS0 voltage scaling for maximum performance
 * - PLL1 configured for 480MHz output
 * - Proper timeout handling to prevent hangs
 *
 * @return SystemError_t SYSTEM_OK for success, error code for failure
 */
SystemError_t Clock_Init_480MHz(void);

/**
 * @brief Get last error from clock configuration
 * @return SystemError_t Last error encountered
 */
SystemError_t Clock_Get_Last_Error(void);

/**
 * @brief Check if system is running at 480MHz
 * @return true if running at 480MHz, false otherwise
 */
bool Clock_Is_Running_At_480MHz(void);

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_CONFIG_480MHZ_H */
