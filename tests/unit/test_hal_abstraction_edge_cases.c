/**
 * @file test_hal_abstraction_edge_cases.c
 * @brief Unit tests for HAL abstraction edge cases in the hardware domain
 *
 * @details This file adds tests for error handling, null pointers, and
 * mock/stub coverage in the HAL abstraction layer. Follows
 * hardware.instructions.md and ssot-config.instructions.md.
 *
 * @author GitHub Copilot
 * @date 2025-08-13
 * @version 0.1.0
 *
 * @note Uses Unity testing framework. Update feature tracker after
 * implementation.
 */

#include "hal_abstraction.h"
#include "unity.h"
#include <string.h>

void setUp(void) {
}
void tearDown(void) {
}

void test_null_pointer_gpio_write(void) {
    // Should not crash or write if port is NULL
    SystemError_t result = HAL_Abstraction_GPIO_Write(NULL, 0, 1);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);
}

void test_null_pointer_gpio_read(void) {
    // Should not crash or read if port is NULL
    int value = HAL_Abstraction_GPIO_Read(NULL, 0);
    TEST_ASSERT_EQUAL(-1, value);
}

void test_mock_tick_and_delay(void) {
    // Should return increasing values and not crash
    uint32_t t1 = HAL_Abstraction_GetTick();
    HAL_Abstraction_Delay(1);
    uint32_t t2 = HAL_Abstraction_GetTick();
    TEST_ASSERT_TRUE(t2 >= t1);
}

void test_mock_spi_transfer_null(void) {
    // Should handle NULL pointers gracefully
    SystemError_t result = HAL_Abstraction_SPI_Transfer(NULL, NULL, NULL, 0);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_null_pointer_gpio_write);
    RUN_TEST(test_null_pointer_gpio_read);
    RUN_TEST(test_mock_tick_and_delay);
    RUN_TEST(test_mock_spi_transfer_null);
    return UNITY_END();
}
