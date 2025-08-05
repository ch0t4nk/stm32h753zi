/**
 * @file test_safety_gtest.cpp
 * @brief GoogleTest-based unit tests for safety systems (C++ wrapper for C code)
 * @author STM32H753ZI Motor Control Project
 * @date 2024
 * 
 * GoogleTest tests for C safety systems code. This provides an alternative
 * to Unity-based testing when GoogleTest is available.
 */

#include <gtest/gtest.h>

extern "C" {
#include "mock_hal.h"
#include "safety_system.h"
#include "emergency_stop.h"
}

class SafetySystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        MockHAL_Reset();
        safety_system_init();
        emergency_stop_init();
    }
    
    void TearDown() override {
        MockHAL_Reset();
    }
};

TEST_F(SafetySystemTest, SystemInitialization) {
    SystemError_t result = safety_system_init();
    EXPECT_EQ(SYSTEM_OK, result);
    
    SafetyState_t state = safety_system_get_state();
    EXPECT_EQ(SAFETY_STATE_SAFE, state);
}

TEST_F(SafetySystemTest, EmergencyStopResponse) {
    // Trigger emergency stop
    MockHAL_SetEmergencyStopState(true);
    safety_system_process();
    
    // Verify safety state transitions to emergency
    SafetyState_t state = safety_system_get_state();
    EXPECT_EQ(SAFETY_STATE_EMERGENCY, state);
    
    // Verify motors are stopped
    EXPECT_TRUE(safety_system_are_motors_stopped());
}

TEST_F(SafetySystemTest, ResponseTimingRequirement) {
    uint32_t start_time = HAL_GetTick();
    
    // Trigger emergency condition
    MockHAL_SetEmergencyStopState(true);
    safety_system_process();
    
    uint32_t response_time = HAL_GetTick() - start_time;
    
    // Verify response time meets SIL-3 requirement (<10ms)
    EXPECT_LT(response_time, 10U);
}

// Main function for GoogleTest
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
