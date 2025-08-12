/**
 * @file mock_drivers.c
 * @brief Mock driver implementations for unit testing
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 */

#include "mock_drivers.h"
#include <stdio.h>
#include <string.h>

/* ==========================================================================
 */
/* Mock State Structures */
/* ==========================================================================
 */

typedef struct {
    SystemError_t return_value;
    uint32_t call_count;
    bool enabled;
} MockDriverFunction_t;

typedef struct {
    float angle_degrees[2];            // Mock angles for motor 0 and 1
    MockDriverFunction_t functions[8]; // Mock various driver functions
    uint32_t l6470_status[2];          // Mock status for motor 0 and 1
    uint32_t l6470_parameters[2][256]; // Mock parameters for each motor
    const char *function_names[8];
} MockDriverState_t;

static MockDriverState_t mock_state = {0};

/* ==========================================================================
 */
/* Private Helper Functions */
/* ==========================================================================
 */

static MockDriverFunction_t *get_function_mock(const char *function_name) {
    for (int i = 0; i < 8; i++) {
        if (mock_state.function_names[i] &&
            strcmp(mock_state.function_names[i], function_name) == 0) {
            return &mock_state.functions[i];
        }
    }

    // Find empty slot for new function
    for (int i = 0; i < 8; i++) {
        if (!mock_state.function_names[i]) {
            mock_state.function_names[i] = function_name;
            mock_state.functions[i].return_value = SYSTEM_OK;
            mock_state.functions[i].enabled = true;
            return &mock_state.functions[i];
        }
    }

    return &mock_state.functions[0]; // Fallback
}

/* ==========================================================================
 */
/* AS5600 Driver Mock Implementations */
/* ==========================================================================
 */

SystemError_t as5600_init_encoder(uint8_t motor_id) {
    MockDriverFunction_t *func = get_function_mock("as5600_init_encoder");
    func->call_count++;

    if (motor_id >= 2) {
        return ERROR_INVALID_PARAMETER;
    }

    return func->return_value;
}

SystemError_t as5600_read_angle_degrees(uint8_t motor_id,
                                        float *angle_degrees) {
    MockDriverFunction_t *func =
        get_function_mock("as5600_read_angle_degrees");
    func->call_count++;

    if (motor_id >= 2 || !angle_degrees) {
        return ERROR_INVALID_PARAMETER;
    }

    *angle_degrees = mock_state.angle_degrees[motor_id];
    return func->return_value;
}

SystemError_t as5600_check_magnet(uint8_t encoder_id, bool *magnet_ok) {
    MockDriverFunction_t *func = get_function_mock("as5600_check_magnet");
    func->call_count++;

    if (encoder_id >= 2 || !magnet_ok) {
        return ERROR_INVALID_PARAMETER;
    }

    // For mock purposes, assume magnet is always detected
    *magnet_ok = true;
    return func->return_value;
}

/* ==========================================================================
 */
/* L6470 Driver Mock Implementations */
/* ==========================================================================
 */

SystemError_t l6470_init(void) {
    MockDriverFunction_t *func = get_function_mock("l6470_init");
    func->call_count++;
    return func->return_value;
}

SystemError_t l6470_init_motor(uint8_t motor_id) {
    MockDriverFunction_t *func = get_function_mock("l6470_init_motor");
    func->call_count++;

    if (motor_id >= 2) {
        return ERROR_INVALID_PARAMETER;
    }

    return func->return_value;
}

SystemError_t l6470_get_parameter(uint8_t motor_id, uint8_t parameter,
                                  uint32_t *value) {
    MockDriverFunction_t *func = get_function_mock("l6470_get_parameter");
    func->call_count++;

    if (motor_id >= 2 || !value) {
        return ERROR_INVALID_PARAMETER;
    }

    *value = mock_state.l6470_parameters[motor_id][parameter];
    return func->return_value;
}

SystemError_t l6470_get_status(uint8_t motor_id, uint32_t *status) {
    MockDriverFunction_t *func = get_function_mock("l6470_get_status");
    func->call_count++;

    if (motor_id >= 2 || !status) {
        return ERROR_INVALID_PARAMETER;
    }

    *status = mock_state.l6470_status[motor_id];
    return func->return_value;
}

SystemError_t l6470_hard_stop(uint8_t motor_id) {
    MockDriverFunction_t *func = get_function_mock("l6470_hard_stop");
    func->call_count++;

    if (motor_id >= 2) {
        return ERROR_INVALID_PARAMETER;
    }

    return func->return_value;
}

/* ==========================================================================
 */
/* Mock Control Functions */
/* ==========================================================================
 */

void MockDrivers_Reset(void) {
    memset(&mock_state, 0, sizeof(MockDriverState_t));

    // Set default values
    for (int i = 0; i < 2; i++) {
        mock_state.angle_degrees[i] = 0.0f;
        mock_state.l6470_status[i] = 0x7E83; // Normal operation status

        // Set default KVAL parameters
        mock_state.l6470_parameters[i][CHIP_KVAL_HOLD_ADDR] = 0x29;
        mock_state.l6470_parameters[i][CHIP_KVAL_RUN_ADDR] = 0x29;
    }

    // Reset function names
    for (int i = 0; i < 8; i++) {
        mock_state.function_names[i] = NULL;
    }
}

void MockDrivers_SetReturnValue(const char *function_name,
                                SystemError_t return_value) {
    MockDriverFunction_t *func = get_function_mock(function_name);
    func->return_value = return_value;
}

void MockDrivers_SetAS5600Angle(uint8_t motor_id, float angle_degrees) {
    if (motor_id < 2) {
        mock_state.angle_degrees[motor_id] = angle_degrees;
    }
}

void MockDrivers_SetL6470Status(uint8_t motor_id, uint32_t status) {
    if (motor_id < 2) {
        mock_state.l6470_status[motor_id] = status;
    }
}

void MockDrivers_SetL6470Parameter(uint8_t motor_id, uint8_t parameter,
                                   uint32_t value) {
    if (motor_id < 2) {
        mock_state.l6470_parameters[motor_id][parameter] = value;
    }
}

uint32_t MockDrivers_GetCallCount(const char *function_name) {
    MockDriverFunction_t *func = get_function_mock(function_name);
    return func->call_count;
}
