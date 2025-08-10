/**
 * @file mock_hal_types.h
 * @brief Mock HAL type definitions for unit testing environment
 *
 * @details This file provides mock STM32 HAL type definitions to enable
 * compilation of STM32-specific driver code in the unit testing environment
 * without requiring the full STM32 HAL library.
 */

#ifndef MOCK_HAL_TYPES_H
#define MOCK_HAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

// Prevent multiple inclusions of HAL
#ifndef STM32H7xx_HAL_H
#define STM32H7xx_HAL_H

// HAL Status definitions
typedef enum {
    HAL_OK = 0x00U,
    HAL_ERROR = 0x01U,
    HAL_BUSY = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

// Mock I2C Handle
typedef struct {
    void *Instance;     // Mock I2C instance pointer
    void *Init;         // Mock I2C init structure pointer
    uint8_t *pBuffPtr;  // Mock buffer pointer
    uint16_t XferSize;  // Mock transfer size
    uint16_t XferCount; // Mock transfer count
    uint32_t State;     // Mock I2C state
    uint32_t ErrorCode; // Mock error code
} I2C_HandleTypeDef;

// Mock SPI Handle
typedef struct {
    void *Instance;       // Mock SPI instance pointer
    void *Init;           // Mock SPI init structure pointer
    uint8_t *pTxBuffPtr;  // Mock TX buffer pointer
    uint16_t TxXferSize;  // Mock TX transfer size
    uint16_t TxXferCount; // Mock TX transfer count
    uint8_t *pRxBuffPtr;  // Mock RX buffer pointer
    uint16_t RxXferSize;  // Mock RX transfer size
    uint16_t RxXferCount; // Mock RX transfer count
    uint32_t State;       // Mock SPI state
    uint32_t ErrorCode;   // Mock error code
} SPI_HandleTypeDef;

// Mock GPIO Port Definition
typedef struct {
    uint32_t MODER;   // Mock GPIO mode register
    uint32_t OTYPER;  // Mock GPIO output type register
    uint32_t OSPEEDR; // Mock GPIO output speed register
    uint32_t PUPDR;   // Mock GPIO pull-up/pull-down register
    uint32_t IDR;     // Mock GPIO input data register
    uint32_t ODR;     // Mock GPIO output data register
    uint32_t BSRR;    // Mock GPIO bit set/reset register
    uint32_t LCKR;    // Mock GPIO configuration lock register
    uint32_t AFR[2];  // Mock GPIO alternate function registers
} GPIO_TypeDef;

// Mock GPIO States
typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET } GPIO_PinState;

// Mock Pin definitions (commonly used)
#define GPIO_PIN_0 ((uint16_t)0x0001)
#define GPIO_PIN_1 ((uint16_t)0x0002)
#define GPIO_PIN_2 ((uint16_t)0x0004)
#define GPIO_PIN_3 ((uint16_t)0x0008)
#define GPIO_PIN_4 ((uint16_t)0x0010)
#define GPIO_PIN_5 ((uint16_t)0x0020)
#define GPIO_PIN_6 ((uint16_t)0x0040)
#define GPIO_PIN_7 ((uint16_t)0x0080)
#define GPIO_PIN_8 ((uint16_t)0x0100)
#define GPIO_PIN_9 ((uint16_t)0x0200)
#define GPIO_PIN_10 ((uint16_t)0x0400)
#define GPIO_PIN_11 ((uint16_t)0x0800)
#define GPIO_PIN_12 ((uint16_t)0x1000)
#define GPIO_PIN_13 ((uint16_t)0x2000)
#define GPIO_PIN_14 ((uint16_t)0x4000)
#define GPIO_PIN_15 ((uint16_t)0x8000)

// Common macro to prevent VDD_VALUE redefinition
#ifndef VDD_VALUE
#define VDD_VALUE 3300UL
#endif

#endif // STM32H7xx_HAL_H

#ifdef __cplusplus
}
#endif

#endif // MOCK_HAL_TYPES_H
