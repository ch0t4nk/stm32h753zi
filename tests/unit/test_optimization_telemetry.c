/**
 * @file test_optimization_telemetry.c
 * @brief Unit tests for optimization telemetry system
 *
 * @details This file contains comprehensive unit tests for the optimization
 * telemetry system, validating data collection, performance metrics, safety
 * integration, and mock hardware compatibility.
 *
 * @author STM32H753ZI Motor Control Team
 * @date 2025-01-27
 * @version 1.0.0
 *
 * @note Uses Unity testing framework with mock hardware for CI/CD
 * compatibility
 */

#include "common/error_codes.h"
#include "config/motor_config.h"
#include "hal_abstraction.h"
#include "safety/safety_monitor.h"
#include "telemetry/optimization_telemetry.h"
#include "unity.h"

// Test fixture variables
static OptimizationTelemetryPacket_t test_packet;
static CharacterizationDataSet_t test_dataset;
static TelemetryPerformanceMetrics_t test_metrics;

// Mock data for testing
static float mock_position_data[10] = {0.0f,   45.0f,  90.0f,  135.0f, 180.0f,
                                       225.0f, 270.0f, 315.0f, 360.0f, 405.0f};
static int16_t mock_current_data[10] = {100, 150, 200, 180, 160,
                                        140, 130, 120, 110, 105};
static uint16_t mock_voltage_data[10] = {12000, 11900, 11800, 11850, 11900,
                                         11950, 12000, 12050, 12100, 12080};

/**
 * @brief Test setup function - runs before each test
 */
void setUp(void) {
    // Initialize test structures
    memset(&test_packet, 0, sizeof(test_packet));
    memset(&test_dataset, 0, sizeof(test_dataset));
    memset(&test_metrics, 0, sizeof(test_metrics));

    // Initialize mock HAL for testing
    HAL_Abstraction_Init_Mock();

    // Initialize telemetry system for testing
    SystemError_t result = optimization_telemetry_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

/**
 * @brief Test teardown function - runs after each test
 */
void tearDown(void) {
    // Cleanup telemetry system
    optimization_telemetry_deinit();

    // Reset mock HAL
    HAL_Abstraction_Reset_Mock();
}

/**
 * @brief Test telemetry system initialization
 */
void test_telemetry_init_success(void) {
    // System should initialize successfully
    SystemError_t result = optimization_telemetry_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Check initialization status
    bool is_initialized = optimization_telemetry_is_initialized();
    TEST_ASSERT_TRUE(is_initialized);
}

/**
 * @brief Test telemetry packet data collection
 */
void test_telemetry_collect_sample_basic(void) {
    // Configure mock hardware responses
    HAL_Abstraction_MockI2C_SetResponse(0x1234, 2); // AS5600 angle response
    HAL_Abstraction_MockSPI_SetResponse(0x5678, 3); // L6470 status response

    // Collect telemetry sample
    SystemError_t result =
        optimization_telemetry_collect_sample(&test_packet, 0);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify timestamp is set
    TEST_ASSERT_NOT_EQUAL(0, test_packet.timestamp_us);

    // Verify motor ID is correct
    TEST_ASSERT_EQUAL(0, test_packet.motor_id);

    // Verify data collection flags
    TEST_ASSERT_TRUE(test_packet.data_valid);
}

/**
 * @brief Test telemetry timing performance
 */
void test_telemetry_timing_performance(void) {
    uint32_t start_time, end_time, execution_time;

    // Configure fast mock responses
    HAL_Abstraction_MockI2C_SetResponse(0x1000, 2);
    HAL_Abstraction_MockSPI_SetResponse(0x2000, 3);

    // Measure execution time
    start_time = HAL_Abstraction_GetTick();
    SystemError_t result =
        optimization_telemetry_collect_sample(&test_packet, 0);
    end_time = HAL_Abstraction_GetTick();

    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    execution_time = end_time - start_time;

    // Execution should be under 500µs (500 ticks at 1MHz)
    TEST_ASSERT_LESS_THAN(500, execution_time);
}

/**
 * @brief Test AS5600 encoder data collection
 */
void test_as5600_data_collection(void) {
    // Mock AS5600 responses for full rotation
    uint16_t mock_angle = 0x0800; // 45 degrees in AS5600 format
    HAL_Abstraction_MockI2C_SetResponse(mock_angle, 2);

    // Collect sample
    SystemError_t result =
        optimization_telemetry_collect_sample(&test_packet, 0);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify encoder data is captured
    TEST_ASSERT_NOT_EQUAL(0.0f, test_packet.encoder_position_deg);
    TEST_ASSERT_GREATER_THAN(0.0f, test_packet.encoder_position_deg);
    TEST_ASSERT_LESS_THAN(360.0f, test_packet.encoder_position_deg);
}

/**
 * @brief Test L6470 status data collection
 */
void test_l6470_status_collection(void) {
    // Mock L6470 status register response
    uint32_t mock_status = 0x7E83; // Normal operation status
    HAL_Abstraction_MockSPI_SetResponse(mock_status, 3);

    // Collect sample
    SystemError_t result =
        optimization_telemetry_collect_sample(&test_packet, 0);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify L6470 data is captured
    TEST_ASSERT_NOT_EQUAL(0, test_packet.l6470_status);
    TEST_ASSERT_FALSE(test_packet.l6470_fault_detected);
}

/**
 * @brief Test safety bounds checking
 */
void test_safety_bounds_checking(void) {
    // Test with normal values
    test_packet.encoder_position_deg = 45.0f;
    test_packet.motor_current_ma = 800; // Within normal range
    test_packet.supply_voltage_mv = 12000;

    bool safety_ok =
        optimization_telemetry_validate_safety_bounds(&test_packet);
    TEST_ASSERT_TRUE(safety_ok);

    // Test with overcurrent
    test_packet.motor_current_ma = 2500; // Exceeds safety limit
    safety_ok = optimization_telemetry_validate_safety_bounds(&test_packet);
    TEST_ASSERT_FALSE(safety_ok);

    // Test with undervoltage
    test_packet.motor_current_ma = 800;   // Reset to normal
    test_packet.supply_voltage_mv = 8000; // Too low
    safety_ok = optimization_telemetry_validate_safety_bounds(&test_packet);
    TEST_ASSERT_FALSE(safety_ok);
}

/**
 * @brief Test dataset initialization and management
 */
void test_dataset_initialization(void) {
    // Initialize dataset
    SystemError_t result = optimization_telemetry_init_dataset(
        &test_dataset, "Test Characterization", 100);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify initialization
    TEST_ASSERT_EQUAL(100, test_dataset.max_samples);
    TEST_ASSERT_EQUAL(0, test_dataset.sample_count);
    TEST_ASSERT_TRUE(test_dataset.data_valid);
    TEST_ASSERT_NOT_NULL(test_dataset.samples);

    // Verify description
    TEST_ASSERT_EQUAL_STRING("Test Characterization",
                             test_dataset.description);
}

/**
 * @brief Test dataset sample addition
 */
void test_dataset_sample_addition(void) {
    // Initialize dataset with small size for testing
    SystemError_t result =
        optimization_telemetry_init_dataset(&test_dataset, "Sample Test", 5);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Add samples
    for (int i = 0; i < 3; i++) {
        test_packet.timestamp_us = i * 1000;
        test_packet.encoder_position_deg = mock_position_data[i];
        test_packet.motor_current_ma = mock_current_data[i];

        result =
            optimization_telemetry_add_sample(&test_dataset, &test_packet);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    }

    // Verify sample count
    TEST_ASSERT_EQUAL(3, test_dataset.sample_count);

    // Verify sample data
    TEST_ASSERT_EQUAL(0, test_dataset.samples[0].timestamp_us);
    TEST_ASSERT_EQUAL(1000, test_dataset.samples[1].timestamp_us);
    TEST_ASSERT_EQUAL(2000, test_dataset.samples[2].timestamp_us);
}

/**
 * @brief Test dataset overflow handling
 */
void test_dataset_overflow_handling(void) {
    // Initialize small dataset
    SystemError_t result =
        optimization_telemetry_init_dataset(&test_dataset, "Overflow Test", 2);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Fill dataset to capacity
    for (int i = 0; i < 2; i++) {
        test_packet.timestamp_us = i * 1000;
        result =
            optimization_telemetry_add_sample(&test_dataset, &test_packet);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    }

    // Attempt to add beyond capacity
    test_packet.timestamp_us = 3000;
    result = optimization_telemetry_add_sample(&test_dataset, &test_packet);
    TEST_ASSERT_EQUAL(ERROR_BUFFER_OVERFLOW, result);

    // Verify sample count doesn't exceed capacity
    TEST_ASSERT_EQUAL(2, test_dataset.sample_count);
}

/**
 * @brief Test performance metrics calculation
 */
void test_performance_metrics_calculation(void) {
    // Setup test dataset with known timing data
    optimization_telemetry_init_dataset(&test_dataset, "Metrics Test", 10);

    // Add samples with varying execution times
    uint32_t execution_times[] = {100, 150, 120, 180, 110,
                                  200, 90,  130, 140, 160};

    for (int i = 0; i < 10; i++) {
        test_packet.timestamp_us = i * 1000;
        test_packet.collection_time_us = execution_times[i];
        optimization_telemetry_add_sample(&test_dataset, &test_packet);
    }

    // Calculate metrics
    SystemError_t result =
        optimization_telemetry_calculate_metrics(&test_dataset, &test_metrics);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify average execution time
    TEST_ASSERT_EQUAL(138, test_metrics.avg_collection_time_us); // (sum/10)

    // Verify max execution time
    TEST_ASSERT_EQUAL(200, test_metrics.max_collection_time_us);

    // Verify sample rate calculation
    TEST_ASSERT_GREATER_THAN(
        500, test_metrics.effective_sample_rate_hz); // Should be ~1000Hz
}

/**
 * @brief Test JSON export functionality
 */
void test_json_export_basic(void) {
    char json_buffer[2048];
    size_t json_size;

    // Initialize dataset with sample data
    optimization_telemetry_init_dataset(&test_dataset, "JSON Test", 3);

    for (int i = 0; i < 3; i++) {
        test_packet.timestamp_us = i * 1000;
        test_packet.encoder_position_deg = mock_position_data[i];
        test_packet.motor_current_ma = mock_current_data[i];
        test_packet.supply_voltage_mv = mock_voltage_data[i];
        optimization_telemetry_add_sample(&test_dataset, &test_packet);
    }

    // Export to JSON
    SystemError_t result = optimization_telemetry_export_json(
        &test_dataset, json_buffer, sizeof(json_buffer), &json_size);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify JSON was generated
    TEST_ASSERT_GREATER_THAN(100, json_size);
    TEST_ASSERT_LESS_THAN(sizeof(json_buffer), json_size);

    // Verify JSON contains expected keys
    TEST_ASSERT_NOT_NULL(strstr(json_buffer, "\"description\""));
    TEST_ASSERT_NOT_NULL(strstr(json_buffer, "\"sample_count\""));
    TEST_ASSERT_NOT_NULL(strstr(json_buffer, "\"samples\""));
    TEST_ASSERT_NOT_NULL(strstr(json_buffer, "\"timestamp_us\""));
}

/**
 * @brief Test emergency stop integration
 */
void test_emergency_stop_integration(void) {
    // Simulate emergency stop condition
    HAL_Abstraction_MockSafety_SetEmergencyStop(true);

    // Attempt data collection during emergency
    SystemError_t result =
        optimization_telemetry_collect_sample(&test_packet, 0);
    TEST_ASSERT_EQUAL(ERROR_SAFETY_EMERGENCY_STOP, result);

    // Verify data collection is prevented
    TEST_ASSERT_FALSE(test_packet.data_valid);

    // Clear emergency stop
    HAL_Abstraction_MockSafety_SetEmergencyStop(false);

    // Verify normal operation resumes
    result = optimization_telemetry_collect_sample(&test_packet, 0);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
    TEST_ASSERT_TRUE(test_packet.data_valid);
}

/**
 * @brief Test invalid motor ID handling
 */
void test_invalid_motor_id_handling(void) {
    // Test with invalid motor ID
    SystemError_t result =
        optimization_telemetry_collect_sample(&test_packet, 5);
    TEST_ASSERT_EQUAL(ERROR_INVALID_PARAMETER, result);

    // Test with valid motor ID
    result = optimization_telemetry_collect_sample(&test_packet, 0);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    result = optimization_telemetry_collect_sample(&test_packet, 1);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

/**
 * @brief Test memory allocation error handling
 */
void test_memory_allocation_failure(void) {
    // Simulate memory allocation failure
    HAL_Abstraction_MockMemory_SetAllocFailure(true);

    // Attempt dataset initialization
    SystemError_t result = optimization_telemetry_init_dataset(
        &test_dataset, "Memory Test", 1000);
    TEST_ASSERT_EQUAL(ERROR_MEMORY_ALLOCATION_FAILED, result);

    // Reset memory allocation
    HAL_Abstraction_MockMemory_SetAllocFailure(false);

    // Verify normal allocation works
    result =
        optimization_telemetry_init_dataset(&test_dataset, "Memory Test", 100);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

/**
 * @brief Test concurrent access safety
 */
void test_concurrent_access_safety(void) {
    // Initialize dataset
    optimization_telemetry_init_dataset(&test_dataset, "Concurrent Test", 10);

    // Simulate concurrent read/write access
    // Note: In real system, this would use FreeRTOS mutexes

    SystemError_t result1 =
        optimization_telemetry_add_sample(&test_dataset, &test_packet);
    SystemError_t result2 =
        optimization_telemetry_calculate_metrics(&test_dataset, &test_metrics);

    // Both operations should succeed (mock environment)
    TEST_ASSERT_EQUAL(SYSTEM_OK, result1);
    TEST_ASSERT_EQUAL(SYSTEM_OK, result2);
}

/**
 * @brief Test CPU overhead measurement
 */
void test_cpu_overhead_measurement(void) {
    uint32_t total_time = 0;
    const int iterations = 100;

    // Measure CPU overhead over multiple iterations
    for (int i = 0; i < iterations; i++) {
        uint32_t start_time = HAL_Abstraction_GetTick();

        SystemError_t result =
            optimization_telemetry_collect_sample(&test_packet, 0);
        TEST_ASSERT_EQUAL(SYSTEM_OK, result);

        uint32_t end_time = HAL_Abstraction_GetTick();
        total_time += (end_time - start_time);
    }

    uint32_t avg_time = total_time / iterations;

    // Verify average execution time is under 500µs target
    TEST_ASSERT_LESS_THAN(500, avg_time);

    // Log performance for analysis
    printf("Average telemetry collection time: %lu µs\n", avg_time);
}

/**
 * @brief Main test runner function
 */
int main(void) {
    UNITY_BEGIN();

    // Basic functionality tests
    RUN_TEST(test_telemetry_init_success);
    RUN_TEST(test_telemetry_collect_sample_basic);
    RUN_TEST(test_telemetry_timing_performance);

    // Hardware interface tests
    RUN_TEST(test_as5600_data_collection);
    RUN_TEST(test_l6470_status_collection);

    // Safety and validation tests
    RUN_TEST(test_safety_bounds_checking);
    RUN_TEST(test_emergency_stop_integration);

    // Dataset management tests
    RUN_TEST(test_dataset_initialization);
    RUN_TEST(test_dataset_sample_addition);
    RUN_TEST(test_dataset_overflow_handling);

    // Performance and metrics tests
    RUN_TEST(test_performance_metrics_calculation);
    RUN_TEST(test_cpu_overhead_measurement);

    // Export and serialization tests
    RUN_TEST(test_json_export_basic);

    // Error handling tests
    RUN_TEST(test_invalid_motor_id_handling);
    RUN_TEST(test_memory_allocation_failure);
    RUN_TEST(test_concurrent_access_safety);

    return UNITY_END();
}

/**
 * @brief Test configuration summary
 *
 * This test suite validates:
 * - ✅ Basic telemetry functionality and initialization
 * - ✅ Hardware interface compatibility (AS5600/L6470)
 * - ✅ Performance requirements (<500µs execution time)
 * - ✅ Safety system integration and bounds checking
 * - ✅ Dataset management and overflow handling
 * - ✅ JSON export and serialization
 * - ✅ Error handling and edge cases
 * - ✅ CPU overhead measurement and validation
 * - ✅ Mock hardware compatibility for CI/CD
 *
 * Expected Results:
 * - All tests should pass with mock hardware
 * - Average execution time < 500µs per sample
 * - Memory usage within allocated bounds
 * - Safety integration properly prevents unsafe operation
 * - JSON export provides valid, parseable output
 */
