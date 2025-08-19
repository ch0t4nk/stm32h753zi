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

// Signal that HAL status type was provided by mock types to avoid
// redefinition in other mock headers
#define MOCK_HAL_STATUS_DEFINED

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

// Minimal watchdog handle types for host testing (expanded to match
// expectations from safety/watchdog code)
typedef struct {
    void *Instance;
    struct {
        uint32_t Prescaler;
        uint32_t Reload;
        uint32_t Window;
    } Init;
    uint32_t State;
} IWDG_HandleTypeDef;

typedef struct {
    void *Instance;
    struct {
        uint32_t Prescaler;
        uint32_t Window;
        uint32_t Counter;
        uint32_t EWIMode;
    } Init;
    uint32_t State;
} WWDG_HandleTypeDef;

// Mock peripheral instance tokens for unit tests
#define IWDG1 ((void *)0x1)
#define WWDG1 ((void *)0x2)

// WWDG related macros expected by watchdog_manager
#define WWDG_EWI_ENABLE 1
#define WWDG_IT_EWI 0x01
#define RESET 0
#define WWDG_PRESCALER_8 8

// Helper macros for test environment (no-op implementations)
#define __HAL_RCC_WWDG1_CLK_ENABLE() ((void)0)
#define __HAL_WWDG_GET_IT_SOURCE(h, it) (RESET)
#define __HAL_WWDG_CLEAR_IT(h, it) ((void)0)

// IWDG related macros expected by watchdog_manager. Guard to avoid
// redefinition with SSOT safety_config.h which may already define them.
#ifndef IWDG_PRESCALER_32
#define IWDG_PRESCALER_32 32
#endif

#ifndef IWDG_WINDOW_DISABLE
#define IWDG_WINDOW_DISABLE 0
#endif

#ifndef IWDG_PR_PR_Pos
#define IWDG_PR_PR_Pos 0
#endif

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
