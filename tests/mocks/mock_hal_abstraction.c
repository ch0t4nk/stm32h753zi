
/*
 * tests/mocks/mock_hal_abstraction.c
 *
 * Clean, single-definition mock HAL abstraction for host/unit tests.
 * Includes lightweight printf instrumentation to trace fault injection
 * and GPIO init calls when debugging failing host-tests.
 */

#include "mock_hal_abstraction.h"
#include "config/error_codes.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#include "hal_abstraction/hal_abstraction.h"

#include <math.h>
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

/* Test hook provided by test harness or archived compatibility layers. */
extern void Test_ResetApplication(void);

void HAL_Abstraction_Mock_Reset(void) {
    memset(&mock_hal_state, 0, sizeof(mock_hal_state));
    memset(&test_mock_state, 0, sizeof(test_mock_state));
    mock_hal_state.interrupts_enabled = true;
    test_mock_state.interrupts_enabled = true;
    mock_hal_state.system_tick = 1;
    test_mock_state.system_tick = 1;

    printf("[mock_hal] Calling Test_ResetApplication() from Mock Reset\n");
    fflush(stdout);
    Test_ResetApplication();
    printf("[mock_hal] Returned from Test_ResetApplication()\n");
    fflush(stdout);
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
        for (int idx = 0; idx < (int)MOCK_GPIO_INDEX_SPACE; idx++) {
            if (idx < 16) {
                test_mock_state.gpio_configured[port][idx] =
                    mock_hal_state.gpio_ports[port].pin_configured[idx];
                test_mock_state.gpio_states[port][idx] =
                    mock_hal_state.gpio_ports[port].pin_states[idx];
            } else {
                test_mock_state.gpio_configured[port][idx] = false;
                test_mock_state.gpio_states[port][idx] = HAL_GPIO_STATE_RESET;
            }
        }
    }

    printf(
        "[mock_hal] snapshot: ESTOP_BUTTON configured=%d, ESTOP_LED "
        "configured=%d, RELAY1 configured=%d, RELAY2 configured=%d\n",
        test_mock_state
            .gpio_configured[ESTOP_BUTTON_PORT][ESTOP_BUTTON_PIN_INDEX],
        test_mock_state.gpio_configured[ESTOP_LED_PORT][ESTOP_LED_PIN_INDEX],
        test_mock_state
            .gpio_configured[SAFETY_RELAY1_PORT][SAFETY_RELAY1_PIN_INDEX],
        test_mock_state
            .gpio_configured[SAFETY_RELAY2_PORT][SAFETY_RELAY2_PIN_INDEX]);
    fflush(stdout);

    for (int i = 0; i < HAL_SPI_INSTANCE_MAX; i++)
        test_mock_state.spi_transaction_count[i] =
            (uint32_t)mock_hal_state.spi_instances[i].call_count;
    for (int i = 0; i < HAL_I2C_INSTANCE_MAX; i++)
        test_mock_state.i2c_transaction_count[i] =
            (uint32_t)mock_hal_state.i2c_instances[i].call_count;

    return &test_mock_state;
}

void MockHAL_Reset(void) {
    HAL_Abstraction_Mock_Reset();
}
void MockHAL_Init(void) {
    HAL_Abstraction_Mock_Reset();
}
MockHAL_State_t *MockHAL_GetState(void) {
    return HAL_Abstraction_Mock_GetState();
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

/* HAL GPIO / Timer / SPI wrappers and implementations follow */

/* If CMSIS device headers are included (firmware build) these types and
 * symbols are already defined. Guard to avoid conflicting redeclarations
 * when compiling the firmware. The mocks are only used for host tests. */
#if !defined(GPIOA) && !defined(GPIO_TypeDef)
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
#endif

SystemError_t HAL_Abstraction_GPIO_Write(HAL_GPIO_Port_t port, uint32_t pin,
                                         HAL_GPIO_State_t state) {
    if (mock_hal_state.inject_gpio_failure)
        return ERROR_HARDWARE_FAULT;
    if (port >= HAL_GPIO_PORT_MAX)
        return ERROR_INVALID_PARAMETER;
    int pin_index = -1;
    if (pin < MOCK_GPIO_INDEX_SPACE)
        pin_index = (int)pin;
    else {
        uint32_t mask = pin;
        if ((mask != 0) && ((mask & (mask - 1UL)) == 0UL)) {
            int idx = 0;
            while ((mask & 1u) == 0u) {
                mask >>= 1;
                idx++;
                if (idx >= (int)MOCK_GPIO_INDEX_SPACE)
                    return ERROR_INVALID_PARAMETER;
            }
            pin_index = idx;
        } else
            return ERROR_INVALID_PARAMETER;
    }
    if (pin_index < 0 || pin_index >= 16)
        return ERROR_INVALID_PARAMETER;
    mock_hal_state.gpio_ports[port].pin_states[pin_index] = state;
    HAL_Abstraction_Mock_GetState();
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_Read(HAL_GPIO_Port_t port, uint32_t pin,
                                        HAL_GPIO_State_t *state) {
    if (state == NULL)
        return ERROR_NULL_POINTER;
    if (port >= HAL_GPIO_PORT_MAX)
        return ERROR_INVALID_PARAMETER;
    if (mock_hal_state.inject_gpio_failure)
        return ERROR_HARDWARE_FAULT;
    int pin_index = -1;
    if (pin < MOCK_GPIO_INDEX_SPACE)
        pin_index = (int)pin;
    else {
        uint32_t mask = pin;
        if ((mask != 0) && ((mask & (mask - 1UL)) == 0UL)) {
            int idx = 0;
            while ((mask & 1u) == 0u) {
                mask >>= 1;
                idx++;
                if (idx >= (int)MOCK_GPIO_INDEX_SPACE)
                    return ERROR_INVALID_PARAMETER;
            }
            pin_index = idx;
        } else
            return ERROR_INVALID_PARAMETER;
    }
    if (pin_index < 0 || pin_index >= 16)
        return ERROR_INVALID_PARAMETER;
    *state = mock_hal_state.gpio_ports[port].pin_states[pin_index];
    HAL_Abstraction_Mock_GetState();
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_Init(HAL_GPIO_Port_t port,
                                        const HAL_GPIO_Config_t *config) {
    if (config == NULL)
        return ERROR_NULL_POINTER;
    printf("[mock_hal] HAL_Abstraction_GPIO_Init ENTRY: "
           "inject_gpio_failure=%d port=%d pin=%u\n",
           (int)mock_hal_state.inject_gpio_failure, (int)port,
           (unsigned)config->pin);
    fflush(stdout);
    if (mock_hal_state.inject_gpio_failure)
        return ERROR_HARDWARE_FAULT;
    if (port >= HAL_GPIO_PORT_MAX)
        return ERROR_INVALID_PARAMETER;
    uint32_t pin = config->pin;
    if (pin == 0)
        return ERROR_INVALID_PARAMETER;
    int pin_index = -1;
    if (pin < MOCK_GPIO_INDEX_SPACE)
        pin_index = (int)pin;
    else {
        uint32_t mask = pin;
        if ((mask != 0) && ((mask & (mask - 1UL)) == 0UL)) {
            int idx = 0;
            while ((mask & 1u) == 0u) {
                mask >>= 1;
                idx++;
                if (idx >= (int)MOCK_GPIO_INDEX_SPACE)
                    return ERROR_INVALID_PARAMETER;
            }
            pin_index = idx;
        } else
            return ERROR_INVALID_PARAMETER;
    }
    if (pin_index < 0 || pin_index >= 16)
        return ERROR_INVALID_PARAMETER;
    mock_hal_state.gpio_ports[port].pin_configured[pin_index] = true;
    mock_hal_state.gpio_ports[port].pin_configs[pin_index] = *config;
    HAL_Abstraction_Mock_GetState();
    return SYSTEM_OK;
}

void MockHAL_InjectFault(uint32_t fault_type, bool enable) {
    if (fault_type & MOCK_FAULT_GPIO_INIT) {
        mock_hal_state.inject_gpio_failure = enable;
        printf(
            "[mock_hal] MockHAL_InjectFault: inject_gpio_failure set -> %d\n",
            (int)mock_hal_state.inject_gpio_failure);
        fflush(stdout);
    }
    if (fault_type & MOCK_FAULT_GPIO_WRITE)
        mock_hal_state.inject_gpio_failure = enable;
    if (fault_type & MOCK_FAULT_SPI_INIT)
        mock_hal_state.inject_spi_failure = enable;
    if (fault_type & MOCK_FAULT_I2C_INIT)
        mock_hal_state.inject_i2c_failure = enable;
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
    if (transaction->tx_data &&
        transaction->data_size <= sizeof(spi->last_tx_data))
        memcpy(spi->last_tx_data, transaction->tx_data,
               transaction->data_size);
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

#endif // UNITY_TESTING

/* Helper implementations required by unit tests and other mocks */
void HAL_Abstraction_Mock_SetGPIOState(HAL_GPIO_Port_t port, uint32_t pin,
                                       HAL_GPIO_State_t state) {
    if (port >= HAL_GPIO_PORT_MAX)
        return;
    int pin_index = -1;
    if (pin < MOCK_GPIO_INDEX_SPACE)
        pin_index = (int)pin;
    else {
        uint32_t mask = pin;
        if ((mask != 0) && ((mask & (mask - 1UL)) == 0UL)) {
            int idx = 0;
            while ((mask & 1u) == 0u) {
                mask >>= 1;
                idx++;
                if (idx >= (int)MOCK_GPIO_INDEX_SPACE)
                    return;
            }
            pin_index = idx;
        } else
            return;
    }
    if (pin_index < 0 || pin_index >= 16)
        return;
    mock_hal_state.gpio_ports[port].pin_states[pin_index] = state;
    mock_hal_state.gpio_ports[port].pin_configured[pin_index] = true;
}

void MockHAL_SetGPIOState(HAL_GPIO_Port_t port, uint32_t pin,
                          HAL_GPIO_State_t state) {
    HAL_Abstraction_Mock_SetGPIOState(port, pin, state);
}

uint32_t HAL_Abstraction_GetTick(void) {
    return mock_hal_state.system_tick;
}
void HAL_Abstraction_Delay(uint32_t delay_ms) {
    mock_hal_state.delay_call_count++;
    mock_hal_state.system_tick += delay_ms;
}
uint32_t HAL_Abstraction_GetMicroseconds(void) {
    return mock_hal_state.system_tick * 1000u;
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
