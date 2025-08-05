/**
 * @file emergency_stop.h
 * @brief Emergency Stop System - STM32H753ZI Stepper Motor Project
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 * 
 * @note Emergency stop implementation following:
 * - .github/instructions/safety-rt.instructions.md
 * - config/safety_config.h (SSOT)
 * 
 * @warning SAFETY-CRITICAL: This system must respond within ESTOP_REACTION_TIME_MS
 */

#ifndef EMERGENCY_STOP_H
#define EMERGENCY_STOP_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Includes                                                                   */
/* ========================================================================== */

#include "stm32h7xx_hal.h"
#include "config/safety_config.h"
#include "config/hardware_config.h"
#include "common/error_codes.h"
#include "safety_system.h"
#include <stdbool.h>
#include <stdint.h>

/* ========================================================================== */
/* Emergency Stop Hardware Configuration                                     */
/* ========================================================================== */

// Emergency stop button configuration (using SSOT hardware config)
#define ESTOP_BUTTON_PORT       GPIOC
#define ESTOP_BUTTON_PIN        GPIO_PIN_13  // User button on Nucleo-144
#define ESTOP_BUTTON_EXTI_IRQn  EXTI15_10_IRQn

// Safety relay outputs (if external safety relays are used)
#define SAFETY_RELAY1_PORT      GPIOB
#define SAFETY_RELAY1_PIN       GPIO_PIN_0
#define SAFETY_RELAY2_PORT      GPIOB
#define SAFETY_RELAY2_PIN       GPIO_PIN_1

// Emergency stop LED indicator
#define ESTOP_LED_PORT          GPIOB
#define ESTOP_LED_PIN           GPIO_PIN_14   // Red LED

/* ========================================================================== */
/* Emergency Stop API Functions                                              */
/* ========================================================================== */

/**
 * @brief Initialize emergency stop system
 * @return SystemError_t Success or error code
 */
SystemError_t emergency_stop_init(void);

/**
 * @brief Execute emergency stop sequence
 * @param source Source of emergency stop trigger
 * @return SystemError_t Success or error code
 */
SystemError_t emergency_stop_execute(EmergencyStopSource_t source);

/**
 * @brief Reset emergency stop (requires manual confirmation)
 * @return SystemError_t Success or error code
 */
SystemError_t emergency_stop_reset(void);

/**
 * @brief Get current emergency stop state
 * @return EmergencyStopState_t Current state
 */
EmergencyStopState_t emergency_stop_get_state(void);

/**
 * @brief Check if emergency stop button is pressed
 * @return bool True if button is pressed
 */
bool is_emergency_stop_pressed(void);

/**
 * @brief Emergency stop interrupt handler
 */
void emergency_stop_interrupt_handler(void);

/**
 * @brief Perform emergency stop self-test
 * @return SystemError_t Success or error code
 */
SystemError_t emergency_stop_self_test(void);

/**
 * @brief Set emergency stop LED state
 * @param state True to turn on LED, false to turn off
 */
void emergency_stop_set_led(bool state);

/**
 * @brief Check emergency stop system health
 * @return SystemError_t Success or error code
 */
SystemError_t emergency_stop_check_health(void);

#ifdef __cplusplus
}
#endif

#endif /* EMERGENCY_STOP_H */
