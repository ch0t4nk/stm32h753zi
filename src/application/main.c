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
#include <stdio.h>

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

    /* Minimal boot console init: ensure USART3 is up early so we can print
     * boot diagnostics regardless of higher-level init order. Uses PD8/PD9
     * (USART3) and AF7 which is the Nucleo default mapping. */
    {
        UART_HandleTypeDef boot_huart;
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        /* Enable GPIO and USART clocks */
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_USART3_CLK_ENABLE();

        /* Configure PD8 = TX, PD9 = RX */
        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* Configure UART */
        boot_huart.Instance = USART3;
        boot_huart.Init.BaudRate = UART_BAUDRATE;
        boot_huart.Init.WordLength = UART_WORDLENGTH_8B;
        boot_huart.Init.StopBits = UART_STOPBITS_1;
        boot_huart.Init.Parity = UART_PARITY_NONE;
        boot_huart.Init.Mode = UART_MODE_TX_RX;
        boot_huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        boot_huart.Init.OverSampling = UART_OVERSAMPLING_16;

        if (HAL_UART_Init(&boot_huart) == HAL_OK) {
            char boot_msg[128];
            int n = snprintf(boot_msg, sizeof(boot_msg),
                             "BOOT: UART_BAUD=%d SAFE_NO_MOTOR_POWER=%d\r\n",
                             (int)UART_BAUDRATE,
#ifdef SAFE_NO_MOTOR_POWER
                             1
#else
                             0
#endif
            );
            if (n > 0) {
                HAL_UART_Transmit(&boot_huart, (uint8_t *)boot_msg,
                                  (uint16_t)n, 100);
            }
            /* Do not deinit - allow higher-level code to reuse USART3 if
             * desired. */
        }
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

    /* Boot-time diagnostic message: report UART baud and SAFE_NO_MOTOR_POWER
     */
#if defined(UART_BAUDRATE)
    {
        char boot_msg[128];
        int n = snprintf(boot_msg, sizeof(boot_msg),
                         "BOOT: UART_BAUD=%d SAFE_NO_MOTOR_POWER=%d\r\n",
                         (int)UART_BAUDRATE,
#ifdef SAFE_NO_MOTOR_POWER
                         1
#else
                         0
#endif
        );
        if (n > 0) {
            /* Use printf if retargeting exists */
            printf("%s", boot_msg);

            /* Fallback: try HAL UART if huart3 symbol is present */
            extern UART_HandleTypeDef
                huart3; /* weak extern - may be present */
            /* Attempt HAL transmit; if huart3 uninitialized this may hang in
             * HAL_MAX_DELAY so we guard by checking huart3.Instance != NULL if
             * the structure is usable. */
            if ((void *)&huart3 != NULL && huart3.Instance != NULL) {
                HAL_UART_Transmit(&huart3, (uint8_t *)boot_msg, (uint16_t)n,
                                  100);
            }
        }
    }
#endif

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
