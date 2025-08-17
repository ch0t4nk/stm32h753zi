/**
 * @file test_mock_safety_events.c
 * @brief Tests for programmable safety event simulation in mock HAL
 * @author STM32H753ZI Project
 * @date 2025-08-16
 */

#include "config/error_codes.h"  // For SystemError_t, SYSTEM_OK
#include "config/motor_config.h" // For MAX_MOTORS and motor constants
#include "mocks/mock_hal_abstraction.h"
#include "unity.h"
#include <stdbool.h>
#include <stdint.h>

void setUp(void) {
    MockHAL_Reset();
}

void tearDown(void) {
    // No teardown needed
}

extern bool estop_is_active(void);
extern bool mock_fault_pin_is_active(void);
extern bool mock_watchdog_timeout_triggered(void);

void test_Estop_ProgrammableSimulation(void) {
    // Normal activation
    MockHAL_SetEstopActive(true);
    TEST_ASSERT_TRUE(estop_is_active());
    MockHAL_SetEstopActive(false);
    TEST_ASSERT_FALSE(estop_is_active());

    // Edge: Set twice to same value
    MockHAL_SetEstopActive(false);
    TEST_ASSERT_FALSE(estop_is_active());
    MockHAL_SetEstopActive(true);
    MockHAL_SetEstopActive(true);
    TEST_ASSERT_TRUE(estop_is_active());

    // Edge: Reset after multiple sets
    MockHAL_SetEstopActive(false);
    TEST_ASSERT_FALSE(estop_is_active());
}

void test_FaultPin_ProgrammableSimulation(void) {
    // Normal activation
    MockHAL_SetFaultPinActive(true);
    TEST_ASSERT_TRUE(mock_fault_pin_is_active());
    MockHAL_SetFaultPinActive(false);
    TEST_ASSERT_FALSE(mock_fault_pin_is_active());

    // Edge: Set twice to same value
    MockHAL_SetFaultPinActive(false);
    TEST_ASSERT_FALSE(mock_fault_pin_is_active());
    MockHAL_SetFaultPinActive(true);
    MockHAL_SetFaultPinActive(true);
    TEST_ASSERT_TRUE(mock_fault_pin_is_active());

    // Edge: Reset after multiple sets
    MockHAL_SetFaultPinActive(false);
    TEST_ASSERT_FALSE(mock_fault_pin_is_active());
}

void test_Watchdog_ProgrammableSimulation(void) {
    // Normal activation
    MockHAL_TriggerWatchdogTimeout(true);
    TEST_ASSERT_TRUE(mock_watchdog_timeout_triggered());
    MockHAL_TriggerWatchdogTimeout(false);
    TEST_ASSERT_FALSE(mock_watchdog_timeout_triggered());

    // Edge: Set twice to same value
    MockHAL_TriggerWatchdogTimeout(false);
    TEST_ASSERT_FALSE(mock_watchdog_timeout_triggered());
    MockHAL_TriggerWatchdogTimeout(true);
    MockHAL_TriggerWatchdogTimeout(true);
    TEST_ASSERT_TRUE(mock_watchdog_timeout_triggered());

    // Edge: Reset after multiple sets
    MockHAL_TriggerWatchdogTimeout(false);
    TEST_ASSERT_FALSE(mock_watchdog_timeout_triggered());
}

void test_InvalidParameterSimulation(void) {
    // Negative: No crash or undefined behavior for repeated resets
    MockHAL_SetEstopActive(false);
    MockHAL_SetEstopActive(false);
    TEST_ASSERT_FALSE(estop_is_active());

    MockHAL_SetFaultPinActive(false);
    MockHAL_SetFaultPinActive(false);
    TEST_ASSERT_FALSE(mock_fault_pin_is_active());

    MockHAL_TriggerWatchdogTimeout(false);
    MockHAL_TriggerWatchdogTimeout(false);
    TEST_ASSERT_FALSE(mock_watchdog_timeout_triggered());
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_Estop_ProgrammableSimulation);
    RUN_TEST(test_FaultPin_ProgrammableSimulation);
    RUN_TEST(test_Watchdog_ProgrammableSimulation);
    RUN_TEST(test_InvalidParameterSimulation);
    return UNITY_END();
}
