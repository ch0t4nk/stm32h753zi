/**
 * @file test_hardware_scope.c
 * @brief Test file for validating hardware.instructions.md scoping
 * @author Copilot Integration Test
 * @date 2025-08-05
 * 
 * @note This file should trigger hardware.instructions.md since it's in src/drivers/
 *       Expected Copilot guidance: GPIO control, memory management, STM32H7 HAL
 */

#include "stm32h7xx_hal.h"

/* ========================================================================== */
/* Test Copilot Integration for Hardware Instructions                        */
/* ========================================================================== */

/**
 * @brief Initialize GPIO pin for motor control
 * 
 * TODO: Copilot should suggest hardware configuration following hardware.instructions.md
 * Expected: GPIO HAL functions, pin configuration, SSOT hardware_config.h usage
 */
void test_gpio_init(void) {
    // Copilot: Please initialize a GPIO pin for L6470 CS control
    // Expected suggestion should include hardware.instructions.md guidance:
    // - Use SSOT hardware_config.h definitions
    // - STM32H7 HAL GPIO functions
    // - Proper pin configuration structs
}

/**
 * @brief Configure SPI peripheral for L6470 communication
 * 
 * TODO: Copilot should suggest SPI configuration following hardware.instructions.md
 * Expected: STM32H7 SPI HAL, DMA configuration, error handling
 */
void test_spi_config(void) {
    // Copilot: Please configure SPI2 for L6470 stepper driver communication
    // Expected suggestion should include hardware.instructions.md guidance:
    // - STM32H7 SPI HAL configuration
    // - DMA setup for efficient communication
    // - Error handling and timeout management
}

/**
 * @brief Memory management for driver buffers
 * 
 * TODO: Copilot should suggest memory management following hardware.instructions.md
 * Expected: STM32H7 memory regions, DMA coherency, cache management
 */
void test_memory_management(void) {
    // Copilot: Please allocate DMA-coherent buffer for SPI communication
    // Expected suggestion should include hardware.instructions.md guidance:
    // - STM32H7 memory regions (AXI SRAM, DTCM, etc.)
    // - Cache coherency considerations
    // - DMA buffer alignment requirements
}
