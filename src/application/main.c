/* STM32H753ZI Stepper Motor Control - Main Application
 * Generated following file-organization.instructions.md
 * SSOT compliance: All configs from src/config/
 * Phase 1 Step 3: Watchdog Integration - COMPLETE
 */

#include "config/build_config.h"
#include "config/clock_config.h"
#include "config/comm_config.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#include "config/safety_config.h"

// TODO: See .github/instructions/stm32h7-spi-l6470.instructions.md for L6470
// integration
// TODO: See .github/instructions/stm32h7-i2c-as5600.instructions.md for AS5600
// integration

/* HAL Headers */
#include "hal_abstraction/hal_abstraction.h"
#include "main.h"
#include "main_application.h"
#include "stm32h7xx_hal.h"

/* Function prototypes */
void Error_Handler(void);

/* Driver Headers (to be implemented) */
// #include "drivers/l6470/l6470_driver.h"
// #include "drivers/as5600/as5600_driver.h"

/**
 * @brief Error Handler
 * @note Follows safety-systems.instructions.md with integrated emergency stop
 */
void Error_Handler(void) {
    /* Critical error - attempt emergency stop if possible */
    if (main_application_is_initialized()) {
        main_application_emergency_stop();
    }

    /* TODO: Implement complete safety shutdown following safety_config.h */
    __disable_irq();
    while (1) {
        /* Emergency stop - all motors halt, system in safe state */
    }
}

/**
 * @brief Main function
 * @note Entry point with integrated safety system and watchdog
 */
int main(void) {
    /* Initialize HAL */
    HAL_Init();

    /* Configure system clock using SSOT approach */
    if (Clock_Init() != HAL_OK) {
        Error_Handler();
    }

    /* TODO: Initialize peripherals following instruction files:
     * - MOTOR_SPI_INSTANCE for L6470 (stm32h7-spi-l6470.instructions.md)
     * - ENCODER_I2C_INSTANCE for AS5600 (stm32h7-i2c-as5600.instructions.md)
     * - UART_INSTANCE for debug (stm32h7-uart-protocol.instructions.md)
     * - Safety systems (safety-systems.instructions.md)
     */

    /* Initialize main application (includes safety system and watchdog) */
    SystemError_t init_result = main_application_init();
    if (init_result != SYSTEM_OK) {
        /* Critical initialization failure - enter error state */
        Error_Handler();
    }

    /* Run self-test to validate safety systems */
    SystemError_t test_result = main_application_self_test();
    if (test_result != SYSTEM_OK) {
        /* Self-test failure - enter error state */
        Error_Handler();
    }

    /* Main control loop with integrated safety and watchdog management */
    while (1) {
        SystemError_t run_result = main_application_run();
        if (run_result != SYSTEM_OK) {
            /* Handle application errors */
            if (run_result == ERROR_SAFETY_EMERGENCY_STOP) {
                /* Emergency stop is active - continue monitoring */
                continue;
            } else if (run_result == ERROR_SAFETY_WATCHDOG_WARNING) {
                /* Watchdog warning - continue but monitor closely */
                continue;
            } else {
                /* Critical error - enter error state */
                Error_Handler();
            }
        }
    }
}
