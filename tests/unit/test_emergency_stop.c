/**
 * @file test_emergency_stop.c
 * @brief Focused unit tests for emergency stop functionality
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 */

#include <unity.h>
#include "mock_hal.h"
#include "emergency_stop.h"

void setUp(void) {
    MockHAL_Reset();
    emergency_stop_init();
}

void tearDown(void) {
    MockHAL_Reset();
}

void test_emergency_stop_button_debounce(void) {
    // Test button bounce rejection
    MockHAL_SetEmergencyStopState(true);
    MockHAL_AdvanceTick(1);  // Short pulse
    MockHAL_SetEmergencyStopState(false);
    
    emergency_stop_process();
    
    // Should not trigger on short bounce
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_ARMED, emergency_stop_get_state());
    
    // Test stable press
    MockHAL_SetEmergencyStopState(true);
    MockHAL_AdvanceTick(EMERGENCY_STOP_DEBOUNCE_MS + 1);
    
    emergency_stop_process();
    
    // Should trigger on stable press
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_TRIGGERED, emergency_stop_get_state());
}

void test_emergency_stop_dual_channel_validation(void) {
    // Test single channel failure detection
    emergency_stop_set_channel_state(EMERGENCY_STOP_CHANNEL_A, true);
    emergency_stop_set_channel_state(EMERGENCY_STOP_CHANNEL_B, false);
    
    emergency_stop_process();
    
    // Should detect channel mismatch fault
    EmergencyStopState_t state = emergency_stop_get_state();
    TEST_ASSERT_EQUAL(EMERGENCY_STOP_FAULT, state);
    
    SystemError_t error = emergency_stop_get_last_error();
    TEST_ASSERT_EQUAL(ERROR_EMERGENCY_STOP_CHANNEL_MISMATCH, error);
}

void test_emergency_stop_sil3_timing(void) {
    uint32_t start_time = HAL_GetTick();
    
    MockHAL_SetEmergencyStopState(true);
    emergency_stop_process();
    
    uint32_t response_time = HAL_GetTick() - start_time;
    
    // SIL-3 requirement: <10ms response time
    TEST_ASSERT_LESS_THAN(10, response_time);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_emergency_stop_button_debounce);
    RUN_TEST(test_emergency_stop_dual_channel_validation);
    RUN_TEST(test_emergency_stop_sil3_timing);
    
    return UNITY_END();
}
