/* cmsis_uart_adapter.h
 * Minimal adapter that exposes a small send API and uses the project's
 * CMSIS USART driver when available (compile-time guard), otherwise falls
 * back to a HAL-based transmit call. This keeps the probe test independent
 * of BSP helpers.
 */
#ifndef CMSIS_UART_ADAPTER_H
#define CMSIS_UART_ADAPTER_H

#include "config/comm_config.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the adapter. Returns 0 on success.
int cmsis_uart_init(void);

// Send raw buffer. Returns 0 on success.
int cmsis_uart_send(const char *buf, size_t len, uint32_t timeout_ms);

// Convenience: send a null-terminated string.
int cmsis_uart_send_str(const char *s);

#ifdef __cplusplus
}
#endif

#endif // CMSIS_UART_ADAPTER_H
