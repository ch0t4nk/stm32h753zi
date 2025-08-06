/**
 * @file test_hal_abstraction_basic.c
 * @brief Basic HAL Abstraction Layer Tests
 * @author STM32H753ZI Project Team
 * @date 2024
 *
 * This test demonstrates the core concepts of the HAL abstraction layer:
 * - Platform-independent hardware access
 * - Mock-based testing without hardware
 * - Clean separation between application and hardware layers
 */

#include "common/error_codes.h"
#include "config/hardware_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include "unity.h"
#include <stdbool.h>
#include <stdint.h>

void setUp(void) {
    // Reset mock state before each test
#ifdef UNITY_TESTING
    // Mock reset function (if available)
#endif
}

void tearDown(void) {
    // Clean up after each test
}

/**
 * @brief Test basic GPIO write operation through HAL abstraction
 */
void test_hal_abstraction_gpio_write_basic(void) {
    // Test GPIO write using HAL abstraction
    SystemError_t result = HAL_Abstraction_GPIO_WritePin(
        MOTOR1_CS_PORT, MOTOR1_CS_PIN, HAL_GPIO_PIN_SET);

    // With proper mock implementation, this should succeed
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

/**
 * @brief Test basic GPIO read operation through HAL abstraction
 */
void test_hal_abstraction_gpio_read_basic(void) {
    HAL_GPIO_State_t state;

    // Test GPIO read using HAL abstraction
    SystemError_t result = HAL_Abstraction_GPIO_ReadPin(
        ESTOP_BUTTON_PORT, ESTOP_BUTTON_PIN, &state);

    // With proper mock implementation, this should succeed
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    // State should be valid
    TEST_ASSERT(state == HAL_GPIO_PIN_RESET || state == HAL_GPIO_PIN_SET);
}

/**
 * @brief Test SPI transaction through HAL abstraction
 */
void test_hal_abstraction_spi_basic(void) {
    uint8_t tx_data[4] = {0x01, 0x02, 0x03, 0x04};
    uint8_t rx_data[4] = {0};

    HAL_SPI_Transaction_t transaction = {.tx_data = tx_data,
                                         .tx_size = sizeof(tx_data),
                                         .rx_data = rx_data,
                                         .rx_size = sizeof(rx_data),
                                         .timeout_ms = 100};

    // Test SPI transaction using HAL abstraction
    SystemError_t result =
        HAL_Abstraction_SPI_Transaction(SPI_MOTOR_BUS, &transaction);

    // With proper mock implementation, this should succeed
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

/**
 * @brief Test I2C transaction through HAL abstraction
 */
void test_hal_abstraction_i2c_basic(void) {
    uint8_t data[2] = {0x10, 0x20};

    HAL_I2C_Transaction_t transaction = {.device_address =
                                             0x36, // AS5600 encoder address
                                         .register_address = 0x0C,
                                         .data = data,
                                         .data_size = sizeof(data),
                                         .timeout_ms = 100};

    // Test I2C transaction using HAL abstraction
    SystemError_t result =
        HAL_Abstraction_I2C_Transaction(I2C_ENCODER_BUS, &transaction);

    // With proper mock implementation, this should succeed
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

/**
 * @brief Test timer functionality through HAL abstraction
 */
void test_hal_abstraction_timer_basic(void) {
    // Test getting system tick
    uint32_t tick1 = HAL_Abstraction_GetTick();

    // Tick should be a reasonable value
    TEST_ASSERT_GREATER_THAN(0, tick1);

    // Test delay function (if implemented)
    HAL_Abstraction_Delay(1); // 1ms delay

    uint32_t tick2 = HAL_Abstraction_GetTick();

    // Time should have advanced
    TEST_ASSERT_GREATER_OR_EQUAL(tick1, tick2);
}

/**
 * @brief Test error handling in HAL abstraction
 */
void test_hal_abstraction_error_handling(void) {
    // Test with invalid parameters
    SystemError_t result = HAL_Abstraction_GPIO_WritePin(
        999, 999, HAL_GPIO_PIN_SET); // Invalid port/pin

    // Should return error for invalid parameters
    TEST_ASSERT_NOT_EQUAL(SYSTEM_OK, result);
}

/**
 * @brief Test HAL abstraction concepts - platform independence
 */
void test_hal_abstraction_platform_independence(void) {
    // This test demonstrates that the same code works on:
    // 1. Real STM32H7 hardware (when using hal_abstraction_stm32h7.c)
    // 2. Host system with mocks (when using hal_abstraction_mock.c)
    // 3. Any future platform with its own implementation

    // GPIO operations
    SystemError_t gpio_result = HAL_Abstraction_GPIO_WritePin(
        MOTOR1_CS_PORT, MOTOR1_CS_PIN, HAL_GPIO_PIN_SET);

    // SPI operations
    uint8_t spi_data[2] = {0xFF, 0x00};
    HAL_SPI_Transaction_t spi_transaction = {.tx_data = spi_data,
                                             .tx_size = 2,
                                             .rx_data = spi_data,
                                             .rx_size = 2,
                                             .timeout_ms = 50};
    SystemError_t spi_result =
        HAL_Abstraction_SPI_Transaction(SPI_MOTOR_BUS, &spi_transaction);

    // Both operations should work regardless of underlying platform
    TEST_ASSERT_EQUAL(SYSTEM_OK, gpio_result);
    TEST_ASSERT_EQUAL(SYSTEM_OK, spi_result);

    // The beauty of HAL abstraction: This exact test runs identically on:
    // - STM32H7 hardware with real GPIO/SPI
    // - Host PC with mock implementations
    // - Any other platform with HAL abstraction implementation
}

/**
 * @brief Main test runner
 */
int main(void) {
    UNITY_BEGIN();

    // Basic functionality tests
    RUN_TEST(test_hal_abstraction_gpio_write_basic);
    RUN_TEST(test_hal_abstraction_gpio_read_basic);
    RUN_TEST(test_hal_abstraction_spi_basic);
    RUN_TEST(test_hal_abstraction_i2c_basic);
    RUN_TEST(test_hal_abstraction_timer_basic);

    // Error handling tests
    RUN_TEST(test_hal_abstraction_error_handling);

    // Conceptual tests
    RUN_TEST(test_hal_abstraction_platform_independence);

    return UNITY_END();
}
