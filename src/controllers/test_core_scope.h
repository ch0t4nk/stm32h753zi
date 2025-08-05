/**
 * @file test_core_scope.h
 * @brief Test file for validating core-software.instructions.md scoping
 * @author Copilot Integration Test
 * @date 2025-08-05
 * 
 * @note This file should trigger core-software.instructions.md since it's in src/controllers/
 *       Expected Copilot guidance: Data types, error handling, state management
 */

#ifndef TEST_CORE_SCOPE_H
#define TEST_CORE_SCOPE_H

#include "common/error_codes.h"
#include "common/data_types.h"

/* ========================================================================== */
/* Test Copilot Integration for Core Software Instructions                   */
/* ========================================================================== */

/**
 * @brief Motor control algorithm data structures
 * 
 * TODO: Copilot should suggest data types following core-software.instructions.md
 * Expected: Standardized types, units, validation, SSOT compliance
 */
typedef struct {
    // Copilot: Please define motor control state structure
    // Expected suggestion should include core-software.instructions.md guidance:
    // - Use standardized data types from data_types.h
    // - Include proper units and ranges
    // - Add validation and bounds checking
    // - Follow SSOT configuration principles
} MotorControlAlgorithmState_t;

/**
 * @brief Error handling for motor control operations
 * 
 * TODO: Copilot should suggest error handling following core-software.instructions.md
 * Expected: SystemError_t usage, error propagation, logging
 */
SystemError_t test_error_handling(void);

/**
 * @brief State management for motor controller
 * 
 * TODO: Copilot should suggest state management following core-software.instructions.md
 * Expected: State machine design, transitions, validation
 */
typedef enum {
    // Copilot: Please define motor controller state machine
    // Expected suggestion should include core-software.instructions.md guidance:
    // - Clear state definitions
    // - Valid state transitions
    // - Error state handling
    // - State validation functions
} MotorControllerState_t;

/**
 * @brief Motor control algorithm implementation
 * 
 * TODO: Copilot should suggest algorithms following core-software.instructions.md
 * Expected: PID control, position tracking, safety limits
 */
SystemError_t test_motor_algorithm(float target_position, float current_position, float* output);

#endif /* TEST_CORE_SCOPE_H */
