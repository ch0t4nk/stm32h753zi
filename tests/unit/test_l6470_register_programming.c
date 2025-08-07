/**
 * @file test_l6470_register_programming.c
 * @brief Unit tests for L6470 register programming validation
 * @author STM32H753ZI Project Team
 * @date 2025-08-07
 *
 * @note Phase 4A System Integration Testing
 * Tests complete L6470 register programming using simulation framework
 */

#include "common/system_state.h"
#include "drivers/l6470/l6470_driver.h"
#include "drivers/l6470/l6470_registers.h"
#include "hal_abstraction/hal_abstraction.h"
#include "unity.h"
#include <string.h>

/* ==========================================================================
 */
/* Test Configuration                                                        */
/* ==========================================================================
 */

#define TEST_MOTOR_ID_1 0
#define TEST_MOTOR_ID_2 1
#define MAX_MOTORS 2

/* ==========================================================================
 */
/* Test Setup and Teardown                                                  */
/* ==========================================================================
 */

void setUp(void) {
    // Initialize HAL abstraction for simulation
    SystemError_t result = HAL_Abstraction_Init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Initialize L6470 driver with simulation backend
    result = l6470_driver_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void tearDown(void) {
    // Clean up after each test
    l6470_driver_deinit();
    HAL_Abstraction_Deinit();
}

/* ==========================================================================
 */
/* L6470 Register Programming Tests                                         */
/* ==========================================================================
 */

/**
 * @brief Test basic register read/write operations
 */
void test_l6470_basic_register_access(void) {
    printf("\n🧪 Testing L6470 basic register read/write operations...\n");

    SystemError_t result;
    uint32_t test_value = 0x12345;
    uint32_t read_value = 0;

    // Test writing to ABS_POS register (current position)
    result =
        l6470_set_parameter(TEST_MOTOR_ID_1, L6470_REG_ABS_POS, test_value);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Test reading back the value
    result =
        l6470_get_parameter(TEST_MOTOR_ID_1, L6470_REG_ABS_POS, &read_value);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify the written value matches the read value
    // Note: ABS_POS is 22-bit register, so mask appropriately
    TEST_ASSERT_EQUAL(test_value & 0x3FFFFF, read_value & 0x3FFFFF);

    printf("✅ Basic register access: PASS\n");
}

/**
 * @brief Test motor configuration register programming
 */
void test_l6470_motor_configuration_registers(void) {
    printf("\n🧪 Testing L6470 motor configuration register programming...\n");

    SystemError_t result;

    // Test MAX_SPEED register (16-bit, max speed in step/s)
    uint32_t max_speed = 0x3FF; // 1023 step/s
    result =
        l6470_set_parameter(TEST_MOTOR_ID_1, L6470_REG_MAX_SPEED, max_speed);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    uint32_t read_max_speed = 0;
    result = l6470_get_parameter(TEST_MOTOR_ID_1, L6470_REG_MAX_SPEED,
                                 &read_max_speed);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(max_speed, read_max_speed & 0x3FF);

    // Test ACCELERATION register (12-bit)
    uint32_t acceleration = 0x500; // Acceleration value
    result = l6470_set_parameter(TEST_MOTOR_ID_1, L6470_REG_ACC, acceleration);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    uint32_t read_acceleration = 0;
    result = l6470_get_parameter(TEST_MOTOR_ID_1, L6470_REG_ACC,
                                 &read_acceleration);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(acceleration, read_acceleration & 0xFFF);

    // Test DECELERATION register (12-bit)
    uint32_t deceleration = 0x500; // Deceleration value
    result = l6470_set_parameter(TEST_MOTOR_ID_1, L6470_REG_DEC, deceleration);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    uint32_t read_deceleration = 0;
    result = l6470_get_parameter(TEST_MOTOR_ID_1, L6470_REG_DEC,
                                 &read_deceleration);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(deceleration, read_deceleration & 0xFFF);

    printf("✅ Motor configuration registers: PASS\n");
}

/**
 * @brief Test current control register programming
 */
void test_l6470_current_control_registers(void) {
    printf("\n🧪 Testing L6470 current control register programming...\n");

    SystemError_t result;

    // Test TVAL (torque regulation current) - 7-bit register
    uint32_t holding_current = 0x29; // Holding current setting
    result = l6470_set_parameter(TEST_MOTOR_ID_1, L6470_REG_TVAL_HOLD,
                                 holding_current);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    uint32_t read_holding_current = 0;
    result = l6470_get_parameter(TEST_MOTOR_ID_1, L6470_REG_TVAL_HOLD,
                                 &read_holding_current);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(holding_current, read_holding_current & 0x7F);

    // Test running current
    uint32_t running_current = 0x29; // Running current setting
    result = l6470_set_parameter(TEST_MOTOR_ID_1, L6470_REG_TVAL_RUN,
                                 running_current);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    uint32_t read_running_current = 0;
    result = l6470_get_parameter(TEST_MOTOR_ID_1, L6470_REG_TVAL_RUN,
                                 &read_running_current);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(running_current, read_running_current & 0x7F);

    // Test acceleration current
    uint32_t accel_current = 0x29; // Acceleration current setting
    result = l6470_set_parameter(TEST_MOTOR_ID_1, L6470_REG_TVAL_ACC,
                                 accel_current);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    uint32_t read_accel_current = 0;
    result = l6470_get_parameter(TEST_MOTOR_ID_1, L6470_REG_TVAL_ACC,
                                 &read_accel_current);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(accel_current, read_accel_current & 0x7F);

    printf("✅ Current control registers: PASS\n");
}

/**
 * @brief Test overcurrent protection register programming
 */
void test_l6470_overcurrent_protection_registers(void) {
    printf(
        "\n🧪 Testing L6470 overcurrent protection register programming...\n");

    SystemError_t result;

    // Test OCD_TH (overcurrent detection threshold) - 4-bit register
    uint32_t ocd_threshold = 0x8; // Overcurrent threshold
    result =
        l6470_set_parameter(TEST_MOTOR_ID_1, L6470_REG_OCD_TH, ocd_threshold);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    uint32_t read_ocd_threshold = 0;
    result = l6470_get_parameter(TEST_MOTOR_ID_1, L6470_REG_OCD_TH,
                                 &read_ocd_threshold);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(ocd_threshold, read_ocd_threshold & 0xF);

    // Test STALL_TH (stall detection threshold) - 7-bit register
    uint32_t stall_threshold = 0x40; // Stall threshold
    result = l6470_set_parameter(TEST_MOTOR_ID_1, L6470_REG_STALL_TH,
                                 stall_threshold);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    uint32_t read_stall_threshold = 0;
    result = l6470_get_parameter(TEST_MOTOR_ID_1, L6470_REG_STALL_TH,
                                 &read_stall_threshold);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(stall_threshold, read_stall_threshold & 0x7F);

    printf("✅ Overcurrent protection registers: PASS\n");
}

/**
 * @brief Test complete motor initialization sequence
 */
void test_l6470_complete_motor_initialization(void) {
    printf("\n🧪 Testing L6470 complete motor initialization sequence...\n");

    SystemError_t result;

    // Test complete motor initialization for both motors
    for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
        printf("  Initializing Motor %d...\n", motor_id);

        result = l6470_init_motor(motor_id);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);

        // Verify key registers are properly configured
        uint32_t max_speed, acceleration, deceleration;

        result =
            l6470_get_parameter(motor_id, L6470_REG_MAX_SPEED, &max_speed);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);
        TEST_ASSERT_NOT_EQUAL(0, max_speed); // Should be non-zero after init

        result = l6470_get_parameter(motor_id, L6470_REG_ACC, &acceleration);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);
        TEST_ASSERT_NOT_EQUAL(0,
                              acceleration); // Should be non-zero after init

        result = l6470_get_parameter(motor_id, L6470_REG_DEC, &deceleration);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);
        TEST_ASSERT_NOT_EQUAL(0,
                              deceleration); // Should be non-zero after init

        printf("  ✅ Motor %d initialization: PASS\n", motor_id);
    }

    printf("✅ Complete motor initialization: PASS\n");
}

/**
 * @brief Test L6470 status register reading
 */
void test_l6470_status_register_reading(void) {
    printf("\n🧪 Testing L6470 status register reading...\n");

    SystemError_t result;
    uint16_t status_register = 0;

    // Test status register reading for both motors
    for (uint8_t motor_id = 0; motor_id < MAX_MOTORS; motor_id++) {
        result = l6470_get_status(motor_id, &status_register);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);

        // Status register should have valid bits set
        // At minimum, HiZ bit patterns should be valid
        TEST_ASSERT_NOT_EQUAL(0xFFFF, status_register); // Not all 1s
        TEST_ASSERT_NOT_EQUAL(0x0000, status_register); // Not all 0s

        printf("  Motor %d status: 0x%04X\n", motor_id, status_register);
    }

    printf("✅ Status register reading: PASS\n");
}

/**
 * @brief Test error handling for invalid register access
 */
void test_l6470_invalid_register_access(void) {
    printf(
        "\n🧪 Testing L6470 error handling for invalid register access...\n");

    SystemError_t result;
    uint32_t dummy_value = 0;

    // Test invalid motor ID
    result = l6470_set_parameter(MAX_MOTORS + 1, L6470_REG_ABS_POS, 0x1234);
    TEST_ASSERT_NOT_EQUAL(SYSTEM_OK, result);

    result =
        l6470_get_parameter(MAX_MOTORS + 1, L6470_REG_ABS_POS, &dummy_value);
    TEST_ASSERT_NOT_EQUAL(SYSTEM_OK, result);

    // Test invalid register address (if applicable)
    // Note: This depends on implementation - some drivers may accept any
    // register

    // Test NULL pointer
    result = l6470_get_parameter(TEST_MOTOR_ID_1, L6470_REG_ABS_POS, NULL);
    TEST_ASSERT_NOT_EQUAL(SYSTEM_OK, result);

    printf("✅ Error handling: PASS\n");
}

/**
 * @brief Test dual motor register programming
 */
void test_l6470_dual_motor_programming(void) {
    printf("\n🧪 Testing L6470 dual motor register programming...\n");

    SystemError_t result;

    // Set different values for each motor
    uint32_t motor1_speed = 0x200;
    uint32_t motor2_speed = 0x300;

    result = l6470_set_parameter(TEST_MOTOR_ID_1, L6470_REG_MAX_SPEED,
                                 motor1_speed);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result = l6470_set_parameter(TEST_MOTOR_ID_2, L6470_REG_MAX_SPEED,
                                 motor2_speed);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify each motor has its own value
    uint32_t read_motor1_speed = 0, read_motor2_speed = 0;

    result = l6470_get_parameter(TEST_MOTOR_ID_1, L6470_REG_MAX_SPEED,
                                 &read_motor1_speed);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(motor1_speed, read_motor1_speed & 0x3FF);

    result = l6470_get_parameter(TEST_MOTOR_ID_2, L6470_REG_MAX_SPEED,
                                 &read_motor2_speed);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(motor2_speed, read_motor2_speed & 0x3FF);

    // Ensure motors have different values
    TEST_ASSERT_NOT_EQUAL(read_motor1_speed, read_motor2_speed);

    printf("✅ Dual motor programming: PASS\n");
}

/* ==========================================================================
 */
/* Test Runner                                                              */
/* ==========================================================================
 */

int main(void) {
    printf("\n");
    printf(
        "================================================================\n");
    printf("🧪 PHASE 4A: L6470 REGISTER PROGRAMMING VALIDATION TESTS\n");
    printf(
        "================================================================\n");
    printf("Testing complete L6470 register programming using simulation\n");
    printf(
        "================================================================\n");

    UNITY_BEGIN();

    // Basic register access tests
    RUN_TEST(test_l6470_basic_register_access);

    // Motor configuration tests
    RUN_TEST(test_l6470_motor_configuration_registers);
    RUN_TEST(test_l6470_current_control_registers);
    RUN_TEST(test_l6470_overcurrent_protection_registers);

    // Integration tests
    RUN_TEST(test_l6470_complete_motor_initialization);
    RUN_TEST(test_l6470_status_register_reading);

    // Error handling tests
    RUN_TEST(test_l6470_invalid_register_access);

    // Dual motor tests
    RUN_TEST(test_l6470_dual_motor_programming);

    printf("\n================================================================"
           "\n");
    printf("🎯 PHASE 4A VALIDATION COMPLETE\n");
    printf(
        "================================================================\n");

    return UNITY_END();
}
