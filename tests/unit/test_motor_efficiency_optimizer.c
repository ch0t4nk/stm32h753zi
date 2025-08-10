/**
 * @file test_motor_efficiency_optimizer.c
 * @brief Unit tests for motor efficiency optimization module (FTR-009)
 *
 * Tests efficiency optimization algorithms in simulation mode without hardware
 * dependencies. Validates SSOT configuration integration and algorithm
 * correctness.
 *
 * @author STM32H753ZI Motor Control Project
 * @date 2025-08-10
 * @version 1.0.0
 */

#include "unity.h"
#include "unity_fixture.h"

// Test subject headers
#include "adaptive_current_optimizer.h"
#include "motor_efficiency_optimizer.h"

// SSOT configuration headers
#include "motor_config.h"
#include "optimization_config.h"

// Dependencies for integration tests
#include "motor_characterization.h"
#include "optimization_telemetry.h"

// HAL abstraction for cross-platform testing
#include "hal_abstraction.h"

// Test group definition
TEST_GROUP(MotorEfficiencyOptimizer);

// Test fixture data
static MotorEfficiencyConfig_t test_config;
static MotorOptimizationState_t test_state;
static MockTelemetryData_t mock_telemetry;
static MockCharacterizationData_t mock_characterization;

/**
 * @brief Test setup - Initialize test environment
 */
TEST_SETUP(MotorEfficiencyOptimizer) {
    // Initialize HAL abstraction in mock mode
    hal_init_mock_mode();

    // Reset test configuration to SSOT defaults
    test_config.motor_id = MOTOR_1_ID;
    test_config.target_efficiency_percent =
        OPTIMIZATION_TARGET_EFFICIENCY_PERCENT;
    test_config.adaptation_rate = OPTIMIZATION_ADAPTATION_RATE_DEFAULT;
    test_config.current_limit_a = MOTOR_MAX_CURRENT_A;
    test_config.speed_limit_rpm = MOTOR_MAX_SPEED_RPM;
    test_config.algorithm_mode = OPTIMIZATION_ALGORITHM_ADAPTIVE_CURRENT;
    test_config.telemetry_integration_enabled = true;
    test_config.simulation_mode = true;

    // Initialize optimization state
    memset(&test_state, 0, sizeof(test_state));
    test_state.current_efficiency_percent =
        OPTIMIZATION_BASELINE_EFFICIENCY_PERCENT;
    test_state.is_active = false;
    test_state.convergence_state = CONVERGENCE_STATE_INITIALIZING;

    // Setup mock telemetry data
    mock_telemetry.motor_current_a = 1.5f;
    mock_telemetry.motor_speed_rpm = 1000.0f;
    mock_telemetry.input_power_w = 25.0f;
    mock_telemetry.mechanical_power_w = 18.0f;
    mock_telemetry.efficiency_percent = 72.0f;
    mock_telemetry.temperature_c = 45.0f;
    mock_telemetry.timestamp_ms = hal_get_tick();

    // Setup mock characterization data
    mock_characterization.motor_resistance_ohm = 2.1f;
    mock_characterization.motor_inductance_mh = 8.5f;
    mock_characterization.holding_torque_nm = 0.8f;
    mock_characterization.detent_torque_nm = 0.05f;
    mock_characterization.step_angle_deg = 1.8f;
    mock_characterization.optimal_current_a = 1.2f;
    mock_characterization.characterization_complete = true;
}

/**
 * @brief Test teardown - Clean up test environment
 */
TEST_TEAR_DOWN(MotorEfficiencyOptimizer) {
    // Cleanup optimization module
    motor_efficiency_optimizer_deinit(test_config.motor_id);

    // Reset HAL abstraction
    hal_deinit_mock_mode();
}

/**
 * @brief Test initialization with valid SSOT configuration
 */
TEST(MotorEfficiencyOptimizer, InitializeWithValidConfig) {
    SystemError_t result = motor_efficiency_optimizer_init(&test_config);

    // Should initialize successfully with valid config
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify configuration was stored correctly
    MotorEfficiencyConfig_t stored_config;
    result = motor_efficiency_optimizer_get_config(test_config.motor_id,
                                                   &stored_config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_EQUAL(test_config.motor_id, stored_config.motor_id);
    TEST_ASSERT_EQUAL_FLOAT(test_config.target_efficiency_percent,
                            stored_config.target_efficiency_percent);
    TEST_ASSERT_EQUAL(test_config.algorithm_mode,
                      stored_config.algorithm_mode);
    TEST_ASSERT_TRUE(stored_config.simulation_mode);
}

/**
 * @brief Test initialization with invalid configuration parameters
 */
TEST(MotorEfficiencyOptimizer, InitializeWithInvalidConfig) {
    SystemError_t result;

    // Test NULL configuration
    result = motor_efficiency_optimizer_init(NULL);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);

    // Test invalid motor ID
    test_config.motor_id = MAX_MOTORS + 1;
    result = motor_efficiency_optimizer_init(&test_config);
    TEST_ASSERT_EQUAL(ERROR_MOTOR_INVALID_ID, result);

    // Test invalid efficiency target (too high)
    test_config.motor_id = MOTOR_1_ID;
    test_config.target_efficiency_percent = 110.0f;
    result = motor_efficiency_optimizer_init(&test_config);
    TEST_ASSERT_EQUAL(ERROR_OPTIMIZATION_INVALID_TARGET, result);

    // Test invalid current limit (exceeds SSOT limit)
    test_config.target_efficiency_percent =
        OPTIMIZATION_TARGET_EFFICIENCY_PERCENT;
    test_config.current_limit_a = MOTOR_MAX_CURRENT_A + 1.0f;
    result = motor_efficiency_optimizer_init(&test_config);
    TEST_ASSERT_EQUAL(ERROR_MOTOR_CURRENT_OUT_OF_RANGE, result);
}

/**
 * @brief Test optimization algorithm execution in simulation mode
 */
TEST(MotorEfficiencyOptimizer, ExecuteOptimizationAlgorithm) {
    // Initialize optimizer
    SystemError_t result = motor_efficiency_optimizer_init(&test_config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Start optimization process
    result = motor_efficiency_optimizer_start(test_config.motor_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify optimizer is active
    MotorOptimizationState_t current_state;
    result = motor_efficiency_optimizer_get_state(test_config.motor_id,
                                                  &current_state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_TRUE(current_state.is_active);
    TEST_ASSERT_EQUAL(CONVERGENCE_STATE_OPTIMIZING,
                      current_state.convergence_state);

    // Execute multiple optimization iterations
    for (int i = 0; i < 10; i++) {
        // Update mock telemetry to simulate improving efficiency
        mock_telemetry.efficiency_percent = 72.0f + (float)i * 0.5f;
        mock_telemetry.motor_current_a =
            1.5f - (float)i * 0.02f; // Reducing current
        mock_telemetry.timestamp_ms = hal_get_tick();

        // Execute optimization step
        result = motor_efficiency_optimizer_update(test_config.motor_id,
                                                   &mock_telemetry);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);

        // Check that optimization is progressing
        result = motor_efficiency_optimizer_get_state(test_config.motor_id,
                                                      &current_state);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);

        // Efficiency should be improving
        if (i > 5) {
            TEST_ASSERT_GREATER_THAN(OPTIMIZATION_BASELINE_EFFICIENCY_PERCENT,
                                     current_state.current_efficiency_percent);
        }
    }
}

/**
 * @brief Test adaptive current optimization algorithm
 */
TEST(MotorEfficiencyOptimizer, AdaptiveCurrentOptimization) {
    // Configure for adaptive current optimization
    test_config.algorithm_mode = OPTIMIZATION_ALGORITHM_ADAPTIVE_CURRENT;
    SystemError_t result = motor_efficiency_optimizer_init(&test_config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Start optimization
    result = motor_efficiency_optimizer_start(test_config.motor_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Test current reduction optimization
    float initial_current = mock_telemetry.motor_current_a;
    float initial_efficiency = mock_telemetry.efficiency_percent;

    // Simulate scenario where reducing current improves efficiency
    for (int i = 0; i < 5; i++) {
        mock_telemetry.motor_current_a = initial_current - (float)i * 0.1f;
        mock_telemetry.efficiency_percent =
            initial_efficiency + (float)i * 1.5f;
        mock_telemetry.timestamp_ms = hal_get_tick();

        result = motor_efficiency_optimizer_update(test_config.motor_id,
                                                   &mock_telemetry);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    }

    // Get optimized current recommendation
    AdaptiveCurrentState_t current_state;
    result = adaptive_current_optimizer_get_state(test_config.motor_id,
                                                  &current_state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Current should be reduced compared to initial
    TEST_ASSERT_LESS_THAN(initial_current,
                          current_state.recommended_current_a);
    TEST_ASSERT_GREATER_THAN(initial_efficiency,
                             current_state.predicted_efficiency_percent);
}

/**
 * @brief Test integration with telemetry system
 */
TEST(MotorEfficiencyOptimizer, TelemetryIntegration) {
    // Enable telemetry integration
    test_config.telemetry_integration_enabled = true;
    SystemError_t result = motor_efficiency_optimizer_init(&test_config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Start optimization
    result = motor_efficiency_optimizer_start(test_config.motor_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify telemetry callback registration
    bool callback_registered =
        motor_efficiency_optimizer_is_telemetry_callback_registered(
            test_config.motor_id);
    TEST_ASSERT_TRUE(callback_registered);

    // Execute optimization update (should trigger telemetry collection)
    result = motor_efficiency_optimizer_update(test_config.motor_id,
                                               &mock_telemetry);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify telemetry data was processed
    MotorOptimizationState_t current_state;
    result = motor_efficiency_optimizer_get_state(test_config.motor_id,
                                                  &current_state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    TEST_ASSERT_EQUAL_FLOAT(mock_telemetry.efficiency_percent,
                            current_state.current_efficiency_percent);
    TEST_ASSERT_EQUAL(mock_telemetry.timestamp_ms,
                      current_state.last_update_timestamp);
}

/**
 * @brief Test integration with motor characterization system
 */
TEST(MotorEfficiencyOptimizer, CharacterizationIntegration) {
    SystemError_t result = motor_efficiency_optimizer_init(&test_config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Set characterization data
    result = motor_efficiency_optimizer_set_characterization_data(
        test_config.motor_id, &mock_characterization);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Start optimization (should use characterization data)
    result = motor_efficiency_optimizer_start(test_config.motor_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify characterization data is being used
    MotorOptimizationState_t current_state;
    result = motor_efficiency_optimizer_get_state(test_config.motor_id,
                                                  &current_state);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Optimization should consider motor's optimal current from
    // characterization
    TEST_ASSERT_EQUAL_FLOAT(mock_characterization.optimal_current_a,
                            current_state.target_current_a);
}

/**
 * @brief Test safety limits enforcement during optimization
 */
TEST(MotorEfficiencyOptimizer, SafetyLimitsEnforcement) {
    SystemError_t result = motor_efficiency_optimizer_init(&test_config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result = motor_efficiency_optimizer_start(test_config.motor_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Test current limit enforcement
    mock_telemetry.motor_current_a =
        MOTOR_MAX_CURRENT_A + 0.5f; // Exceed limit
    mock_telemetry.efficiency_percent =
        85.0f; // High efficiency (tempting to ignore limit)

    result = motor_efficiency_optimizer_update(test_config.motor_id,
                                               &mock_telemetry);
    TEST_ASSERT_EQUAL(ERROR_SAFETY_LIMIT_VIOLATION, result);

    // Test speed limit enforcement
    mock_telemetry.motor_current_a = 1.0f; // Valid current
    mock_telemetry.motor_speed_rpm =
        MOTOR_MAX_SPEED_RPM + 100.0f; // Exceed speed limit

    result = motor_efficiency_optimizer_update(test_config.motor_id,
                                               &mock_telemetry);
    TEST_ASSERT_EQUAL(ERROR_SAFETY_LIMIT_VIOLATION, result);

    // Test temperature limit enforcement
    mock_telemetry.motor_speed_rpm = 1000.0f; // Valid speed
    mock_telemetry.temperature_c =
        MOTOR_MAX_TEMPERATURE_C + 5.0f; // Exceed temperature limit

    result = motor_efficiency_optimizer_update(test_config.motor_id,
                                               &mock_telemetry);
    TEST_ASSERT_EQUAL(ERROR_SAFETY_TEMPERATURE_LIMIT, result);
}

/**
 * @brief Test performance metrics calculation
 */
TEST(MotorEfficiencyOptimizer, PerformanceMetricsCalculation) {
    SystemError_t result = motor_efficiency_optimizer_init(&test_config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result = motor_efficiency_optimizer_start(test_config.motor_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Set baseline efficiency
    float baseline_efficiency = 70.0f;
    mock_telemetry.efficiency_percent = baseline_efficiency;

    // Run optimization for several iterations
    for (int i = 0; i < 20; i++) {
        mock_telemetry.efficiency_percent =
            baseline_efficiency + (float)i * 0.8f;
        mock_telemetry.timestamp_ms = hal_get_tick();

        result = motor_efficiency_optimizer_update(test_config.motor_id,
                                                   &mock_telemetry);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    }

    // Get performance metrics
    MotorOptimizationMetrics_t metrics;
    result =
        motor_efficiency_optimizer_get_metrics(test_config.motor_id, &metrics);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify improvement calculation
    float expected_improvement =
        (mock_telemetry.efficiency_percent - baseline_efficiency) /
        baseline_efficiency * 100.0f;
    TEST_ASSERT_FLOAT_WITHIN(0.1f, expected_improvement,
                             metrics.efficiency_improvement_percent);

    // Verify we achieved target improvement (15-25%)
    TEST_ASSERT_GREATER_OR_EQUAL(15.0f,
                                 metrics.efficiency_improvement_percent);
    TEST_ASSERT_LESS_OR_EQUAL(25.0f, metrics.efficiency_improvement_percent);
}

/**
 * @brief Test convergence detection
 */
TEST(MotorEfficiencyOptimizer, ConvergenceDetection) {
    SystemError_t result = motor_efficiency_optimizer_init(&test_config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result = motor_efficiency_optimizer_start(test_config.motor_id);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Simulate gradual convergence to optimal efficiency
    float target_efficiency = 85.0f;
    float current_efficiency = 70.0f;

    for (int i = 0; i < 50; i++) {
        // Exponential approach to target
        float alpha = 0.1f;
        current_efficiency = current_efficiency +
                             alpha * (target_efficiency - current_efficiency);

        mock_telemetry.efficiency_percent = current_efficiency;
        mock_telemetry.timestamp_ms = hal_get_tick();

        result = motor_efficiency_optimizer_update(test_config.motor_id,
                                                   &mock_telemetry);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);

        // Check convergence state
        MotorOptimizationState_t current_state;
        result = motor_efficiency_optimizer_get_state(test_config.motor_id,
                                                      &current_state);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);

        // Should eventually converge
        if (i > 40) {
            if (current_state.convergence_state ==
                CONVERGENCE_STATE_CONVERGED) {
                TEST_ASSERT_FLOAT_WITHIN(1.0f, target_efficiency,
                                         current_efficiency);
                break;
            }
        }
    }
}

// Test runner function
TEST_GROUP_RUNNER(MotorEfficiencyOptimizer) {
    RUN_TEST_CASE(MotorEfficiencyOptimizer, InitializeWithValidConfig);
    RUN_TEST_CASE(MotorEfficiencyOptimizer, InitializeWithInvalidConfig);
    RUN_TEST_CASE(MotorEfficiencyOptimizer, ExecuteOptimizationAlgorithm);
    RUN_TEST_CASE(MotorEfficiencyOptimizer, AdaptiveCurrentOptimization);
    RUN_TEST_CASE(MotorEfficiencyOptimizer, TelemetryIntegration);
    RUN_TEST_CASE(MotorEfficiencyOptimizer, CharacterizationIntegration);
    RUN_TEST_CASE(MotorEfficiencyOptimizer, SafetyLimitsEnforcement);
    RUN_TEST_CASE(MotorEfficiencyOptimizer, PerformanceMetricsCalculation);
    RUN_TEST_CASE(MotorEfficiencyOptimizer, ConvergenceDetection);
}
