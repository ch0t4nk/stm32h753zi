/**
 * @file test_motor_characterization.c
 * @brief Unit tests for motor characterization system
 *
 * @details This file contains comprehensive unit tests for the motor
 * characterization framework, validating parameter identification,
 * optimization algorithms, and L6470 configuration calculation.
 *
 * @author STM32H753ZI Motor Control Team
 * @date 2025-01-27
 * @version 1.0.0
 *
 * @note Uses Unity testing framework with mathematical validation
 */

#include "common/error_codes.h"
#include "config/motor_config.h"
#include "controllers/motor_characterization.h"
#include "hal_abstraction.h"
#include "telemetry/optimization_telemetry.h"
#include "unity.h"
#include <math.h>

// Test fixture variables
static MotorPhysicalParameters_t test_physical_params;
static OptimalControlParameters_t test_control_params;
static CharacterizationTestConfig_t test_config;
static CharacterizationResults_t test_results;
static CharacterizationDataSet_t test_dataset;

// Mock characterization data for mathematical validation
static const float mock_step_response_time[] = {0.0f,   0.001f, 0.002f, 0.003f,
                                                0.004f, 0.005f, 0.006f, 0.007f,
                                                0.008f, 0.009f, 0.010f};

static const float mock_step_response_position[] = {
    0.0f,   5.2f,   18.1f,  35.3f,  55.8f, 78.4f,
    101.2f, 122.8f, 142.1f, 158.3f, 171.0f};

static const float mock_step_response_velocity[] = {
    0.0f,    520.0f,  950.0f,  1200.0f, 1350.0f, 1420.0f,
    1450.0f, 1460.0f, 1455.0f, 1445.0f, 1430.0f};

// Expected parameter ranges for validation
#define EXPECTED_INERTIA_MIN 0.0001f    // kg⋅m²
#define EXPECTED_INERTIA_MAX 0.01f      // kg⋅m²
#define EXPECTED_DAMPING_MIN 0.001f     // N⋅m⋅s/rad
#define EXPECTED_DAMPING_MAX 0.1f       // N⋅m⋅s/rad
#define EXPECTED_TORQUE_CONST_MIN 0.01f // N⋅m/A
#define EXPECTED_TORQUE_CONST_MAX 0.5f  // N⋅m/A

/**
 * @brief Test setup function - runs before each test
 */
void setUp(void) {
    // Initialize test structures
    memset(&test_physical_params, 0, sizeof(test_physical_params));
    memset(&test_control_params, 0, sizeof(test_control_params));
    memset(&test_config, 0, sizeof(test_config));
    memset(&test_results, 0, sizeof(test_results));
    memset(&test_dataset, 0, sizeof(test_dataset));

    // Initialize mock HAL for testing
    HAL_Abstraction_Init_Mock();

    // Initialize characterization system
    SystemError_t result = motor_characterization_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

/**
 * @brief Test teardown function - runs after each test
 */
void tearDown(void) {
    // Cleanup characterization system
    motor_characterization_deinit();

    // Reset mock HAL
    HAL_Abstraction_Reset_Mock();
}

/**
 * @brief Test characterization system initialization
 */
void test_characterization_init_success(void) {
    SystemError_t result = motor_characterization_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify system is ready for characterization
    bool is_ready = motor_characterization_is_ready();
    TEST_ASSERT_TRUE(is_ready);
}

/**
 * @brief Test step response configuration
 */
void test_step_response_config_validation(void) {
    // Configure valid step response test
    test_config.test_type = CHARACTERIZATION_STEP_RESPONSE;
    test_config.motor_id = 0;
    test_config.step_amplitude_deg = 90.0f;
    test_config.max_duration_ms = 1000;
    test_config.sample_rate_hz = 1000;
    test_config.safety_enabled = true;

    SystemError_t result =
        motor_characterization_validate_config(&test_config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Test invalid motor ID
    test_config.motor_id = 5;
    result = motor_characterization_validate_config(&test_config);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);

    // Test excessive step amplitude
    test_config.motor_id = 0;
    test_config.step_amplitude_deg = 720.0f; // Too large
    result = motor_characterization_validate_config(&test_config);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);
}

/**
 * @brief Test frequency sweep configuration
 */
void test_frequency_sweep_config_validation(void) {
    // Configure valid frequency sweep test
    test_config.test_type = CHARACTERIZATION_FREQUENCY_SWEEP;
    test_config.motor_id = 0;
    test_config.frequency_start_hz = 0.1f;
    test_config.frequency_end_hz = 100.0f;
    test_config.amplitude_deg = 10.0f;
    test_config.points_per_decade = 10;
    test_config.safety_enabled = true;

    SystemError_t result =
        motor_characterization_validate_config(&test_config);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Test invalid frequency range
    test_config.frequency_start_hz = 100.0f;
    test_config.frequency_end_hz = 10.0f; // End < Start
    result = motor_characterization_validate_config(&test_config);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);
}

/**
 * @brief Test step response data analysis
 */
void test_step_response_analysis(void) {
    // Create mock step response dataset
    optimization_telemetry_init_dataset(&test_dataset, "Step Response Test",
                                        11);

    // Add mock step response data
    OptimizationTelemetryPacket_t packet;
    for (int i = 0; i < 11; i++) {
        memset(&packet, 0, sizeof(packet));
        packet.timestamp_us = (uint32_t)(mock_step_response_time[i] * 1000000);
        packet.encoder_position_deg = mock_step_response_position[i];
        packet.calculated_velocity_dps = mock_step_response_velocity[i];
        packet.motor_current_ma = 1000; // Constant current for step
        packet.data_valid = true;

        optimization_telemetry_add_sample(&test_dataset, &packet);
    }

    // Analyze step response
    SystemError_t result = motor_characterization_analyze_step_response(
        &test_dataset, &test_physical_params);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Validate calculated parameters are reasonable
    TEST_ASSERT_GREATER_THAN(EXPECTED_INERTIA_MIN,
                             test_physical_params.moment_of_inertia_kg_m2);
    TEST_ASSERT_LESS_THAN(EXPECTED_INERTIA_MAX,
                          test_physical_params.moment_of_inertia_kg_m2);

    TEST_ASSERT_GREATER_THAN(EXPECTED_DAMPING_MIN,
                             test_physical_params.viscous_damping_nm_s_rad);
    TEST_ASSERT_LESS_THAN(EXPECTED_DAMPING_MAX,
                          test_physical_params.viscous_damping_nm_s_rad);

    TEST_ASSERT_GREATER_THAN(0.0f, test_physical_params.system_bandwidth_hz);
    TEST_ASSERT_LESS_THAN(1000.0f, test_physical_params.system_bandwidth_hz);
}

/**
 * @brief Test time constant identification
 */
void test_time_constant_identification(void) {
    // Create exponential response data: response = 1 - exp(-t/tau)
    // where tau = 0.1 seconds (known test value)
    optimization_telemetry_init_dataset(&test_dataset, "Time Constant Test",
                                        20);

    const float tau = 0.1f;         // Time constant
    const float amplitude = 100.0f; // Response amplitude

    OptimizationTelemetryPacket_t packet;
    for (int i = 0; i < 20; i++) {
        float time = i * 0.01f; // 10ms intervals
        float response = amplitude * (1.0f - expf(-time / tau));

        memset(&packet, 0, sizeof(packet));
        packet.timestamp_us = (uint32_t)(time * 1000000);
        packet.encoder_position_deg = response;
        packet.data_valid = true;

        optimization_telemetry_add_sample(&test_dataset, &packet);
    }

    // Identify time constant
    float identified_tau;
    SystemError_t result = motor_characterization_identify_time_constant(
        &test_dataset, &identified_tau);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify identified time constant is close to expected value (±10%)
    float error_percent = fabsf(identified_tau - tau) / tau * 100.0f;
    TEST_ASSERT_LESS_THAN(10.0f, error_percent);
}

/**
 * @brief Test friction characterization
 */
void test_friction_characterization(void) {
    // Create dataset with varying velocity and torque data
    optimization_telemetry_init_dataset(&test_dataset, "Friction Test", 15);

    // Mock friction model: torque = coulomb + viscous * velocity
    const float coulomb_friction = 0.05f;  // N⋅m
    const float viscous_friction = 0.002f; // N⋅m⋅s/rad

    OptimizationTelemetryPacket_t packet;
    for (int i = 0; i < 15; i++) {
        float velocity = (i - 7) * 10.0f; // -70 to +70 rad/s
        float expected_torque =
            coulomb_friction * (velocity > 0 ? 1.0f : -1.0f) +
            viscous_friction * velocity;
        float current =
            expected_torque / 0.1f; // Assume 0.1 N⋅m/A torque constant

        memset(&packet, 0, sizeof(packet));
        packet.timestamp_us = i * 10000; // 10ms intervals
        packet.calculated_velocity_dps =
            velocity * 180.0f / M_PI; // Convert to deg/s
        packet.motor_current_ma = (int16_t)(current * 1000);
        packet.data_valid = true;

        optimization_telemetry_add_sample(&test_dataset, &packet);
    }

    // Characterize friction
    SystemError_t result = motor_characterization_analyze_friction(
        &test_dataset, &test_physical_params);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Validate friction parameters (±20% tolerance for mock data)
    float coulomb_error =
        fabsf(test_physical_params.coulomb_friction_nm - coulomb_friction) /
        coulomb_friction * 100.0f;
    TEST_ASSERT_LESS_THAN(20.0f, coulomb_error);

    float viscous_error = fabsf(test_physical_params.viscous_damping_nm_s_rad -
                                viscous_friction) /
                          viscous_friction * 100.0f;
    TEST_ASSERT_LESS_THAN(20.0f, viscous_error);
}

/**
 * @brief Test L6470 parameter optimization
 */
void test_l6470_parameter_optimization(void) {
    // Set known physical parameters
    test_physical_params.moment_of_inertia_kg_m2 = 0.001f;
    test_physical_params.viscous_damping_nm_s_rad = 0.01f;
    test_physical_params.coulomb_friction_nm = 0.05f;
    test_physical_params.torque_constant_nm_a = 0.1f;
    test_physical_params.max_current_a = 2.0f;
    test_physical_params.max_velocity_rad_s = 50.0f;

    // Calculate optimal L6470 parameters
    SystemError_t result = motor_characterization_calculate_l6470_params(
        &test_physical_params, &test_control_params);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Validate L6470 parameters are within valid ranges
    TEST_ASSERT_GREATER_THAN(0x00, test_control_params.kval_hold);
    TEST_ASSERT_LESS_THAN(0xFF, test_control_params.kval_hold);

    TEST_ASSERT_GREATER_THAN(0x00, test_control_params.kval_run);
    TEST_ASSERT_LESS_THAN(0xFF, test_control_params.kval_run);

    TEST_ASSERT_GREATER_THAN(0x00, test_control_params.kval_acc);
    TEST_ASSERT_LESS_THAN(0xFF, test_control_params.kval_acc);

    TEST_ASSERT_GREATER_THAN(0x00, test_control_params.kval_dec);
    TEST_ASSERT_LESS_THAN(0xFF, test_control_params.kval_dec);

    // Verify acceleration parameters
    TEST_ASSERT_GREATER_THAN(0x001, test_control_params.acc_steps_s2);
    TEST_ASSERT_LESS_THAN(0xFFF, test_control_params.acc_steps_s2);

    TEST_ASSERT_GREATER_THAN(0x001, test_control_params.dec_steps_s2);
    TEST_ASSERT_LESS_THAN(0xFFF, test_control_params.dec_steps_s2);
}

/**
 * @brief Test comprehensive characterization workflow
 */
void test_comprehensive_characterization(void) {
    // Configure comprehensive characterization
    test_config.test_type = CHARACTERIZATION_COMPREHENSIVE;
    test_config.motor_id = 0;
    test_config.step_amplitude_deg = 45.0f;
    test_config.frequency_start_hz = 0.1f;
    test_config.frequency_end_hz = 50.0f;
    test_config.max_duration_ms = 5000;
    test_config.sample_rate_hz = 1000;
    test_config.safety_enabled = true;

    // Mock successful hardware responses
    HAL_Abstraction_MockMotor_SetReadyState(0, true);
    HAL_Abstraction_MockSafety_SetEmergencyStop(false);

    // Run comprehensive characterization
    SystemError_t result =
        motor_characterization_run_comprehensive(&test_config, &test_results);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify results structure is populated
    TEST_ASSERT_TRUE(test_results.characterization_complete);
    TEST_ASSERT_TRUE(test_results.physical_params_valid);
    TEST_ASSERT_TRUE(test_results.control_params_valid);

    // Verify timing information
    TEST_ASSERT_GREATER_THAN(0, test_results.total_characterization_time_ms);
    TEST_ASSERT_LESS_THAN(test_config.max_duration_ms,
                          test_results.total_characterization_time_ms);
}

/**
 * @brief Test parameter validation
 */
void test_parameter_validation(void) {
    // Test valid parameters
    test_physical_params.moment_of_inertia_kg_m2 = 0.001f;
    test_physical_params.viscous_damping_nm_s_rad = 0.01f;
    test_physical_params.coulomb_friction_nm = 0.05f;
    test_physical_params.torque_constant_nm_a = 0.1f;
    test_physical_params.max_current_a = 2.0f;
    test_physical_params.max_velocity_rad_s = 50.0f;

    bool valid =
        motor_characterization_validate_physical_params(&test_physical_params);
    TEST_ASSERT_TRUE(valid);

    // Test invalid inertia (negative)
    test_physical_params.moment_of_inertia_kg_m2 = -0.001f;
    valid =
        motor_characterization_validate_physical_params(&test_physical_params);
    TEST_ASSERT_FALSE(valid);

    // Test invalid torque constant (zero)
    test_physical_params.moment_of_inertia_kg_m2 = 0.001f; // Reset
    test_physical_params.torque_constant_nm_a = 0.0f;
    valid =
        motor_characterization_validate_physical_params(&test_physical_params);
    TEST_ASSERT_FALSE(valid);
}

/**
 * @brief Test safety integration during characterization
 */
void test_safety_integration(void) {
    // Configure test with safety enabled
    test_config.test_type = CHARACTERIZATION_STEP_RESPONSE;
    test_config.motor_id = 0;
    test_config.step_amplitude_deg = 90.0f;
    test_config.safety_enabled = true;

    // Simulate emergency stop during characterization
    HAL_Abstraction_MockSafety_SetEmergencyStop(true);

    SystemError_t result =
        motor_characterization_run_comprehensive(&test_config, &test_results);
    TEST_ASSERT_EQUAL(ERROR_SAFETY_EMERGENCY_STOP, result);

    // Verify characterization is aborted
    TEST_ASSERT_FALSE(test_results.characterization_complete);

    // Clear emergency stop and retry
    HAL_Abstraction_MockSafety_SetEmergencyStop(false);
    HAL_Abstraction_MockMotor_SetReadyState(0, true);

    result =
        motor_characterization_run_comprehensive(&test_config, &test_results);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_TRUE(test_results.characterization_complete);
}

/**
 * @brief Test mathematical accuracy of parameter identification
 */
void test_mathematical_accuracy(void) {
    // Create precise test data with known mathematical relationship
    // Second-order system: H(s) = K / (s² + 2ζωₙs + ωₙ²)
    const float natural_freq = 10.0f; // rad/s
    const float damping_ratio = 0.5f; // dimensionless
    const float gain = 1.0f;          // dimensionless

    optimization_telemetry_init_dataset(&test_dataset, "Math Accuracy Test",
                                        50);

    // Generate step response for known second-order system
    OptimizationTelemetryPacket_t packet;
    for (int i = 0; i < 50; i++) {
        float time = i * 0.01f; // 10ms intervals

        // Analytical solution for step response of second-order system
        float damped_freq =
            natural_freq * sqrtf(1.0f - damping_ratio * damping_ratio);
        float phase = atanf(damped_freq / (damping_ratio * natural_freq));
        float envelope = expf(-damping_ratio * natural_freq * time);
        float response =
            gain *
            (1.0f - envelope * cosf(damped_freq * time + phase) / cosf(phase));

        memset(&packet, 0, sizeof(packet));
        packet.timestamp_us = (uint32_t)(time * 1000000);
        packet.encoder_position_deg =
            response * 180.0f / M_PI; // Convert to degrees
        packet.data_valid = true;

        optimization_telemetry_add_sample(&test_dataset, &packet);
    }

    // Analyze step response
    SystemError_t result = motor_characterization_analyze_step_response(
        &test_dataset, &test_physical_params);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Calculate expected bandwidth (should be close to natural frequency)
    float expected_bandwidth = natural_freq / (2.0f * M_PI);
    float bandwidth_error =
        fabsf(test_physical_params.system_bandwidth_hz - expected_bandwidth) /
        expected_bandwidth * 100.0f;

    // Allow ±15% error for numerical identification
    TEST_ASSERT_LESS_THAN(15.0f, bandwidth_error);
}

/**
 * @brief Test error handling and edge cases
 */
void test_error_handling(void) {
    // Test NULL pointer handling
    SystemError_t result = motor_characterization_analyze_step_response(
        NULL, &test_physical_params);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);

    result = motor_characterization_analyze_step_response(&test_dataset, NULL);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);

    // Test empty dataset
    optimization_telemetry_init_dataset(&test_dataset, "Empty Test", 10);
    result = motor_characterization_analyze_step_response(
        &test_dataset, &test_physical_params);
    TEST_ASSERT_EQUAL(ERROR_INSUFFICIENT_DATA, result);

    // Test insufficient data
    OptimizationTelemetryPacket_t packet;
    memset(&packet, 0, sizeof(packet));
    packet.data_valid = true;
    optimization_telemetry_add_sample(&test_dataset, &packet);

    result = motor_characterization_analyze_step_response(
        &test_dataset, &test_physical_params);
    TEST_ASSERT_EQUAL(ERROR_INSUFFICIENT_DATA, result);
}

/**
 * @brief Test performance requirements
 */
void test_performance_requirements(void) {
    // Create realistic dataset size
    optimization_telemetry_init_dataset(&test_dataset, "Performance Test",
                                        1000);

    // Fill with realistic data
    OptimizationTelemetryPacket_t packet;
    for (int i = 0; i < 1000; i++) {
        memset(&packet, 0, sizeof(packet));
        packet.timestamp_us = i * 1000; // 1ms intervals
        packet.encoder_position_deg =
            sinf(i * 0.1f) * 90.0f; // Sinusoidal motion
        packet.calculated_velocity_dps = cosf(i * 0.1f) * 9.0f;
        packet.motor_current_ma = 1000 + (int16_t)(sinf(i * 0.05f) * 200);
        packet.data_valid = true;

        optimization_telemetry_add_sample(&test_dataset, &packet);
    }

    // Measure analysis time
    uint32_t start_time = HAL_Abstraction_GetTick();
    SystemError_t result = motor_characterization_analyze_step_response(
        &test_dataset, &test_physical_params);
    uint32_t end_time = HAL_Abstraction_GetTick();

    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    uint32_t analysis_time = end_time - start_time;

    // Analysis should complete within 100ms for 1000 samples
    TEST_ASSERT_LESS_THAN(100000, analysis_time); // 100ms in µs

    printf("Analysis time for 1000 samples: %lu µs\n", analysis_time);
}

/**
 * @brief Main test runner function
 */
int main(void) {
    UNITY_BEGIN();

    // Basic functionality tests
    RUN_TEST(test_characterization_init_success);
    RUN_TEST(test_step_response_config_validation);
    RUN_TEST(test_frequency_sweep_config_validation);

    // Analysis algorithm tests
    RUN_TEST(test_step_response_analysis);
    RUN_TEST(test_time_constant_identification);
    RUN_TEST(test_friction_characterization);

    // L6470 optimization tests
    RUN_TEST(test_l6470_parameter_optimization);
    RUN_TEST(test_parameter_validation);

    // Integration tests
    RUN_TEST(test_comprehensive_characterization);
    RUN_TEST(test_safety_integration);

    // Mathematical accuracy tests
    RUN_TEST(test_mathematical_accuracy);

    // Performance and error handling tests
    RUN_TEST(test_performance_requirements);
    RUN_TEST(test_error_handling);

    return UNITY_END();
}

/**
 * @brief Test configuration summary
 *
 * This test suite validates:
 * - ✅ Characterization system initialization and configuration
 * - ✅ Step response analysis with mathematical validation
 * - ✅ Time constant identification accuracy (±10%)
 * - ✅ Friction characterization algorithms
 * - ✅ L6470 parameter optimization and validation
 * - ✅ Comprehensive characterization workflow
 * - ✅ Safety system integration and emergency handling
 * - ✅ Mathematical accuracy with known system models
 * - ✅ Performance requirements (<100ms for 1000 samples)
 * - ✅ Error handling and edge case management
 *
 * Expected Results:
 * - Mathematical identification accuracy within ±15%
 * - L6470 parameters within valid register ranges
 * - Analysis performance under 100ms for typical datasets
 * - Safety integration prevents unsafe characterization
 * - Comprehensive workflow produces valid optimization parameters
 */
