/**
 * @file test_safety_systems.c
 * @brief Unit tests for safety systems using Unity framework
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 * 
 * Comprehensive unit tests for safety-critical systems including:
 * - Emergency stop functionality
 * - Watchdog management
 * - Fault monitoring
 * - Safety state management
 */

#include <unity.h>
#include "mock_hal.h"
#include "safety_system.h"
#include "emergency_stop.h"
#include "watchdog_manager.h"
#include "fault_monitor.h"

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

void setUp(void) {
    // Reset mock HAL state before each test
    MockHAL_Reset();
    
    // Initialize safety systems in test mode
    safety_system_init();
    emergency_stop_init();
    watchdog_init();
    fault_monitor_init();
}

void tearDown(void) {
    // Clean up after each test
    MockHAL_Reset();
}

// =============================================================================
// EMERGENCY STOP TESTS
// =============================================================================

void test_emergency_stop_initialization(void) {
    SystemError_t result = emergency_stop_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    EmergencyStopState_t state = emergency_stop_get_state();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_ARMED, state);
}

void test_emergency_stop_activation(void) {
    // Simulate emergency stop button press
    MockHAL_SetEmergencyStopState(true);
    
    // Process emergency stop
    emergency_stop_process();
    
    // Verify emergency stop is triggered
    EmergencyStopState_t state = emergency_stop_get_state();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_TRIGGERED, state);
    
    // Verify safety actions were taken
    TEST_ASSERT_TRUE(emergency_stop_is_active());
}

void test_emergency_stop_timing_requirement(void) {
    uint32_t start_time = HAL_GetTick();
    
    // Trigger emergency stop
    MockHAL_SetEmergencyStopState(true);
    emergency_stop_process();
    
    uint32_t response_time = HAL_GetTick() - start_time;
    
    // Verify response time is under 10ms (SIL-3 requirement)
    TEST_ASSERT_LESS_THAN(10, response_time);
}

void test_emergency_stop_reset_sequence(void) {
    // Trigger emergency stop
    MockHAL_SetEmergencyStopState(true);
    emergency_stop_process();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_TRIGGERED, emergency_stop_get_state());
    
    // Release emergency stop
    MockHAL_SetEmergencyStopState(false);
    
    // Attempt reset (should require manual reset)
    SystemError_t result = emergency_stop_reset();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Verify state transitions to reset pending
    EmergencyStopState_t state = emergency_stop_get_state();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_RESET_PENDING, state);
}

// =============================================================================
// WATCHDOG TESTS
// =============================================================================

void test_watchdog_initialization(void) {
    SystemError_t result = watchdog_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    WatchdogState_t state = watchdog_get_state();
    TEST_ASSERT_EQUAL(WATCHDOG_ACTIVE, state);
}

void test_watchdog_refresh_normal_operation(void) {
    uint32_t initial_count = MockHAL_GetWatchdogRefreshCount();
    
    // Refresh watchdog
    SystemError_t result = watchdog_refresh();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Verify HAL refresh was called
    uint32_t final_count = MockHAL_GetWatchdogRefreshCount();
    TEST_ASSERT_EQUAL(initial_count + 1, final_count);
}

void test_watchdog_timeout_detection(void) {
    // Advance time beyond watchdog timeout
    MockHAL_AdvanceTick(WATCHDOG_TIMEOUT_MS + 100);
    
    // Process watchdog
    watchdog_process();
    
    // Verify timeout was detected
    WatchdogState_t state = watchdog_get_state();
    TEST_ASSERT_EQUAL(WATCHDOG_TIMEOUT, state);
}

void test_watchdog_fault_recovery(void) {
    // Simulate watchdog timeout
    MockHAL_AdvanceTick(WATCHDOG_TIMEOUT_MS + 100);
    watchdog_process();
    TEST_ASSERT_EQUAL(WATCHDOG_TIMEOUT, watchdog_get_state());
    
    // Perform recovery
    SystemError_t result = watchdog_recover();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Verify recovery
    WatchdogState_t state = watchdog_get_state();
    TEST_ASSERT_EQUAL(WATCHDOG_ACTIVE, state);
}

// =============================================================================
// FAULT MONITOR TESTS
// =============================================================================

void test_fault_monitor_initialization(void) {
    SystemError_t result = fault_monitor_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    FaultState_t state = fault_monitor_get_state();
    TEST_ASSERT_EQUAL(FAULT_STATE_NORMAL, state);
}

void test_fault_detection_l6470_fault(void) {
    // Simulate L6470 fault pin activation
    MockHAL_SetFaultPinState(true);
    
    // Process fault monitoring
    fault_monitor_process();
    
    // Verify fault was detected
    FaultState_t state = fault_monitor_get_state();
    TEST_ASSERT_EQUAL(FAULT_STATE_L6470_FAULT, state);
    
    // Verify fault is reported as critical
    TEST_ASSERT_TRUE(fault_monitor_is_critical_fault());
}

void test_fault_logging(void) {
    // Trigger a fault
    MockHAL_SetFaultPinState(true);
    fault_monitor_process();
    
    // Verify fault was logged
    uint32_t fault_count = fault_monitor_get_fault_count();
    TEST_ASSERT_GREATER_THAN(0, fault_count);
    
    // Verify fault details
    FaultInfo_t fault_info;
    SystemError_t result = fault_monitor_get_last_fault(&fault_info);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(FAULT_L6470_FLAG, fault_info.fault_type);
}

void test_fault_recovery_sequence(void) {
    // Trigger fault
    MockHAL_SetFaultPinState(true);
    fault_monitor_process();
    TEST_ASSERT_EQUAL(FAULT_STATE_L6470_FAULT, fault_monitor_get_state());
    
    // Clear fault condition
    MockHAL_SetFaultPinState(false);
    
    // Attempt recovery
    SystemError_t result = fault_monitor_clear_fault();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Verify normal state restored
    FaultState_t state = fault_monitor_get_state();
    TEST_ASSERT_EQUAL(FAULT_STATE_NORMAL, state);
}

// =============================================================================
// INTEGRATED SAFETY SYSTEM TESTS
// =============================================================================

void test_safety_system_initialization(void) {
    SystemError_t result = safety_system_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    SafetyState_t state = safety_system_get_state();
    TEST_ASSERT_EQUAL(SAFETY_STATE_SAFE, state);
}

void test_safety_system_emergency_response(void) {
    // Trigger emergency stop
    MockHAL_SetEmergencyStopState(true);
    
    // Process safety system
    safety_system_process();
    
    // Verify safety state transitions to emergency
    SafetyState_t state = safety_system_get_state();
    TEST_ASSERT_EQUAL(SAFETY_STATE_EMERGENCY, state);
    
    // Verify all motors are stopped
    TEST_ASSERT_TRUE(safety_system_are_motors_stopped());
}

void test_safety_system_fault_response(void) {
    // Trigger L6470 fault
    MockHAL_SetFaultPinState(true);
    
    // Process safety system
    safety_system_process();
    
    // Verify safety state transitions to fault
    SafetyState_t state = safety_system_get_state();
    TEST_ASSERT_EQUAL(SAFETY_STATE_FAULT, state);
}

void test_safety_system_degraded_operation(void) {
    // Simulate non-critical fault
    safety_system_set_degraded_mode(true);
    
    // Process safety system
    safety_system_process();
    
    // Verify degraded state
    SafetyState_t state = safety_system_get_state();
    TEST_ASSERT_EQUAL(SAFETY_STATE_DEGRADED, state);
    
    // Verify limited operations are allowed
    TEST_ASSERT_FALSE(safety_system_are_motors_stopped());
    TEST_ASSERT_TRUE(safety_system_is_degraded());
}

// =============================================================================
// PERFORMANCE AND TIMING TESTS
// =============================================================================

void test_safety_system_response_time(void) {
    uint32_t start_time = HAL_GetTick();
    
    // Trigger emergency condition
    MockHAL_SetEmergencyStopState(true);
    safety_system_process();
    
    uint32_t response_time = HAL_GetTick() - start_time;
    
    // Verify response time meets safety requirements
    TEST_ASSERT_LESS_THAN(10, response_time); // <10ms for SIL-3
}

void test_safety_system_periodic_processing(void) {
    uint32_t initial_call_count = MockHAL_GetCallCount();
    
    // Simulate periodic processing for 100ms
    for (int i = 0; i < 10; i++) {
        MockHAL_AdvanceTick(10);
        safety_system_process();
    }
    
    // Verify processing occurred
    uint32_t final_call_count = MockHAL_GetCallCount();
    TEST_ASSERT_GREATER_THAN(initial_call_count, final_call_count);
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    UNITY_BEGIN();
    
    // Emergency stop tests
    RUN_TEST(test_emergency_stop_initialization);
    RUN_TEST(test_emergency_stop_activation);
    RUN_TEST(test_emergency_stop_timing_requirement);
    RUN_TEST(test_emergency_stop_reset_sequence);
    
    // Watchdog tests
    RUN_TEST(test_watchdog_initialization);
    RUN_TEST(test_watchdog_refresh_normal_operation);
    RUN_TEST(test_watchdog_timeout_detection);
    RUN_TEST(test_watchdog_fault_recovery);
    
    // Fault monitor tests
    RUN_TEST(test_fault_monitor_initialization);
    RUN_TEST(test_fault_detection_l6470_fault);
    RUN_TEST(test_fault_logging);
    RUN_TEST(test_fault_recovery_sequence);
    
    // Integrated safety system tests
    RUN_TEST(test_safety_system_initialization);
    RUN_TEST(test_safety_system_emergency_response);
    RUN_TEST(test_safety_system_fault_response);
    RUN_TEST(test_safety_system_degraded_operation);
    
    // Performance tests
    RUN_TEST(test_safety_system_response_time);
    RUN_TEST(test_safety_system_periodic_processing);
    
    return UNITY_END();
}
