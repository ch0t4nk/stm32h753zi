/**
 * @file mock_hal.h
 * @brief Mock HAL header for unit testing STM32H7 safety systems
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 */

#ifndef MOCK_HAL_H
#define MOCK_HAL_H

#ifdef UNITY_TESTING
// Only use mock HAL during testing

#include <stdbool.h>
#include <stdint.h>
/* Ensure HAL abstraction types and enums are available for mock definitions */
#include "hal_abstraction/hal_abstraction.h"

// Prevent real HAL inclusion by defining HAL guard
#define __STM32H7xx_HAL_H
#define __STM32H7xx_HAL_DEF_H
#define __STM32H7xx_HAL_GPIO_H
#define __STM32H7xx_HAL_IWDG_H

// =============================================================================
// MOCK TYPE DEFINITIONS
// ==============================================================================

// Mock GPIO types
typedef struct {
    uint32_t Pin;
    uint32_t Mode;
    uint32_t Pull;
    uint32_t Speed;
    uint32_t Alternate;
} GPIO_InitTypeDef;

typedef struct {
    uint32_t dummy; // Placeholder for actual GPIO register structure
} GPIO_TypeDef;

typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET = 1 } GPIO_PinState;

// Mock SysTick
typedef struct {
    volatile uint32_t VAL; // Current Value Register
} SysTick_Type;

extern SysTick_Type *SysTick;

// Mock HAL status
typedef enum {
    HAL_OK = 0x00U,
    HAL_ERROR = 0x01U,
    HAL_BUSY = 0x02U,
    HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

// Mock IWDG handle
typedef struct {
    uint32_t dummy; // Placeholder for actual IWDG structure
} IWDG_HandleTypeDef;

// GPIO pin definitions for testing
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

// Mock GPIO port instances
extern GPIO_TypeDef *GPIOA;
extern GPIO_TypeDef *GPIOB;
extern GPIO_TypeDef *GPIOC;

// =============================================================================
// MOCK STATE TRACKING
// =============================================================================

/* Reuse canonical mock state from mock_hal_abstraction.h to ensure a single
 * authoritative layout is used across all test translation units. */
#ifndef MOCK_GPIO_INDEX_SPACE
#define MOCK_GPIO_INDEX_SPACE 32
#endif
#include "mock_hal_abstraction.h"

// =============================================================================
// MOCK HAL FUNCTION DECLARATIONS
// =============================================================================

// Mock initialization and control
void MockHAL_Init(void);
void MockHAL_Reset(void);
MockHAL_State_t *MockHAL_GetState(void);

// Mock GPIO functions
void HAL_GPIO_Init(GPIO_TypeDef *GPIOx, GPIO_InitTypeDef *GPIO_Init);
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                       GPIO_PinState PinState);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

// Mock timer functions
uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t Delay);

// Mock watchdog functions
HAL_StatusTypeDef HAL_IWDG_Refresh(IWDG_HandleTypeDef *hiwdg);

// Mock utility functions
void MockHAL_SetTick(uint32_t tick);
void MockHAL_AdvanceTick(uint32_t increment);
void MockHAL_SetEmergencyStopState(bool active);
bool MockHAL_GetEmergencyStopState(void);
void MockHAL_SetFaultPinState(bool active);
bool MockHAL_WasGPIOWritten(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                            GPIO_PinState expected_state);
uint32_t MockHAL_GetCallCount(void);
uint32_t MockHAL_GetWatchdogRefreshCount(void);
void MockHAL_PrintState(void);

#endif // UNITY_TESTING
#endif // MOCK_HAL_H
