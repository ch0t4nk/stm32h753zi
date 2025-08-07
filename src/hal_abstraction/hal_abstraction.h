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
/* Includes */
/* ==========================================================================
 */

#include "common/error_codes.h"
#include "config/hardware_config.h"
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 */
/* HAL Abstraction Types and Enums                                           */
/* ==========================================================================
 */

/**
 * @brief GPIO pin state enumeration
 */
typedef enum {
    HAL_GPIO_STATE_RESET = 0, ///< GPIO pin in low state
    HAL_GPIO_STATE_SET = 1    ///< GPIO pin in high state
} HAL_GPIO_State_t;

/**
 * @brief GPIO pin mode constants (platform-independent)
 */
#define HAL_GPIO_MODE_INPUT 0x00000000U
#define HAL_GPIO_MODE_OUTPUT_PP 0x00000001U
#define HAL_GPIO_MODE_OUTPUT_OD 0x00000011U
#define HAL_GPIO_MODE_AF_PP 0x00000002U
#define HAL_GPIO_MODE_AF_OD 0x00000012U
#define HAL_GPIO_MODE_ANALOG 0x00000003U
#define HAL_GPIO_MODE_IT_RISING 0x10110000U
#define HAL_GPIO_MODE_IT_FALLING 0x10210000U
#define HAL_GPIO_MODE_IT_RISING_FALLING 0x10310000U
#define HAL_GPIO_MODE_EVT_RISING 0x10120000U
#define HAL_GPIO_MODE_EVT_FALLING 0x10220000U
#define HAL_GPIO_MODE_EVT_RISING_FALLING 0x10320000U

/**
 * @brief GPIO pull-up/pull-down constants (platform-independent)
 */
#define HAL_GPIO_NOPULL 0x00000000U
#define HAL_GPIO_PULLUP 0x00000001U
#define HAL_GPIO_PULLDOWN 0x00000002U

/**
 * @brief GPIO speed constants (platform-independent)
 */
#define HAL_GPIO_SPEED_FREQ_LOW 0x00000000U
#define HAL_GPIO_SPEED_FREQ_MEDIUM 0x00000001U
#define HAL_GPIO_SPEED_FREQ_HIGH 0x00000002U
#define HAL_GPIO_SPEED_FREQ_VERY_HIGH 0x00000003U

/**
 * @brief GPIO pin configuration structure
 */
typedef struct {
    uint32_t pin;       ///< GPIO pin number
    uint32_t mode;      ///< GPIO mode (input, output, etc.)
    uint32_t pull;      ///< Pull-up/down configuration
    uint32_t speed;     ///< GPIO speed setting
    uint32_t alternate; ///< Alternate function selection
} HAL_GPIO_Config_t;

/**
 * @brief SPI transaction configuration
 */
typedef struct {
    uint8_t *tx_data;    ///< Transmit data buffer
    uint8_t *rx_data;    ///< Receive data buffer
    uint16_t data_size;  ///< Data size in bytes
    uint32_t timeout_ms; ///< Transaction timeout
} HAL_SPI_Transaction_t;

/**
 * @brief I2C transaction configuration
 */
typedef struct {
    uint16_t device_address;   ///< I2C device address
    uint16_t register_address; ///< Register address (if applicable)
    uint8_t *data;             ///< Data buffer
    uint16_t data_size;        ///< Data size in bytes
    uint32_t timeout_ms;       ///< Transaction timeout
    bool use_register_address; ///< Whether to use register addressing
} HAL_I2C_Transaction_t;

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

/* ==========================================================================
 */
/* GPIO Abstraction Interface                                                */
/* ==========================================================================
 */

/**
 * @brief Initialize GPIO pin with specified configuration
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

/**
 * @brief Toggle GPIO pin state
 * @param port GPIO port identifier
 * @param pin GPIO pin number
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_GPIO_Toggle(HAL_GPIO_Port_t port, uint32_t pin);

/**
 * @brief Enable GPIO interrupt for specified pin
 * @param port GPIO port identifier
 * @param pin GPIO pin number
 * @param trigger_type Interrupt trigger type
 * @param priority Interrupt priority
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_GPIO_EnableInterrupt(HAL_GPIO_Port_t port,
                                                   uint32_t pin,
                                                   uint32_t trigger_type,
                                                   uint8_t priority);

/* ==========================================================================
 */
/* SPI Abstraction Interface                                                 */
/* ==========================================================================
 */

/**
 * @brief Initialize SPI peripheral
 * @param instance SPI instance identifier
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_SPI_Init(HAL_SPI_Instance_t instance);

/**
 * @brief Perform SPI transmit/receive transaction
 * @param instance SPI instance identifier
 * @param transaction Transaction configuration
 * @return SystemError_t Success or error code
 */
SystemError_t
HAL_Abstraction_SPI_TransmitReceive(HAL_SPI_Instance_t instance,
                                    const HAL_SPI_Transaction_t *transaction);

/**
 * @brief Perform SPI transmit-only transaction
 * @param instance SPI instance identifier
 * @param tx_data Transmit data buffer
 * @param data_size Data size in bytes
 * @param timeout_ms Transaction timeout
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_SPI_Transmit(HAL_SPI_Instance_t instance,
                                           const uint8_t *tx_data,
                                           uint16_t data_size,
                                           uint32_t timeout_ms);

/**
 * @brief Perform SPI receive-only transaction
 * @param instance SPI instance identifier
 * @param rx_data Receive data buffer
 * @param data_size Data size in bytes
 * @param timeout_ms Transaction timeout
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_SPI_Receive(HAL_SPI_Instance_t instance,
                                          uint8_t *rx_data, uint16_t data_size,
                                          uint32_t timeout_ms);

/* ==========================================================================
 */
/* I2C Abstraction Interface                                                 */
/* ==========================================================================
 */

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
 * @brief Perform I2C transmit transaction
 * @param instance I2C instance identifier
 * @param device_address I2C device address
 * @param tx_data Transmit data buffer
 * @param data_size Data size in bytes
 * @param timeout_ms Transaction timeout
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_I2C_Transmit(HAL_I2C_Instance_t instance,
                                           uint16_t device_address,
                                           const uint8_t *tx_data,
                                           uint16_t data_size,
                                           uint32_t timeout_ms);

/**
 * @brief Perform I2C receive transaction
 * @param instance I2C instance identifier
 * @param device_address I2C device address
 * @param rx_data Receive data buffer
 * @param data_size Data size in bytes
 * @param timeout_ms Transaction timeout
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_I2C_Receive(HAL_I2C_Instance_t instance,
                                          uint16_t device_address,
                                          uint8_t *rx_data, uint16_t data_size,
                                          uint32_t timeout_ms);

/* ==========================================================================
 */
/* Timer Abstraction Interface                                               */
/* ==========================================================================
 */

/**
 * @brief Initialize timer peripheral
 * @param instance Timer instance identifier
 * @param config Timer configuration
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_Timer_Init(HAL_Timer_Instance_t instance,
                                         const HAL_Timer_Config_t *config);

/**
 * @brief Start timer
 * @param instance Timer instance identifier
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_Timer_Start(HAL_Timer_Instance_t instance);

/**
 * @brief Stop timer
 * @param instance Timer instance identifier
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_Timer_Stop(HAL_Timer_Instance_t instance);

/**
 * @brief Get timer counter value
 * @param instance Timer instance identifier
 * @param counter Pointer to store counter value
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_Timer_GetCounter(HAL_Timer_Instance_t instance,
                                               uint32_t *counter);

/* ==========================================================================
 */
/* System Timing Abstraction Interface                                       */
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

/* ==========================================================================
 */
/* Watchdog Abstraction Interface                                            */
/* ==========================================================================
 */

/**
 * @brief Initialize watchdog timer
 * @param timeout_ms Watchdog timeout in milliseconds
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_Watchdog_Init(uint32_t timeout_ms);

/**
 * @brief Refresh watchdog timer
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_Watchdog_Refresh(void);

/* ==========================================================================
 */
/* System Control Abstraction Interface                                      */
/* ==========================================================================
 */

/**
 * @brief Initialize hardware abstraction layer
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_Init(void);

/**
 * @brief Enable interrupts globally
 */
void HAL_Abstraction_EnableInterrupts(void);

/**
 * @brief Disable interrupts globally
 */
void HAL_Abstraction_DisableInterrupts(void);

/**
 * @brief Perform system reset
 */
void HAL_Abstraction_SystemReset(void);

/* ==========================================================================
 */
/* HAL Abstraction Configuration Functions                                   */
/* ==========================================================================
 */

/**
 * @brief Configure motor control specific hardware
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_ConfigureMotorControlHardware(void);

/**
 * @brief Configure safety system hardware
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_ConfigureSafetyHardware(void);

/**
 * @brief Configure communication interfaces
 * @return SystemError_t Success or error code
 */
SystemError_t HAL_Abstraction_ConfigureCommunicationHardware(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_ABSTRACTION_H */
