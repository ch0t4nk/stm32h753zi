/**
 * @file test_api_scope.h
 * @brief Test file for validating api-ui.instructions.md scoping
 * @author Copilot Integration Test
 * @date 2025-08-05
 * 
 * @note This file should trigger api-ui.instructions.md since it's in src/application/
 *       Expected Copilot guidance: API design, user interfaces, documentation generation
 */

#ifndef TEST_API_SCOPE_H
#define TEST_API_SCOPE_H

#include "common/error_codes.h"

/* ========================================================================== */
/* Test Copilot Integration for API/UI Instructions                         */
/* ========================================================================== */

/**
 * @brief Public API for motor control system
 * 
 * TODO: Copilot should suggest API design following api-ui.instructions.md
 * Expected: RESTful design, consistent interfaces, documentation
 */
typedef struct {
    // Copilot: Please design public API structure for motor control
    // Expected suggestion should include api-ui.instructions.md guidance:
    // - Consistent function naming conventions
    // - Clear parameter validation
    // - Comprehensive error reporting
    // - Version compatibility management
} MotorControlAPI_t;

/**
 * @brief User interface command structure
 * 
 * TODO: Copilot should suggest UI design following api-ui.instructions.md
 * Expected: User-friendly interfaces, validation, feedback
 */
typedef struct {
    // Copilot: Please design user command interface structure
    // Expected suggestion should include api-ui.instructions.md guidance:
    // - Human-readable command formats
    // - Input validation and sanitization
    // - Clear success/error feedback
    // - Help and documentation integration
} UserCommand_t;

/**
 * @brief Initialize motor control API
 * 
 * TODO: Copilot should suggest API initialization following api-ui.instructions.md
 * Expected: Initialization sequence, resource allocation, configuration
 */
SystemError_t motor_api_init(void);

/**
 * @brief External integration interface
 * 
 * TODO: Copilot should suggest integration design following api-ui.instructions.md
 * Expected: Plugin architecture, data exchange, protocol adaptation
 */
SystemError_t test_external_integration(void);

/**
 * @brief Documentation generation functions
 * 
 * TODO: Copilot should suggest documentation following api-ui.instructions.md
 * Expected: Auto-generated docs, API reference, examples
 */
void test_documentation_generation(void);

#endif /* TEST_API_SCOPE_H */
