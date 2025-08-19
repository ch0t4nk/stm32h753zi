#ifdef UNITY_TESTING
// Fallback typedefs for legacy HAL types if not present in current
// hal_abstraction.h
#include "config/error_codes.h"
#include "hal_abstraction/hal_abstraction.h"
#include <stdbool.h>
#include <stdint.h>

#ifndef HAL_GPIO_CONFIG_T_DEFINED
#define HAL_GPIO_CONFIG_T_DEFINED
typedef struct {
    uint32_t pin;
    uint32_t mode;
    uint32_t pull;
    uint32_t speed;
    uint32_t alternate;
} HAL_GPIO_Config_t;
#endif

#ifndef HAL_SPI_TRANSACTION_T_DEFINED
#define HAL_SPI_TRANSACTION_T_DEFINED
typedef struct {
    uint8_t *tx_data;
    uint8_t *rx_data;
    uint16_t data_size;
    uint32_t timeout_ms;
} HAL_SPI_Transaction_t;
#endif

#ifndef HAL_I2C_TRANSACTION_T_DEFINED
#define HAL_I2C_TRANSACTION_T_DEFINED
typedef struct {
    uint16_t device_address;
    uint16_t register_address;
    uint8_t *data;
    uint16_t data_size;
    uint32_t timeout_ms;
    bool use_register_address;
} HAL_I2C_Transaction_t;
#endif

// Fallback enum values for instance/port max if not defined
#ifndef HAL_GPIO_PORT_MAX
#define HAL_GPIO_PORT_MAX 8
#endif
#ifndef HAL_SPI_INSTANCE_MAX
#define HAL_SPI_INSTANCE_MAX 3
#endif
#ifndef HAL_I2C_INSTANCE_MAX
#define HAL_I2C_INSTANCE_MAX 3
#endif
#ifndef HAL_TIMER_INSTANCE_MAX
#define HAL_TIMER_INSTANCE_MAX 3
#endif

// Internal mock types and state for use in mock_hal_abstraction.c
#ifndef MOCK_GPIOPORT_INTERNAL_T_DEFINED
#define MOCK_GPIOPORT_INTERNAL_T_DEFINED
typedef struct {
    HAL_GPIO_State_t pin_states[16];
    bool pin_configured[16];
    HAL_GPIO_Config_t pin_configs[16];
} MockGPIOPort_Internal_t;
#endif

#ifndef MOCK_SPI_INTERNAL_T_DEFINED
#define MOCK_SPI_INTERNAL_T_DEFINED
typedef struct {
    uint8_t last_tx_data[256];
    uint8_t last_rx_data[256];
    uint16_t last_data_size;
    uint32_t call_count;
    SystemError_t return_value;
    bool initialized;
    uint8_t response_data[256];
    uint16_t response_size;
    bool response_set;
} MockSPI_Internal_t;
#endif

#ifndef MOCK_I2C_INTERNAL_T_DEFINED
#define MOCK_I2C_INTERNAL_T_DEFINED
typedef struct {
    uint16_t last_device_address;
    uint16_t last_register_address;
    uint8_t last_data[256];
    uint16_t last_data_size;
    uint32_t call_count;
    SystemError_t return_value;
    bool initialized;
    uint8_t response_data[256];
    uint16_t response_size;
    bool response_set;
} MockI2C_Internal_t;
#endif

#ifndef MOCK_TIMER_INTERNAL_T_DEFINED
#define MOCK_TIMER_INTERNAL_T_DEFINED
typedef struct {
    uint32_t counter_value;
    bool running;
    bool initialized;
    HAL_Timer_Config_t config;
} MockTimer_Internal_t;
#endif

#ifndef MOCK_HAL_INTERNAL_STATE_T_DEFINED
#define MOCK_HAL_INTERNAL_STATE_T_DEFINED
typedef struct {
    MockGPIOPort_Internal_t gpio_ports[HAL_GPIO_PORT_MAX];
    MockSPI_Internal_t spi_instances[HAL_SPI_INSTANCE_MAX];
    MockI2C_Internal_t i2c_instances[HAL_I2C_INSTANCE_MAX];
    MockTimer_Internal_t timer_instances[HAL_TIMER_INSTANCE_MAX];

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

    // Programmable safety event simulation
    bool estop_active;
    bool fault_pin_active;
    bool watchdog_timeout_triggered;
} MockHAL_Internal_State_t;
#endif

// Expose the internal state for use in mock_hal_abstraction.c
extern MockHAL_Internal_State_t mock_hal_state;
#endif
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

#include "config/error_codes.h"  // For SystemError_t, SYSTEM_OK
#include "config/motor_config.h" // For MAX_MOTORS and motor constants
#include "hal_abstraction/hal_abstraction.h"
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 */
/* Mock Control Functions */
/* ==========================================================================
 */

/**
 * @brief Set mock emergency stop active state for safety event simulation
 * @param active true = E-stop active, false = not active
 */
void MockHAL_SetEstopActive(bool active);

/**
 * @brief Set mock fault pin state (e.g., motor driver fault)
 * @param active true = fault pin asserted, false = not asserted
 */
void MockHAL_SetFaultPinActive(bool active);

/**
 * @brief Simulate watchdog timeout event (next check will trigger timeout)
 * @param trigger true = next check triggers timeout, false = normal
 */
void MockHAL_TriggerWatchdogTimeout(bool trigger);

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
 * @param state Pin state to return (should not use HAL_GPIO_State_t)
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
 * @param state Pin state to set (should not use HAL_GPIO_State_t)
 * @param state Pin state to set
 */
void HAL_Abstraction_Mock_SetGPIOState(HAL_GPIO_Port_t port, uint32_t pin,
                                       HAL_GPIO_State_t state);

/**
 * @brief Set programmable SPI response data for next read/transaction
 *
 * Programs the mock SPI to return the given data buffer on the next SPI read
 * or transmit/receive operation. The response is one-shot: after being used,
 * it is cleared. Use before calling code that expects a specific SPI response.
 *
 * Example usage in a test:
 *   uint8_t status_bytes[3] = {0x83, 0x7E, 0x00};
 *   MockHAL_SetSPIResponse(0, status_bytes, 3);
 *   // ... call function that reads SPI ...
 *
 * @param instance SPI instance (usually 0)
 * @param data Response data buffer
 * @param size Data size in bytes
 */
void MockHAL_SetSPIResponse(HAL_SPI_Instance_t instance, const uint8_t *data,
                            uint16_t size);

/**
 * @brief Set programmable I2C response data for next read/transaction
 *
 * Programs the mock I2C to return the given data buffer on the next I2C read
 * or memory read operation. The response is one-shot: after being used, it is
 * cleared. Use before calling code that expects a specific I2C response.
 *
 * Example usage in a test:
 *   uint8_t angle_bytes[2] = {0x00, 0x08};
 *   MockHAL_SetI2CResponse(0, angle_bytes, 2);
 *   // ... call function that reads I2C ...
 *
 * @param instance I2C instance (usually 0)
 * @param data Response data buffer
 * @param size Data size in bytes
 */
void MockHAL_SetI2CResponse(HAL_I2C_Instance_t instance, const uint8_t *data,
                            uint16_t size);

/**
 * @brief Get number of times a function was called
 * @param function_name Function name
 * @return Number of calls
 */
/**
 * @brief Get number of times functions have been called (legacy API)
 *
 * Backwards-compatible no-argument API used by existing unit tests.
 * Newer tests can be extended later to provide per-function call counts.
 */
uint32_t MockHAL_GetCallCount(void);

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

// Allow tests to index by either pin index [0..15] or pin bitmask (1<<n).
// The original value (1<<16) creates extremely large global arrays which
// can cause memory layout differences across translation units and lead to
// runtime crashes in host tests. Use a small expanded index space that
// covers common bitmask/index usage while keeping memory reasonable.
// If a test requires larger mask-index support, increase this value
// locally in the test and adapt the mock implementation accordingly.
#define MOCK_GPIO_INDEX_SPACE 32
typedef struct {
    HAL_GPIO_State_t pin_states[HAL_GPIO_PORT_MAX][MOCK_GPIO_INDEX_SPACE];
    bool pin_written[HAL_GPIO_PORT_MAX][MOCK_GPIO_INDEX_SPACE];
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
    bool gpio_configured[HAL_GPIO_PORT_MAX]
                        [MOCK_GPIO_INDEX_SPACE]; // GPIO configuration status
    HAL_GPIO_State_t gpio_states[HAL_GPIO_PORT_MAX]
                                [MOCK_GPIO_INDEX_SPACE];  // GPIO pin states
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
// Ensure helper prototypes are declared for all test translation units.
// Some older tests expect these functions to be available; provide
// declarations here to avoid implicit-function-declaration errors.
#ifndef MOCK_HAL_HELPER_PROTOTYPES_H
#define MOCK_HAL_HELPER_PROTOTYPES_H

/**
 * @brief Program SPI response for next mock transaction (one-shot)
 */
void MockHAL_SetSPIResponse(HAL_SPI_Instance_t instance, const uint8_t *data,
                            uint16_t size);

/**
 * @brief Program I2C response for next mock transaction (one-shot)
 */
void MockHAL_SetI2CResponse(HAL_I2C_Instance_t instance, const uint8_t *data,
                            uint16_t size);

/**
 * @brief Set mock tick value
 */
void MockHAL_SetTick(uint32_t tick_value);

/**
 * @brief Get pointer to internal mock state (for verification)
 */
MockHAL_State_t *MockHAL_GetState(void);

#endif /* MOCK_HAL_HELPER_PROTOTYPES_H */
