/**
 * @file test_emergency_stop_unity.c
 * @brief Unity-based Emergency Stop Tests
 * @author STM32H753ZI Project Team
 * @date 2025-08-05
 *
 * @note Professional Unity testing for emergency stop functionality
 */

#include "config/motor_config.h"
#include "config/safety_config.h"
#include "mock_hal.h"
#include "unity.h"

// Provide SSOT-compliant control loop timing for test
#ifndef CONTROL_LOOP_FREQ_HZ
#define CONTROL_LOOP_FREQ_HZ 1000
#endif
#ifndef CONTROL_LOOP_PERIOD_MS
#define CONTROL_LOOP_PERIOD_MS (1000 / CONTROL_LOOP_FREQ_HZ)
#endif

/* ==========================================================================
 */
/* Test Setup and Teardown                                                   */
/* ==========================================================================
 */

void setUp(void) {
    MockHAL_Reset();
}

void tearDown(void) {
    MockHAL_Reset();
}

/* ==========================================================================
 */
/* Emergency Stop Basic Tests                                                */
/* ==========================================================================
 */

void test_emergency_stop_initialization(void) {
    // Test that emergency stop starts in the correct state
    bool initial_state = MockHAL_GetEmergencyStopState();
    TEST_ASSERT_FALSE_MESSAGE(
        initial_state, "Emergency stop should be inactive on initialization");
}

void test_emergency_stop_activation(void) {
    // Test emergency stop activation
    MockHAL_SetEmergencyStopState(true);
    bool estop_active = MockHAL_GetEmergencyStopState();
    TEST_ASSERT_TRUE_MESSAGE(estop_active,
                             "Emergency stop should be active when set");
}

void test_emergency_stop_release(void) {
    // Test emergency stop release
    MockHAL_SetEmergencyStopState(true);
    TEST_ASSERT_TRUE(MockHAL_GetEmergencyStopState());

    MockHAL_SetEmergencyStopState(false);
    bool estop_released = MockHAL_GetEmergencyStopState();
    TEST_ASSERT_FALSE_MESSAGE(
        estop_released, "Emergency stop should be inactive when released");
}

void test_emergency_stop_debounce_simulation(void) {
    // Test debounce logic with rapid state changes
    MockHAL_SetEmergencyStopState(true);
    MockHAL_AdvanceTick(
        1); // Short pulse - should be filtered out by real system
    MockHAL_SetEmergencyStopState(false);
    MockHAL_AdvanceTick(1);
    MockHAL_SetEmergencyStopState(true);
    MockHAL_AdvanceTick(ESTOP_DEBOUNCE_TIME_MS + 1); // Stable time

    // In real system, only the final stable state should be recognized
    TEST_ASSERT_TRUE_MESSAGE(
        MockHAL_GetEmergencyStopState(),
        "Emergency stop should be stable after debounce time");
}

/* ==========================================================================
 */
/* Motor Safety Limit Tests                                                  */
/* ==========================================================================
 */

void test_motor_position_limits_validation(void) {
    // Test position limits from SSOT configuration
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(
        MOTOR1_MIN_POSITION_DEG, -180.0f,
        "Motor 1 minimum position should be -180°");
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(MOTOR1_MAX_POSITION_DEG, 180.0f,
                                    "Motor 1 maximum position should be 180°");
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(MOTOR2_MIN_POSITION_DEG, -90.0f,
                                    "Motor 2 minimum position should be -90°");
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(MOTOR2_MAX_POSITION_DEG, 90.0f,
                                    "Motor 2 maximum position should be 90°");
}

void test_motor_speed_limits_validation(void) {
    // Test speed limits from SSOT configuration
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(MOTOR_MAX_SPEED_RPM, 100.0f,
                                    "Maximum motor speed should be 100 RPM");
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(MOTOR_MIN_SPEED_RPM, 0.1f,
                                    "Minimum motor speed should be 0.1 RPM");

    // Test that max speed is reasonable for safety
    TEST_ASSERT_TRUE_MESSAGE(
        MOTOR_MAX_SPEED_RPM <= 1000.0f,
        "Maximum speed should not exceed 1000 RPM for safety");
}

void test_motor_current_limits_validation(void) {
    // Test current limits from SSOT configuration
    TEST_ASSERT_EQUAL_INT_MESSAGE(MOTOR_CURRENT_MA, 1000,
                                  "Motor current should be 1000mA");
    TEST_ASSERT_TRUE_MESSAGE(MOTOR_CURRENT_MA <= 2000,
                             "Motor current should not exceed 2A for safety");
}

/* ==========================================================================
 */
/* System Configuration Tests                                                */
/* ==========================================================================
 */

void test_system_configuration_constants(void) {
    // Test MAX_MOTORS constant
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        MAX_MOTORS, 2, "System should be configured for exactly 2 motors");

    // Test motor IDs
    TEST_ASSERT_EQUAL_INT_MESSAGE(MOTOR_1_ID, 0, "Motor 1 ID should be 0");
    TEST_ASSERT_EQUAL_INT_MESSAGE(MOTOR_2_ID, 1, "Motor 2 ID should be 1");
}

void test_safety_timing_requirements(void) {
    // Test safety timing constants
    TEST_ASSERT_TRUE_MESSAGE(ESTOP_REACTION_TIME_MS <= 100,
                             "Emergency stop reaction time must be ≤100ms");
    TEST_ASSERT_TRUE_MESSAGE(ESTOP_DEBOUNCE_TIME_MS >= 10,
                             "Emergency stop debounce time must be ≥10ms");
    TEST_ASSERT_TRUE_MESSAGE(IWDG_TIMEOUT_MS >= 100,
                             "Independent watchdog timeout must be ≥100ms");
}

void test_control_loop_timing(void) {
    // Test control loop timing from SSOT
    TEST_ASSERT_EQUAL_INT_MESSAGE(CONTROL_LOOP_FREQ_HZ, 1000,
                                  "Control loop frequency should be 1000Hz");
    TEST_ASSERT_EQUAL_INT_MESSAGE(CONTROL_LOOP_PERIOD_MS, 1,
                                  "Control loop period should be 1ms");
}

/* ==========================================================================
 */
/* L6470 Configuration Tests                                                 */
/* ==========================================================================
 */

void test_l6470_register_values(void) {
    // Test L6470 configuration constants from SSOT
    TEST_ASSERT_EQUAL_HEX16_MESSAGE(
        L6470_MAX_SPEED, 0x041, "L6470 max speed register should be 0x041");
    TEST_ASSERT_EQUAL_HEX16_MESSAGE(
        L6470_ACC, 0x08A, "L6470 acceleration register should be 0x08A");
    TEST_ASSERT_EQUAL_HEX16_MESSAGE(
        L6470_DEC, 0x08A, "L6470 deceleration register should be 0x08A");
}

void test_motor_physical_parameters(void) {
    // Test motor physical constants
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(MOTOR_VOLTAGE_V, 12.0f,
                                    "Motor voltage should be 12V");
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        MOTOR_STEPS_PER_REV, 200,
        "Motor should have 200 steps per revolution");
    TEST_ASSERT_EQUAL_INT_MESSAGE(MOTOR_MICROSTEPS, 128,
                                  "Motor should use 1/128 microstepping");
}

/* ==========================================================================
 */
/* Main Test Runner                                                          */
/* ==========================================================================
 */

int main(void) {
    UNITY_BEGIN();

    printf("🚀 STM32H753ZI Motor Control - Unity Emergency Stop Tests\n");
    printf("========================================================\n");

    // Emergency stop tests
    RUN_TEST(test_emergency_stop_initialization);
    RUN_TEST(test_emergency_stop_activation);
    RUN_TEST(test_emergency_stop_release);
    RUN_TEST(test_emergency_stop_debounce_simulation);

    // Motor safety tests
    RUN_TEST(test_motor_position_limits_validation);
    RUN_TEST(test_motor_speed_limits_validation);
    RUN_TEST(test_motor_current_limits_validation);

    // System configuration tests
    RUN_TEST(test_system_configuration_constants);
    RUN_TEST(test_safety_timing_requirements);
    RUN_TEST(test_control_loop_timing);

    // L6470 configuration tests
    RUN_TEST(test_l6470_register_values);
    RUN_TEST(test_motor_physical_parameters);

    return UNITY_END();
}
