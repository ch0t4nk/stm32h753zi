/**
 * @file emergency_stop_abstracted.h
 * @brief Emergency Stop System with HAL Abstraction - STM32H753ZI Stepper
 * Motor Project
 * @author STM32H753ZI Project Team
 * @date 2024
 *
 * @note Emergency stop implementation using HAL abstraction layer:
 * - .github/instructions/safety-rt.instructions.md
 * - .github/instructions/hardware.instructions.md
 * - config/safety_config.h (SSOT)
 *
 * @warning SAFETY-CRITICAL: This system must respond within
 * ESTOP_REACTION_TIME_MS
 */

#ifndef EMERGENCY_STOP_ABSTRACTED_H
#define EMERGENCY_STOP_ABSTRACTED_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 */
/* Includes */
/* ==========================================================================
 */

#include "common/error_codes.h"
#include "config/hardware_config.h"
#include "config/safety_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 */
/* Emergency Stop State Types                                                */
/* ==========================================================================
 */

/*
 * @brief Emergency stop states
 *
 * Guarded to avoid conflicts with the project's SSOT `config/safety_config.h`.
 */
#ifndef EMERGENCY_STOP_STATE_T_DEFINED
#define EMERGENCY_STOP_STATE_T_DEFINED
typedef enum {
    EMERGENCY_STOP_UNINITIALIZED = 0, ///< System not initialized
    EMERGENCY_STOP_ARMED = 1,         ///< System armed and ready
    EMERGENCY_STOP_TRIGGERED = 2,     ///< Emergency stop activated
    EMERGENCY_STOP_RESET_PENDING =
        3,                   ///< Reset requested, awaiting confirmation
    EMERGENCY_STOP_FAULT = 4 ///< System fault detected
} EmergencyStopState_t;
#endif

/* If the SSOT defines different names for armed/reset states, provide
 * non-invasive fallback macros so archived C can use EMERGENCY_STOP_ARMED
 * and EMERGENCY_STOP_RESET_PENDING without causing compile errors. Map
 * 'armed' -> NORMAL and 'reset pending' -> RECOVERY to avoid duplicate
 * case values in the archived implementation when SSOT is present. */
#ifndef EMERGENCY_STOP_ARMED
#define EMERGENCY_STOP_ARMED EMERGENCY_STOP_NORMAL
#endif
#ifndef EMERGENCY_STOP_RESET_PENDING
#define EMERGENCY_STOP_RESET_PENDING EMERGENCY_STOP_RECOVERY
#endif

/**
 * @brief Emergency stop trigger sources
 */
/*
 * Guard EmergencyStopSource_t to avoid redefinition when SSOT headers are
 * included in the same translation unit.
 */
#ifndef EMERGENCY_STOP_SOURCE_T_DEFINED
#define EMERGENCY_STOP_SOURCE_T_DEFINED
typedef enum {
    ESTOP_SOURCE_UNKNOWN = 0,
    ESTOP_SOURCE_BUTTON = 1,
    ESTOP_SOURCE_SOFTWARE = 2,
    ESTOP_SOURCE_COMMUNICATION = 3,
    ESTOP_SOURCE_SAFETY_MONITOR = 4,
    ESTOP_SOURCE_MOTOR_FAULT = 5,
    ESTOP_SOURCE_ENCODER_FAULT = 6,
    ESTOP_SOURCE_WATCHDOG = 7,
    ESTOP_SOURCE_SYSTEM_FAULT = 8
} EmergencyStopSource_t;
#endif

/* ==========================================================================
 */
/* Emergency Stop Hardware Configuration                                     */
/* ==========================================================================
 */

// Emergency stop hardware configuration (from SSOT hardware_config.h)
// Note: Pin definitions are in hardware_config.h following SSOT principles
#define ESTOP_BUTTON_TRIGGER_TYPE HAL_GPIO_MODE_IT_FALLING

// Emergency stop LED indicator uses LED_RED from SSOT hardware_config.h
// ESTOP_LED_PORT and ESTOP_LED_PIN are defined in hardware_config.h

/* ==========================================================================
 */
/* Emergency Stop API Functions                                              */
/* ==========================================================================
 */

/**
 * @brief Initialize emergency stop system with HAL abstraction
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
 * @brief Process emergency stop state machine (call periodically)
 * @return SystemError_t Success or error code
 */
SystemError_t emergency_stop_process(void);

/**
 * @brief Get current emergency stop state
 * @return EmergencyStopState_t Current state
 */
EmergencyStopState_t emergency_stop_get_state(void);

/**
 * @brief Check if emergency stop is currently active
 * @return bool True if emergency stop is active
 */
bool emergency_stop_is_active(void);

/**
 * @brief Check if emergency stop button is pressed (using HAL abstraction)
 * @return bool True if button is pressed
 */
bool emergency_stop_is_button_pressed(void);

/**
 * @brief Emergency stop interrupt handler (called by HAL abstraction)
 */
void emergency_stop_interrupt_handler(void);

/**
 * @brief Perform emergency stop self-test
 * @return SystemError_t Success or error code
 */
SystemError_t emergency_stop_self_test(void);

/**
 * @brief Set emergency stop LED state (using HAL abstraction)
 * @param state True to turn on LED, false to turn off
 * @return SystemError_t Success or error code
 */
SystemError_t emergency_stop_set_led(bool state);

/**
 * @brief Check emergency stop system health
 * @return SystemError_t Success or error code
 */
SystemError_t emergency_stop_check_health(void);

/**
 * @brief Get last emergency stop trigger source
 * @return EmergencyStopSource_t Last trigger source
 */
EmergencyStopSource_t emergency_stop_get_last_source(void);

/**
 * @brief Get emergency stop statistics
 * @param trigger_count Pointer to store total trigger count
 * @param last_trigger_time Pointer to store last trigger timestamp
 * @return SystemError_t Success or error code
 */
SystemError_t emergency_stop_get_statistics(uint32_t *trigger_count,
                                            uint32_t *last_trigger_time);

#ifdef __cplusplus
}
#endif

#endif /* EMERGENCY_STOP_ABSTRACTED_H */
