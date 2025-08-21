/* Minimal USART3 probe test
 * - Initializes probe instrumentation GPIO (SSOT: PROBE_INSTR_GPIO_PORT/PIN)
 * - Initializes USART3 (PD8=TX, PD9=RX) via HAL
 * - Sends "PROBE:COM1_USART3" every 500ms for 10s, toggling probe GPIO
 */

#include "config/hardware_config.h"
#include "stm32h7xx_hal.h"
#include <string.h>

// Forward-declare startup helper (main will call uart_probe_device_test_start)
void uart_probe_device_test_start(void);

static UART_HandleTypeDef huart3;

static void MX_USART3_Init(void) {
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    // Configure PD8 (TX) PD9 (RX)
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    huart3.Instance = USART3;
    huart3.Init.BaudRate = DEBUG_UART_BAUDRATE;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_DeInit(&huart3);
    HAL_UART_Init(&huart3);
}

static void probe_gpio_init(void) {
    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitTypeDef gpio_init = {0};
    gpio_init.Pin = PROBE_INSTR_GPIO_PIN;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(PROBE_INSTR_GPIO_PORT, &gpio_init);
}

void uart_probe_device_test_start(void) {
    probe_gpio_init();
    MX_USART3_Init();

    const char *banner = "PROBE:COM1_USART3\r\n";
    size_t len = strlen(banner);

    for (int i = 0; i < 20; ++i) {
        HAL_GPIO_WritePin(PROBE_INSTR_GPIO_PORT, PROBE_INSTR_GPIO_PIN,
                          GPIO_PIN_SET);
        HAL_UART_Transmit(&huart3, (uint8_t *)banner, (uint16_t)len, 200);
        HAL_GPIO_WritePin(PROBE_INSTR_GPIO_PORT, PROBE_INSTR_GPIO_PIN,
                          GPIO_PIN_RESET);
        HAL_Delay(500);
    }
}
/* Sequential UART probe device test
 * - Initializes candidate USARTs (currently targets USART3 via BSP) one at a
 * time
 * - Emits an identifying ASCII banner on each
 * - Does not enable any motor outputs (honors SAFE_NO_MOTOR_POWER)
 * - Uses BSP_COM_Init for COM1 (USART3) when available to reuse MSP init
 */

#include "config/comm_config.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#include "drivers/adaptation/cmsis_uart_adapter.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include <stdio.h>

#ifndef DEBUG_UART_BAUDRATE
#define DEBUG_UART_BAUDRATE 115200
#endif

static void probe_uart_banner(HUART_HandleTypeDef *huart, const char *name) {
    char banner[64];
    int len = snprintf(banner, sizeof(banner), "PROBE:%s\r\n", name);
    if (len > 0) {
        // Toggle instrumentation pin before transmit
        HAL_GPIO_WritePin(PROBE_INSTR_GPIO_PORT, PROBE_INSTR_GPIO_PIN,
                          GPIO_PIN_SET);
        HAL_UART_Transmit(huart, (uint8_t *)banner, (uint16_t)len, 200);
        HAL_GPIO_WritePin(PROBE_INSTR_GPIO_PORT, PROBE_INSTR_GPIO_PIN,
                          GPIO_PIN_RESET);
    }
}

void uart_probe_start(void) {
    // Initialize probe instrumentation GPIO (low-risk): configure pin as
    // output
    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitTypeDef gpio_init = {0};
    gpio_init.Pin = PROBE_INSTR_GPIO_PIN;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(PROBE_INSTR_GPIO_PORT, &gpio_init);

    // Respect SAFE_NO_MOTOR_POWER at compile time; runtime enforcement is the
    // responsibility of the motor subsystem. This probe will not touch motor
    // power rails.

    // Candidate: COM1 (USART3) via BSP helper
#if (USE_BSP_COM_FEATURE > 0)
    COM_InitTypeDef comInit;
    comInit.BaudRate = DEBUG_UART_BAUDRATE;
    comInit.WordLength = COM_WORDLENGTH_8B;
    comInit.StopBits = COM_STOPBITS_1;
    comInit.Parity = COM_PARITY_NONE;
    comInit.HwFlowCtl = COM_HWCONTROL_NONE;

    if (BSP_COM_Init(COM1, &comInit) == BSP_ERROR_NONE) {
        extern UART_HandleTypeDef hcom_uart[];
        // Send banner repeatedly for 10s (every 500ms) to avoid missing early
        // sends
        for (int i = 0; i < 20; ++i) {
            probe_uart_banner(&hcom_uart[COM1], "COM1_USART3");
            HAL_Delay(500);
        }
    }
#else
    // Use CMSIS/HAL adapter when BSP COM is disabled
    if (cmsis_uart_init() == 0) {
        // Repeated send for 10s
        for (int i = 0; i < 20; ++i) {
            cmsis_uart_send_str("PROBE:COM1_USART3\r\n");
            HAL_Delay(500);
        }
    }
#endif

    // Delay to allow host to capture banner
    HAL_Delay(100);
}

// Entry point used by main when TEST_UART_PROBE is defined
void uart_probe_device_test_start(void) {
    uart_probe_start();
}
