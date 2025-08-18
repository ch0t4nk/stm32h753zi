
/**
 * @file hal_abstraction.h
 * @brief Hardware Abstraction Layer (HAL) Interface for STM32H753ZI
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 *
 * This file defines a clean hardware abstraction interface that isolates
 * application code from direct STM32 HAL dependencies, enabling:
 * - Clean unit testing with mocks
 * - Portable code architecture
 * - Professional embedded design patterns
 * - Hardware-independent application logic
 *
 * @note Follows .github/instructions/hardware.instructions.md and
 *       .github/instructions/core-software.instructions.md patterns
 */

#ifndef HAL_ABSTRACTION_H
#define HAL_ABSTRACTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config/error_codes.h"
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 */
/* HAL Abstraction Types and Enums                                           */
/* ==========================================================================
 */

// SSOT: GPIO pin state abstraction
#ifndef HAL_GPIO_STATE_T_DEFINED
#define HAL_GPIO_STATE_T_DEFINED
typedef enum {
    HAL_GPIO_STATE_RESET = 0, ///< Pin logic low (reset)
    HAL_GPIO_STATE_SET = 1    ///< Pin logic high (set)
} HAL_GPIO_State_t;
#endif

/**
 * @brief I2C transaction configuration
 */
#ifndef HAL_I2C_TRANSACTION_T_DEFINED
#define HAL_I2C_TRANSACTION_T_DEFINED
typedef struct {
    uint16_t device_address;   ///< I2C device address
    uint16_t register_address; ///< Register address (if applicable)
    uint8_t *data;             ///< Data buffer
    uint16_t data_size;        ///< Data size in bytes
    uint32_t timeout_ms;       ///< Transaction timeout
    bool use_register_address; ///< Whether to use register addressing
} HAL_I2C_Transaction_t;
#endif

/**
 * @brief GPIO pin configuration structure
 */
#ifndef HAL_GPIO_CONFIG_T_DEFINED
#define HAL_GPIO_CONFIG_T_DEFINED
typedef struct {
    uint32_t pin;       ///< GPIO pin number
    uint32_t mode;      ///< GPIO mode (input, output, etc.)
    uint32_t pull;      ///< Pull-up/down configuration
    uint32_t speed;     ///< GPIO speed setting
    uint32_t alternate; ///< Alternate function selection
} HAL_GPIO_Config_t;
#endif

/**
 * @brief Timer configuration structure
 */
#ifndef HAL_TIMER_CONFIG_T_DEFINED
#define HAL_TIMER_CONFIG_T_DEFINED
typedef struct {
    uint32_t frequency_hz; ///< Timer frequency in Hz
    bool auto_reload;      ///< Auto-reload enable
    bool interrupt_enable; ///< Interrupt enable
    uint8_t priority;      ///< Interrupt priority
} HAL_Timer_Config_t;
#endif

/**
 * @brief Peripheral instance identifiers
 */
#ifndef HAL_SPI_INSTANCE_T_DEFINED
#define HAL_SPI_INSTANCE_T_DEFINED
typedef enum {
    HAL_SPI_INSTANCE_1 = 0,
    HAL_SPI_INSTANCE_2,
    HAL_SPI_INSTANCE_3,
    HAL_SPI_INSTANCE_MAX
} HAL_SPI_Instance_t;
#endif

#ifndef HAL_I2C_INSTANCE_T_DEFINED
#define HAL_I2C_INSTANCE_T_DEFINED
typedef enum {
    HAL_I2C_INSTANCE_1 = 0,
    HAL_I2C_INSTANCE_2,
    HAL_I2C_INSTANCE_3,
    HAL_I2C_INSTANCE_MAX
} HAL_I2C_Instance_t;
#endif

#ifndef HAL_TIMER_INSTANCE_T_DEFINED
#define HAL_TIMER_INSTANCE_T_DEFINED
typedef enum {
    HAL_TIMER_INSTANCE_1 = 0,
    HAL_TIMER_INSTANCE_2,
    HAL_TIMER_INSTANCE_3,
    HAL_TIMER_INSTANCE_MAX
} HAL_Timer_Instance_t;
#endif

#ifndef HAL_GPIO_PORT_T_DEFINED
#define HAL_GPIO_PORT_T_DEFINED
typedef enum {
    HAL_GPIO_PORT_A = 0,
    HAL_GPIO_PORT_B,
    HAL_GPIO_PORT_C,
    HAL_GPIO_PORT_D,
    HAL_GPIO_PORT_E,
    HAL_GPIO_PORT_F,
    HAL_GPIO_PORT_G,
    HAL_GPIO_PORT_H,
    HAL_GPIO_PORT_MAX
} HAL_GPIO_Port_t;
#endif

/* ==========================================================================
 */
/* HAL Abstraction Function Declarations */
/* ==========================================================================
 */

/**
 * @brief Get system tick count in milliseconds
 * @return uint32_t Current system tick count
 */
uint32_t HAL_Abstraction_GetTick(void);

/**
 * @brief Blocking delay in milliseconds
 * @param delay_ms Delay time in milliseconds
 */
void HAL_Abstraction_Delay(uint32_t delay_ms);

/**
 * @brief Get high-precision microsecond timestamp
 * @return uint32_t Microsecond timestamp
 */
uint32_t HAL_Abstraction_GetMicroseconds(void);

/**
 * @brief Initialize I2C peripheral
 * @param instance I2C instance identifier
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_I2C_Init(HAL_I2C_Instance_t instance);

/**
 * @brief Perform I2C memory write transaction
 * @param instance I2C instance identifier
 * @param transaction Transaction configuration
 * @return SystemError_t Success or error code
 */
SystemError_t
HAL_Abstraction_I2C_MemWrite(HAL_I2C_Instance_t instance,
                             const HAL_I2C_Transaction_t *transaction);

/**
 * @brief Perform I2C memory read transaction
 * @param instance I2C instance identifier
 * @param transaction Transaction configuration
 * @return SystemError_t Success or error code
 */
SystemError_t
HAL_Abstraction_I2C_MemRead(HAL_I2C_Instance_t instance,
                            const HAL_I2C_Transaction_t *transaction);

/**
 * @brief Initialize GPIO pin
 * @param port GPIO port identifier
 * @param config GPIO configuration
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_GPIO_Init(HAL_GPIO_Port_t port,
                                        const HAL_GPIO_Config_t *config);

/**
 * @brief Write value to GPIO pin
 * @param port GPIO port identifier
 * @param pin GPIO pin number
 * @param state Pin state to set
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_GPIO_Write(HAL_GPIO_Port_t port, uint32_t pin,
                                         HAL_GPIO_State_t state);

/**
 * @brief Read value from GPIO pin
 * @param port GPIO port identifier
 * @param pin GPIO pin number
 * @param state Pointer to store pin state
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_GPIO_Read(HAL_GPIO_Port_t port, uint32_t pin,
                                        HAL_GPIO_State_t *state);

#ifdef __cplusplus
}
#endif

#endif /* HAL_ABSTRACTION_H */
