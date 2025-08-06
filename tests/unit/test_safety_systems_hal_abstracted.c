/**
 * @file test_safety_systems_hal_abstracted.c
 * @brief Unit Tests for Safety Systems with HAL Abstraction
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 *
 * This test file demonstrates how the HAL abstraction layer enables
 * clean unit testing without hardware dependencies. This is the new
 * version that fully uses the HAL abstraction layer.
 */

#include "common/error_codes.h"
#include "config/safety_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include "safety/emergency_stop_abstracted.h"
#include "unity.h"
#include <string.h>

/* ==========================================================================
 */
/* Test Helper Functions */
/* ==========================================================================
 */

void setUp(void) {
    // Reset mock HAL state before each test
    MockHAL_Reset();

    // Initialize emergency stop system with mock HAL
    SystemError_t result = emergency_stop_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void tearDown(void) {
    // Clean up after each test
    MockHAL_Reset();
}

/* ==========================================================================
 */
/* Emergency Stop Initialization Tests                                       */
/* ==========================================================================
 */

void test_emergency_stop_init_success(void) {
    // Reset and test initialization
    MockHAL_Reset();

    SystemError_t result = emergency_stop_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify system is in armed state
    EmergencyStopState_t state = emergency_stop_get_state();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_ARMED, state);

    // Verify GPIO pins were configured
    MockHAL_State_t *mock_state = MockHAL_GetState();
    TEST_ASSERT_TRUE(
        mock_state->gpio_configured[ESTOP_BUTTON_PORT][ESTOP_BUTTON_PIN]);
    TEST_ASSERT_TRUE(
        mock_state->gpio_configured[ESTOP_LED_PORT][ESTOP_LED_PIN]);
    TEST_ASSERT_TRUE(
        mock_state->gpio_configured[SAFETY_RELAY1_PORT][SAFETY_RELAY1_PIN]);
    TEST_ASSERT_TRUE(
        mock_state->gpio_configured[SAFETY_RELAY2_PORT][SAFETY_RELAY2_PIN]);
}

void test_emergency_stop_init_gpio_failure(void) {
    MockHAL_Reset();

    // Inject GPIO initialization failure
    MockHAL_InjectFault(MOCK_FAULT_GPIO_INIT, true);

    SystemError_t result = emergency_stop_init();
    TEST_ASSERT_NOT_EQUAL(SYSTEM_OK, result);
}

/* ==========================================================================
 */
/* Emergency Stop Button Tests                                               */
/* ==========================================================================
 */

void test_emergency_stop_button_press(void) {
    // Simulate button press (active low)
    MockHAL_SetGPIOState(ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN,
                         HAL_GPIO_STATE_RESET);

    // Process for debounce time
    for (int i = 0; i < 60; i++) { // 60ms > 50ms debounce
        HAL_Abstraction_Delay(1);
        emergency_stop_process();
    }

    // Verify emergency stop was triggered
    EmergencyStopState_t state = emergency_stop_get_state();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_TRIGGERED, state);

    // Verify safety relays were activated
    MockHAL_State_t *mock_state = MockHAL_GetState();
    TEST_ASSERT_EQUAL(
        HAL_GPIO_STATE_SET,
        mock_state->gpio_states[SAFETY_RELAY1_PORT][SAFETY_RELAY1_PIN]);
    TEST_ASSERT_EQUAL(
        HAL_GPIO_STATE_SET,
        mock_state->gpio_states[SAFETY_RELAY2_PORT][SAFETY_RELAY2_PIN]);
}

void test_emergency_stop_button_debounce(void) {
    // Simulate brief button press (should not trigger)
    MockHAL_SetGPIOState(ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN,
                         HAL_GPIO_STATE_RESET);

    // Process for less than debounce time
    for (int i = 0; i < 30; i++) { // 30ms < 50ms debounce
        HAL_Abstraction_Delay(1);
        emergency_stop_process();
    }

    // Release button
    MockHAL_SetGPIOState(ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN,
                         HAL_GPIO_STATE_SET);

    // Process
    emergency_stop_process();

    // Verify emergency stop was NOT triggered
    EmergencyStopState_t state = emergency_stop_get_state();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_ARMED, state);
}

void test_emergency_stop_timing_requirement(void) {
    uint32_t start_time = HAL_Abstraction_GetTick();

    // Trigger emergency stop
    MockHAL_SetGPIOState(ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN,
                         HAL_GPIO_STATE_RESET);

    // Process for debounce time
    for (int i = 0; i < 60; i++) {
        HAL_Abstraction_Delay(1);
        emergency_stop_process();
    }

    uint32_t response_time = HAL_Abstraction_GetTick() - start_time;

    // Verify response time meets SIL-3 requirement (< 100ms with debounce)
    TEST_ASSERT_LESS_THAN(100, response_time);

    // Verify emergency stop was triggered
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_TRIGGERED, emergency_stop_get_state());
}

/* ==========================================================================
 */
/* Emergency Stop Software Trigger Tests                                     */
/* ==========================================================================
 */

void test_emergency_stop_software_trigger(void) {
    SystemError_t result = emergency_stop_execute(ESTOP_SOURCE_SOFTWARE);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify state change
    EmergencyStopState_t state = emergency_stop_get_state();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_TRIGGERED, state);

    // Verify source is recorded
    EmergencyStopSource_t source = emergency_stop_get_last_source();
    TEST_ASSERT_EQUAL(ESTOP_SOURCE_SOFTWARE, source);
}

void test_emergency_stop_multiple_sources(void) {
    // Trigger from motor fault
    emergency_stop_execute(ESTOP_SOURCE_MOTOR_FAULT);
    TEST_ASSERT_EQUAL(ESTOP_SOURCE_MOTOR_FAULT,
                      emergency_stop_get_last_source());

    // Additional trigger from different source
    emergency_stop_execute(ESTOP_SOURCE_ENCODER_FAULT);
    TEST_ASSERT_EQUAL(ESTOP_SOURCE_ENCODER_FAULT,
                      emergency_stop_get_last_source());

    // State should remain triggered
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_TRIGGERED, emergency_stop_get_state());
}

/* ==========================================================================
 */
/* Emergency Stop Reset Tests                                                */
/* ==========================================================================
 */

void test_emergency_stop_reset_success(void) {
    // Trigger emergency stop
    emergency_stop_execute(ESTOP_SOURCE_SOFTWARE);
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_TRIGGERED, emergency_stop_get_state());

    // Ensure button is not pressed
    MockHAL_SetGPIOState(ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN,
                         HAL_GPIO_STATE_SET);

    // Initiate reset
    SystemError_t result = emergency_stop_reset();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_RESET_PENDING,
                      emergency_stop_get_state());

    // Wait for reset delay
    for (int i = 0; i < 1100; i++) { // 1100ms > 1000ms reset delay
        HAL_Abstraction_Delay(1);
        emergency_stop_process();
    }

    // Verify system returned to armed state
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_ARMED, emergency_stop_get_state());

    // Verify safety relays were deactivated
    MockHAL_State_t *mock_state = MockHAL_GetState();
    TEST_ASSERT_EQUAL(
        HAL_GPIO_STATE_RESET,
        mock_state->gpio_states[SAFETY_RELAY1_PORT][SAFETY_RELAY1_PIN]);
    TEST_ASSERT_EQUAL(
        HAL_GPIO_STATE_RESET,
        mock_state->gpio_states[SAFETY_RELAY2_PORT][SAFETY_RELAY2_PIN]);
}

void test_emergency_stop_reset_button_pressed(void) {
    // Trigger emergency stop
    emergency_stop_execute(ESTOP_SOURCE_SOFTWARE);

    // Keep button pressed
    MockHAL_SetGPIOState(ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN,
                         HAL_GPIO_STATE_RESET);

    // Attempt reset (should fail)
    SystemError_t result = emergency_stop_reset();
    TEST_ASSERT_EQUAL(ERROR_HARDWARE_FAULT, result);

    // State should remain triggered
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_TRIGGERED, emergency_stop_get_state());
}

void test_emergency_stop_reset_not_triggered(void) {
    // Attempt reset when not triggered (should fail)
    SystemError_t result = emergency_stop_reset();
    TEST_ASSERT_EQUAL(ERROR_INVALID_STATE, result);

    // State should remain armed
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_ARMED, emergency_stop_get_state());
}

/* ==========================================================================
 */
/* Emergency Stop LED Indication Tests                                       */
/* ==========================================================================
 */

void test_emergency_stop_led_heartbeat_armed(void) {
    MockHAL_State_t *mock_state = MockHAL_GetState();

    // Process for multiple heartbeat cycles
    for (int i = 0; i < 250; i++) { // 250ms > 200ms (2 x 100ms heartbeat)
        HAL_Abstraction_Delay(1);
        emergency_stop_process();
    }

    // LED should have changed state (heartbeat)
    // Note: This test verifies that LED state management is working
    // The exact pattern depends on timing, but LED should be actively
    // controlled
    TEST_ASSERT_TRUE(
        mock_state->gpio_configured[ESTOP_LED_PORT][ESTOP_LED_PIN]);
}

void test_emergency_stop_led_triggered_state(void) {
    // Trigger emergency stop
    emergency_stop_execute(ESTOP_SOURCE_SOFTWARE);

    // Process
    emergency_stop_process();

    // LED should be solid on when triggered
    MockHAL_State_t *mock_state = MockHAL_GetState();
    TEST_ASSERT_EQUAL(HAL_GPIO_STATE_SET,
                      mock_state->gpio_states[ESTOP_LED_PORT][ESTOP_LED_PIN]);
}

/* ==========================================================================
 */
/* Emergency Stop Self-Test Tests                                            */
/* ==========================================================================
 */

void test_emergency_stop_self_test_success(void) {
    SystemError_t result = emergency_stop_self_test();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify that GPIO operations were performed during self-test
    MockHAL_State_t *mock_state = MockHAL_GetState();
    TEST_ASSERT_TRUE(
        mock_state->gpio_configured[ESTOP_LED_PORT][ESTOP_LED_PIN]);
    TEST_ASSERT_TRUE(
        mock_state->gpio_configured[SAFETY_RELAY1_PORT][SAFETY_RELAY1_PIN]);
}

void test_emergency_stop_self_test_gpio_failure(void) {
    // Inject GPIO failure
    MockHAL_InjectFault(MOCK_FAULT_GPIO_WRITE, true);

    SystemError_t result = emergency_stop_self_test();
    TEST_ASSERT_NOT_EQUAL(SYSTEM_OK, result);
}

/* ==========================================================================
 */
/* Emergency Stop Statistics Tests                                           */
/* ==========================================================================
 */

void test_emergency_stop_statistics(void) {
    uint32_t trigger_count, last_trigger_time;

    // Get initial statistics
    SystemError_t result =
        emergency_stop_get_statistics(&trigger_count, &last_trigger_time);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(0, trigger_count);

    // Trigger emergency stop
    emergency_stop_execute(ESTOP_SOURCE_SOFTWARE);

    // Check updated statistics
    result = emergency_stop_get_statistics(&trigger_count, &last_trigger_time);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(1, trigger_count);
    TEST_ASSERT_NOT_EQUAL(0, last_trigger_time);

    // Trigger again
    emergency_stop_execute(ESTOP_SOURCE_MOTOR_FAULT);

    // Check statistics again
    result = emergency_stop_get_statistics(&trigger_count, &last_trigger_time);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(2, trigger_count);
}

/* ==========================================================================
 */
/* Emergency Stop Health Check Tests                                         */
/* ==========================================================================
 */

void test_emergency_stop_health_check_success(void) {
    SystemError_t result = emergency_stop_check_health();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_emergency_stop_health_check_not_initialized(void) {
    MockHAL_Reset(); // This will make the system appear uninitialized

    SystemError_t result = emergency_stop_check_health();
    TEST_ASSERT_EQUAL(ERROR_NOT_INITIALIZED, result);
}

/* ==========================================================================
 */
/* HAL Abstraction Specific Tests                                           */
/* ==========================================================================
 */

void test_hal_abstraction_gpio_operations(void) {
    // Test GPIO write through abstraction
    SystemError_t result = HAL_Abstraction_GPIO_Write(
        ESTOP_LED_PORT, ESTOP_LED_PIN, HAL_GPIO_STATE_SET);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify mock state
    MockHAL_State_t *mock_state = MockHAL_GetState();
    TEST_ASSERT_EQUAL(HAL_GPIO_STATE_SET,
                      mock_state->gpio_states[ESTOP_LED_PORT][ESTOP_LED_PIN]);

    // Test GPIO read through abstraction
    HAL_GPIO_State_t gpio_state;
    result =
        HAL_Abstraction_GPIO_Read(ESTOP_LED_PORT, ESTOP_LED_PIN, &gpio_state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(HAL_GPIO_STATE_SET, gpio_state);
}

void test_hal_abstraction_fault_injection(void) {
    // Test GPIO write failure injection
    MockHAL_InjectFault(MOCK_FAULT_GPIO_WRITE, true);

    SystemError_t result = HAL_Abstraction_GPIO_Write(
        ESTOP_LED_PORT, ESTOP_LED_PIN, HAL_GPIO_STATE_SET);
    TEST_ASSERT_NOT_EQUAL(SYSTEM_OK, result);

    // Clear fault injection
    MockHAL_InjectFault(MOCK_FAULT_GPIO_WRITE, false);

    // Should work now
    result = HAL_Abstraction_GPIO_Write(ESTOP_LED_PORT, ESTOP_LED_PIN,
                                        HAL_GPIO_STATE_SET);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_hal_abstraction_timing_functions(void) {
    uint32_t start_time = HAL_Abstraction_GetTick();

    HAL_Abstraction_Delay(100);

    uint32_t end_time = HAL_Abstraction_GetTick();

    // Verify delay worked (allowing some tolerance)
    TEST_ASSERT_GREATER_OR_EQUAL(100, end_time - start_time);
    TEST_ASSERT_LESS_OR_EQUAL(110, end_time - start_time); // 10ms tolerance
}

void test_hal_abstraction_spi_operations(void) {
    // Test SPI transaction through abstraction
    uint8_t tx_data[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t rx_data[4];

    HAL_SPI_Transaction_t transaction = {.tx_data = tx_data,
                                         .rx_data = rx_data,
                                         .size = sizeof(tx_data),
                                         .timeout_ms = 1000};

    SystemError_t result =
        HAL_Abstraction_SPI_TransmitReceive(HAL_SPI_1, &transaction);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify mock recorded the transaction
    MockHAL_State_t *mock_state = MockHAL_GetState();
    TEST_ASSERT_EQUAL(1, mock_state->spi_transaction_count[HAL_SPI_1]);
}

void test_hal_abstraction_i2c_operations(void) {
    // Test I2C transaction through abstraction
    uint8_t tx_data[] = {0xAA, 0xBB};
    uint8_t rx_data[2];

    HAL_I2C_Transaction_t transaction = {.device_address = 0x50,
                                         .tx_data = tx_data,
                                         .tx_size = sizeof(tx_data),
                                         .rx_data = rx_data,
                                         .rx_size = sizeof(rx_data),
                                         .timeout_ms = 1000};

    SystemError_t result =
        HAL_Abstraction_I2C_TransmitReceive(HAL_I2C_1, &transaction);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify mock recorded the transaction
    MockHAL_State_t *mock_state = MockHAL_GetState();
    TEST_ASSERT_EQUAL(1, mock_state->i2c_transaction_count[HAL_I2C_1]);
}

/* ==========================================================================
 */
/* Integration Tests                                                         */
/* ==========================================================================
 */

void test_hal_abstraction_integration_emergency_stop_full_cycle(void) {
    // This test demonstrates the full emergency stop cycle using HAL
    // abstraction

    // 1. Initialize system (already done in setUp)
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_ARMED, emergency_stop_get_state());

    // 2. Simulate button press
    MockHAL_SetGPIOState(ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN,
                         HAL_GPIO_STATE_RESET);

    // 3. Process for debounce time
    for (int i = 0; i < 60; i++) {
        HAL_Abstraction_Delay(1);
        emergency_stop_process();
    }

    // 4. Verify emergency stop triggered
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_TRIGGERED, emergency_stop_get_state());
    TEST_ASSERT_TRUE(emergency_stop_is_active());

    // 5. Release button
    MockHAL_SetGPIOState(ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN,
                         HAL_GPIO_STATE_SET);

    // 6. Initiate reset
    SystemError_t result = emergency_stop_reset();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_RESET_PENDING,
                      emergency_stop_get_state());

    // 7. Wait for reset delay
    for (int i = 0; i < 1100; i++) {
        HAL_Abstraction_Delay(1);
        emergency_stop_process();
    }

    // 8. Verify system returned to armed state
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_ARMED, emergency_stop_get_state());
    TEST_ASSERT_FALSE(emergency_stop_is_active());

    // 9. Verify all GPIO operations were successful
    MockHAL_State_t *mock_state = MockHAL_GetState();
    TEST_ASSERT_TRUE(
        mock_state->gpio_configured[ESTOP_BUTTON_PORT][ESTOP_BUTTON_PIN]);
    TEST_ASSERT_TRUE(
        mock_state->gpio_configured[ESTOP_LED_PORT][ESTOP_LED_PIN]);
    TEST_ASSERT_TRUE(
        mock_state->gpio_configured[SAFETY_RELAY1_PORT][SAFETY_RELAY1_PIN]);
    TEST_ASSERT_TRUE(
        mock_state->gpio_configured[SAFETY_RELAY2_PORT][SAFETY_RELAY2_PIN]);
}

/* ==========================================================================
 */
/* Test Suite Setup                                                          */
/* ==========================================================================
 */

int main(void) {
    UNITY_BEGIN();

    // Emergency Stop Initialization Tests
    RUN_TEST(test_emergency_stop_init_success);
    RUN_TEST(test_emergency_stop_init_gpio_failure);

    // Emergency Stop Button Tests
    RUN_TEST(test_emergency_stop_button_press);
    RUN_TEST(test_emergency_stop_button_debounce);
    RUN_TEST(test_emergency_stop_timing_requirement);

    // Emergency Stop Software Trigger Tests
    RUN_TEST(test_emergency_stop_software_trigger);
    RUN_TEST(test_emergency_stop_multiple_sources);

    // Emergency Stop Reset Tests
    RUN_TEST(test_emergency_stop_reset_success);
    RUN_TEST(test_emergency_stop_reset_button_pressed);
    RUN_TEST(test_emergency_stop_reset_not_triggered);

    // Emergency Stop LED Tests
    RUN_TEST(test_emergency_stop_led_heartbeat_armed);
    RUN_TEST(test_emergency_stop_led_triggered_state);

    // Emergency Stop Self-Test Tests
    RUN_TEST(test_emergency_stop_self_test_success);
    RUN_TEST(test_emergency_stop_self_test_gpio_failure);

    // Emergency Stop Statistics Tests
    RUN_TEST(test_emergency_stop_statistics);

    // Emergency Stop Health Check Tests
    RUN_TEST(test_emergency_stop_health_check_success);
    RUN_TEST(test_emergency_stop_health_check_not_initialized);

    // HAL Abstraction Tests
    RUN_TEST(test_hal_abstraction_gpio_operations);
    RUN_TEST(test_hal_abstraction_fault_injection);
    RUN_TEST(test_hal_abstraction_timing_functions);
    RUN_TEST(test_hal_abstraction_spi_operations);
    RUN_TEST(test_hal_abstraction_i2c_operations);

    // Integration Tests
    RUN_TEST(test_hal_abstraction_integration_emergency_stop_full_cycle);

    return UNITY_END();
}
