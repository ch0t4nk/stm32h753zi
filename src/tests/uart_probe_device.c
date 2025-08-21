/* Sequential UART probe device test
 * - Initializes candidate USARTs (currently targets USART3 via BSP) one at a
 * time
 * - Emits an identifying ASCII banner on each
 * - Does not enable any motor outputs (honors SAFE_NO_MOTOR_POWER)
 * - Uses BSP_COM_Init for COM1 (USART3) when available to reuse MSP init
 */

#include "config/comm_config.h"
#include "config/motor_config.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include <stdio.h>

#ifndef DEBUG_UART_BAUDRATE
#define DEBUG_UART_BAUDRATE 115200
#endif

static void probe_uart_banner(UART_HandleTypeDef *huart, const char *name) {
    char banner[64];
    int len = snprintf(banner, sizeof(banner), "PROBE:%s\r\n", name);
    if (len > 0) {
        HAL_UART_Transmit(huart, (uint8_t *)banner, (uint16_t)len, 200);
    }
}

void uart_probe_start(void) {
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
        probe_uart_banner(&hcom_uart[COM1], "COM1_USART3");
    }
#endif

    // Delay to allow host to capture banner
    HAL_Delay(100);
}

// Entry point used by main when TEST_UART_PROBE is defined
void uart_probe_device_test_start(void) {
    uart_probe_start();
}
