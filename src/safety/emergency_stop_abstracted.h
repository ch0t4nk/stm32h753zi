
/**
 * @file emergency_stop_abstracted.h
 * @brief Emergency Stop System with HAL Abstraction - STM32H753ZI Stepper
 * Motor Project (SSOT-compliant)
 * @author STM32H753ZI Project Team
 * @date 2025
 *
 * @note Emergency stop implementation using HAL abstraction layer:
 * - .github/instructions/safety-rt.instructions.md
 * - .github/instructions/hardware.instructions.md
 * - config/safety_config.h (SSOT)
 *
 * @warning SAFETY-CRITICAL: This system must respond within
 * ESTOP_REACTION_TIME_MS
 */

#ifndef SAFETY_EMERGENCY_STOP_ABSTRACTED_H
#define SAFETY_EMERGENCY_STOP_ABSTRACTED_H

#ifdef __cplusplus
extern "C" {
#endif

// SSOT: Always include error codes and stdint before any type usage
#include "common/error_codes.h"
#include "config/hardware_config.h"
#include "config/safety_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 */
/* Emergency Stop State Types (SSOT)                                         */
/* ==========================================================================
 */

// Modern SSOT-compliant E-Stop state and source enums
typedef enum {
    ESTOP_STATE_UNINITIALIZED = 0,
    ESTOP_STATE_ARMED,
    ESTOP_STATE_TRIGGERED,
    ESTOP_STATE_RESET_PENDING,
    ESTOP_STATE_FAULT
} EstopState_t;

typedef enum {
    ESTOP_SRC_UNKNOWN = 0,
    ESTOP_SRC_BUTTON,
    ESTOP_SRC_SOFTWARE,
    ESTOP_SRC_COMMUNICATION,
    ESTOP_SRC_SAFETY_MONITOR,
    ESTOP_SRC_MOTOR_FAULT,
    ESTOP_SRC_ENCODER_FAULT,
    ESTOP_SRC_WATCHDOG,
    ESTOP_SRC_SYSTEM_FAULT
} EstopSource_t;

/* ==========================================================================
 */
/* Emergency Stop API Functions (SSOT-compliant)                             */
/* ==========================================================================
 */

// --- Modern, SSOT-compliant E-Stop API ---

/**
 * @brief Initialize the E-Stop system (HAL-abstracted, SSOT-driven)
 * @return SystemError_t
 */
SystemError_t estop_init(void);

/**
 * @brief Trigger E-Stop (with source)
 * @param source (EstopSource_t)
 * @return SystemError_t
 */
SystemError_t estop_trigger(EstopSource_t source);

/**
 * @brief Reset E-Stop (requires confirmation, debounced)
 * @return SystemError_t
 */
SystemError_t estop_reset(void);

/**
 * @brief Periodic E-Stop state machine processing (call in safety loop)
 * @return SystemError_t
 */
SystemError_t estop_process(void);

/**
 * @brief Get current E-Stop state
 * @return EstopState_t
 */
EstopState_t estop_get_state(void);

/**
 * @brief Check if E-Stop is currently active
 * @return true if active
 */
bool estop_is_active(void);

/**
 * @brief Check if E-Stop button is pressed (HAL-abstracted)
 * @return true if pressed
 */
bool estop_button_pressed(void);

/**
 * @brief E-Stop interrupt handler (HAL-abstracted)
 */
void estop_irq_handler(void);

/**
 * @brief E-Stop self-test (diagnostics)
 * @return SystemError_t
 */
SystemError_t estop_self_test(void);

/**
 * @brief Set E-Stop LED (HAL-abstracted)
 * @param on true=LED on, false=off
 * @return SystemError_t
 */
SystemError_t estop_set_led(bool on);

/**
 * @brief Check E-Stop system health
 * @return SystemError_t
 */
SystemError_t estop_check_health(void);

/**
 * @brief Get last E-Stop trigger source
 * @return EstopSource_t
 */
EstopSource_t estop_last_source(void);

/**
 * @brief Get E-Stop statistics (total triggers, last trigger time)
 * @param trigger_count [out]
 * @param last_trigger_time [out]
 * @return SystemError_t
 */
SystemError_t estop_get_stats(uint32_t *trigger_count,
                              uint32_t *last_trigger_time);

// --- Legacy API: Deprecated, use new estop_ API ---
// (Optionally provide wrappers for backward compatibility)
// Legacy API compatibility for tests
#define emergency_stop_init estop_init
// ...etc...

// --- Discussion: If any new methods are needed for telemetry, test, or
// advanced safety, add TODOs here ---
// TODO: estop_get_fault_flags(), estop_set_test_mode(), estop_get_telemetry(),
// etc.

#ifdef __cplusplus
}
#endif

#endif /* SAFETY_EMERGENCY_STOP_ABSTRACTED_H */
