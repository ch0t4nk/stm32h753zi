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

/* ==========================================================================
 * Legacy-compatible E-Stop and Safety Relay Pin Macros for Test Compatibility
 * These are required for test and mock code that expects the old macro names.
 * All values are mapped to the correct HAL abstraction enums and indices.
 * ==========================================================================
 */
#ifndef ESTOP_BUTTON_PORT
#define ESTOP_BUTTON_PORT HAL_GPIO_PORT_C
#endif
#ifndef ESTOP_BUTTON_PIN
#define ESTOP_BUTTON_PIN 13
#endif
#ifndef ESTOP_BUTTON_PIN_INDEX
#define ESTOP_BUTTON_PIN_INDEX 13
#endif
#ifndef ESTOP_LED_PORT
#define ESTOP_LED_PORT HAL_GPIO_PORT_B
#endif
#ifndef ESTOP_LED_PIN
#define ESTOP_LED_PIN 14
#endif
#ifndef ESTOP_LED_PIN_INDEX
#define ESTOP_LED_PIN_INDEX 14
#endif
#ifndef SAFETY_RELAY1_PORT
#define SAFETY_RELAY1_PORT HAL_GPIO_PORT_C
#endif
#ifndef SAFETY_RELAY1_PIN
#define SAFETY_RELAY1_PIN 4
#endif
#ifndef SAFETY_RELAY1_PIN_INDEX
#define SAFETY_RELAY1_PIN_INDEX 4
#endif
#ifndef SAFETY_RELAY2_PORT
#define SAFETY_RELAY2_PORT HAL_GPIO_PORT_C
#endif
#ifndef SAFETY_RELAY2_PIN
#define SAFETY_RELAY2_PIN 5
#endif
#ifndef SAFETY_RELAY2_PIN_INDEX
#define SAFETY_RELAY2_PIN_INDEX 5
#endif

// ============================================================================
// Interrupt Request (IRQ) Numbers for Safety-Critical Interrupt Priorities
// ============================================================================

// Emergency Stop and Safety-Critical IRQs
#define EMERGENCY_STOP_IRQ EXTI15_10_IRQn   // Emergency stop button interrupt
#define SAFETY_TIMER_IRQ TIM1_UP_TIM10_IRQn // Safety monitor timer interrupt

// Real-Time Control IRQs
#define CONTROL_TIMER_IRQ TIM2_IRQn // Control loop timer interrupt

// Motor Communication IRQs
#define MOTOR_SPI_IRQ SPI1_IRQn // L6470 SPI communication interrupt

// Sensor and Encoder IRQs
#define ENCODER_I2C1_IRQ I2C1_EV_IRQn // AS5600 encoder 1 I2C interrupt
#define ENCODER_I2C2_IRQ I2C2_EV_IRQn // AS5600 encoder 2 I2C interrupt

// System Communication IRQs
#define SYSTEM_UART_IRQ USART3_IRQn    // System UART interrupt
#define SYSTEM_CAN_IRQ FDCAN1_IT0_IRQn // System CAN interrupt

// System Management IRQs
#define SYSTEM_TICK_IRQ SysTick_IRQn // System tick timer interrupt

// ============================================================================
// Motor Control Constants
// ============================================================================

// Position Control Limits
#define MAX_POSITION_ERROR_STEPS 100 // Maximum position error in steps

// ============================================================================
// AS5600 Magnetic Encoder Configuration
// ============================================================================

#define AS5600_I2C_ADDRESS_8BIT 0x6C // AS5600 I2C address (8-bit format)

#endif /* HARDWARE_CONFIG_H */
