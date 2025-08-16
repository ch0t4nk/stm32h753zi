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

/* ==========================================================================
 */
/* HAL Abstraction Types and Enums                                           */
/* ==========================================================================
 */

/**
 * @brief Timer configuration structure
 */
typedef struct {
    uint32_t frequency_hz; ///< Timer frequency in Hz
    bool auto_reload;      ///< Auto-reload enable
    bool interrupt_enable; ///< Interrupt enable
    uint8_t priority;      ///< Interrupt priority
} HAL_Timer_Config_t;

/**
 * @brief Peripheral instance identifiers
 */
typedef enum {
    HAL_SPI_INSTANCE_1 = 0,
    HAL_SPI_INSTANCE_2,
    HAL_SPI_INSTANCE_3,
    HAL_SPI_INSTANCE_MAX
} HAL_SPI_Instance_t;

typedef enum {
    HAL_I2C_INSTANCE_1 = 0,
    HAL_I2C_INSTANCE_2,
    HAL_I2C_INSTANCE_3,
    HAL_I2C_INSTANCE_MAX
} HAL_I2C_Instance_t;

typedef enum {
    HAL_TIMER_INSTANCE_1 = 0,
    HAL_TIMER_INSTANCE_2,
    HAL_TIMER_INSTANCE_3,
    HAL_TIMER_INSTANCE_MAX
} HAL_Timer_Instance_t;

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

#ifdef __cplusplus
}
#endif

#endif /* HAL_ABSTRACTION_H */
