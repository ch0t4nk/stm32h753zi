/**
 * @file test_hal_abstraction_integration.c
 * @brief Integration tests for HAL abstraction with mock implementation
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 *
 * This file tests the HAL abstraction interface using the mock implementation
 * to ensure the interface works correctly and all functions are properly
 * exposed for testing.
 */

#include "hal_abstraction/hal_abstraction.h"
#include "unity.h"

/* ==========================================================================
 */
/* Mock Control Functions                                                    */
/* ==========================================================================
 */

extern void HAL_Abstraction_Mock_Reset(void);
extern void HAL_Abstraction_Mock_SetGPIOState(HAL_GPIO_Port_t port,
                                              uint32_t pin,
                                              HAL_GPIO_State_t state);
extern void HAL_Abstraction_Mock_InjectSPIFailure(bool enable);
extern void HAL_Abstraction_Mock_InjectI2CFailure(bool enable);
extern void HAL_Abstraction_Mock_AdvanceTick(uint32_t increment);

/* ==========================================================================
 */
/* Test Setup and Teardown                                                  */
/* ==========================================================================
 */

void setUp(void) {
    HAL_Abstraction_Mock_Reset();
}

void tearDown(void) {
    // Reset any fault injection
    HAL_Abstraction_Mock_InjectSPIFailure(false);
    HAL_Abstraction_Mock_InjectI2CFailure(false);
}

/* ==========================================================================
 */
/* GPIO Tests                                                               */
/* ==========================================================================
 */

void test_GPIO_Write_Valid_Parameters(void) {
    SystemError_t result =
        HAL_Abstraction_GPIO_Write(HAL_GPIO_PORT_A, 0, HAL_GPIO_STATE_SET);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result =
        HAL_Abstraction_GPIO_Write(HAL_GPIO_PORT_B, 15, HAL_GPIO_STATE_RESET);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_GPIO_Read_Valid_Parameters(void) {
    HAL_GPIO_State_t state;

    // Set known state and verify read
    HAL_Abstraction_Mock_SetGPIOState(HAL_GPIO_PORT_A, 0, HAL_GPIO_STATE_SET);
    SystemError_t result =
        HAL_Abstraction_GPIO_Read(HAL_GPIO_PORT_A, 0, &state);

    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(HAL_GPIO_STATE_SET, state);
}

void test_GPIO_Read_NULL_Pointer(void) {
    SystemError_t result = HAL_Abstraction_GPIO_Read(HAL_GPIO_PORT_A, 0, NULL);
    TEST_ASSERT_EQUAL(ERROR_NULL_POINTER, result);
}

void test_GPIO_Toggle_Valid_Parameters(void) {
    SystemError_t result = HAL_Abstraction_GPIO_Toggle(HAL_GPIO_PORT_A, 0);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

/* ==========================================================================
 */
/* SPI Tests                                                                */
/* ==========================================================================
 */

void test_SPI_Init_Valid_Instance(void) {
    SystemError_t result = HAL_Abstraction_SPI_Init(HAL_SPI_INSTANCE_1);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_SPI_Transmit_Valid_Parameters(void) {
    uint8_t test_data[] = {0x01, 0x02, 0x03, 0x04};

    SystemError_t result = HAL_Abstraction_SPI_Transmit(
        HAL_SPI_INSTANCE_1, test_data, sizeof(test_data), 1000);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_SPI_Transmit_NULL_Data(void) {
    SystemError_t result =
        HAL_Abstraction_SPI_Transmit(HAL_SPI_INSTANCE_1, NULL, 4, 1000);
    TEST_ASSERT_EQUAL(ERROR_NULL_POINTER, result);
}

void test_SPI_TransmitReceive_Valid_Transaction(void) {
    uint8_t tx_data[] = {0xAA, 0xBB};
    uint8_t rx_data[2] = {0};

    HAL_SPI_Transaction_t transaction = {.tx_data = tx_data,
                                         .rx_data = rx_data,
                                         .data_size = sizeof(tx_data),
                                         .timeout_ms = 1000};

    SystemError_t result =
        HAL_Abstraction_SPI_TransmitReceive(HAL_SPI_INSTANCE_1, &transaction);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify mock filled rx_data with test pattern
    TEST_ASSERT_EQUAL(0xA5, rx_data[0]); // Mock uses 0xA5 + index pattern
    TEST_ASSERT_EQUAL(0xA6, rx_data[1]);
}

void test_SPI_Fault_Injection(void) {
    uint8_t test_data[] = {0x01, 0x02};

    // Enable fault injection
    HAL_Abstraction_Mock_InjectSPIFailure(true);

    SystemError_t result = HAL_Abstraction_SPI_Transmit(
        HAL_SPI_INSTANCE_1, test_data, sizeof(test_data), 1000);
    TEST_ASSERT_EQUAL(ERROR_HARDWARE_FAULT, result);
}

/* ==========================================================================
 */
/* I2C Tests                                                                */
/* ==========================================================================
 */

void test_I2C_Init_Valid_Instance(void) {
    SystemError_t result = HAL_Abstraction_I2C_Init(HAL_I2C_INSTANCE_1);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_I2C_MemWrite_Valid_Transaction(void) {
    uint8_t test_data[] = {0x12, 0x34};

    HAL_I2C_Transaction_t transaction = {.device_address = 0x50,
                                         .register_address = 0x10,
                                         .data = test_data,
                                         .data_size = sizeof(test_data),
                                         .timeout_ms = 1000};

    SystemError_t result =
        HAL_Abstraction_I2C_MemWrite(HAL_I2C_INSTANCE_1, &transaction);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_I2C_MemRead_AS5600_Simulation(void) {
    uint8_t rx_data[2] = {0};

    HAL_I2C_Transaction_t transaction = {
        .device_address = 0x36,   // AS5600 address
        .register_address = 0x0C, // RAW_ANGLE register
        .data = rx_data,
        .data_size = sizeof(rx_data),
        .timeout_ms = 1000};

    SystemError_t result =
        HAL_Abstraction_I2C_MemRead(HAL_I2C_INSTANCE_1, &transaction);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify mock simulated AS5600 response
    TEST_ASSERT_EQUAL(0x12, rx_data[0]); // Mock angle high byte
    TEST_ASSERT_EQUAL(0x34, rx_data[1]); // Mock angle low byte
}

void test_I2C_Fault_Injection(void) {
    uint8_t test_data[] = {0x01, 0x02};

    HAL_I2C_Transaction_t transaction = {.device_address = 0x50,
                                         .register_address = 0x10,
                                         .data = test_data,
                                         .data_size = sizeof(test_data),
                                         .timeout_ms = 1000};

    // Enable fault injection
    HAL_Abstraction_Mock_InjectI2CFailure(true);

    SystemError_t result =
        HAL_Abstraction_I2C_MemWrite(HAL_I2C_INSTANCE_1, &transaction);
    TEST_ASSERT_EQUAL(ERROR_HARDWARE_FAULT, result);
}

/* ==========================================================================
 */
/* Timing Tests                                                             */
/* ==========================================================================
 */

void test_GetTick_Returns_Valid_Tick(void) {
    uint32_t tick = HAL_Abstraction_GetTick();
    TEST_ASSERT_GREATER_OR_EQUAL(0, tick);
}

void test_Delay_Advances_Tick(void) {
    uint32_t initial_tick = HAL_Abstraction_GetTick();

    HAL_Abstraction_Delay(50);

    uint32_t final_tick = HAL_Abstraction_GetTick();
    TEST_ASSERT_GREATER_OR_EQUAL(initial_tick + 50, final_tick);
}

void test_GetMicroseconds_Returns_Valid_Time(void) {
    uint32_t microseconds = HAL_Abstraction_GetMicroseconds();
    TEST_ASSERT_GREATER_OR_EQUAL(0, microseconds);
}

void test_Mock_AdvanceTick_Function(void) {
    uint32_t initial_tick = HAL_Abstraction_GetTick();

    HAL_Abstraction_Mock_AdvanceTick(100);

    uint32_t final_tick = HAL_Abstraction_GetTick();
    TEST_ASSERT_EQUAL(initial_tick + 100, final_tick);
}

/* ==========================================================================
 */
/* System Control Tests                                                     */
/* ==========================================================================
 */

void test_HAL_Init_Success(void) {
    SystemError_t result = HAL_Abstraction_Init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_Watchdog_Init_Success(void) {
    SystemError_t result = HAL_Abstraction_Watchdog_Init(1000);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_Watchdog_Refresh_Success(void) {
    SystemError_t result = HAL_Abstraction_Watchdog_Refresh();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

/* ==========================================================================
 */
/* Main Test Runner                                                         */
/* ==========================================================================
 */

int main(void) {
    UNITY_BEGIN();

    // GPIO Tests
    RUN_TEST(test_GPIO_Write_Valid_Parameters);
    RUN_TEST(test_GPIO_Read_Valid_Parameters);
    RUN_TEST(test_GPIO_Read_NULL_Pointer);
    RUN_TEST(test_GPIO_Toggle_Valid_Parameters);

    // SPI Tests
    RUN_TEST(test_SPI_Init_Valid_Instance);
    RUN_TEST(test_SPI_Transmit_Valid_Parameters);
    RUN_TEST(test_SPI_Transmit_NULL_Data);
    RUN_TEST(test_SPI_TransmitReceive_Valid_Transaction);
    RUN_TEST(test_SPI_Fault_Injection);

    // I2C Tests
    RUN_TEST(test_I2C_Init_Valid_Instance);
    RUN_TEST(test_I2C_MemWrite_Valid_Transaction);
    RUN_TEST(test_I2C_MemRead_AS5600_Simulation);
    RUN_TEST(test_I2C_Fault_Injection);

    // Timing Tests
    RUN_TEST(test_GetTick_Returns_Valid_Tick);
    RUN_TEST(test_Delay_Advances_Tick);
    RUN_TEST(test_GetMicroseconds_Returns_Valid_Time);
    RUN_TEST(test_Mock_AdvanceTick_Function);

    // System Control Tests
    RUN_TEST(test_HAL_Init_Success);
    RUN_TEST(test_Watchdog_Init_Success);
    RUN_TEST(test_Watchdog_Refresh_Success);

    return UNITY_END();
}
