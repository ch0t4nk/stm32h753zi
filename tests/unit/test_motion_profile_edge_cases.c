/**
 * @file test_motion_profile_edge_cases.c
 * @brief Unit tests for motion profile edge cases in the core software domain
 *
 * @details This file adds tests for edge cases and error handling in the
 * motion profile controller, including zero-duration, negative/invalid
 * parameters, and boundary conditions. Follows core-software.instructions.md.
 *
 * @author GitHub Copilot
 * @date 2025-08-13
 * @version 0.1.0
 *
 * @note Uses Unity testing framework. Update feature tracker after
 * implementation.
 */

#include "controllers/motion_profile.h"
#include "unity.h"
#include <string.h>

static MotionProfile_t test_profile;

void setUp(void) {
    memset(&test_profile, 0, sizeof(test_profile));
}

void tearDown(void) {
    // No teardown needed
}

void test_zero_duration_profile(void) {
    MotionProfileConfig_t config = {0};
    config.duration_ms = 0;
    SystemError_t result = motion_profile_init(&test_profile, &config);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);
}

void test_negative_parameters(void) {
    MotionProfileConfig_t config = {0};
    config.duration_ms = 1000;
    config.amplitude_deg = -90.0f;
    SystemError_t result = motion_profile_init(&test_profile, &config);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);
}

void test_boundary_conditions(void) {
    MotionProfileConfig_t config = {0};
    config.duration_ms = 1;
    config.amplitude_deg = 0.0f;
    SystemError_t result = motion_profile_init(&test_profile, &config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

void test_null_pointer_handling(void) {
    SystemError_t result = motion_profile_init(NULL, NULL);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_zero_duration_profile);
    RUN_TEST(test_negative_parameters);
    RUN_TEST(test_boundary_conditions);
    RUN_TEST(test_null_pointer_handling);
    return UNITY_END();
}
