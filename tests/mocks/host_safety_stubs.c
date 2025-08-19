// Minimal host-only stubs to satisfy linker for host unit tests.
#include "common/error_codes.h"
#include "safety/emergency_stop_abstracted.h"
#include "safety/failsafe_manager.h"
#include "safety/fault_monitor.h"
#include "safety/watchdog_manager.h"

// Provide simple stub implementations that return SYSTEM_OK or safe defaults.

SystemError_t interrupt_priorities_init(void) {
    return SYSTEM_OK;
}

SystemError_t failsafe_manager_init(void) {
    return SYSTEM_OK;
}

// estop_check_health is provided by estop compatibility wrapper; do not
// provide a duplicate here.

SystemError_t fault_monitor_self_test(void) {
    return SYSTEM_OK;
}

// Provide check_motor_limits stub referenced by fault_monitor.c
SystemError_t check_motor_limits(uint8_t motor_id) {
    (void)motor_id;
    return SYSTEM_OK;
}

// fault_monitor exposes record and check functions. Provide thin wrappers if
// missing.
L6470FaultFlags_t fault_monitor_check_l6470_status(uint8_t motor_id) {
    (void)motor_id;
    return 0; // no faults
}

SystemError_t fault_monitor_record_motor_fault(uint8_t motor_id,
                                               MotorFaultType_t fault_type,
                                               FaultSeverity_t severity,
                                               uint32_t additional_data) {
    (void)motor_id;
    (void)fault_type;
    (void)severity;
    (void)additional_data;
    return SYSTEM_OK;
}

// check_system_health may be implemented in failsafe_manager; provide a stub.
SystemError_t check_system_health(void) {
    return SYSTEM_OK;
}

// Provide watchdog stubs if the host build pulls watchdog symbols from safety
// layer.
SystemError_t watchdog_init(void) {
    return SYSTEM_OK;
}

SystemError_t watchdog_refresh(void) {
    return SYSTEM_OK;
}
