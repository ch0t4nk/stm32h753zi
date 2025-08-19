/**
 * @file emergency_stop.c
 * @brief Emergency Stop System Implementation - STM32H753ZI Stepper Motor
 * Project Legacy RTOS implementation restored for build compatibility.
 */

#include "emergency_stop.h"
#include "hal_abstraction/hal_abstraction.h"
#include <string.h>

static bool estop_initialized = false;
static uint32_t estop_timestamp = 0;
static uint32_t estop_source = 0;
static bool estop_active = false;
static uint32_t estop_trigger_count = 0;

SystemError_t emergency_stop_init(void) {
    estop_initialized = true;
    estop_active = false;
    estop_timestamp = 0;
    estop_source = 0;
    return SYSTEM_OK;
}

SystemError_t emergency_stop_execute(uint32_t source) {
    if (!estop_initialized)
        return ERROR_NOT_INITIALIZED;
    estop_active = true;
    estop_timestamp = HAL_Abstraction_GetTick();
    estop_source = source;
    estop_trigger_count++;
    // Hardware: trigger GPIO, relay, or other estop mechanism here
    return SYSTEM_OK;
}

SystemError_t emergency_stop_reset(void) {
    if (!estop_initialized)
        return ERROR_NOT_INITIALIZED;
    estop_active = false;
    estop_timestamp = 0;
    estop_source = 0;
    return SYSTEM_OK;
}

SystemError_t emergency_stop_self_test(void) {
    if (!estop_initialized)
        return ERROR_NOT_INITIALIZED;
    // Optionally test estop hardware here
    return SYSTEM_OK;
}

SystemError_t emergency_stop_check_health(void) {
    if (!estop_initialized)
        return ERROR_NOT_INITIALIZED;
    // Optionally check estop health here
    return SYSTEM_OK;
}

bool is_emergency_stop_pressed(void) {
    // Optionally read GPIO or input pin here
    return estop_active;
}

uint32_t emergency_stop_get_timestamp(void) {
    return estop_timestamp;
}

uint32_t emergency_stop_get_source(void) {
    return estop_source;
}

// Process state machine - simple host-friendly implementation
SystemError_t emergency_stop_process(void) {
    if (!estop_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // In host tests, process may be invoked periodically; simulate
    // no-op behavior for now.
    return SYSTEM_OK;
}

bool emergency_stop_is_active(void) {
    return estop_active;
}

// Return the last trigger source (legacy name)
uint32_t emergency_stop_get_last_source(void) {
    return estop_source;
}

SystemError_t emergency_stop_get_statistics(uint32_t *trigger_count,
                                            uint32_t *last_trigger_time) {
    if (!estop_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    if (trigger_count)
        *trigger_count = estop_trigger_count;
    if (last_trigger_time)
        *last_trigger_time = estop_timestamp;
    return SYSTEM_OK;
}
