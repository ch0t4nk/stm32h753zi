/**
 * @file optimization_compatibility.c
 * @brief Compatibility layer implementation for optimization module
 * development
 * @author STM32H753ZI Motor Control Project
 * @date 2025-08-10
 *
 * This file provides stub implementations for functions that are referenced
 * but not yet implemented, allowing the optimization module to compile and
 * run in simulation mode.
 *
 * NOTE: This is a temporary file for Phase 1.4 development. Functions should
 * be moved to their proper driver implementations as they are completed.
 */

#include "optimization_compatibility.h"
#include "common/error_codes.h"
#include "config/motor_config.h"
#include "drivers/as5600/as5600_driver.h"
#include "drivers/l6470/l6470_driver.h"
#include "safety/emergency_stop_abstracted.h"
#include <math.h>

// =============================================================================
// STATIC VARIABLES FOR SIMULATION
// =============================================================================

static float simulated_motor_positions[MAX_MOTORS] = {0.0f, 0.0f};
static bool simulated_emergency_active = false;
static uint32_t simulation_counter = 0;

// =============================================================================
// ENCODER COMPATIBILITY FUNCTIONS
// =============================================================================

SystemError_t as5600_read_position(uint8_t encoder_id, float *position_deg) {
    if (encoder_id >= MAX_MOTORS || position_deg == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // In simulation mode, use the actual as5600_read_angle_degrees function
    // if available, otherwise provide simulated data
    SystemError_t result = as5600_read_angle_degrees(encoder_id, position_deg);

    if (result == SYSTEM_OK) {
        return SYSTEM_OK;
    }

    // Fallback to simulation if hardware not available
    // Simulate a slowly changing position for testing
    simulation_counter++;
    simulated_motor_positions[encoder_id] =
        sinf((float)simulation_counter * 0.01f) * 90.0f;

    *position_deg = simulated_motor_positions[encoder_id];

    return SYSTEM_OK;
}

// =============================================================================
// SAFETY SYSTEM COMPATIBILITY FUNCTIONS
// =============================================================================

bool safety_system_is_emergency_active(void) {
    // Get actual emergency stop status
    EmergencyStopState_t estop_state = emergency_stop_get_state();

    // Check if emergency stop is triggered or in fault state
    return (estop_state == EMERGENCY_STOP_TRIGGERED ||
            estop_state == EMERGENCY_STOP_FAULT);
}

// =============================================================================
// L6470 PARAMETER SETTING COMPATIBILITY FUNCTIONS
// =============================================================================

SystemError_t l6470_set_kval_hold(uint8_t motor_id, uint8_t kval_hold) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }

    // Use the generic parameter setting function
    return l6470_set_parameter(motor_id, L6470_REG_KVAL_HOLD, kval_hold);
}

SystemError_t l6470_set_kval_run(uint8_t motor_id, uint8_t kval_run) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }

    return l6470_set_parameter(motor_id, L6470_REG_KVAL_RUN, kval_run);
}

SystemError_t l6470_set_kval_acc(uint8_t motor_id, uint8_t kval_acc) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }

    return l6470_set_parameter(motor_id, L6470_REG_KVAL_ACC, kval_acc);
}

SystemError_t l6470_set_kval_dec(uint8_t motor_id, uint8_t kval_dec) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }

    return l6470_set_parameter(motor_id, L6470_REG_KVAL_DEC, kval_dec);
}

SystemError_t l6470_set_acceleration(uint8_t motor_id, uint16_t acceleration) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }

    return l6470_set_parameter(motor_id, L6470_REG_ACC, acceleration);
}

SystemError_t l6470_set_deceleration(uint8_t motor_id, uint16_t deceleration) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }

    return l6470_set_parameter(motor_id, L6470_REG_DEC, deceleration);
}

SystemError_t l6470_set_max_speed(uint8_t motor_id, uint16_t max_speed) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }

    return l6470_set_parameter(motor_id, L6470_REG_MAX_SPEED, max_speed);
}

void l6470_emergency_stop(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS) {
        return;
    }

    // Use hard stop for emergency
    l6470_hard_stop(motor_id);

    // Set simulation flag
    simulated_emergency_active = true;
}
