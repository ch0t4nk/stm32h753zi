/* cmsis_uart_adapter.c
 * Minimal adapter implementation.
 */

#include "cmsis_uart_adapter.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include <stdint.h>
#include <string.h>

// SSOT: communication constants
#include "config/comm_config.h"

// Sanity-check SSOT value at compile time
_Static_assert((DEBUG_UART_BAUDRATE > 0) && (DEBUG_UART_BAUDRATE <= 1000000),
               "DEBUG_UART_BAUDRATE must be >0 and <= 1,000,000");

// CMSIS driver types/controls — we declare minimal control symbols used below
#ifdef __has_include
#if __has_include(<Driver_USART.h>)
#include <Driver_USART.h>
#endif
#endif

// If CMSIS driver is available, prefer it. We detect by presence of a weak
// symbol that is usually provided in the CMSIS driver layer (Driver_USART3).
#if defined(Driver_USART3) || defined(__DRIVER_USART_PRESENT)
// If the project's build exposes Driver_USART3, declare it here. The linker
// will resolve it when CMSIS drivers are linked into the final firmware.
extern ARM_DRIVER_USART Driver_USART3;
#define HAVE_CMSIS_USART 1
#endif

// NOTE: experiments previously forced HAL fallback. For FTR-042 we prefer a
// CMSIS shim (Driver_USART3) that delegates to HAL; do not force HAL fallback
// here. Define FORCE_HAL_FALLBACK via build flags only for temporary tests.

int cmsis_uart_init(void) {
#if defined(HAVE_CMSIS_USART) && !defined(FORCE_HAL_FALLBACK)
    // Initialize CMSIS USART3 instance
    int32_t status = Driver_USART3.Initialize(NULL);
    if (status != 0)
        return -1;
    // Power up and configure asynchronous mode at default baud from config
    status = Driver_USART3.PowerControl(ARM_POWER_FULL);
    if (status != 0)
        return -1;
    // Set mode and default baud
    Driver_USART3.Control(ARM_USART_MODE_ASYNCHRONOUS, DEBUG_UART_BAUDRATE);
    Driver_USART3.Control(ARM_USART_CONTROL_TX, 1);
    return 0;
#else
    // Nothing to init for HAL path; HAL will use existing USART handles.
    return 0;
#endif
}

int cmsis_uart_send(const char *buf, size_t len, uint32_t timeout_ms) {
#if defined(HAVE_CMSIS_USART) && !defined(FORCE_HAL_FALLBACK)
    int32_t status = Driver_USART3.Send((const void *)buf, (uint32_t)len);
    if (status != 0)
        return -1;
    // Busy-wait for completion by polling GetTxCount (simple, small adapter).
    uint32_t sent = 0;
    for (uint32_t t = 0; t < timeout_ms; t += 5) {
        sent = Driver_USART3.GetTxCount();
        if (sent >= len)
            return 0;
        HAL_Delay(5);
    }
    return -1;
#else
    // Fallback: use HAL handle from BSP-style extern if available
    extern UART_HandleTypeDef hcom_uart[]; // weak link if BSP provides it
    if (&hcom_uart[COM1] != NULL) {
        HAL_StatusTypeDef st = HAL_UART_Transmit(
            &hcom_uart[COM1], (uint8_t *)buf, (uint16_t)len, timeout_ms);
        return (st == HAL_OK) ? 0 : -1;
    }
    return -1;
#endif
}

int cmsis_uart_send_str(const char *s) {
    return cmsis_uart_send(s, strlen(s), 200);
}
