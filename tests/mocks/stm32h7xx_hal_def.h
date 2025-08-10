/**
 * @file stm32h7xx_hal_def.h
 * @brief Mock STM32H7xx HAL definitions for unit testing
 *
 * This file provides mock definitions for HAL common definitions
 * to support unit testing without actual hardware dependencies.
 */

#ifndef STM32H7XX_HAL_DEF_H
#define STM32H7XX_HAL_DEF_H

#ifdef __cplusplus
extern "C" {
#endif

// Include our mock HAL types first
#include "mock_hal_types.h"

// Common HAL definitions
#ifndef NULL
#define NULL ((void *)0)
#endif

// HAL status and error definitions should come from mock_hal_types.h
// Additional definitions can be added here as needed

#ifdef __cplusplus
}
#endif

#endif /* STM32H7XX_HAL_DEF_H */
