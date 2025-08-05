/**
 * @file test_overlap_communication.c
 * @brief Test file for validating overlapping instruction patterns in src/communication/
 * @author Copilot Integration Test
 * @date 2025-08-05
 * 
 * @note This file should trigger BOTH:
 *       - comms.instructions.md (src/communication/**/*.{c,h})
 *       - api-ui.instructions.md (src/{communication,application}/**/*.{c,h})
 *       Expected: Combined guidance from both instruction files
 */

#include "stm32h7xx_hal.h"
#include "common/error_codes.h"

/* ========================================================================== */
/* Test Copilot Integration for Communication Overlapping Patterns         */
/* ========================================================================== */

/**
 * @brief Communication protocol with API design
 * 
 * TODO: Copilot should suggest implementation combining BOTH instruction files:
 * - comms.instructions.md: Communication protocols, UART/CAN/SPI/I2C interfaces
 * - api-ui.instructions.md: API design, user interfaces, documentation generation
 */
SystemError_t test_communication_api_overlap(void) {
    // Copilot: Please implement UART communication protocol with public API
    // Expected suggestion should include guidance from BOTH:
    // 1. comms.instructions.md:
    //    - Protocol frame structure and parsing
    //    - Error detection and recovery
    //    - Hardware interface configuration
    // 2. api-ui.instructions.md:
    //    - Clean API design patterns
    //    - User-friendly interface functions
    //    - Documentation and examples
    
    return SYSTEM_OK;
}

/**
 * @brief External communication interface
 * 
 * TODO: Test how both instruction files influence external integration
 * Expected: Protocol design with API considerations
 */
void test_external_protocol_api(void) {
    // Copilot: Please implement external device communication with user API
    // Expected combination:
    // - Low-level protocol implementation (comms)
    // - High-level API wrapper (api-ui)
    // - Error handling for both layers
    // - Documentation for users
}

/**
 * @brief Network communication with user interface
 * 
 * TODO: Validate guidance for networked motor control
 * Expected: Network protocols with user-friendly APIs
 */
SystemError_t test_network_ui_integration(void) {
    // Copilot: Please implement CAN network communication with web API
    // Expected guidance combining:
    // - CAN protocol specifics (comms)
    // - RESTful API design (api-ui)
    // - User interface considerations (api-ui)
    // - Network topology management (comms)
    
    return SYSTEM_OK;
}
