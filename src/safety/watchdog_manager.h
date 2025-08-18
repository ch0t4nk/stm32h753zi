/**
 * @file watchdog_manager.h
 * @brief Watchdog Management System - STM32H753ZI Stepper Motor Project
 * Legacy RTOS header restored for build compatibility.
 */

#ifndef WATCHDOG_MANAGER_H
#define WATCHDOG_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/error_codes.h"
#include "config/safety_config.h"
#include "safety_system.h"
#include <stdbool.h>
#include <stdint.h>

SystemError_t watchdog_init(void);
SystemError_t watchdog_refresh(void);
SystemError_t watchdog_check_health(void);
WatchdogConfig_t get_watchdog_config(void);
SystemError_t watchdog_self_test(void);
SystemError_t watchdog_set_enabled(bool enabled);
SystemError_t watchdog_get_statistics(uint32_t *refresh_count,
                                      uint32_t *timeout_count,
                                      uint32_t *missed_count);
bool watchdog_refresh_due(void);
uint32_t watchdog_time_until_refresh(void);

#ifdef __cplusplus
}
#endif

#endif /* WATCHDOG_MANAGER_H */
