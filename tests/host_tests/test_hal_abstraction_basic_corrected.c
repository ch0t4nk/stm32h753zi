/**
 * @file test_hal_abstraction_basic_corrected.c
 * @brief Basic HAL Abstraction Layer Tests (Corrected)
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * Simple test to demonstrate HAL abstraction concepts:
 * - Platform-independent GPIO operations
 * - Hardware-agnostic SPI transactions
 * - I2C device communication abstraction
 * - Error handling verification
 * - Demonstrate mock vs real hardware behavior
 */

#include "../../src/config/hardware_config.h"
#include "../../src/hal_abstraction/hal_abstraction.h"
#include "unity.h"

/**
 * @brief Test setup function
 */
void setUp(void) {
    // Test setup - mocks automatically initialized
}

/**
 * @brief Test teardown function
 */
void tearDown(void) {
    // Test cleanup - mocks automatically reset
}

/**
 * @brief Test basic GPIO write functionality
 */
void test_hal_abstraction_gpio_write_basic(void) {
    printf("Testing GPIO write abstraction...\n");

    // Test GPIO write using HAL abstraction - using pin 9 as example (Motor
    // CS)
    SystemError_t result =
        HAL_Abstraction_GPIO_Write(HAL_GPIO_PORT_A, 9, HAL_GPIO_STATE_SET);

    // Should succeed with mock implementation
    TEST_ASSERT_EQUAL(ERROR_CODE_SUCCESS, result);

    printf("GPIO write test passed!\n");
}

/**
 * @brief Test basic GPIO read functionality
 */
void test_hal_abstraction_gpio_read_basic(void) {
    printf("Testing GPIO read abstraction...\n");

    HAL_GPIO_State_t state;
    // Test reading pin 10 as example (Motor FLAG)
    SystemError_t result =
        HAL_Abstraction_GPIO_Read(HAL_GPIO_PORT_A, 10, &state);

    // Should succeed with mock implementation
    TEST_ASSERT_EQUAL(ERROR_CODE_SUCCESS, result);
    // State should be valid (either high or low)
    TEST_ASSERT(state == HAL_GPIO_STATE_RESET || state == HAL_GPIO_STATE_SET);

    printf("GPIO read test passed! State: %d\n", state);
}

/**
 * @brief Test basic SPI transaction functionality
 */
void test_hal_abstraction_spi_basic(void) {
    printf("Testing SPI transaction abstraction...\n");

    uint8_t tx_data[] = {0x01, 0x02};
    uint8_t rx_data[2] = {0};

    HAL_SPI_Transaction_t transaction = {.tx_data = tx_data,
                                         .rx_data = rx_data,
                                         .data_size = sizeof(tx_data),
                                         .timeout_ms = 100};

    // Test SPI transaction using HAL abstraction
    SystemError_t result =
        HAL_Abstraction_SPI_TransmitReceive(HAL_SPI_INSTANCE_2, &transaction);

    // Should succeed with mock implementation
    TEST_ASSERT_EQUAL(ERROR_CODE_SUCCESS, result);

    printf("SPI transaction test passed!\n");
}

/**
 * @brief Test basic I2C transaction functionality
 */
void test_hal_abstraction_i2c_basic(void) {
    printf("Testing I2C transaction abstraction...\n");

    uint8_t data[] = {0x0C, 0x0D}; // Example register addresses

    HAL_I2C_Transaction_t transaction = {.device_address =
                                             0x36, // AS5600 address
                                         .register_address = 0x0C,
                                         .data = data,
                                         .data_size = sizeof(data),
                                         .timeout_ms = 100,
                                         .use_register_address = true};

    // Test I2C transaction using HAL abstraction
    SystemError_t result =
        HAL_Abstraction_I2C_MemRead(HAL_I2C_INSTANCE_1, &transaction);

    // Should succeed with mock implementation
    TEST_ASSERT_EQUAL(ERROR_CODE_SUCCESS, result);

    printf("I2C transaction test passed!\n");
}

/**
 * @brief Test HAL abstraction error handling
 */
void test_hal_abstraction_error_handling(void) {
    printf("Testing HAL abstraction error handling...\n");

    // Test with invalid parameters to trigger error codes
    HAL_GPIO_State_t state;

    // Test with invalid GPIO port (should return error)
    SystemError_t result =
        HAL_Abstraction_GPIO_Read(999, 999, &state); // Invalid port/pin

    // Should return an error code (not success)
    TEST_ASSERT_NOT_EQUAL(ERROR_CODE_SUCCESS, result);

    printf("Error handling test passed! Error code: %d\n", result);
}

/**
 * @brief Test HAL abstraction platform independence
 */
void test_hal_abstraction_platform_independence(void) {
    printf("Testing platform independence...\n");

    // This test demonstrates that the same API works whether we're running
    // on real hardware or mock implementation - the test code is identical

    // GPIO operation
    SystemError_t gpio_result = HAL_Abstraction_GPIO_Write(
        HAL_GPIO_PORT_A, 9, HAL_GPIO_STATE_SET); // Pin 9 example
    TEST_ASSERT_EQUAL(ERROR_CODE_SUCCESS, gpio_result);

    // SPI operation
    uint8_t spi_data[] = {0xAA, 0xBB};
    HAL_SPI_Transaction_t spi_transaction = {.tx_data = spi_data,
                                             .rx_data = spi_data,
                                             .data_size = 2,
                                             .timeout_ms = 50};

    SystemError_t spi_result = HAL_Abstraction_SPI_TransmitReceive(
        HAL_SPI_INSTANCE_2, &spi_transaction);
    TEST_ASSERT_EQUAL(ERROR_CODE_SUCCESS, spi_result);

    printf("Platform independence test passed!\n");
}

/**
 * @brief Unity test runner
 */
int main(void) {
    UNITY_BEGIN();

    printf("=== HAL Abstraction Basic Tests ===\n");
    printf(
        "This demonstrates the HAL abstraction working with mock hardware\n");
    printf("The same tests would work with real STM32H7 hardware!\n\n");

    RUN_TEST(test_hal_abstraction_gpio_write_basic);
    RUN_TEST(test_hal_abstraction_gpio_read_basic);
    RUN_TEST(test_hal_abstraction_spi_basic);
    RUN_TEST(test_hal_abstraction_i2c_basic);
    RUN_TEST(test_hal_abstraction_error_handling);
    RUN_TEST(test_hal_abstraction_platform_independence);

    printf("\n=== HAL Abstraction Tests Complete ===\n");

    return UNITY_END();
}
