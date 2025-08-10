/**
 * @file mock_drivers.h
 * @brief Mock driver functions for unit testing
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * This file provides mock implementations of driver functions
 * for unit testing optimization_telemetry.c without requiring
 * actual driver headers with HAL dependencies.
 */

#ifndef MOCK_DRIVERS_H
#define MOCK_DRIVERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common/error_codes.h"
#include <stdint.h>

/* ==========================================================================
 */
/* AS5600 Driver Mock Functions */
/* ==========================================================================
 */

/**
 * @brief Mock AS5600 encoder initialization
 * @param motor_id Motor identifier
 * @return SystemError_t Mock return value
 */
SystemError_t as5600_init_encoder(uint8_t motor_id);

/**
 * @brief Mock AS5600 angle reading in degrees
 * @param motor_id Motor identifier
 * @param angle_degrees Pointer to store angle in degrees
 * @return SystemError_t Mock return value
 */
SystemError_t as5600_read_angle_degrees(uint8_t motor_id,
                                        float *angle_degrees);

/* ==========================================================================
 */
/* L6470 Driver Mock Functions */
/* ==========================================================================
 */

/**
 * @brief Mock L6470 driver initialization
 * @return SystemError_t Mock return value
 */
SystemError_t l6470_init(void);

/**
 * @brief Mock L6470 parameter reading
 * @param motor_id Motor identifier
 * @param parameter Parameter address
 * @param value Pointer to store parameter value
 * @return SystemError_t Mock return value
 */
SystemError_t l6470_get_parameter(uint8_t motor_id, uint8_t parameter,
                                  uint32_t *value);

/**
 * @brief Mock L6470 status reading
 * @param motor_id Motor identifier
 * @param status Pointer to store status value
 * @return SystemError_t Mock return value
 */
SystemError_t l6470_get_status(uint8_t motor_id, uint32_t *status);

/**
 * @brief Mock L6470 hard stop
 * @param motor_id Motor identifier
 * @return SystemError_t Mock return value
 */
SystemError_t l6470_hard_stop(uint8_t motor_id);

/* ==========================================================================
 */
/* L6470 Register Constants (Mock Values) */
/* ==========================================================================
 */

// Mock KVAL register addresses
#define CHIP_KVAL_HOLD_ADDR 0x09
#define CHIP_KVAL_RUN_ADDR 0x0A

// Mock L6470 status bit definitions
#define L6470_STATUS_TH_WRN 0x0004
#define L6470_STATUS_STEP_LOSS_A 0x0008
#define L6470_STATUS_STEP_LOSS_B 0x0010
#define L6470_STATUS_OCD 0x0020

/* ==========================================================================
 */
/* Mock Control Functions */
/* ==========================================================================
 */

/**
 * @brief Reset all driver mocks to default state
 */
void MockDrivers_Reset(void);

/**
 * @brief Set return value for driver functions
 * @param function_name Function name
 * @param return_value Return value to use
 */
void MockDrivers_SetReturnValue(const char *function_name,
                                SystemError_t return_value);

/**
 * @brief Set AS5600 angle value to return
 * @param motor_id Motor identifier
 * @param angle_degrees Angle value in degrees
 */
void MockDrivers_SetAS5600Angle(uint8_t motor_id, float angle_degrees);

/**
 * @brief Set L6470 status value to return
 * @param motor_id Motor identifier
 * @param status Status value
 */
void MockDrivers_SetL6470Status(uint8_t motor_id, uint32_t status);

/**
 * @brief Set L6470 parameter value to return
 * @param motor_id Motor identifier
 * @param parameter Parameter address
 * @param value Parameter value
 */
void MockDrivers_SetL6470Parameter(uint8_t motor_id, uint8_t parameter,
                                   uint32_t value);

/**
 * @brief Get call count for driver function
 * @param function_name Function name
 * @return Number of times function was called
 */
uint32_t MockDrivers_GetCallCount(const char *function_name);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_DRIVERS_H */
