/* init_stubs.c
 * Minimal initialization stubs to satisfy host test linking.
 * These are only for HOST_TESTING builds; when building for the
 * ARM firmware the real implementations are linked and having these
 * stubs causes multiple definition link errors. Guard with
 * HOST_TESTING so they are excluded from the firmware build.
 */

#ifdef HOST_TESTING
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
#endif /* HOST_TESTING */
