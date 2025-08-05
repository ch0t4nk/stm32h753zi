/* STM32H7 HAL Configuration for Stepper Motor Project
 * Generated following hardware-config.instructions.md
 * SSOT compliance: hardware_config.h integration
 */

#ifndef STM32H7XX_HAL_CONF_H
#define STM32H7XX_HAL_CONF_H

// TODO: See .github/instructions/hardware-pins.instructions.md for pin mapping details
// TODO: See .github/instructions/stm32h7-spi-l6470.instructions.md for SPI configuration

#include "config/hardware_config.h"  // SSOT hardware definitions

/* STM32H7 HAL Module Configuration */
#define HAL_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED      // L6470 communication
#define HAL_I2C_MODULE_ENABLED      // AS5600 encoders
#define HAL_UART_MODULE_ENABLED     // Debug/control interface
#define HAL_FDCAN_MODULE_ENABLED    // CAN communication
#define HAL_ETH_MODULE_ENABLED      // Ethernet communication
#define HAL_GPIO_MODULE_ENABLED     // GPIO control

/* Clock Configuration (from hardware_config.h) */
#define HSE_VALUE               ((uint32_t)SYSTEM_CLOCK_HSE_HZ)
#define HSI_VALUE               ((uint32_t)16000000U)
#define CSI_VALUE               ((uint32_t)4000000U)
#define HSE_STARTUP_TIMEOUT     ((uint32_t)100U)
#define HSI_STARTUP_TIMEOUT     ((uint32_t)5000U)
#define CSI_STARTUP_TIMEOUT     ((uint32_t)5000U)

/* SPI Configuration for L6470 */
#define HAL_SPI_TIMEOUT_DEFAULT_VALUE   5000U

/* I2C Configuration for AS5600 */
#define HAL_I2C_TIMEOUT_DEFAULT_VALUE   10000U

#endif /* STM32H7XX_HAL_CONF_H */
