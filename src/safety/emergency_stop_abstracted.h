/*
 * emergency_stop_abstracted.h
 *
 * Minimal compatibility header for host-tests that exposes the "estop_"
 * API names used by unit tests while re-using the project's SSOT enums
 * from `config/safety_config.h` to avoid duplicate type definitions.
 */

#ifndef PROJECT_EMERGENCY_STOP_ABSTRACTED_H
#define PROJECT_EMERGENCY_STOP_ABSTRACTED_H

#include "config/error_codes.h"
#include "config/safety_config.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Map test-facing Estop types to SSOT enums
typedef EmergencyStopState_t EstopState_t;
typedef EmergencyStopSource_t EstopSource_t;

/* Legacy compatibility aliases used by older tests and archived code. These
 * map the legacy ESTOP_SRC_* and ESTOP_STATE_* names (used throughout the
 * codebase and tests) to the SSOT canonical names above.
 */
#ifndef ESTOP_SRC_SOFTWARE
#define ESTOP_SRC_SOFTWARE ESTOP_SOURCE_SOFTWARE
#endif
/* Map legacy motor/encoder fault names to existing SSOT sources. These are
 * best-effort mappings to allow archived tests to run without changing
 * the SSOT. */
#ifndef ESTOP_SRC_MOTOR_FAULT
#define ESTOP_SRC_MOTOR_FAULT ESTOP_SOURCE_OVERCURRENT
#endif
#ifndef ESTOP_SRC_ENCODER_FAULT
#define ESTOP_SRC_ENCODER_FAULT ESTOP_SOURCE_POSITION_LIMIT
#endif

/* Map legacy armed/reset state names to SSOT states that represent the
 * same conceptual behavior: 'armed' -> normal ready state, 'reset_pending'
 * -> recovery state. This avoids introducing new enum values and prevents
 * duplicate case labels in archived code compiled under the SSOT. */
#ifndef ESTOP_STATE_ARMED
#define ESTOP_STATE_ARMED EMERGENCY_STOP_NORMAL
#endif
#ifndef ESTOP_STATE_TRIGGERED
#define ESTOP_STATE_TRIGGERED EMERGENCY_STOP_TRIGGERED
#endif
#ifndef ESTOP_STATE_RESET_PENDING
#define ESTOP_STATE_RESET_PENDING EMERGENCY_STOP_RECOVERY
#endif

// Test-facing API (thin wrappers implemented in src/safety/estop_compat.c)
SystemError_t estop_init(void);
SystemError_t estop_trigger(EstopSource_t source);
SystemError_t estop_reset(void);
SystemError_t estop_process(void);
EstopState_t estop_get_state(void);
bool estop_is_active(void);
SystemError_t estop_self_test(void);
SystemError_t estop_get_stats(uint32_t *trigger_count,
                              uint32_t *last_trigger_time);
EstopSource_t estop_last_source(void);
SystemError_t estop_check_health(void);

/*
/* Legacy emergency_stop_* API compatibility.
 *
 * The archived implementation and some older tests expect integer-typed
 * parameters/returns (uint32_t) for sources/states. Declare the legacy
 * API using integer types here so both the archived C and the compatibility
 * wrappers share the same ABI and avoid conflicting prototypes.
 */

/* Fallback mappings for legacy state/sources that are not present in the
 * modern SSOT. These create non-invasive aliases so older tests using
 * ESTOP_STATE_* / ESTOP_SRC_* still compile. Assumption: map legacy motor
 * and encoder fault sources to the closest existing SSOT sources.
 */
#ifndef EMERGENCY_STOP_ARMED
#define EMERGENCY_STOP_ARMED EMERGENCY_STOP_TRIGGERED
#endif

#ifndef EMERGENCY_STOP_RESET_PENDING
#define EMERGENCY_STOP_RESET_PENDING EMERGENCY_STOP_RECOVERY
#endif

#ifndef ESTOP_SRC_MOTOR_FAULT
/* Assumption: map motor fault to overcurrent source */
#define ESTOP_SRC_MOTOR_FAULT ESTOP_SOURCE_OVERCURRENT
#endif

#ifndef ESTOP_SRC_ENCODER_FAULT
/* Assumption: map encoder fault to position limit source */
#define ESTOP_SRC_ENCODER_FAULT ESTOP_SOURCE_POSITION_LIMIT
#endif

/* Declare legacy API with integer-based signatures to match archived C.
 * estop_compat.c provides modern estop_* wrappers that cast to/from these
 * integer values. */
SystemError_t emergency_stop_init(void);
SystemError_t emergency_stop_execute(uint32_t source);
SystemError_t emergency_stop_reset(void);
SystemError_t emergency_stop_process(void);
uint32_t emergency_stop_get_state(void);
bool emergency_stop_is_active(void);
SystemError_t emergency_stop_self_test(void);
SystemError_t emergency_stop_set_led(bool state);
SystemError_t emergency_stop_check_health(void);
uint32_t emergency_stop_get_last_source(void);
SystemError_t emergency_stop_get_statistics(uint32_t *trigger_count,
                                            uint32_t *last_trigger_time);
/* Provide legacy alias for trigger name used by some tests */
#ifndef emergency_stop_trigger
#define emergency_stop_trigger emergency_stop_execute
#endif

#ifdef __cplusplus
}
#endif

#endif // PROJECT_EMERGENCY_STOP_ABSTRACTED_H
