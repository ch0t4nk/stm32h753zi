/**
 * @file hardware_config.h
 * @brief Hardware Configuration - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note This file contains ALL hardware pin assignments and peripheral
 * configurations. Never hardcode these values elsewhere - always reference
 * this SSOT.
 *
 * TODO: See .github/instructions/hardware-pins.instructions.md for complete
 * pin mapping documentation
 * TODO: See .github/instructions/hardware-pins.instructions.md for
 * MCU-specific configuration details
 */

#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#ifdef FIRMWARE_BUILD
// Firmware build: Include STM32 HAL headers
#include "stm32h7xx_hal.h"
#else
// Host test build: Include HAL abstraction headers for enums
#include "hal_abstraction/hal_abstraction.h"
#endif

/* ==========================================================================
 */
/* MCU Configuration (SSOT)                                                  */
/* ==========================================================================
 */
#define MCU_FAMILY "STM32H7"
#define MCU_PART_NUMBER "STM32H753ZI"
#define MCU_CORE_FREQUENCY_HZ 480000000UL
#define MCU_PACKAGE "LQFP144"

/* STM32H7 Clock Configuration (SSOT) - Corrected for 8MHz HSE */
#define SYSTEM_CLOCK_HSE_HZ                                                   \
    8000000UL                   // External crystal on Nucleo-144 (8MHz actual)
#define SYSTEM_CLOCK_PLL_M 2    // HSE prescaler: 8MHz/2 = 4MHz
#define SYSTEM_CLOCK_PLL_N 240  // PLL multiplier: 4MHz*240 = 960MHz
#define SYSTEM_CLOCK_PLL_P 2    // SYSCLK divider: 960MHz/2 = 480MHz
#define SYSTEM_CLOCK_AHB_DIV 2  // AHB prescaler: 480MHz/2 = 240MHz
#define SYSTEM_CLOCK_APB1_DIV 2 // APB1 prescaler: 240MHz/2 = 120MHz
#define SYSTEM_CLOCK_APB2_DIV 2 // APB2 prescaler: 240MHz/2 = 120MHz
#define SYSTEM_CLOCK_APB3_DIV 2 // APB3 prescaler: 240MHz/2 = 120MHz
#define SYSTEM_CLOCK_APB4_DIV 2 // APB4 prescaler: 240MHz/2 = 120MHz

/* X-CUBE-SPN2 + MCSDK Integration Framework (SSOT) */
#define MOTOR_CONTROL_FRAMEWORK_SPN2                                          \
    1 // Primary: X-CUBE-SPN2 stepper framework
#define MOTOR_CONTROL_FRAMEWORK_MCSDK                                         \
    1                               // Secondary: Selective MCSDK enhancement
#define MOTOR_CONTROL_HYBRID_MODE 1 // Enable hybrid SPN2+MCSDK integration

// ...existing code...

#endif /* HARDWARE_CONFIG_H */
