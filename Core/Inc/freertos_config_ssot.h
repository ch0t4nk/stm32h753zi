/**
 * @file freertos_config_ssot.h
 * @brief FreeRTOS Configuration SSOT Integration
 * @author STM32H753ZI Project Team
 * @date 2025-08-15
 *
 * @note This file integrates FreeRTOS configuration with the centralized
 * clock configuration SSOT to ensure consistent timing across the system.
 */

#ifndef FREERTOS_CONFIG_SSOT_H
#define FREERTOS_CONFIG_SSOT_H

#include "config/clock_config.h"

/* Ensure FreeRTOS uses SSOT timing configuration */
#ifndef FREERTOS_TICK_RATE_HZ
#error "FREERTOS_TICK_RATE_HZ not defined in clock_config.h"
#endif

/* Validate FreeRTOS tick rate matches system tick rate */
#if FREERTOS_TICK_RATE_HZ != SYSTEM_TICK_FREQUENCY_HZ
#error "FreeRTOS tick rate must match system tick frequency in clock_config.h"
#endif

#endif /* FREERTOS_CONFIG_SSOT_H */
