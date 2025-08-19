

#include "mock_hal_abstraction.h" // For MockHAL_State_t, etc.
#include "config/error_codes.h"   // For SystemError_t, SYSTEM_OK
#include "config/motor_config.h"  // For MAX_MOTORS and motor constants
#include "hal_abstraction/hal_abstraction.h" // For HAL_GPIO_Config_t, etc.
#include <stdbool.h>
#include <stdint.h>

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
#include "mock_hal_abstraction.h"

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
/* Internal Mock State Structures (Implementation Details)                  */
/* ==========================================================================
 */

// --- Programmable Safety Event Simulation API ---
void MockHAL_SetEstopActive(bool active) {
    mock_hal_state.estop_active = active;
}

void MockHAL_SetFaultPinActive(bool active) {
    mock_hal_state.fault_pin_active = active;
}

void MockHAL_TriggerWatchdogTimeout(bool trigger) {
    mock_hal_state.watchdog_timeout_triggered = trigger;
}

/* ==========================================================================
 */
/* Mock State Variable                                                       */
/* ==========================================================================
 */

MockHAL_Internal_State_t mock_hal_state = {0};

/* ==========================================================================
 */
/* Test-Compatible State Structure (Forward Interface to Tests)             */
/* ==========================================================================
 */

// Use the public MockHAL_State_t from the header so tests see correct sizes
static MockHAL_State_t test_mock_state = {0};

// Helper: convert bitmask pin to index [0..15]; returns -1 if invalid
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
    memset(&test_mock_state, 0, sizeof(test_mock_state));
    mock_hal_state.interrupts_enabled = true;
    test_mock_state.interrupts_enabled = true;
    // Initialize system tick to non-zero to avoid zero-edge cases in tests
    mock_hal_state.system_tick = 1;
    test_mock_state.system_tick = 1;
}

/**
 * @brief Get pointer to test-compatible mock state for test verification
 * @return MockHAL_State_t* Pointer to test-compatible mock state
 */
static MockHAL_State_t *HAL_Abstraction_Mock_GetState(void) {
    // Synchronize state between internal and test-compatible structures
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

    // Copy GPIO states and mirror to bitmask indices
    for (int port = 0; port < HAL_GPIO_PORT_MAX; port++) {
        // copy per-pin configured/state for 16 pins
        for (int idx = 0; idx < 16; idx++) {
            test_mock_state.gpio_configured[port][idx] =
                mock_hal_state.gpio_ports[port].pin_configured[idx];
            test_mock_state.gpio_states[port][idx] =
                mock_hal_state.gpio_ports[port].pin_states[idx];
        }
        // Also mirror into bitmask-style indices if tests use those
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

    // Copy peripheral transaction counts
    for (int i = 0; i < HAL_SPI_INSTANCE_MAX; i++) {
        test_mock_state.spi_transaction_count[i] =
            (uint32_t)mock_hal_state.spi_instances[i].call_count;
    }
    for (int i = 0; i < HAL_I2C_INSTANCE_MAX; i++) {
        test_mock_state.i2c_transaction_count[i] =
            (uint32_t)mock_hal_state.i2c_instances[i].call_count;
    }

    return &test_mock_state;
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
// helper functions (MockHAL_SetSPIResponse, MockHAL_SetI2CResponse) are
// defined above. Continue with I2C Mock Implementation.

/* ==========================================================================
 */
/* Programmable SPI/I2C Response Helpers (visible to linker/tests)          */
/* ==========================================================================
 */

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

    MockI2C_Internal_t *i2c = &mock_hal_state.i2c_instances[instance];

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
            // Fill with test
            // pattern for AS5600
            // simulation
            if (transaction->device_address == 0x36) {       // AS5600
                                                             // address
                if (transaction->register_address == 0x0C) { // RAW_ANGLE
                                                             // register
                    transaction->data[0] = 0x12;             // Mock
                                                             // angle
                                                             // high
                                                             // byte
                    transaction->data[1] = 0x34;             // Mock
                                                             // angle
                                                             // low
                                                             // byte
                } else if (transaction->register_address == 0x0E) { // ANGLE
                                                                    // register
                    transaction->data[0] = 0x56;                    // Mock
                                                                    // angle
                                                                    // high
                                                                    // byte
                    transaction->data[1] = 0x78;                    // Mock
                                                                    // angle
                                                                    // low
                                                                    // byte
                } else {
                    for (uint16_t i = 0; i < transaction->data_size; i++) {
                        transaction->data[i] = 0xC5 + i;
                    }
                }
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
    if (tx_data == NULL || instance >= HAL_I2C_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }

    if (mock_hal_state.inject_i2c_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    MockI2C_Internal_t *i2c = &mock_hal_state.i2c_instances[instance];

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

    MockI2C_Internal_t *i2c = &mock_hal_state.i2c_instances[instance];

    i2c->last_device_address = device_address;

    if (i2c->response_set && i2c->response_size >= data_size) {
        memcpy(rx_data, i2c->response_data, data_size);
        memcpy(i2c->last_data, i2c->response_data, data_size);
        i2c->response_set = false;
    } else {
        // Fill with test pattern
        for (uint16_t i = 0; i < data_size; i++) {
            rx_data[i] = 0xAB + i;
        }
        if (data_size <= sizeof(i2c->last_data)) {
            memcpy(i2c->last_data, rx_data, data_size);
        }
    }
    // Programmable SPI/I2C response helpers are
    // defined at file scope (above). Continue.

    i2c->last_data_size = data_size;
    i2c->call_count++;

    return i2c->return_value;
}

/**
 * @brief Combined I2C
 * transmit/receive operation
 * (expected by tests)
 */
SystemError_t
HAL_Abstraction_I2C_TransmitReceive(HAL_I2C_Instance_t instance,
                                    const HAL_I2C_Transaction_t *transaction) {
    if (transaction == NULL || instance >= HAL_I2C_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }

    if (mock_hal_state.inject_i2c_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    MockI2C_Internal_t *i2c = &mock_hal_state.i2c_instances[instance];

    i2c->last_device_address = transaction->device_address;
    i2c->last_register_address = transaction->register_address;

    if (transaction->data &&
        transaction->data_size <= sizeof(i2c->last_data)) {
        // Fill with test pattern
        // for reads, record for
        // writes
        if (transaction->device_address == 0x36) {              // AS5600
                                                                // address
            if (transaction->register_address == 0x0C) {        // RAW_ANGLE
                                                                // register
                transaction->data[0] = 0x12;                    // Mock
                                                                // angle
                                                                // high
                                                                // byte
                transaction->data[1] = 0x34;                    // Mock
                                                                // angle
                                                                // low
                                                                // byte
            } else if (transaction->register_address == 0x0E) { // ANGLE
                                                                // register
                transaction->data[0] = 0x56;                    // Mock
                                                                // angle
                                                                // high
                                                                // byte
                transaction->data[1] = 0x78;                    // Mock
                                                                // angle
                                                                // low
                                                                // byte
            } else {
                // Generic test
                // pattern
                for (uint16_t i = 0; i < transaction->data_size; i++) {
                    transaction->data[i] = 0xC5 + i;
                }
            }
        }
        memcpy(i2c->last_data, transaction->data, transaction->data_size);
    }

    i2c->last_data_size = transaction->data_size;
    i2c->call_count++;

    return i2c->return_value;
}

/* ==========================================================================
 */
/* Timer Mock Implementation */
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

    MockTimer_Internal_t *timer = &mock_hal_state.timer_instances[instance];
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

    MockTimer_Internal_t *timer = &mock_hal_state.timer_instances[instance];
    timer->running = true;

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_Timer_Stop(HAL_Timer_Instance_t instance) {
    if (instance >= HAL_TIMER_INSTANCE_MAX) {
        return ERROR_INVALID_PARAMETER;
    }

    MockTimer_Internal_t *timer = &mock_hal_state.timer_instances[instance];
    timer->running = false;

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_Timer_GetCounter(HAL_Timer_Instance_t instance,
                                               uint32_t *counter) {
    if (counter == NULL || instance >= HAL_TIMER_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }

    MockTimer_Internal_t *timer = &mock_hal_state.timer_instances[instance];
    *counter = timer->counter_value;

    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* System Timing Mock
 * Implementation */
/* ==========================================================================
 */

uint32_t HAL_Abstraction_GetTick(void) {
    return mock_hal_state.system_tick;
}

void HAL_Abstraction_Delay(uint32_t delay_ms) {
    mock_hal_state.delay_call_count++;
    mock_hal_state.system_tick += delay_ms; // Simulate time
                                            // passage
}

uint32_t HAL_Abstraction_GetMicroseconds(void) {
    return mock_hal_state.system_tick * 1000; // Simple
                                              // conversion for
                                              // testing
}

/* ==========================================================================
 */
/* Watchdog Mock Implementation */
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
/* System Control Mock
 * Implementation */
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
    // For testing, just reset mock
    // state
    HAL_Abstraction_Mock_Reset();
}

/* ==========================================================================
 */
/* Configuration Functions Mock
 * Implementation */
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

/* ==========================================================================
 */
/* Driver Abstraction Mock
 * Implementation (FTR-013) */
/* ==========================================================================
 */

/**
 * @brief AS5600 Encoder Driver
 * Mock Implementation
 *
 * Mock implementation for unit
 * testing that provides
 * controllable behavior without
 * requiring actual AS5600
 * hardware.
 */

SystemError_t HAL_Abstraction_AS5600_Init(uint8_t encoder_id) {
    // Mock implementation - just
    // validate parameters and
    // track calls
    if (encoder_id >= 2) { // Assuming max 2
                           // encoders
        return ERROR_INVALID_PARAMETER;
    }

    // Could add call tracking here
    // if needed for test
    // verification
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_AS5600_ReadAngle(uint8_t encoder_id,
                                               float *angle_degrees) {
    // Mock implementation -
    // provide predictable test
    // data
    if (encoder_id >= 2) {
        return ERROR_INVALID_PARAMETER;
    }

    if (angle_degrees == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Return mock angle data for
    // testing Test can override
    // this by setting up mock
    // state if needed
    *angle_degrees = 45.0f + (encoder_id * 90.0f); // Different
                                                   // values per
                                                   // encoder
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_AS5600_CheckMagnet(uint8_t encoder_id,
                                                 bool *magnet_detected) {
    // Mock implementation - return
    // magnet detected for testing
    if (encoder_id >= 2) {
        return ERROR_INVALID_PARAMETER;
    }

    if (magnet_detected == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Mock: assume magnet is
    // always detected for testing
    *magnet_detected = true;
    return SYSTEM_OK;
}

/**
 * @brief L6470 Motor Driver Mock
 * Implementation
 *
 * Mock implementation for unit
 * testing that provides
 * controllable behavior without
 * requiring actual L6470 hardware.
 */

SystemError_t HAL_Abstraction_L6470_Init(uint8_t motor_id) {
    // Mock implementation - just
    // validate parameters
    if (motor_id >= 2) { // Assuming max 2
                         // motors
        return ERROR_INVALID_PARAMETER;
    }

    // Could add initialization
    // tracking here if needed for
    // test verification
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_L6470_GetStatus(uint8_t motor_id,
                                              uint32_t *status) {
    // Mock implementation -
    // provide predictable test
    // data
    if (motor_id >= 2) {
        return ERROR_INVALID_PARAMETER;
    }

    if (status == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Return mock status (normal
    // operation)
    *status = 0x7E83; // Normal L6470
                      // status value for
                      // testing
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_L6470_GetParameter(uint8_t motor_id,
                                                 uint8_t param,
                                                 uint32_t *value) {
    // Mock implementation -
    // provide predictable test
    // data
    if (motor_id >= 2) {
        return ERROR_INVALID_PARAMETER;
    }

    if (value == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    // Return mock parameter value
    // based on parameter requested
    switch (param) {
    case 0x09:                            // ABS_POS register
        *value = 1000 + (motor_id * 500); // Different
                                          // position per
                                          // motor
        break;
    case 0x11:           // STATUS register
        *value = 0x7E83; // Normal
                         // status
        break;
    default:
        *value = 0; // Default value for
                    // unknown
                    // parameters
        break;
    }

    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_L6470_HardStop(uint8_t motor_id) {
    // Mock implementation - just
    // validate parameters
    if (motor_id >= 2) {
        return ERROR_INVALID_PARAMETER;
    }

    // Mock: hard stop always
    // succeeds in test environment
    return SYSTEM_OK;
}

/* ==========================================================================
 */
/* Additional Mock Helpers and Compatibility Exports                        */
/* These provide the older MockHAL_* symbols and HAL_Abstraction GPIO/SPI
 * implementations expected by host tests.
 */

/**
 * @brief Inject a high-level mock fault for testing
 */
void MockHAL_InjectFault(uint32_t fault_type, bool enable) {
    if (fault_type & MOCK_FAULT_GPIO_INIT) {
        mock_hal_state.inject_gpio_failure = enable;
    }
    if (fault_type & MOCK_FAULT_GPIO_WRITE) {
        mock_hal_state.inject_gpio_failure = enable;
    }
    if (fault_type & MOCK_FAULT_SPI_INIT) {
        mock_hal_state.inject_spi_failure = enable;
    }
    if (fault_type & MOCK_FAULT_I2C_INIT) {
        mock_hal_state.inject_i2c_failure = enable;
    }
}

/**
 * @brief Set a GPIO state in the mock (tests call this)
 */
void MockHAL_SetGPIOState(HAL_GPIO_Port_t port, uint32_t pin,
                          HAL_GPIO_State_t state) {
    int idx = mock_pin_to_index(pin);
    if (idx < 0 || port >= HAL_GPIO_PORT_MAX) {
        return;
    }
    mock_hal_state.gpio_ports[port].pin_states[idx] = state;
    mock_hal_state.gpio_ports[port].pin_configured[idx] = true;
}

/**
 * @brief Low-level helper used by some unit tests to directly set GPIO state
 */
void HAL_Abstraction_Mock_SetGPIOState(HAL_GPIO_Port_t port, uint32_t pin,
                                       HAL_GPIO_State_t state) {
    MockHAL_SetGPIOState(port, pin, state);
}

/**
 * @brief Mock implementation of HAL_Abstraction_GPIO_Write for host tests
 */
SystemError_t HAL_Abstraction_GPIO_Write(HAL_GPIO_Port_t port, uint32_t pin,
                                         HAL_GPIO_State_t state) {
    if (port >= HAL_GPIO_PORT_MAX) {
        return ERROR_INVALID_PARAMETER;
    }
    if (mock_hal_state.inject_gpio_failure) {
        return ERROR_HARDWARE_FAULT;
    }
    int idx = mock_pin_to_index(pin);
    if (idx < 0) {
        return ERROR_INVALID_PARAMETER;
    }
    mock_hal_state.gpio_ports[port].pin_states[idx] = state;
    mock_hal_state.gpio_ports[port].pin_configured[idx] = true;
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_Init(HAL_GPIO_Port_t port,
                                        const HAL_GPIO_Config_t *config) {
    if (config == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (port >= HAL_GPIO_PORT_MAX) {
        return ERROR_INVALID_PARAMETER;
    }
    int idx = mock_pin_to_index(config->pin);
    if (idx < 0) {
        return ERROR_INVALID_PARAMETER;
    }
    // mark configured; default state pulled-up (SET) unless specified
    mock_hal_state.gpio_ports[port].pin_configured[idx] = true;
    mock_hal_state.gpio_ports[port].pin_states[idx] = HAL_GPIO_STATE_SET;
    return SYSTEM_OK;
}

SystemError_t HAL_Abstraction_GPIO_EnableInterrupt(HAL_GPIO_Port_t port,
                                                   uint32_t pin,
                                                   uint32_t trigger_type,
                                                   uint32_t priority) {
    if (port >= HAL_GPIO_PORT_MAX) {
        return ERROR_INVALID_PARAMETER;
    }
    int idx = mock_pin_to_index(pin);
    if (idx < 0) {
        return ERROR_INVALID_PARAMETER;
    }
    // For host tests, just mark interrupts enabled and record config
    mock_hal_state.interrupts_enabled = true;
    mock_hal_state.gpio_ports[port].pin_configured[idx] = true;
    return SYSTEM_OK;
}

/**
 * @brief Mock implementation of HAL_Abstraction_GPIO_Read for host tests
 */
SystemError_t HAL_Abstraction_GPIO_Read(HAL_GPIO_Port_t port, uint32_t pin,
                                        HAL_GPIO_State_t *out_state) {
    if (out_state == NULL) {
        return ERROR_NULL_POINTER;
    }
    if (port >= HAL_GPIO_PORT_MAX) {
        return ERROR_INVALID_PARAMETER;
    }
    int idx = mock_pin_to_index(pin);
    if (idx < 0) {
        return ERROR_INVALID_PARAMETER;
    }
    *out_state = mock_hal_state.gpio_ports[port].pin_states[idx];
    return SYSTEM_OK;
}

/**
 * @brief Mock SPI transmit/receive helper used by higher-level code under test
 */
SystemError_t
HAL_Abstraction_SPI_TransmitReceive(HAL_SPI_Instance_t instance,
                                    const HAL_SPI_Transaction_t *transaction) {
    if (transaction == NULL || instance >= HAL_SPI_INSTANCE_MAX) {
        return ERROR_NULL_POINTER;
    }
    if (mock_hal_state.inject_spi_failure) {
        return ERROR_HARDWARE_FAULT;
    }

    MockSPI_Internal_t *spi = &mock_hal_state.spi_instances[instance];

    // If a response was pre-programmed, copy it into rx buffer
    if (transaction->rx_data && spi->response_set) {
        size_t copy_size = (transaction->data_size <= spi->response_size)
                               ? transaction->data_size
                               : spi->response_size;
        memcpy(transaction->rx_data, spi->response_data, copy_size);
        spi->response_set = false; // one-shot
    } else if (transaction->rx_data) {
        // Fill with a deterministic test pattern
        for (uint16_t i = 0; i < transaction->data_size; i++) {
            transaction->rx_data[i] = (uint8_t)(0xA0 + (i & 0xFF));
        }
    }

    spi->call_count++;
    return SYSTEM_OK;
}
#endif // UNITY_TESTING
