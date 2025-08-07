/**
 * @file mock_hal_abstraction.h
 * @brief Mock HAL Abstraction for Unit Testing
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * This file provides mock implementations of HAL abstraction functions
 * for unit testing without requiring actual hardware.
 */

#ifndef MOCK_HAL_ABSTRACTION_H
#define MOCK_HAL_ABSTRACTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_abstraction/hal_abstraction.h"
#include "unity.h"

/* ==========================================================================
 */
/* Mock Control Functions */
/* ==========================================================================
 */

/**
 * @brief Reset all mock states to default
 */
void MockHAL_Reset(void);

/**
 * @brief Set expected return value for HAL functions
 * @param function_name Name of function to mock
 * @param return_value Return value to use
 */
void MockHAL_SetReturnValue(const char *function_name,
                            SystemError_t return_value);

/**
 * @brief Set GPIO pin state for reading
 * @param port GPIO port
 * @param pin GPIO pin number
 * @param state Pin state to return
 */
void MockHAL_SetGPIOState(HAL_GPIO_Port_t port, uint32_t pin,
                          HAL_GPIO_State_t state);

/**
 * @brief Verify GPIO pin was written with expected value
 * @param port GPIO port
 * @param pin GPIO pin number
 * @return true if pin was set, false otherwise
 */
bool MockHAL_WasGPIOPinSet(HAL_GPIO_Port_t port, uint32_t pin);

/**
 * @brief Set system tick value for HAL_Abstraction_GetTick()
 * @param tick_value Tick value to return
 */
void MockHAL_SetTick(uint32_t tick_value);

/**
 * @brief Set GPIO pin state for testing (low-level function)
 * @param port GPIO port
 * @param pin GPIO pin number
 * @param state Pin state to set
 */
void HAL_Abstraction_Mock_SetGPIOState(HAL_GPIO_Port_t port, uint32_t pin,
                                       HAL_GPIO_State_t state);

/**
 * @brief Set SPI response data
 * @param instance SPI instance
 * @param data Response data buffer
 * @param size Data size
 */
void MockHAL_SetSPIResponse(HAL_SPI_Instance_t instance, const uint8_t *data,
                            uint16_t size);

/**
 * @brief Set I2C response data
 * @param instance I2C instance
 * @param data Response data buffer
 * @param size Data size
 */
void MockHAL_SetI2CResponse(HAL_I2C_Instance_t instance, const uint8_t *data,
                            uint16_t size);

/**
 * @brief Get number of times a function was called
 * @param function_name Function name
 * @return Number of calls
 */
uint32_t MockHAL_GetCallCount(const char *function_name);

/**
 * @brief Verify function was called with expected parameters
 * @param function_name Function name
 * @param parameter_index Parameter index (0-based)
 * @param expected_value Expected parameter value
 * @return true if parameter matches, false otherwise
 */
bool MockHAL_VerifyParameter(const char *function_name,
                             uint32_t parameter_index,
                             uint32_t expected_value);

/* ==========================================================================
 */
/* Mock State Structures */
/* ==========================================================================
 */

typedef struct {
    SystemError_t return_value;
    uint32_t call_count;
    uint32_t parameters[8]; // Store up to 8 parameters per call
    bool enabled;
} MockFunction_t;

typedef struct {
    HAL_GPIO_State_t pin_states[HAL_GPIO_PORT_MAX][16]; // 16 pins per port max
    bool pin_written[HAL_GPIO_PORT_MAX][16];
    MockFunction_t gpio_functions;
} MockGPIO_t;

typedef struct {
    uint8_t response_data[256];
    uint16_t response_size;
    MockFunction_t spi_functions;
} MockSPI_t;

typedef struct {
    uint8_t response_data[256];
    uint16_t response_size;
    MockFunction_t i2c_functions;
} MockI2C_t;

typedef struct {
    uint32_t tick_value;
    uint32_t delay_called_count;
    MockFunction_t timer_functions;
} MockTimer_t;

/**
 * @brief Complete mock HAL state structure for test verification
 * This matches the internal mock state used by tests
 */
typedef struct {
    bool gpio_configured[HAL_GPIO_PORT_MAX][16]; // GPIO configuration status
    HAL_GPIO_State_t gpio_states[HAL_GPIO_PORT_MAX][16];  // GPIO pin states
    uint32_t spi_transaction_count[HAL_SPI_INSTANCE_MAX]; // SPI call counts
    uint32_t i2c_transaction_count[HAL_I2C_INSTANCE_MAX]; // I2C call counts
    uint32_t system_tick;            // System tick counter
    uint32_t delay_call_count;       // Delay call counter
    uint32_t watchdog_refresh_count; // Watchdog refresh counter
    bool interrupts_enabled;         // Interrupt state
    bool hal_initialized;            // HAL initialization state

    // Fault injection flags
    bool inject_spi_failure;
    bool inject_i2c_failure;
    bool inject_gpio_failure;
    bool inject_timer_failure;
} MockHAL_State_t;

/* ==========================================================================
 */
/* Mock Implementation Helpers */
/* ==========================================================================
 */

/**
 * @brief Initialize mock system for testing
 */
void MockHAL_Init(void);

/**
 * @brief Cleanup mock system after testing
 */
void MockHAL_Cleanup(void);

/* ==========================================================================
 */
/* Additional Mock Functions for Test Compatibility                          */
/* ==========================================================================
 */

/**
 * @brief Get pointer to mock state for test verification
 * @return Pointer to mock state structure for validation
 */
MockHAL_State_t *MockHAL_GetState(void);

/**
 * @brief Inject failure for testing error handling
 * @param fault_type Type of fault to inject
 * @param enable Enable/disable fault injection
 */
void MockHAL_InjectFault(uint32_t fault_type, bool enable);

/* ==========================================================================
 */
/* Mock Fault Injection Constants                                            */
/* ==========================================================================
 */

#define MOCK_FAULT_GPIO_INIT 0x01
#define MOCK_FAULT_GPIO_WRITE 0x02
#define MOCK_FAULT_SPI_INIT 0x04
#define MOCK_FAULT_I2C_INIT 0x08

/* ==========================================================================
 */
/* HAL Instance Constants for Tests                                          */
/* ==========================================================================
 */

#define HAL_SPI_1 HAL_SPI_INSTANCE_1
#define HAL_I2C_1 HAL_I2C_INSTANCE_1

#ifdef __cplusplus
}
#endif

#endif /* MOCK_HAL_ABSTRACTION_H */
