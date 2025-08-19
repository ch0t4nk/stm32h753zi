/*
 * Clean mock HAL abstraction implementation for host/unit tests.
 * This file is included only under UNITY_TESTING.
 */

#include "mock_hal_abstraction.h"
#include "config/error_codes.h"
#include "config/motor_config.h"
#include "hal_abstraction/hal_abstraction.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef UNITY_TESTING

/* Internal state used by mocks */
MockHAL_Internal_State_t mock_hal_state = {0};
static MockHAL_State_t test_mock_state = {0};

static int mock_pin_to_index(uint32_t pin_mask) {
    if (pin_mask == 0)
        return -1;
    int idx = 0;
    while ((pin_mask & 1u) == 0u) {
        pin_mask >>= 1;
        idx++;
        if (idx > 15)
            return -1;
    }
    return idx;
}

void MockHAL_SetEstopActive(bool active) {
    mock_hal_state.estop_active = active;
}
void MockHAL_SetFaultPinActive(bool active) {
    mock_hal_state.fault_pin_active = active;
}
void MockHAL_TriggerWatchdogTimeout(bool trigger) {
    mock_hal_state.watchdog_timeout_triggered = trigger;
}

void HAL_Abstraction_Mock_Reset(void) {
    memset(&mock_hal_state, 0, sizeof(mock_hal_state));
    memset(&test_mock_state, 0, sizeof(test_mock_state));
    mock_hal_state.interrupts_enabled = true;
    test_mock_state.interrupts_enabled = true;
    mock_hal_state.system_tick = 1;
    test_mock_state.system_tick = 1;
}

static MockHAL_State_t *HAL_Abstraction_Mock_GetState(void) {
    test_mock_state.system_tick = mock_hal_state.system_tick;
    test_mock_state.delay_call_count = mock_hal_state.delay_call_count;
    test_mock_state.watchdog_refresh_count =
        mock_hal_state.watchdog_refresh_count;
    test_mock_state.interrupts_enabled = mock_hal_state.interrupts_enabled;
    test_mock_state.hal_initialized = mock_hal_state.hal_initialized;
    test_mock_state.inject_spi_failure = mock_hal_state.inject_spi_failure;
    test_mock_state.inject_i2c_failure = mock_hal_state.inject_i2c_failure;
    test_mock_state.inject_gpio_failure = mock_hal_state.inject_gpio_failure;
    test_mock_state.inject_timer_failure = mock_hal_state.inject_timer_failure;

    for (int port = 0; port < HAL_GPIO_PORT_MAX; port++) {
        for (int idx = 0; idx < 16; idx++) {
            test_mock_state.gpio_configured[port][idx] =
                mock_hal_state.gpio_ports[port].pin_configured[idx];
            test_mock_state.gpio_states[port][idx] =
                mock_hal_state.gpio_ports[port].pin_states[idx];
        }
        for (int bit = 0; bit < 16; bit++) {
            uint32_t mask_index = (1u << bit);
            if (mask_index < MOCK_GPIO_INDEX_SPACE) {
                test_mock_state.gpio_configured[port][mask_index] =
                    mock_hal_state.gpio_ports[port].pin_configured[bit];
                test_mock_state.gpio_states[port][mask_index] =
                    mock_hal_state.gpio_ports[port].pin_states[bit];
            }
        }
    }

    for (int i = 0; i < HAL_SPI_INSTANCE_MAX; i++)
        test_mock_state.spi_transaction_count[i] =
            (uint32_t)mock_hal_state.spi_instances[i].call_count;
    for (int i = 0; i < HAL_I2C_INSTANCE_MAX; i++)
        test_mock_state.i2c_transaction_count[i] =
            (uint32_t)mock_hal_state.i2c_instances[i].call_count;

    return &test_mock_state;
}

void HAL_Abstraction_Mock_SetI2CReturnValue(HAL_I2C_Instance_t instance,
                                            SystemError_t value) {
    if (instance < HAL_I2C_INSTANCE_MAX)
        mock_hal_state.i2c_instances[instance].return_value = value;
}

void MockHAL_SetSPIResponse(HAL_SPI_Instance_t instance, const uint8_t *data,
                            uint16_t size) {
    if (instance >= HAL_SPI_INSTANCE_MAX || data == NULL || size == 0)
        return;
    MockSPI_Internal_t *spi = &mock_hal_state.spi_instances[instance];
    size_t copy_size = (size <= sizeof(spi->response_data))
                           ? size
                           : sizeof(spi->response_data);
    memcpy(spi->response_data, data, copy_size);
    spi->response_size = (uint16_t)copy_size;
    spi->response_set = true;
}

void MockHAL_SetI2CResponse(HAL_I2C_Instance_t instance, const uint8_t *data,
                            uint16_t size) {
    if (instance >= HAL_I2C_INSTANCE_MAX || data == NULL || size == 0)
        return;
    MockI2C_Internal_t *i2c = &mock_hal_state.i2c_instances[instance];
    size_t copy_size = (size <= sizeof(i2c->response_data))
                           ? size
                           : sizeof(i2c->response_data);
    memcpy(i2c->response_data, data, copy_size);
    i2c->response_size = (uint16_t)copy_size;
    i2c->response_set = true;
}

SystemError_t HAL_Abstraction_I2C_Init(HAL_I2C_Instance_t instance) {
    if (instance >= HAL_I2C_INSTANCE_MAX)
        return ERROR_INVALID_PARAMETER;
    mock_hal_state.i2c_instances[instance].initialized = true;
    return SYSTEM_OK;
}

SystemError_t
HAL_Abstraction_I2C_MemWrite(HAL_I2C_Instance_t instance,
                             const HAL_I2C_Transaction_t *transaction) {
    if (transaction == NULL || instance >= HAL_I2C_INSTANCE_MAX)
        return ERROR_NULL_POINTER;
    if (mock_hal_state.inject_i2c_failure)
        return ERROR_HARDWARE_FAULT;
    MockI2C_Internal_t *i2c = &mock_hal_state.i2c_instances[instance];
    i2c->last_device_address = transaction->device_address;
    i2c->last_register_address = transaction->register_address;
    if (transaction->data && transaction->data_size <= sizeof(i2c->last_data))
        memcpy(i2c->last_data, transaction->data, transaction->data_size);
    i2c->last_data_size = transaction->data_size;
    i2c->call_count++;
    return i2c->return_value;
}

SystemError_t
HAL_Abstraction_I2C_MemRead(HAL_I2C_Instance_t instance,
                            const HAL_I2C_Transaction_t *transaction) {
    if (transaction == NULL || instance >= HAL_I2C_INSTANCE_MAX)
        return ERROR_NULL_POINTER;
    if (mock_hal_state.inject_i2c_failure)
        return ERROR_HARDWARE_FAULT;
    MockI2C_Internal_t *i2c = &mock_hal_state.i2c_instances[instance];
    i2c->last_device_address = transaction->device_address;
    i2c->last_register_address = transaction->register_address;
    if (transaction->data &&
        transaction->data_size <= sizeof(i2c->last_data)) {
        if (i2c->response_set &&
            i2c->response_size >= transaction->data_size) {
            memcpy(transaction->data, i2c->response_data,
                   transaction->data_size);
            memcpy(i2c->last_data, i2c->response_data, transaction->data_size);
            i2c->response_set = false;
        } else {
            if (transaction->device_address == 0x36) {
                if (transaction->register_address == 0x0C) {
                    transaction->data[0] = 0x12;
                    transaction->data[1] = 0x34;
                } else if (transaction->register_address == 0x0E) {
                    transaction->data[0] = 0x56;
                    transaction->data[1] = 0x78;
                } else
                    for (uint16_t i = 0; i < transaction->data_size; i++)
                        transaction->data[i] = 0xC5 + i;
            }
            memcpy(i2c->last_data, transaction->data, transaction->data_size);
        }
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
    if (tx_data == NULL || instance >= HAL_I2C_INSTANCE_MAX)
        return ERROR_NULL_POINTER;
    if (mock_hal_state.inject_i2c_failure)
        return ERROR_HARDWARE_FAULT;
    MockI2C_Internal_t *i2c = &mock_hal_state.i2c_instances[instance];
    i2c->last_device_address = device_address;
    if (data_size <= sizeof(i2c->last_data))
        memcpy(i2c->last_data, tx_data, data_size);
    i2c->last_data_size = data_size;
    i2c->call_count++;
    return i2c->return_value;
}

SystemError_t HAL_Abstraction_I2C_Receive(HAL_I2C_Instance_t instance,
                                          uint16_t device_address,
                                          uint8_t *rx_data, uint16_t data_size,
                                          uint32_t timeout_ms) {
    if (rx_data == NULL || instance >= HAL_I2C_INSTANCE_MAX)
        return ERROR_NULL_POINTER;
    if (mock_hal_state.inject_i2c_failure)
        return ERROR_HARDWARE_FAULT;
    MockI2C_Internal_t *i2c = &mock_hal_state.i2c_instances[instance];
    i2c->last_device_address = device_address;
    if (i2c->response_set && i2c->response_size >= data_size) {
        memcpy(rx_data, i2c->response_data, data_size);
        memcpy(i2c->last_data, i2c->response_data, data_size);
        i2c->response_set = false;
    } else {
        for (uint16_t i = 0; i < data_size; i++)
            rx_data[i] = 0xAB + i;
        if (data_size <= sizeof(i2c->last_data))
            memcpy(i2c->last_data, rx_data, data_size);
    }
    i2c->last_data_size = data_size;
    i2c->call_count++;
    return i2c->return_value;
}

SystemError_t
HAL_Abstraction_I2C_TransmitReceive(HAL_I2C_Instance_t instance,
                                    const HAL_I2C_Transaction_t *transaction) {
    if (transaction == NULL || instance >= HAL_I2C_INSTANCE_MAX)
        return ERROR_NULL_POINTER;
    if (mock_hal_state.inject_i2c_failure)
        return ERROR_HARDWARE_FAULT;
    MockI2C_Internal_t *i2c = &mock_hal_state.i2c_instances[instance];
    i2c->last_device_address = transaction->device_address;
    i2c->last_register_address = transaction->register_address;
    if (transaction->data &&
        transaction->data_size <= sizeof(i2c->last_data)) {
        if (transaction->device_address == 0x36) {
            if (transaction->register_address == 0x0C) {
                transaction->data[0] = 0x12;
                transaction->data[1] = 0x34;
            } else if (transaction->register_address == 0x0E) {
                transaction->data[0] = 0x56;
                transaction->data[1] = 0x78;
            } else
                for (uint16_t i = 0; i < transaction->data_size; i++)
                    transaction->data[i] = 0xC5 + i;
        }
        memcpy(i2c->last_data, transaction->data, transaction->data_size);
    }
    i2c->last_data_size = transaction->data_size;
    i2c->call_count++;
    return i2c->return_value;
}

/* ---------------------------------------------------------------------------
 * Legacy-to-mock mapping wrappers. These adapt the HAL_Abstraction_* API
 * expected by archived/legacy code to the existing mock_hal functions
 * implemented in tests/mocks/mock_hal.c (HAL_GPIO_WritePin/HAL_GPIO_ReadPin).
 * ---------------------------------------------------------------------------
 */

/* Minimal forward declarations for mock HAL symbols used by wrappers.
 * We avoid including tests/mocks/mock_hal.h here to prevent conflicting
 * typedefs with mock_hal_abstraction.h which is already included by tests.
 */
typedef struct {
    uint32_t dummy;
} GPIO_TypeDef;
typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET = 1 } GPIO_PinState;
extern GPIO_TypeDef *GPIOA;
extern GPIO_TypeDef *GPIOB;
extern GPIO_TypeDef *GPIOC;
void HAL_GPIO_WritePin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                       GPIO_PinState PinState);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

SystemError_t HAL_Abstraction_GPIO_Write(HAL_GPIO_Port_t port, uint32_t pin,
                                         HAL_GPIO_State_t state) {
    GPIO_TypeDef *gpio = GPIOA;
    switch (port) {
    case HAL_GPIO_PORT_A:
        gpio = GPIOA;
        break;
    case HAL_GPIO_PORT_B:
        gpio = GPIOB;
        break;
    case HAL_GPIO_PORT_C:
        gpio = GPIOC;
    default:
        break;
    }
    uint16_t pin_mask = (uint16_t)(pin & 0xFFFFu);
    GPIO_PinState ps =
        (state == HAL_GPIO_STATE_SET) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(gpio, pin_mask, ps);
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_Read(HAL_GPIO_Port_t port, uint32_t pin,
                                        HAL_GPIO_State_t *state) {
    if (state == NULL)
        return ERROR_NULL_POINTER;
    GPIO_TypeDef *gpio = GPIOA;
    switch (port) {
    case HAL_GPIO_PORT_A:
        gpio = GPIOA;
        break;
    case HAL_GPIO_PORT_B:
        gpio = GPIOB;
        break;
    case HAL_GPIO_PORT_C:
        gpio = GPIOC;
    default:
        break;
    }
    uint16_t pin_mask = (uint16_t)(pin & 0xFFFFu);
    GPIO_PinState ps = HAL_GPIO_ReadPin(gpio, pin_mask);
    *state = (ps == GPIO_PIN_SET) ? HAL_GPIO_STATE_SET : HAL_GPIO_STATE_RESET;
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_Init(HAL_GPIO_Port_t port,
                                        const HAL_GPIO_Config_t *config) {
    if (config == NULL)
        return ERROR_NULL_POINTER;
    /* In the mock, simply mark the pin as configured in mock_hal_state */
    if (port >= HAL_GPIO_PORT_MAX)
        return ERROR_INVALID_PARAMETER;
    uint32_t pin = config->pin;
    if (pin == 0)
        return ERROR_INVALID_PARAMETER;
    uint8_t pin_index = 0;
    uint32_t mask = pin;
    while (mask >>= 1)
        pin_index++;
    if (pin_index < 16) {
        mock_hal_state.gpio_ports[port].pin_configured[pin_index] = true;
        mock_hal_state.gpio_ports[port].pin_configs[pin_index] = *config;
        return SYSTEM_OK;
    }
    return ERROR_INVALID_PARAMETER;
}

SystemError_t HAL_Abstraction_GPIO_EnableInterrupt(HAL_GPIO_Port_t port,
                                                   uint32_t pin,
                                                   uint32_t trigger_type,
                                                   uint32_t priority) {
    (void)port;
    (void)pin;
    (void)trigger_type;
    (void)priority;
    return SYSTEM_OK;
}

void MockHAL_SetGPIOState(HAL_GPIO_Port_t port, uint32_t pin,
                          HAL_GPIO_State_t state) {
    /* Delegate to HAL_Abstraction_Mock_SetGPIOState which manipulates
     * mock_hal_state directly. */
    HAL_Abstraction_Mock_SetGPIOState(port, pin, state);
}

SystemError_t HAL_Abstraction_Timer_Init(HAL_Timer_Instance_t instance,
                                         const HAL_Timer_Config_t *config) {
    if (config == NULL || instance >= HAL_TIMER_INSTANCE_MAX)
        return ERROR_NULL_POINTER;
    if (mock_hal_state.inject_timer_failure)
        return ERROR_HARDWARE_FAULT;
    MockTimer_Internal_t *timer = &mock_hal_state.timer_instances[instance];
    timer->initialized = true;
    timer->config = *config;
    timer->counter_value = 0;
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_Timer_Start(HAL_Timer_Instance_t instance) {
    if (instance >= HAL_TIMER_INSTANCE_MAX)
        return ERROR_INVALID_PARAMETER;
    if (mock_hal_state.inject_timer_failure)
        return ERROR_HARDWARE_FAULT;
    MockTimer_Internal_t *timer = &mock_hal_state.timer_instances[instance];
    timer->running = true;
    /* Ensure timer counter is non-zero on start so telemetry timestamps are
     * meaningful in host tests. Use system_tick as a base (ms) and convert
     * to microseconds for the timer counter. */
    if (timer->counter_value == 0) {
        timer->counter_value = mock_hal_state.system_tick * 1000u;
        if (timer->counter_value == 0)
            timer->counter_value = 1000u; /* 1 ms default */
    }
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_Timer_Stop(HAL_Timer_Instance_t instance) {
    if (instance >= HAL_TIMER_INSTANCE_MAX)
        return ERROR_INVALID_PARAMETER;
    MockTimer_Internal_t *timer = &mock_hal_state.timer_instances[instance];
    timer->running = false;
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_Timer_GetCounter(HAL_Timer_Instance_t instance,
                                               uint32_t *counter) {
    if (counter == NULL || instance >= HAL_TIMER_INSTANCE_MAX)
        return ERROR_NULL_POINTER;
    MockTimer_Internal_t *timer = &mock_hal_state.timer_instances[instance];
    *counter = timer->counter_value;
    return SYSTEM_OK;
}

uint32_t HAL_Abstraction_GetTick(void) {
    return mock_hal_state.system_tick;
}
void HAL_Abstraction_Delay(uint32_t delay_ms) {
    mock_hal_state.delay_call_count++;
    mock_hal_state.system_tick += delay_ms;
}
uint32_t HAL_Abstraction_GetMicroseconds(void) {
    return mock_hal_state.system_tick * 1000;
}

SystemError_t HAL_Abstraction_Watchdog_Init(uint32_t timeout_ms) {
    return SYSTEM_OK;
}
SystemError_t HAL_Abstraction_Watchdog_Refresh(void) {
    mock_hal_state.watchdog_refresh_count++;
    return SYSTEM_OK;
}

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
    HAL_Abstraction_Mock_Reset();
}

SystemError_t HAL_Abstraction_ConfigureMotorControlHardware(void) {
    return SYSTEM_OK;
}
SystemError_t HAL_Abstraction_ConfigureSafetyHardware(void) {
    return SYSTEM_OK;
}
SystemError_t HAL_Abstraction_ConfigureCommunicationHardware(void) {
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_AS5600_Init(uint8_t encoder_id) {
    if (encoder_id >= 2)
        return ERROR_INVALID_PARAMETER;
    return SYSTEM_OK;
}
SystemError_t HAL_Abstraction_AS5600_ReadAngle(uint8_t encoder_id,
                                               float *angle_degrees) {
    if (encoder_id >= 2 || angle_degrees == NULL)
        return ERROR_INVALID_PARAMETER;
    /* If a mock I2C response has been programmed, consume it and
     * convert the two-byte little-endian raw angle value into degrees.
     * Tests and legacy code in this repo use a 14-bit representation where
     * raw range [0..16383] maps to [0..360) degrees. If no response is
     * present, default to 0.0f which is a safe idle value and avoids
     * false safety violations when commanded_position is not set by the
     * test harness. */
    MockI2C_Internal_t *i2c = &mock_hal_state.i2c_instances[encoder_id];
    if (i2c->response_set && i2c->response_size >= 2) {
        uint16_t raw = (uint16_t)i2c->response_data[0] |
                       ((uint16_t)i2c->response_data[1] << 8);
        i2c->response_set = false;    /* consume */
        const float scale = 16384.0f; /* 14-bit mapping */
        *angle_degrees = (raw / scale) * 360.0f;
    } else {
        *angle_degrees = 0.0f;
    }
    return SYSTEM_OK;
}
SystemError_t HAL_Abstraction_AS5600_CheckMagnet(uint8_t encoder_id,
                                                 bool *magnet_detected) {
    if (encoder_id >= 2 || magnet_detected == NULL)
        return ERROR_INVALID_PARAMETER;
    *magnet_detected = true;
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_L6470_Init(uint8_t motor_id) {
    if (motor_id >= 2)
        return ERROR_INVALID_PARAMETER;
    return SYSTEM_OK;
}
SystemError_t HAL_Abstraction_L6470_GetStatus(uint8_t motor_id,
                                              uint32_t *status) {
    if (motor_id >= 2 || status == NULL)
        return ERROR_INVALID_PARAMETER;
    /* If a mock SPI response has been programmed for this SPI instance,
     * consume up to 3 bytes and form the 24-bit L6470 status register.
     * Otherwise, return a sensible default (0x7E83) used in tests. */
    MockSPI_Internal_t *spi = &mock_hal_state.spi_instances[motor_id];
    if (spi->response_set && spi->response_size >= 3) {
        uint32_t b0 = (uint32_t)spi->response_data[0];
        uint32_t b1 = (uint32_t)spi->response_data[1];
        uint32_t b2 = (uint32_t)spi->response_data[2];
        *status = (b0) | (b1 << 8) | (b2 << 16);
        spi->response_set = false; /* consume */
    } else {
        /* Default to zero (no thermal/stall/overcurrent bits) to avoid
         * false-positive safety warnings in host tests that don't set a
         * specific SPI response. */
        *status = 0x0000;
    }
    return SYSTEM_OK;
}
SystemError_t HAL_Abstraction_L6470_GetParameter(uint8_t motor_id,
                                                 uint8_t param,
                                                 uint32_t *value) {
    if (motor_id >= 2 || value == NULL)
        return ERROR_INVALID_PARAMETER;
    switch (param) {
    case 0x09:
        *value = 1000 + (motor_id * 500);
        break;
    case 0x11:
        *value = 0x7E83;
        break;
    default:
        *value = 0;
        break;
    }
    return SYSTEM_OK;
}
SystemError_t HAL_Abstraction_L6470_HardStop(uint8_t motor_id) {
    if (motor_id >= 2)
        return ERROR_INVALID_PARAMETER;
    return SYSTEM_OK;
}

void MockHAL_InjectFault(uint32_t fault_type, bool enable) {
    if (fault_type & MOCK_FAULT_GPIO_INIT)
        mock_hal_state.inject_gpio_failure = enable;
    if (fault_type & MOCK_FAULT_GPIO_WRITE)
        mock_hal_state.inject_gpio_failure = enable;
    if (fault_type & MOCK_FAULT_SPI_INIT)
        mock_hal_state.inject_spi_failure = enable;
    if (fault_type & MOCK_FAULT_I2C_INIT)
        mock_hal_state.inject_i2c_failure = enable;
}

/*** Compatibility wrappers and additional helpers expected by legacy tests
 * ***/

/* Legacy wrappers MockHAL_Reset / MockHAL_GetState / MockHAL_SetTick are
 * provided by the primary mock implementation (tests/mocks/mock_hal.c).
 * Do not redefine them here to avoid duplicate symbol link errors.
 */

/* Provide HAL_Abstraction_Mock_SetGPIOState implementation (non-duplicated)
 * and let MockHAL_* names be resolved from the main mock file. */

void HAL_Abstraction_Mock_SetGPIOState(HAL_GPIO_Port_t port, uint32_t pin,
                                       HAL_GPIO_State_t state) {
    if (port < HAL_GPIO_PORT_MAX) {
        /* accept either bitmask or index */
        if (pin < 16) {
            /* direct index */
            mock_hal_state.gpio_ports[port].pin_states[pin] = state;
        } else {
            /* treat as mask */
            uint8_t pin_index = 0;
            uint32_t mask = pin;
            while (mask >>= 1)
                pin_index++;
            if (pin_index < 16)
                mock_hal_state.gpio_ports[port].pin_states[pin_index] = state;
        }
    }
}

/* SPI transmit/receive minimal mock to satisfy legacy callers */
SystemError_t
HAL_Abstraction_SPI_TransmitReceive(HAL_SPI_Instance_t instance,
                                    const HAL_SPI_Transaction_t *transaction) {
    if (transaction == NULL || instance >= HAL_SPI_INSTANCE_MAX)
        return ERROR_NULL_POINTER;
    if (mock_hal_state.inject_spi_failure)
        return ERROR_HARDWARE_FAULT;

    MockSPI_Internal_t *spi = &mock_hal_state.spi_instances[instance];

    /* Record tx data if provided */
    if (transaction->tx_data &&
        transaction->data_size <= sizeof(spi->last_tx_data)) {
        memcpy(spi->last_tx_data, transaction->tx_data,
               transaction->data_size);
    }

    /* Provide programmable response if set, otherwise simple test pattern */
    if (transaction->rx_data &&
        transaction->data_size <= sizeof(spi->last_rx_data)) {
        if (spi->response_set &&
            spi->response_size >= transaction->data_size) {
            memcpy(transaction->rx_data, spi->response_data,
                   transaction->data_size);
            spi->response_set = false;
        } else {
            for (uint16_t i = 0; i < transaction->data_size; i++)
                transaction->rx_data[i] = (uint8_t)(0xA0 + (i & 0xFF));
            memcpy(spi->last_rx_data, transaction->rx_data,
                   transaction->data_size);
        }
    }

    spi->last_data_size = transaction->data_size;
    spi->call_count++;
    return spi->return_value;
}

SystemError_t HAL_Abstraction_SPI_Init(HAL_SPI_Instance_t instance) {
    if (instance >= HAL_SPI_INSTANCE_MAX)
        return ERROR_INVALID_PARAMETER;
    mock_hal_state.spi_instances[instance].initialized = true;
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_SPI_Transmit(HAL_SPI_Instance_t instance,
                                           const uint8_t *tx_data,
                                           uint16_t data_size,
                                           uint32_t timeout_ms) {
    if (tx_data == NULL || instance >= HAL_SPI_INSTANCE_MAX)
        return ERROR_NULL_POINTER;
    if (mock_hal_state.inject_spi_failure)
        return ERROR_HARDWARE_FAULT;
    MockSPI_Internal_t *spi = &mock_hal_state.spi_instances[instance];
    if (data_size <= sizeof(spi->last_tx_data))
        memcpy(spi->last_tx_data, tx_data, data_size);
    spi->last_data_size = data_size;
    spi->call_count++;
    return spi->return_value;
}

SystemError_t HAL_Abstraction_SPI_Receive(HAL_SPI_Instance_t instance,
                                          uint8_t *rx_data, uint16_t data_size,
                                          uint32_t timeout_ms) {
    if (rx_data == NULL || instance >= HAL_SPI_INSTANCE_MAX)
        return ERROR_NULL_POINTER;
    if (mock_hal_state.inject_spi_failure)
        return ERROR_HARDWARE_FAULT;
    MockSPI_Internal_t *spi = &mock_hal_state.spi_instances[instance];
    if (spi->response_set && spi->response_size >= data_size) {
        memcpy(rx_data, spi->response_data, data_size);
        spi->response_set = false;
    } else {
        for (uint16_t i = 0; i < data_size; i++)
            rx_data[i] = (uint8_t)(0xB0 + (i & 0xFF));
        if (data_size <= sizeof(spi->last_rx_data))
            memcpy(spi->last_rx_data, rx_data, data_size);
    }
    spi->last_data_size = data_size;
    spi->call_count++;
    return spi->return_value;
}

#endif // UNITY_TESTING
