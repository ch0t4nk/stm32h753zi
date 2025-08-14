/**
 * @brief Simple test main header for UART validation
 */
#ifndef UART_TEST_H
#define UART_TEST_H

#include "stm32h7xx_hal.h"

/* Function prototypes */
void SystemClock_Config(void);
void UART_Init(void);
void GPIO_Init(void);
void Error_Handler(void);

#endif /* UART_TEST_H */
