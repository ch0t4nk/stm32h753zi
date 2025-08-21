/* driver_usart3_shim.c
 * Minimal CMSIS driver shim that exposes Driver_USART3 and forwards
 * transmit operations to the HAL BSP UART handle hcom_uart[COM1].
 * This is intentionally tiny and only implements the subset used by the
 * cmsis_uart_adapter (Initialize, PowerControl, Control, Send, GetTxCount).
 */

#include "driver_usart3_shim.h"
#include "config/comm_config.h"
#include "stm32h7xx_hal.h"

// Declare the BSP UART array provided by the board support package.
// The adapter already references this symbol in the HAL fallback path,
// but our shim will also use it to implement Driver_USART3.
extern UART_HandleTypeDef hcom_uart[];

// Simple internal tracking for bytes sent (polled path)
static volatile uint32_t shim_tx_count = 0;

static int32_t shim_Initialize(ARM_SignalEvent_t cb_event) {
    (void)cb_event;
    shim_tx_count = 0;
    return 0; // ARM_DRIVER_OK (simplified)
}

static int32_t shim_Uninitialize(void) {
    return 0;
}

static int32_t shim_PowerControl(int32_t state) {
    (void)state;
    return 0;
}

static int32_t shim_Control(uint32_t control, uint32_t arg) {
    (void)control;
    (void)arg;
    return 0;
}

static int32_t shim_Send(const void *data, uint32_t num) {
    if (hcom_uart == NULL)
        return -1;
    // Use HAL blocking transmit to keep shim simple and reliable for probe
    HAL_StatusTypeDef st = HAL_UART_Transmit(&hcom_uart[COM1], (uint8_t *)data,
                                             (uint16_t)num, 500);
    if (st == HAL_OK) {
        shim_tx_count += num;
        return 0;
    }
    return -1;
}

static uint32_t shim_GetTxCount(void) {
    return shim_tx_count;
}

// Exported instance (name expected by adapter)
ARM_DRIVER_USART_SHIM Driver_USART3 = {
    .Initialize = shim_Initialize,
    .Uninitialize = shim_Uninitialize,
    .PowerControl = shim_PowerControl,
    .Send = shim_Send,
    .GetTxCount = shim_GetTxCount,
    .Control = shim_Control,
};
