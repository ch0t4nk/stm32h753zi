/**
 * @file optimization_compatibility.h
 * @brief Compatibility layer for optimization module development
 * @author STM32H753ZI Motor Control Project
 * @date 2025-08-10
 *
 * This file provides compatibility functions and stubs for functions that are
 * referenced but not yet implemented, allowing the optimization module to
 * compile and run in simulation mode.
 *
 * NOTE: This is a temporary file for Phase 1.4 development. Functions should
 * be moved to their proper driver implementations as they are completed.
 */

#ifndef OPTIMIZATION_COMPATIBILITY_H
#define OPTIMIZATION_COMPATIBILITY_H

#include "common/error_codes.h"
#include "config/motor_config.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// COMPATIBILITY FUNCTION DECLARATIONS
// =============================================================================

/**
 * @brief Compatibility wrapper for encoder position reading
 * @param encoder_id Encoder identifier
 * @param position_deg Pointer to store position in degrees
 * @return SystemError_t Success or error code
 */
SystemError_t as5600_read_position(uint8_t encoder_id, float *position_deg);

/**
 * @brief Compatibility wrapper for safety system status
 * @return bool True if emergency stop is active
 */
bool safety_system_is_emergency_active(void);

/**
 * @brief Compatibility wrapper for L6470 KVAL hold setting
 * @param motor_id Motor identifier
 * @param kval_hold KVAL hold value
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_set_kval_hold(uint8_t motor_id, uint8_t kval_hold);

/**
 * @brief Compatibility wrapper for L6470 KVAL run setting
 * @param motor_id Motor identifier
 * @param kval_run KVAL run value
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_set_kval_run(uint8_t motor_id, uint8_t kval_run);

/**
 * @brief Compatibility wrapper for L6470 KVAL acceleration setting
 * @param motor_id Motor identifier
 * @param kval_acc KVAL acceleration value
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_set_kval_acc(uint8_t motor_id, uint8_t kval_acc);

/**
 * @brief Compatibility wrapper for L6470 KVAL deceleration setting
 * @param motor_id Motor identifier
 * @param kval_dec KVAL deceleration value
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_set_kval_dec(uint8_t motor_id, uint8_t kval_dec);

/**
 * @brief Compatibility wrapper for L6470 acceleration setting
 * @param motor_id Motor identifier
 * @param acceleration Acceleration value
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_set_acceleration(uint8_t motor_id, uint16_t acceleration);

/**
 * @brief Compatibility wrapper for L6470 deceleration setting
 * @param motor_id Motor identifier
 * @param deceleration Deceleration value
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_set_deceleration(uint8_t motor_id, uint16_t deceleration);

/**
 * @brief Compatibility wrapper for L6470 max speed setting
 * @param motor_id Motor identifier
 * @param max_speed Maximum speed value
 * @return SystemError_t Success or error code
 */
SystemError_t l6470_set_max_speed(uint8_t motor_id, uint16_t max_speed);

/**
 * @brief Compatibility wrapper for L6470 emergency stop
 * @param motor_id Motor identifier
 */
void l6470_emergency_stop(uint8_t motor_id);

#ifdef __cplusplus
}
#endif

#endif // OPTIMIZATION_COMPATIBILITY_H
