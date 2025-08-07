/**
 * @file hal_abstraction_stm32h7.c
 * @brief STM32H7 Hardware Abstraction Layer Implementation
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 *
 * This file implements the HAL abstraction interface for STM32H753ZI,
 * providing hardware-specific implementations while maintaining the
 * abstract interface for application code.
 *
 * @note Only included when not in UNITY_TESTING mode
 */

#include "hal_abstraction.h"

#ifndef UNITY_TESTING
/* ==========================================================================
 */
/* STM32H7 HAL Includes                                                      */
/* ==========================================================================
 */

#include "config/hardware_config.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_iwdg.h"
#include "stm32h7xx_hal_tim.h"
#include <string.h>

/* ==========================================================================
 */
/* Private Variables                                                         */
/* ==========================================================================
 */

// External handles from main.c or peripheral initialization
extern SPI_HandleTypeDef hspi2;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern IWDG_HandleTypeDef hiwdg;

// Internal state tracking
static bool hal_abstraction_initialized = false;

/* ==========================================================================
 */
/* Private Helper Functions                                                  */
/* ==========================================================================
 */

/**
 * @brief Convert abstraction GPIO port to STM32 GPIO port
 */
static GPIO_TypeDef *get_stm32_gpio_port(HAL_GPIO_Port_t port) {
    switch (port) {
    case HAL_GPIO_PORT_A:
        return GPIOA;
    case HAL_GPIO_PORT_B:
        return GPIOB;
    case HAL_GPIO_PORT_C:
        return GPIOC;
    case HAL_GPIO_PORT_D:
        return GPIOD;
    case HAL_GPIO_PORT_E:
        return GPIOE;
    case HAL_GPIO_PORT_F:
        return GPIOF;
    case HAL_GPIO_PORT_G:
        return GPIOG;
    case HAL_GPIO_PORT_H:
        return GPIOH;
    default:
        return NULL;
    }
}

/**
 * @brief Convert abstraction SPI instance to STM32 SPI handle
 */
static SPI_HandleTypeDef *get_stm32_spi_handle(HAL_SPI_Instance_t instance) {
    switch (instance) {
    case HAL_SPI_INSTANCE_2:
        return &hspi2; // L6470 motor drivers
    default:
        return NULL;
    }
}

/**
 * @brief Convert abstraction I2C instance to STM32 I2C handle
 */
static I2C_HandleTypeDef *get_stm32_i2c_handle(HAL_I2C_Instance_t instance) {
    switch (instance) {
    case HAL_I2C_INSTANCE_1:
        return &hi2c1; // AS5600 encoder 1
    case HAL_I2C_INSTANCE_2:
        return &hi2c2; // AS5600 encoder 2
    default:
        return NULL;
    }
}

/**
 * @brief Convert abstraction timer instance to STM32 timer handle
 */
static TIM_HandleTypeDef *
get_stm32_timer_handle(HAL_Timer_Instance_t instance) {
    switch (instance) {
    case HAL_TIMER_INSTANCE_2:
        return &htim2; // Control loop timer
    case HAL_TIMER_INSTANCE_3:
        return &htim3; // General purpose timer
    default:
        return NULL;
    }
}

/**
 * @brief Convert HAL status to SystemError_t
 */
static SystemError_t convert_hal_status(HAL_StatusTypeDef hal_status) {
    switch (hal_status) {
    case HAL_OK:
        return SYSTEM_OK;
    case HAL_ERROR:
        return ERROR_HARDWARE_FAILURE;
    case HAL_BUSY:
        return ERROR_BUSY;
    case HAL_TIMEOUT:
        return ERROR_TIMEOUT;
    default:
        return ERROR_UNKNOWN;
    }
}

/* ==========================================================================
 */
/* GPIO Abstraction Implementation                                           */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_GPIO_Init(HAL_GPIO_Port_t port,
                                        const HAL_GPIO_Config_t *config) {
    if (config == NULL) {
        return ERROR_NULL_POINTER;
    }

    GPIO_TypeDef *gpio_port = get_stm32_gpio_port(port);
    if (gpio_port == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Enable GPIO clock
    switch (port) {
    case HAL_GPIO_PORT_A:
        __HAL_RCC_GPIOA_CLK_ENABLE();
        break;
    case HAL_GPIO_PORT_B:
        __HAL_RCC_GPIOB_CLK_ENABLE();
        break;
    case HAL_GPIO_PORT_C:
        __HAL_RCC_GPIOC_CLK_ENABLE();
        break;
    case HAL_GPIO_PORT_D:
        __HAL_RCC_GPIOD_CLK_ENABLE();
        break;
    case HAL_GPIO_PORT_E:
        __HAL_RCC_GPIOE_CLK_ENABLE();
        break;
    case HAL_GPIO_PORT_F:
        __HAL_RCC_GPIOF_CLK_ENABLE();
        break;
    case HAL_GPIO_PORT_G:
        __HAL_RCC_GPIOG_CLK_ENABLE();
        break;
    case HAL_GPIO_PORT_H:
        __HAL_RCC_GPIOH_CLK_ENABLE();
        break;
    default:
        return ERROR_INVALID_PARAMETER;
    }

    // Configure GPIO
    GPIO_InitTypeDef gpio_init = {0};
    gpio_init.Pin = config->pin;
    gpio_init.Mode = config->mode;
    gpio_init.Pull = config->pull;
    gpio_init.Speed = config->speed;
    gpio_init.Alternate = config->alternate;

    HAL_GPIO_Init(gpio_port, &gpio_init);

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_Write(HAL_GPIO_Port_t port, uint32_t pin,
                                         HAL_GPIO_State_t state) {
    GPIO_TypeDef *gpio_port = get_stm32_gpio_port(port);
    if (gpio_port == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    GPIO_PinState pin_state =
        (state == HAL_GPIO_STATE_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(gpio_port, pin, pin_state);

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_Read(HAL_GPIO_Port_t port, uint32_t pin,
                                        HAL_GPIO_State_t *state) {
    if (state == NULL) {
        return ERROR_NULL_POINTER;
    }

    GPIO_TypeDef *gpio_port = get_stm32_gpio_port(port);
    if (gpio_port == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    GPIO_PinState pin_state = HAL_GPIO_ReadPin(gpio_port, pin);
    *state = (pin_state == GPIO_PIN_SET) ? HAL_GPIO_STATE_SET
                                         : HAL_GPIO_STATE_RESET;

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_Toggle(HAL_GPIO_Port_t port, uint32_t pin) {
    GPIO_TypeDef *gpio_port = get_stm32_gpio_port(port);
    if (gpio_port == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    HAL_GPIO_TogglePin(gpio_port, pin);

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_EnableInterrupt(HAL_GPIO_Port_t port,
                                                   uint32_t pin,
                                                   uint32_t trigger_type,
                                                   uint8_t priority) {
    GPIO_TypeDef *gpio_port = get_stm32_gpio_port(port);
    if (gpio_port == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Configure GPIO for interrupt
    GPIO_InitTypeDef gpio_init = {0};
    gpio_init.Pin = pin;
    gpio_init.Mode = trigger_type;
    gpio_init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(gpio_port, &gpio_init);

    // Configure NVIC
    IRQn_Type irqn;
    if (pin <= GPIO_PIN_4) {
        irqn = (IRQn_Type)(EXTI0_IRQn + __builtin_ctz(pin));
    } else if (pin <= GPIO_PIN_9) {
        irqn = EXTI9_5_IRQn;
    } else {
        irqn = EXTI15_10_IRQn;
    }

    HAL_NVIC_SetPriority(irqn, priority, 0);
    HAL_NVIC_EnableIRQ(irqn);

    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* SPI Abstraction Implementation                                            */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_SPI_Init(HAL_SPI_Instance_t instance) {
    SPI_HandleTypeDef *hspi = get_stm32_spi_handle(instance);
    if (hspi == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // SPI should be initialized by CubeMX/main.c
    // This function validates that it's ready
    if (hspi->Instance == NULL) {
        return ERROR_NOT_INITIALIZED;
    }

    return SYSTEM_OK;
}

SystemError_t
HAL_Abstraction_SPI_TransmitReceive(HAL_SPI_Instance_t instance,
                                    const HAL_SPI_Transaction_t *transaction) {
    if (transaction == NULL) {
        return ERROR_NULL_POINTER;
    }

    SPI_HandleTypeDef *hspi = get_stm32_spi_handle(instance);
    if (hspi == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(
        hspi, transaction->tx_data, transaction->rx_data,
        transaction->data_size, transaction->timeout_ms);

    return convert_hal_status(status);
}

SystemError_t HAL_Abstraction_SPI_Transmit(HAL_SPI_Instance_t instance,
                                           const uint8_t *tx_data,
                                           uint16_t data_size,
                                           uint32_t timeout_ms) {
    if (tx_data == NULL) {
        return ERROR_NULL_POINTER;
    }

    SPI_HandleTypeDef *hspi = get_stm32_spi_handle(instance);
    if (hspi == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    HAL_StatusTypeDef status =
        HAL_SPI_Transmit(hspi, (uint8_t *)tx_data, data_size, timeout_ms);

    return convert_hal_status(status);
}

SystemError_t HAL_Abstraction_SPI_Receive(HAL_SPI_Instance_t instance,
                                          uint8_t *rx_data, uint16_t data_size,
                                          uint32_t timeout_ms) {
    if (rx_data == NULL) {
        return ERROR_NULL_POINTER;
    }

    SPI_HandleTypeDef *hspi = get_stm32_spi_handle(instance);
    if (hspi == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    HAL_StatusTypeDef status =
        HAL_SPI_Receive(hspi, rx_data, data_size, timeout_ms);

    return convert_hal_status(status);
}

/* ==========================================================================
 */
/* I2C Abstraction Implementation                                            */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_I2C_Init(HAL_I2C_Instance_t instance) {
    I2C_HandleTypeDef *hi2c = get_stm32_i2c_handle(instance);
    if (hi2c == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // I2C should be initialized by CubeMX/main.c
    // This function validates that it's ready
    if (hi2c->Instance == NULL) {
        return ERROR_NOT_INITIALIZED;
    }

    return SYSTEM_OK;
}

SystemError_t
HAL_Abstraction_I2C_MemWrite(HAL_I2C_Instance_t instance,
                             const HAL_I2C_Transaction_t *transaction) {
    if (transaction == NULL || transaction->data == NULL) {
        return ERROR_NULL_POINTER;
    }

    I2C_HandleTypeDef *hi2c = get_stm32_i2c_handle(instance);
    if (hi2c == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    HAL_StatusTypeDef status;
    if (transaction->use_register_address) {
        status =
            HAL_I2C_Mem_Write(hi2c, transaction->device_address << 1,
                              transaction->register_address,
                              I2C_MEMADD_SIZE_8BIT, transaction->data,
                              transaction->data_size, transaction->timeout_ms);
    } else {
        status = HAL_I2C_Master_Transmit(
            hi2c, transaction->device_address << 1, transaction->data,
            transaction->data_size, transaction->timeout_ms);
    }

    return convert_hal_status(status);
}

SystemError_t
HAL_Abstraction_I2C_MemRead(HAL_I2C_Instance_t instance,
                            const HAL_I2C_Transaction_t *transaction) {
    if (transaction == NULL || transaction->data == NULL) {
        return ERROR_NULL_POINTER;
    }

    I2C_HandleTypeDef *hi2c = get_stm32_i2c_handle(instance);
    if (hi2c == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    HAL_StatusTypeDef status;
    if (transaction->use_register_address) {
        status =
            HAL_I2C_Mem_Read(hi2c, transaction->device_address << 1,
                             transaction->register_address,
                             I2C_MEMADD_SIZE_8BIT, transaction->data,
                             transaction->data_size, transaction->timeout_ms);
    } else {
        status = HAL_I2C_Master_Receive(
            hi2c, transaction->device_address << 1, transaction->data,
            transaction->data_size, transaction->timeout_ms);
    }

    return convert_hal_status(status);
}

SystemError_t HAL_Abstraction_I2C_Transmit(HAL_I2C_Instance_t instance,
                                           uint16_t device_address,
                                           const uint8_t *tx_data,
                                           uint16_t data_size,
                                           uint32_t timeout_ms) {
    if (tx_data == NULL) {
        return ERROR_NULL_POINTER;
    }

    I2C_HandleTypeDef *hi2c = get_stm32_i2c_handle(instance);
    if (hi2c == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        hi2c, device_address << 1, (uint8_t *)tx_data, data_size, timeout_ms);

    return convert_hal_status(status);
}

SystemError_t HAL_Abstraction_I2C_Receive(HAL_I2C_Instance_t instance,
                                          uint16_t device_address,
                                          uint8_t *rx_data, uint16_t data_size,
                                          uint32_t timeout_ms) {
    if (rx_data == NULL) {
        return ERROR_NULL_POINTER;
    }

    I2C_HandleTypeDef *hi2c = get_stm32_i2c_handle(instance);
    if (hi2c == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(
        hi2c, device_address << 1, rx_data, data_size, timeout_ms);

    return convert_hal_status(status);
}

/* ==========================================================================
 */
/* Timer Abstraction Implementation                                          */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_Timer_Init(HAL_Timer_Instance_t instance,
                                         const HAL_Timer_Config_t *config) {
    if (config == NULL) {
        return ERROR_NULL_POINTER;
    }

    TIM_HandleTypeDef *htim = get_stm32_timer_handle(instance);
    if (htim == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Timer should be initialized by CubeMX/main.c
    // This function validates configuration
    if (htim->Instance == NULL) {
        return ERROR_NOT_INITIALIZED;
    }

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_Timer_Start(HAL_Timer_Instance_t instance) {
    TIM_HandleTypeDef *htim = get_stm32_timer_handle(instance);
    if (htim == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    HAL_StatusTypeDef status = HAL_TIM_Base_Start_IT(htim);
    return convert_hal_status(status);
}

SystemError_t HAL_Abstraction_Timer_Stop(HAL_Timer_Instance_t instance) {
    TIM_HandleTypeDef *htim = get_stm32_timer_handle(instance);
    if (htim == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    HAL_StatusTypeDef status = HAL_TIM_Base_Stop_IT(htim);
    return convert_hal_status(status);
}

SystemError_t HAL_Abstraction_Timer_GetCounter(HAL_Timer_Instance_t instance,
                                               uint32_t *counter) {
    if (counter == NULL) {
        return ERROR_NULL_POINTER;
    }

    TIM_HandleTypeDef *htim = get_stm32_timer_handle(instance);
    if (htim == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *counter = __HAL_TIM_GET_COUNTER(htim);
    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* System Timing Abstraction Implementation                                  */
/* ==========================================================================
 */

uint32_t HAL_Abstraction_GetTick(void) {
    return HAL_GetTick();
}

void HAL_Abstraction_Delay(uint32_t delay_ms) {
    HAL_Delay(delay_ms);
}

uint32_t HAL_Abstraction_GetMicroseconds(void) {
    // Use SysTick for microsecond timing
    uint32_t tick = HAL_GetTick();
    uint32_t systick_val = SysTick->VAL;
    uint32_t systick_load = SysTick->LOAD;

    // Calculate microseconds within current millisecond
    uint32_t us_in_tick = ((systick_load - systick_val) * 1000) / systick_load;

    return (tick * 1000) + us_in_tick;
}

/* ==========================================================================
 */
/* Watchdog Abstraction Implementation                                       */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_Watchdog_Init(uint32_t timeout_ms) {
    // Watchdog should be initialized by CubeMX/main.c
    // This function validates that it's ready
    if (hiwdg.Instance == NULL) {
        return ERROR_NOT_INITIALIZED;
    }

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_Watchdog_Refresh(void) {
    HAL_StatusTypeDef status = HAL_IWDG_Refresh(&hiwdg);
    return convert_hal_status(status);
}

/* ==========================================================================
 */
/* System Control Abstraction Implementation                                 */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_Init(void) {
    if (hal_abstraction_initialized) {
        return SYSTEM_OK;
    }

    // Validate that required peripherals are initialized
    SystemError_t result;

    result = HAL_Abstraction_SPI_Init(HAL_SPI_INSTANCE_2);
    if (result != SYSTEM_OK) {
        return result;
    }

    result = HAL_Abstraction_I2C_Init(HAL_I2C_INSTANCE_1);
    if (result != SYSTEM_OK) {
        return result;
    }

    result = HAL_Abstraction_I2C_Init(HAL_I2C_INSTANCE_2);
    if (result != SYSTEM_OK) {
        return result;
    }

    hal_abstraction_initialized = true;
    return SYSTEM_OK;
}

void HAL_Abstraction_EnableInterrupts(void) {
    __enable_irq();
}

void HAL_Abstraction_DisableInterrupts(void) {
    __disable_irq();
}

void HAL_Abstraction_SystemReset(void) {
    HAL_NVIC_SystemReset();
}

/* ==========================================================================
 */
/* Configuration Functions Implementation */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_ConfigureMotorControlHardware(void) {
    SystemError_t result;

    // Configure L6470 SPI CS pin
    HAL_GPIO_Config_t cs_config = {.pin = MOTOR_SPI_CS_PIN,
                                   .mode = GPIO_MODE_OUTPUT_PP,
                                   .pull = GPIO_NOPULL,
                                   .speed = GPIO_SPEED_FREQ_HIGH,
                                   .alternate = 0};
    result = HAL_Abstraction_GPIO_Init(HAL_GPIO_PORT_A, &cs_config);
    if (result != SYSTEM_OK)
        return result;

    // Set CS high (inactive)
    result = HAL_Abstraction_GPIO_Write(HAL_GPIO_PORT_A, MOTOR_SPI_CS_PIN,
                                        HAL_GPIO_STATE_SET);
    if (result != SYSTEM_OK)
        return result;

    // Configure L6470 FLAG pin (input with pull-up)
    HAL_GPIO_Config_t flag_config = {.pin = MOTOR_FLAG_PIN,
                                     .mode = GPIO_MODE_INPUT,
                                     .pull = GPIO_PULLUP,
                                     .speed = GPIO_SPEED_FREQ_LOW,
                                     .alternate = 0};
    result = HAL_Abstraction_GPIO_Init(HAL_GPIO_PORT_A, &flag_config);
    if (result != SYSTEM_OK)
        return result;

    // Configure L6470 BUSY pin (input with pull-up)
    HAL_GPIO_Config_t busy_config = {.pin = MOTOR_BUSY_PIN,
                                     .mode = GPIO_MODE_INPUT,
                                     .pull = GPIO_PULLUP,
                                     .speed = GPIO_SPEED_FREQ_LOW,
                                     .alternate = 0};
    result = HAL_Abstraction_GPIO_Init(HAL_GPIO_PORT_A, &busy_config);

    return result;
}

SystemError_t HAL_Abstraction_ConfigureSafetyHardware(void) {
    SystemError_t result;

    // Configure emergency stop button (input with pull-up)
    HAL_GPIO_Config_t estop_config = {.pin = USER_BUTTON_PIN,
                                      .mode = GPIO_MODE_IT_FALLING,
                                      .pull = GPIO_PULLUP,
                                      .speed = GPIO_SPEED_FREQ_LOW,
                                      .alternate = 0};
    result = HAL_Abstraction_GPIO_Init(HAL_GPIO_PORT_C, &estop_config);
    if (result != SYSTEM_OK)
        return result;

    // Enable emergency stop interrupt
    result = HAL_Abstraction_GPIO_EnableInterrupt(
        HAL_GPIO_PORT_C, USER_BUTTON_PIN, GPIO_MODE_IT_FALLING,
        0); // Highest priority
    if (result != SYSTEM_OK)
        return result;

    // Configure status LEDs
    HAL_GPIO_Config_t led_config = {.pin = LED_GREEN_PIN,
                                    .mode = GPIO_MODE_OUTPUT_PP,
                                    .pull = GPIO_NOPULL,
                                    .speed = GPIO_SPEED_FREQ_LOW,
                                    .alternate = 0};
    result = HAL_Abstraction_GPIO_Init(HAL_GPIO_PORT_B, &led_config);
    if (result != SYSTEM_OK)
        return result;

    led_config.pin = LED_RED_PIN;
    result = HAL_Abstraction_GPIO_Init(HAL_GPIO_PORT_B, &led_config);

    return result;
}

SystemError_t HAL_Abstraction_ConfigureCommunicationHardware(void) {
    // Communication interfaces (SPI, I2C) are configured by CubeMX
    // This function validates they are ready for use
    SystemError_t result;

    result = HAL_Abstraction_SPI_Init(HAL_SPI_INSTANCE_2);
    if (result != SYSTEM_OK)
        return result;

    result = HAL_Abstraction_I2C_Init(HAL_I2C_INSTANCE_1);
    if (result != SYSTEM_OK)
        return result;

    result = HAL_Abstraction_I2C_Init(HAL_I2C_INSTANCE_2);

    return result;
}

#endif /* !UNITY_TESTING */
