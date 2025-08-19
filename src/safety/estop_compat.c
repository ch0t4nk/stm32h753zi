/*
 * estop_compat.c
 * Compatibility wrappers that adapt the legacy emergency_stop_* implementation
 * without pulling in the archive header which defines enums/typedefs that
 * conflict with the modern SSOT headers. This file includes the modern
 * SSOT API header and forward-declares the legacy functions with plain C
 * types (uint32_t / bool) so we can call the legacy implementation while
 * keeping type definitions separate.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Include modern SSOT-compliant estop API so we get EstopState_t,
 * EstopSource_t and SystemError_t definitions used by callers in this project.
 */
#include "safety/emergency_stop_abstracted.h"

/* Include the compatibility header which declares the legacy integer-typed
 * emergency_stop_* API and the modern estop_ wrappers; this prevents us
 * from accidentally using mismatched prototypes. */
#include "safety/emergency_stop_abstracted.h"

/* Compatibility wrappers: convert between modern SSOT enums and legacy
 * integer-based values by casting. This keeps translation-unit-level
 * includes clean and prevents enum/typedef redefinition errors. */

SystemError_t estop_init(void) {
    printf("[estop_compat] estop_init() called\n");
    fflush(stdout);
    return emergency_stop_init();
}

SystemError_t estop_trigger(EstopSource_t source) {
    printf("[estop_compat] estop_trigger() source=%u\n", (unsigned)source);
    fflush(stdout);
    return emergency_stop_execute((uint32_t)source);
}

SystemError_t estop_reset(void) {
    return emergency_stop_reset();
}

SystemError_t estop_process(void) {
    /* occasional tracer to see progress during long loops */
    static int call_count = 0;
    if ((call_count++ & 0x3FF) == 0) {
        printf("[estop_compat] estop_process() call_count=%d\n", call_count);
        fflush(stdout);
    }
    return emergency_stop_process();
}

EstopState_t estop_get_state(void) {
    return (EstopState_t)emergency_stop_get_state();
}

bool estop_is_active(void) {
    return emergency_stop_is_active();
}

SystemError_t estop_self_test(void) {
    return emergency_stop_self_test();
}

SystemError_t estop_get_stats(uint32_t *trigger_count,
                              uint32_t *last_trigger_time) {
    return emergency_stop_get_statistics(trigger_count, last_trigger_time);
}

EstopSource_t estop_last_source(void) {
    return (EstopSource_t)emergency_stop_get_last_source();
}

SystemError_t estop_check_health(void) {
    return emergency_stop_check_health();
}
