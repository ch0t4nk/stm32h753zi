/*
 * uart_dma_device_test.c
 *
 * Minimal device-side UART DMA test helper.
 * Build into a test firmware variant (guard with TEST_UART_DMA) and call
 * `uart_dma_device_test_start()` from main after HAL and peripheral init.
 *
 * This file intentionally keeps logic small and test-focused: it starts
 * circular RX DMA on `huart3` and echoes received blocks back via TX DMA.
 */

#include "config/comm_config.h" /* SSOT: communication constants */
#include "stm32h7xx_hal.h"

#ifdef TEST_UART_DMA

/* External handles expected from application bring-up */
extern UART_HandleTypeDef huart3;

/* Test buffer size - must match host expectations */
#define UART_TEST_BUF_SZ 512
static uint8_t uart_test_rx_buf[UART_TEST_BUF_SZ];

/* Start the device-side UART DMA test
 * - starts circular RX DMA on huart3
 * - on RX complete/half callbacks the code will queue a TX DMA echo
 */
void uart_dma_device_test_start(void) {
    /* Start circular RX DMA */
    if (HAL_UART_Receive_DMA(&huart3, uart_test_rx_buf, UART_TEST_BUF_SZ) !=
        HAL_OK) {
        /* Handle error: blink LED or freeze for debug */
        while (1) {
            /* Intentional hard-fault loop for visibility in tests */
        }
    }
}

/* Simple DMA event handlers - application should route these to the HAL
 * callbacks (these are weak hooks called by the HAL IRQ flow). These
 * implementations intentionally do minimal work: queue TX DMA for echo.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart3) {
        /* Echo back full buffer via TX DMA */
        HAL_UART_Transmit_DMA(huart, uart_test_rx_buf, UART_TEST_BUF_SZ);
    }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart3) {
        /* Echo back first half to exercise half-complete path */
        HAL_UART_Transmit_DMA(huart, uart_test_rx_buf, UART_TEST_BUF_SZ / 2);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart3) {
        /* In tests, on error restart reception */
        HAL_UART_AbortReceive(huart);
        HAL_Delay(10);
        HAL_UART_Receive_DMA(huart, uart_test_rx_buf, UART_TEST_BUF_SZ);
    }
}

#endif /* TEST_UART_DMA */
