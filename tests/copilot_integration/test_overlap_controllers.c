/**
 * @file test_overlap_controllers.c
 * @brief Test file for validating overlapping instruction patterns in src/controllers/
 * @author Copilot Integration Test
 * @date 2025-08-05
 * 
 * @note This file should trigger BOTH:
 *       - core-software.instructions.md (src/{common,controllers,drivers}/**/*.{c,h})
 *       - safety-rt.instructions.md (src/{safety,controllers}/**/*.{c,h})
 *       Expected: Combined guidance from both instruction files
 */

#include "stm32h7xx_hal.h"
#include "common/error_codes.h"

/* ========================================================================== */
/* Test Copilot Integration for Overlapping Instruction Patterns            */
/* ========================================================================== */

/**
 * @brief Motor controller with safety and core software guidance
 * 
 * TODO: Copilot should suggest implementation combining BOTH instruction files:
 * - core-software.instructions.md: Data types, error handling, state management
 * - safety-rt.instructions.md: Safety systems, real-time control, timing
 */
SystemError_t test_overlapping_guidance(void) {
    // Copilot: Please implement a motor controller with safety features
    // Expected suggestion should include guidance from BOTH:
    // 1. core-software.instructions.md:
    //    - Standardized data types and error codes
    //    - State management patterns
    //    - Motor control algorithms
    // 2. safety-rt.instructions.md:
    //    - Real-time timing requirements
    //    - Safety monitoring and fault detection
    //    - Emergency stop procedures
    
    return SYSTEM_OK;
}

/**
 * @brief Test instruction priority and combination
 * 
 * TODO: Validate how Copilot handles multiple instruction files
 * Expected: Coherent combination of relevant guidance from both files
 */
void test_instruction_priority(void) {
    // Copilot: Please implement position control with safety limits
    // Expected: Combined guidance showing how core software principles
    // integrate with safety requirements for motor control
    
    // This should demonstrate:
    // - Core software: Position algorithms, data validation
    // - Safety: Position limits, emergency stops, fault detection
}

/**
 * @brief Real-time motor control algorithm
 * 
 * TODO: Test specific overlapping concepts
 * Expected: Algorithm design with safety considerations
 */
void test_realtime_algorithm_with_safety(void) {
    // Copilot: Please implement real-time PID controller with safety monitoring
    // Expected combination:
    // - Algorithm structure (core-software)
    // - Timing constraints (safety-rt)
    // - Error handling (both)
    // - State management (both)
}
