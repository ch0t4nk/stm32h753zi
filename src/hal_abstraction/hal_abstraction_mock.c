/**
 * @file hal_abstraction_mock.c
 * @brief Mock Hardware Abstraction Layer Implementation for Testing
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 *
 * This file provides a mock implementation of the HAL abstraction interface
 * for unit testing, allowing tests to run without real hardware and providing
 * controllable behavior for comprehensive testing scenarios.
 *
 * @note Only included when in UNITY_TESTING mode
 */

#include "hal_abstraction.h"

#ifdef UNITY_TESTING
/* ==========================================================================
 */
/* Test Includes                                                             */
/* ==========================================================================
 */

#include <stdio.h>
#include <string.h>

/* ==========================================================================
 */
/* Mock State Structures                                                     */
/* ==========================================================================
 */

typedef struct {
    HAL_GPIO_State_t pin_states[16]; // Max 16 pins per port
    bool pin_configured[16];
    HAL_GPIO_Config_t pin_configs[16];
} MockGPIOPort_t;

typedef struct {
    uint8_t last_tx_data[256];
    uint8_t last_rx_data[256];
    uint16_t last_data_size;
    uint32_t call_count;
    SystemError_t return_value;
    bool initialized;
} MockSPI_t;

typedef struct {
    uint16_t last_device_address;
    uint16_t last_register_address;
    uint8_t last_data[256];
    uint16_t last_data_size;
    uint32_t call_count;
    SystemError_t return_value;
    bool initialized;
} MockI2C_t;

typedef struct {
    uint32_t counter_value;
    bool running;
    bool initialized;
    HAL_Timer_Config_t config;
} MockTimer_t;

typedef struct {
    MockGPIOPort_t gpio_ports[HAL_GPIO_PORT_MAX];
    MockSPI_t spi_instances[HAL_SPI_INSTANCE_MAX];
    MockI2C_t i2c_instances[HAL_I2C_INSTANCE_MAX];
    MockTimer_t timer_instances[HAL_TIMER_INSTANCE_MAX];

    uint32_t system_tick;
    uint32_t delay_call_count;
    uint32_t watchdog_refresh_count;
    bool interrupts_enabled;
    bool hal_initialized;

    // Fault injection
    bool inject_spi_failure;
    bool inject_i2c_failure;
    bool inject_gpio_failure;
    bool inject_timer_failure;
} MockHAL_State_t;

/* ==========================================================================
 */
/* Mock State Variable                                                       */
/* ==========================================================================
 */

static MockHAL_State_t mock_hal_state = {0};

/* ==========================================================================
 */
/* Mock Control Functions                                                    */
/* ==========================================================================
 */

/**
 * @brief Reset all mock state to default values
 */
void HAL_Abstraction_Mock_Reset(void) {
    memset(&mock_hal_state, 0, sizeof(mock_hal_state));
    mock_hal_state.interrupts_enabled = true;
}

/**
 * @brief Get pointer to mock state for test verification
 * @return MockHAL_State_t* Pointer to mock state
 */
MockHAL_State_t *HAL_Abstraction_Mock_GetState(void) {
    return &mock_hal_state;
}

/**
 * @brief Set mock SPI return value for next operation
 */
void HAL_Abstraction_Mock_SetSPIReturnValue(HAL_SPI_Instance_t instance,
                                            SystemError_t value) {
    if (instance < HAL_SPI_INSTANCE_MAX) {
        mock_hal_state.spi_instances[instance].return_value = value;
    }
}

/**
 * @brief Set mock I2C return value for next operation
 */
void HAL_Abstraction_Mock_SetI2CReturnValue(HAL_I2C_Instance_t instance,
                                            SystemError_t value) {
    if (instance < HAL_I2C_INSTANCE_MAX) {
        mock_hal_state.i2c_instances[instance].return_value = value;
    }
}

/**
 * @brief Inject SPI failure for testing error handling
 */
void HAL_Abstraction_Mock_InjectSPIFailure(bool enable) {
    mock_hal_state.inject_spi_failure = enable;
}

/**
 * @brief Inject I2C failure for testing error handling
 */
void HAL_Abstraction_Mock_InjectI2CFailure(bool enable) {
    mock_hal_state.inject_i2c_failure = enable;
}

/**
 * @brief Set GPIO pin state for testing input reads
 */
void HAL_Abstraction_Mock_SetGPIOState(HAL_GPIO_Port_t port, uint32_t pin,
                                       HAL_GPIO_State_t state) {
    if (port < HAL_GPIO_PORT_MAX && pin < 16) {
        // Find pin index (supports GPIO_PIN_0 to GPIO_PIN_15)
        uint8_t pin_index = 0;
        uint32_t pin_mask = pin;
        while (pin_mask >>= 1)
            pin_index++;

        if (pin_index < 16) {
            mock_hal_state.gpio_ports[port].pin_states[pin_index] = state;
        }
    }
}

/**
 * @brief Advance mock system tick for testing timing
 */
void HAL_Abstraction_Mock_AdvanceTick(uint32_t increment) {
    mock_hal_state.system_tick += increment;
}

/* ==========================================================================
 */
/* GPIO Mock Implementation                                                  */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_GPIO_Init(HAL_GPIO_Port_t port,
                                        const HAL_GPIO_Config_t *config) {
    if (config == NULL || port >= HAL_GPIO_PORT_MAX) {
        return ERROR_NULL_POINTER;
    }

    if (mock_hal_state.inject_gpio_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    // Find pin index
    uint8_t pin_index = 0;
    uint32_t pin_mask = config->pin;
    while (pin_mask >>= 1)
        pin_index++;

    if (pin_index < 16) {
        mock_hal_state.gpio_ports[port].pin_configured[pin_index] = true;
        mock_hal_state.gpio_ports[port].pin_configs[pin_index] = *config;
    }

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_Write(HAL_GPIO_Port_t port, uint32_t pin,
                                         HAL_GPIO_State_t state) {
    if (port >= HAL_GPIO_PORT_MAX) {
        return ERROR_INVALID_PARAMETER;
    }

    if (mock_hal_state.inject_gpio_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    // Find pin index
    uint8_t pin_index = 0;
    uint32_t pin_mask = pin;
    while (pin_mask >>= 1)
        pin_index++;

    if (pin_index < 16) {
        mock_hal_state.gpio_ports[port].pin_states[pin_index] = state;
    }

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_Read(HAL_GPIO_Port_t port, uint32_t pin,
                                        HAL_GPIO_State_t *state) {
    if (state == NULL || port >= HAL_GPIO_PORT_MAX) {
        return ERROR_NULL_POINTER;
    }

    if (mock_hal_state.inject_gpio_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    // Find pin index
    uint8_t pin_index = 0;
    uint32_t pin_mask = pin;
    while (pin_mask >>= 1)
        pin_index++;

    if (pin_index < 16) {
        *state = mock_hal_state.gpio_ports[port].pin_states[pin_index];
    } else {
        *state = HAL_GPIO_STATE_RESET;
    }

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_Toggle(HAL_GPIO_Port_t port, uint32_t pin) {
    if (port >= HAL_GPIO_PORT_MAX) {
        return ERROR_INVALID_PARAMETER;
    }

    if (mock_hal_state.inject_gpio_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    // Find pin index
    uint8_t pin_index = 0;
    uint32_t pin_mask = pin;
    while (pin_mask >>= 1)
        pin_index++;

    if (pin_index < 16) {
        HAL_GPIO_State_t current =
            mock_hal_state.gpio_ports[port].pin_states[pin_index];
        mock_hal_state.gpio_ports[port].pin_states[pin_index] =
            (current == HAL_GPIO_STATE_SET) ? HAL_GPIO_STATE_RESET
                                            : HAL_GPIO_STATE_SET;
    }

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_EnableInterrupt(HAL_GPIO_Port_t port,
                                                   uint32_t pin,
                                                   uint32_t trigger_type,
                                                   uint8_t priority) {
    if (port >= HAL_GPIO_PORT_MAX) {
        return ERROR_INVALID_PARAMETER;
    }

    if (mock_hal_state.inject_gpio_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    // Mock implementation just records that interrupt was configured
    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* SPI Mock Implementation                                                   */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_SPI_Init(HAL_SPI_Instance_t instance) {
    if (instance >= HAL_SPI_INSTANCE_MAX) {
        return ERROR_INVALID_PARAMETER;
    }

    mock_hal_state.spi_instances[instance].initialized = true;
    return SYSTEM_OK;
}

SystemError_t
HAL_Abstraction_SPI_TransmitReceive(HAL_SPI_Instance_t instance,
                                    const HAL_SPI_Transaction_t *transaction) {
    if (transaction == NULL || instance >= HAL_SPI_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }

    if (mock_hal_state.inject_spi_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    MockSPI_t *spi = &mock_hal_state.spi_instances[instance];

    // Record transaction details
    if (transaction->tx_data &&
        transaction->data_size <= sizeof(spi->last_tx_data)) {
        memcpy(spi->last_tx_data, transaction->tx_data,
               transaction->data_size);
    }

    if (transaction->rx_data &&
        transaction->data_size <= sizeof(spi->last_rx_data)) {
        // For testing, fill rx_data with test pattern
        for (uint16_t i = 0; i < transaction->data_size; i++) {
            transaction->rx_data[i] = 0xA5 + i; // Test pattern
        }
        memcpy(spi->last_rx_data, transaction->rx_data,
               transaction->data_size);
    }

    spi->last_data_size = transaction->data_size;
    spi->call_count++;

    return spi->return_value;
}

SystemError_t HAL_Abstraction_SPI_Transmit(HAL_SPI_Instance_t instance,
                                           const uint8_t *tx_data,
                                           uint16_t data_size,
                                           uint32_t timeout_ms) {
    if (tx_data == NULL || instance >= HAL_SPI_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }

    if (mock_hal_state.inject_spi_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    MockSPI_t *spi = &mock_hal_state.spi_instances[instance];

    if (data_size <= sizeof(spi->last_tx_data)) {
        memcpy(spi->last_tx_data, tx_data, data_size);
    }

    spi->last_data_size = data_size;
    spi->call_count++;

    return spi->return_value;
}

SystemError_t HAL_Abstraction_SPI_Receive(HAL_SPI_Instance_t instance,
                                          uint8_t *rx_data, uint16_t data_size,
                                          uint32_t timeout_ms) {
    if (rx_data == NULL || instance >= HAL_SPI_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }

    if (mock_hal_state.inject_spi_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    MockSPI_t *spi = &mock_hal_state.spi_instances[instance];

    // Fill with test pattern
    for (uint16_t i = 0; i < data_size; i++) {
        rx_data[i] = 0x5A + i;
    }

    if (data_size <= sizeof(spi->last_rx_data)) {
        memcpy(spi->last_rx_data, rx_data, data_size);
    }

    spi->last_data_size = data_size;
    spi->call_count++;

    return spi->return_value;
}

/* ==========================================================================
 */
/* I2C Mock Implementation                                                   */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_I2C_Init(HAL_I2C_Instance_t instance) {
    if (instance >= HAL_I2C_INSTANCE_MAX) {
        return ERROR_INVALID_PARAMETER;
    }

    mock_hal_state.i2c_instances[instance].initialized = true;
    return SYSTEM_OK;
}

SystemError_t
HAL_Abstraction_I2C_MemWrite(HAL_I2C_Instance_t instance,
                             const HAL_I2C_Transaction_t *transaction) {
    if (transaction == NULL || instance >= HAL_I2C_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }

    if (mock_hal_state.inject_i2c_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    MockI2C_t *i2c = &mock_hal_state.i2c_instances[instance];

    i2c->last_device_address = transaction->device_address;
    i2c->last_register_address = transaction->register_address;

    if (transaction->data &&
        transaction->data_size <= sizeof(i2c->last_data)) {
        memcpy(i2c->last_data, transaction->data, transaction->data_size);
    }

    i2c->last_data_size = transaction->data_size;
    i2c->call_count++;

    return i2c->return_value;
}

SystemError_t
HAL_Abstraction_I2C_MemRead(HAL_I2C_Instance_t instance,
                            const HAL_I2C_Transaction_t *transaction) {
    if (transaction == NULL || instance >= HAL_I2C_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }

    if (mock_hal_state.inject_i2c_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    MockI2C_t *i2c = &mock_hal_state.i2c_instances[instance];

    i2c->last_device_address = transaction->device_address;
    i2c->last_register_address = transaction->register_address;

    if (transaction->data &&
        transaction->data_size <= sizeof(i2c->last_data)) {
        // Fill with test pattern for AS5600 simulation
        if (transaction->device_address == 0x36) {       // AS5600 address
            if (transaction->register_address == 0x0C) { // RAW_ANGLE register
                transaction->data[0] = 0x12; // Mock angle high byte
                transaction->data[1] = 0x34; // Mock angle low byte
            } else if (transaction->register_address ==
                       0x0E) {               // ANGLE register
                transaction->data[0] = 0x56; // Mock angle high byte
                transaction->data[1] = 0x78; // Mock angle low byte
            }
        }
        memcpy(i2c->last_data, transaction->data, transaction->data_size);
    }

    i2c->last_data_size = transaction->data_size;
    i2c->call_count++;

    return i2c->return_value;
}

SystemError_t HAL_Abstraction_I2C_Transmit(HAL_I2C_Instance_t instance,
                                           uint16_t device_address,
                                           const uint8_t *tx_data,
                                           uint16_t data_size,
                                           uint32_t timeout_ms) {
    if (tx_data == NULL || instance >= HAL_I2C_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }

    if (mock_hal_state.inject_i2c_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    MockI2C_t *i2c = &mock_hal_state.i2c_instances[instance];

    i2c->last_device_address = device_address;

    if (data_size <= sizeof(i2c->last_data)) {
        memcpy(i2c->last_data, tx_data, data_size);
    }

    i2c->last_data_size = data_size;
    i2c->call_count++;

    return i2c->return_value;
}

SystemError_t HAL_Abstraction_I2C_Receive(HAL_I2C_Instance_t instance,
                                          uint16_t device_address,
                                          uint8_t *rx_data, uint16_t data_size,
                                          uint32_t timeout_ms) {
    if (rx_data == NULL || instance >= HAL_I2C_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }

    if (mock_hal_state.inject_i2c_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    MockI2C_t *i2c = &mock_hal_state.i2c_instances[instance];

    i2c->last_device_address = device_address;

    // Fill with test pattern
    for (uint16_t i = 0; i < data_size; i++) {
        rx_data[i] = 0xAB + i;
    }

    if (data_size <= sizeof(i2c->last_data)) {
        memcpy(i2c->last_data, rx_data, data_size);
    }

    i2c->last_data_size = data_size;
    i2c->call_count++;

    return i2c->return_value;
}

/* ==========================================================================
 */
/* Timer Mock Implementation                                                 */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_Timer_Init(HAL_Timer_Instance_t instance,
                                         const HAL_Timer_Config_t *config) {
    if (config == NULL || instance >= HAL_TIMER_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }

    if (mock_hal_state.inject_timer_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    MockTimer_t *timer = &mock_hal_state.timer_instances[instance];
    timer->initialized = true;
    timer->config = *config;
    timer->counter_value = 0;

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_Timer_Start(HAL_Timer_Instance_t instance) {
    if (instance >= HAL_TIMER_INSTANCE_MAX) {
        return ERROR_INVALID_PARAMETER;
    }

    if (mock_hal_state.inject_timer_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    MockTimer_t *timer = &mock_hal_state.timer_instances[instance];
    timer->running = true;

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_Timer_Stop(HAL_Timer_Instance_t instance) {
    if (instance >= HAL_TIMER_INSTANCE_MAX) {
        return ERROR_INVALID_PARAMETER;
    }

    MockTimer_t *timer = &mock_hal_state.timer_instances[instance];
    timer->running = false;

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_Timer_GetCounter(HAL_Timer_Instance_t instance,
                                               uint32_t *counter) {
    if (counter == NULL || instance >= HAL_TIMER_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }

    MockTimer_t *timer = &mock_hal_state.timer_instances[instance];
    *counter = timer->counter_value;

    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* System Timing Mock Implementation                                         */
/* ==========================================================================
 */

uint32_t HAL_Abstraction_GetTick(void) {
    return mock_hal_state.system_tick;
}

void HAL_Abstraction_Delay(uint32_t delay_ms) {
    mock_hal_state.delay_call_count++;
    mock_hal_state.system_tick += delay_ms; // Simulate time passage
}

uint32_t HAL_Abstraction_GetMicroseconds(void) {
    return mock_hal_state.system_tick * 1000; // Simple conversion for testing
}

/* ==========================================================================
 */
/* Watchdog Mock Implementation                                              */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_Watchdog_Init(uint32_t timeout_ms) {
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_Watchdog_Refresh(void) {
    mock_hal_state.watchdog_refresh_count++;
    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* System Control Mock Implementation                                        */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_Init(void) {
    mock_hal_state.hal_initialized = true;
    return SYSTEM_OK;
}

void HAL_Abstraction_EnableInterrupts(void) {
    mock_hal_state.interrupts_enabled = true;
}

void HAL_Abstraction_DisableInterrupts(void) {
    mock_hal_state.interrupts_enabled = false;
}

void HAL_Abstraction_SystemReset(void) {
    // For testing, just reset mock state
    HAL_Abstraction_Mock_Reset();
}

/* ==========================================================================
 */
/* Configuration Functions Mock Implementation                               */
/* ==========================================================================
 */

SystemError_t HAL_Abstraction_ConfigureMotorControlHardware(void) {
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_ConfigureSafetyHardware(void) {
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_ConfigureCommunicationHardware(void) {
    return SYSTEM_OK;
}

#endif /* UNITY_TESTING */
