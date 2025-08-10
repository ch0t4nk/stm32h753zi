/**
 * @file stm32h7xx.h
 * @brief Mock STM32H7xx device header for unit testing
 *
 * This file provides mock definitions for the STM32H7xx device header
 * to support unit testing without actual hardware dependencies.
 */

#ifndef STM32H7XX_H
#define STM32H7XX_H

#ifdef __cplusplus
extern "C" {
#endif

// Include our mock HAL types
#include "mock_hal_types.h"

// Include main HAL mock
#include "stm32h7xx_hal.h"

#ifdef __cplusplus
}
#endif

#endif /* STM32H7XX_H */
