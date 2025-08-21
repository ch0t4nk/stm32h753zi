/* Minimal CMSIS USART shim for Driver_USART3
 * This file provides a tiny ARM_DRIVER_USART-compatible instance named
 * Driver_USART3 that delegates transmit-only operations to the existing
 * HAL BSP UART handle (hcom_uart[COM1]). Intended for test/probe builds
 * where a full CMSIS driver is not available but the adapter expects
 * a symbol named Driver_USART3.
 */

#ifndef DRIVER_USART3_SHIM_H
#define DRIVER_USART3_SHIM_H

#include <stdint.h>

// CMSIS driver status codes and typedefs (minimal)
typedef int32_t ARM_DRIVER_VERSION;
typedef void (*ARM_SignalEvent_t)(uint32_t event);

typedef struct {
    int32_t (*Initialize)(ARM_SignalEvent_t cb_event);
    int32_t (*Uninitialize)(void);
    int32_t (*PowerControl)(int32_t state);
    int32_t (*Send)(const void *data, uint32_t num);
    uint32_t (*GetTxCount)(void);
    int32_t (*Control)(uint32_t control, uint32_t arg);
} ARM_DRIVER_USART_SHIM;

/* Expose the symbol name expected by the adapter. We implement a small
 * instance below in driver_usart3_shim.c named Driver_USART3.
 */
extern ARM_DRIVER_USART_SHIM Driver_USART3;

#endif // DRIVER_USART3_SHIM_H
