/**
 * @file test_fault_monitor.c
 * @brief Focused unit tests for fault monitoring system
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 */

#include <unity.h>
#include "mock_hal.h"
#include "fault_monitor.h"

void setUp(void) {
    MockHAL_Reset();
    fault_monitor_init();
}

void tearDown(void) {
    MockHAL_Reset();
}

void test_fault_priority_handling(void) {
    // Inject multiple faults simultaneously
    fault_monitor_inject_fault(FAULT_L6470_FLAG);
    fault_monitor_inject_fault(FAULT_COMMUNICATION_TIMEOUT);
    fault_monitor_inject_fault(FAULT_SENSOR_DISCONNECTED);
    
    fault_monitor_process();
    
    // Verify highest priority fault is handled first
    FaultInfo_t fault_info;
    SystemError_t result = fault_monitor_get_active_fault(&fault_info);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(FAULT_L6470_FLAG, fault_info.fault_type); // Highest priority
}

void test_fault_persistence_tracking(void) {
    // Inject intermittent fault
    fault_monitor_inject_fault(FAULT_COMMUNICATION_TIMEOUT);
    fault_monitor_process();
    
    // Clear fault condition
    fault_monitor_clear_fault_condition(FAULT_COMMUNICATION_TIMEOUT);
    MockHAL_AdvanceTick(100);
    fault_monitor_process();
    
    // Re-inject same fault
    fault_monitor_inject_fault(FAULT_COMMUNICATION_TIMEOUT);
    fault_monitor_process();
    
    // Verify persistence counter incremented
    FaultStats_t stats = fault_monitor_get_fault_statistics(FAULT_COMMUNICATION_TIMEOUT);
    TEST_ASSERT_EQUAL(2, stats.occurrence_count);
}

void test_fault_escalation_logic(void) {
    FaultType_t test_fault = FAULT_SENSOR_DISCONNECTED;
    
    // Inject fault multiple times to trigger escalation
    for (int i = 0; i < FAULT_ESCALATION_THRESHOLD; i++) {
        fault_monitor_inject_fault(test_fault);
        fault_monitor_process();
        fault_monitor_clear_fault_condition(test_fault);
        MockHAL_AdvanceTick(100);
    }
    
    // Inject one more time to trigger escalation
    fault_monitor_inject_fault(test_fault);
    fault_monitor_process();
    
    // Verify fault was escalated to critical
    TEST_ASSERT_TRUE(fault_monitor_is_fault_escalated(test_fault));
}

void test_fault_recovery_validation(void) {
    // Inject L6470 fault
    MockHAL_SetFaultPinState(true);
    fault_monitor_process();
    TEST_ASSERT_EQUAL(FAULT_STATE_L6470_FAULT, fault_monitor_get_state());
    
    // Clear hardware condition
    MockHAL_SetFaultPinState(false);
    
    // Attempt recovery
    SystemError_t result = fault_monitor_clear_fault();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    
    // Verify recovery validation
    MockHAL_AdvanceTick(FAULT_RECOVERY_VALIDATION_TIME_MS);
    fault_monitor_process();
    
    TEST_ASSERT_EQUAL(FAULT_STATE_NORMAL, fault_monitor_get_state());
}

void test_fault_logging_capacity(void) {
    // Fill fault log to capacity
    for (int i = 0; i < FAULT_LOG_MAX_ENTRIES + 5; i++) {
        fault_monitor_inject_fault(FAULT_COMMUNICATION_TIMEOUT);
        fault_monitor_process();
        fault_monitor_clear_fault_condition(FAULT_COMMUNICATION_TIMEOUT);
        MockHAL_AdvanceTick(10);
    }
    
    // Verify log doesn't overflow
    uint32_t log_count = fault_monitor_get_log_count();
    TEST_ASSERT_LESS_OR_EQUAL(FAULT_LOG_MAX_ENTRIES, log_count);
    
    // Verify oldest entries were overwritten
    FaultInfo_t oldest_fault;
    SystemError_t result = fault_monitor_get_fault_by_index(0, &oldest_fault);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_GREATER_THAN(0, oldest_fault.timestamp);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_fault_priority_handling);
    RUN_TEST(test_fault_persistence_tracking);
    RUN_TEST(test_fault_escalation_logic);
    RUN_TEST(test_fault_recovery_validation);
    RUN_TEST(test_fault_logging_capacity);
    
    return UNITY_END();
}
