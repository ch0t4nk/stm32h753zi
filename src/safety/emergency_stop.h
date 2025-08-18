/**
 * @file emergency_stop.h
 * @brief Emergency Stop System - STM32H753ZI Stepper Motor Project
 * Legacy RTOS header restored for build compatibility.
 */

#ifndef EMERGENCY_STOP_H
#define EMERGENCY_STOP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/error_codes.h" // SystemError_t
#include "config/hardware_config.h"
#include "config/safety_config.h"
#include "safety_system.h"
#include <stdbool.h>
#include <stdint.h>

#define ESTOP_BUTTON_EXTI_IRQn EXTI15_10_IRQn

SystemError_t emergency_stop_init(void);
SystemError_t emergency_stop_execute(uint32_t source);
SystemError_t emergency_stop_reset(void);
SystemError_t emergency_stop_self_test(void);
SystemError_t emergency_stop_check_health(void);
bool is_emergency_stop_pressed(void);
uint32_t emergency_stop_get_timestamp(void);
uint32_t emergency_stop_get_source(void);

#ifdef __cplusplus
}
#endif

#endif /* EMERGENCY_STOP_H */
