/**
 * @file mock_drivers.h
 * @brief Mock driver declarations for unit testing
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note This file provides mock implementations of AS5600 encoder
 *       and L6470 motor driver functions for host-based unit testing.
 */

#ifndef MOCK_DRIVERS_H
#define MOCK_DRIVERS_H

#include "common/error_codes.h"
#include "config/l6470_registers_generated.h" // SSOT for L6470 register constants
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 * AS5600 Encoder Mock Functions
 * ==========================================================================
 */

/**
 * @brief Mock initialization of AS5600 encoder
 * @param motor_id Motor identifier (0 or 1)
 * @return SystemError_t Success or error code
 */
SystemError_t as5600_init_encoder(uint8_t motor_id);

/**
 * @brief Mock read angle from AS5600 encoder
 * @param motor_id Motor identifier (0 or 1)
 * @param angle_degrees Pointer to store angle in degrees
 * @return SystemError_t Success or error code
 */
SystemError_t as5600_read_angle_degrees(uint8_t motor_id,
                                        float *angle_degrees);

/**
 * @brief Mock check magnet detection status
 * @param encoder_id Encoder identifier (0 or 1)
 * @param magnet_ok Pointer to store magnet detection status
 * @return SystemError_t Success or error code
 */
SystemError_t as5600_check_magnet(uint8_t encoder_id, bool *magnet_ok);

/* ==========================================================================
 * L6470 Motor Driver Mock Functions
 * ==========================================================================
 */

/**
 * @brief Mock initialization of L6470 driver system
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_init(void);

/**
 * @brief Mock initialization of specific L6470 motor
 * @param motor_id Motor identifier (0 or 1)
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_init_motor(uint8_t motor_id);

/**
 * @brief Mock get parameter from L6470 motor
 * @param motor_id Motor identifier (0 or 1)
 * @param parameter Parameter register address
 * @param value Pointer to store parameter value
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_get_parameter(uint8_t motor_id, uint8_t parameter,
                                  uint32_t *value);

/**
 * @brief Mock get status from L6470 motor
 * @param motor_id Motor identifier (0 or 1)
 * @param status Pointer to store status value
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_get_status(uint8_t motor_id, uint32_t *status);

/**
 * @brief Mock hard stop command for L6470 motor
 * @param motor_id Motor identifier (0 or 1)
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_hard_stop(uint8_t motor_id);

/* ==========================================================================
 * Mock Driver Control Functions
 * ==========================================================================
 */

/**
 * @brief Reset all mock driver states
 */
void MockDrivers_Reset(void);

/**
 * @brief Set return value for mock driver function
 * @param function_name Name of function to configure
 * @param return_value Return value to set
 */
void MockDrivers_SetReturnValue(const char *function_name,
                                SystemError_t return_value);

/**
 * @brief Set mock AS5600 angle value
 * @param motor_id Motor identifier (0 or 1)
 * @param angle_degrees Angle value in degrees
 */
void MockDrivers_SetAS5600Angle(uint8_t motor_id, float angle_degrees);

/**
 * @brief Set mock L6470 status value
 * @param motor_id Motor identifier (0 or 1)
 * @param status Status value to set
 */
void MockDrivers_SetL6470Status(uint8_t motor_id, uint32_t status);

/**
 * @brief Set mock L6470 parameter value
 * @param motor_id Motor identifier (0 or 1)
 * @param parameter Parameter register address
 * @param value Parameter value to set
 */
void MockDrivers_SetL6470Parameter(uint8_t motor_id, uint8_t parameter,
                                   uint32_t value);

/**
 * @brief Get call count for mock driver function
 * @param function_name Name of function to check
 * @return uint32_t Number of times function was called
 */
uint32_t MockDrivers_GetCallCount(const char *function_name);

#endif /* MOCK_DRIVERS_H */
