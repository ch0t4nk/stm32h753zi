/* init_stubs.c
 * Minimal initialization stubs to satisfy host test linking.
 */

#include "config/error_codes.h"

SystemError_t emergency_stop_init(void) {
    return SYSTEM_OK;
}
SystemError_t watchdog_init(void) {
    return SYSTEM_OK;
}
SystemError_t timing_precision_init(void) {
    return SYSTEM_OK;
}
SystemError_t interrupt_priorities_init(void) {
    return SYSTEM_OK;
}
SystemError_t failsafe_manager_init(void) {
    return SYSTEM_OK;
}
