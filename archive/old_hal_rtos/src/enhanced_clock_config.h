/* Enhanced Clock Configuration Header
 * STM32H753ZI HSE startup improvements
 */

#ifndef ENHANCED_CLOCK_CONFIG_H
#define ENHANCED_CLOCK_CONFIG_H

#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Clock source identifiers */
#define CLOCK_SOURCE_HSI 0
#define CLOCK_SOURCE_CSI 1
#define CLOCK_SOURCE_HSE 2
#define CLOCK_SOURCE_PLL1 3

/* Function prototypes */
HAL_StatusTypeDef Enhanced_HSE_Config(void);
HAL_StatusTypeDef Fallback_HSI_Config(void);
void Enhanced_SystemClock_Config(void);
uint32_t Check_Active_Clock_Source(void);
uint32_t Get_System_Clock_Frequency(void);

#ifdef __cplusplus
}
#endif

#endif /* ENHANCED_CLOCK_CONFIG_H */
