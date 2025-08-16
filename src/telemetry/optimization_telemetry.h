#ifndef OPTIMIZATION_TELEMETRY_H
#define OPTIMIZATION_TELEMETRY_H

#include "common/data_types.h"
#include "config/error_codes.h"
#include "config/freertos_config_ssot.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#include "hal_abstraction.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file optimization_telemetry.h
 * @brief Focused telemetry data acquisition for motor characterization and
 * optimization
 *
 * This module provides efficient, targeted data collection for motor tuning
 * algorithms. Designed for <2% CPU overhead while maintaining 1kHz control
 * loop compatibility.
 *
 * Key Features:
 * - High-resolution telemetry collection (1kHz sustained)
 * - On-board real-time data processing
 * - Integration with existing Phase 5A telemetry dashboard
 * - Safety-bounded data collection with emergency stops
 * - Manufacturer-compliant parameter monitoring
 */

// ================================================================================================
// CONFIGURATION AND CONSTANTS
// ================================================================================================

// Telemetry buffer sizes (optimized for STM32H7 memory)
#define CHARACTERIZATION_BUFFER_SIZE                                          \
    (2000) // 2000 samples for 2-second tests at 1kHz
#define TELEMETRY_SAMPLE_RATE_MAX_HZ (1000)    // Maximum sample rate (1kHz)
#define TELEMETRY_SAMPLE_RATE_DEFAULT_HZ (500) // Default sample rate (500Hz)
#define TELEMETRY_MEMORY_POOL_SIZE_KB (8)      // Memory pool size (8KB target)

// Performance monitoring constants
#define TELEMETRY_CPU_OVERHEAD_TARGET_PCT (2.0f) // Target CPU overhead (<2%)
#define TELEMETRY_TIMING_TOLERANCE_US (100)      // Timing tolerance (±100µs)
#define TELEMETRY_SAFETY_CHECK_INTERVAL_MS (10) // Safety check interval (10ms)

// Validation and calibration constants
#define TELEMETRY_CALIBRATION_SAMPLES (100) // Calibration sample count
#define TELEMETRY_VALIDATION_DURATION_MS                                      \
    (5000)                                    // Validation test duration (5s)
#define TELEMETRY_ACCURACY_TARGET_PCT (95.0f) // Target accuracy (95%)

// ================================================================================================
// DATA STRUCTURES
// ================================================================================================

/**
 * @brief High-resolution telemetry packet for motor characterization
 *
 * Optimized structure for efficient data collection with minimal memory
 * footprint. All fields aligned for efficient STM32H7 access patterns.
 */
typedef struct {
    // === AS5600 Encoder Data ===
    float position_degrees;  ///< Current position (0.088° resolution)
    float velocity_dps;      ///< Calculated velocity (degrees/second)
    float acceleration_dps2; ///< Calculated acceleration (degrees/second²)

    // === L6470 Driver Data ===
    float motor_current_a;     ///< Measured current from L6470 ADC (Amperes)
    uint16_t kval_hold_actual; ///< Current KVAL_HOLD setting (0-255)
    uint16_t kval_run_actual;  ///< Current KVAL_RUN setting (0-255)
    uint8_t status_flags;      ///< L6470 status register bits
    bool thermal_warning;      ///< Thermal warning flag from L6470
    bool stall_detected;       ///< Stall detection flag from L6470
    bool overcurrent_detected; ///< Overcurrent protection flag

    // === System Performance Data ===
    uint32_t timestamp_us; ///< High-resolution timestamp (microseconds)
    uint32_t
        control_loop_time_us;  ///< Control loop execution time (microseconds)
    float power_consumption_w; ///< Calculated power consumption (Watts)
    float thermal_performance; ///< Temperature-based performance metric
                               ///< (0.0-1.0)
    float cpu_utilization_percent; ///< Current CPU utilization (percentage)

    // === Test-Specific Data ===
    float
        commanded_position; ///< Commanded position for current test (degrees)
    float commanded_velocity; ///< Commanded velocity for current test (dps)
    float position_error;     ///< Position tracking error (degrees)
    float control_effort;     ///< PID controller output (-1.0 to 1.0)

    // === Quality Metrics ===
    uint8_t data_quality_score;  ///< Data quality indicator (0-100)
    bool safety_bounds_ok;       ///< All safety bounds within limits
    uint32_t sample_sequence_id; ///< Sequential sample identifier
} OptimizationTelemetryPacket_t;

/**
 * @brief Characterization test types for targeted data collection
 */
typedef enum {
    CHAR_TEST_TYPE_STEP_RESPONSE,   ///< Step response test for time constant
                                    ///< identification
    CHAR_TEST_TYPE_FREQUENCY_SWEEP, ///< Frequency response test for system
                                    ///< identification
    CHAR_TEST_TYPE_LOAD_VARIATION, ///< Load variation test for adaptive tuning
    CHAR_TEST_TYPE_THERMAL_CYCLING, ///< Thermal cycling test for temperature
                                    ///< compensation
    CHAR_TEST_TYPE_EFFICIENCY_MAP,  ///< Efficiency mapping test for power
                                    ///< optimization
    CHAR_TEST_TYPE_CUSTOM           ///< User-defined custom test
} CharacterizationTestType_t;

/**
 * @brief Characterization dataset for batch analysis
 *
 * Container for collected telemetry data with metadata for analysis
 * algorithms. Memory usage: ~196KB for 2000 samples (within STM32H7
 * capabilities).
 */
typedef struct {
    OptimizationTelemetryPacket_t
        samples[CHARACTERIZATION_BUFFER_SIZE]; ///< Telemetry samples
    uint32_t sample_count;     ///< Number of valid samples in buffer
    uint32_t sample_rate_hz;   ///< Sampling frequency used for collection
    uint32_t test_duration_ms; ///< Actual test duration (milliseconds)
    CharacterizationTestType_t
        test_type;                 ///< Type of characterization test performed
    uint32_t test_start_timestamp; ///< Test start timestamp (system ticks)
    float test_parameters[4];      ///< Test-specific parameters (amplitude,
                                   ///< frequency, etc.)
    uint8_t motor_id;              ///< Motor identifier (0-1)
    bool data_valid;               ///< Data validity flag
    uint32_t checksum;             ///< Data integrity checksum
} CharacterizationDataSet_t;

/**
 * @brief Test configuration for characterization
 *
 * Configuration structure for setting up targeted characterization tests.
 */
typedef struct {
    CharacterizationTestType_t test_type; ///< Type of test to perform
    float step_amplitude_deg;     ///< Step response amplitude (degrees)
    float frequency_start_hz;     ///< Frequency sweep start (Hz)
    float frequency_end_hz;       ///< Frequency sweep end (Hz)
    uint32_t test_duration_ms;    ///< Test duration (milliseconds)
    uint32_t sample_rate_hz;      ///< Data collection sample rate (Hz)
    bool enable_safety_limits;    ///< Enable safety bound checking during test
    float safety_current_limit_a; ///< Maximum current limit for test (Amperes)
    float safety_speed_limit_dps; ///< Maximum speed limit for test
                                  ///< (degrees/second)
    float safety_error_limit_deg; ///< Maximum position error limit (degrees)
} CharacterizationTestConfig_t;

/**
 * @brief Telemetry system performance metrics
 *
 * Performance monitoring for telemetry system validation and optimization.
 */
typedef struct {
    float cpu_overhead_percent;  ///< CPU overhead of telemetry system
                                 ///< (percentage)
    uint32_t memory_usage_bytes; ///< Current memory usage (bytes)
    uint32_t
        average_sample_time_us;    ///< Average time per sample (microseconds)
    uint32_t max_sample_time_us;   ///< Maximum time per sample (microseconds)
    uint32_t missed_samples_count; ///< Number of missed samples due to timing
    float timing_accuracy_percent; ///< Timing accuracy vs target sample rate
                                   ///< (percentage)
    bool real_time_compatible;     ///< Compatible with 1kHz control loop
    uint32_t total_samples_collected; ///< Total samples collected since init
} TelemetryPerformanceMetrics_t;

// ================================================================================================
// PUBLIC API FUNCTIONS
// ================================================================================================

/**
 * @brief Initialize optimization telemetry system for specified motor
 *
 * Sets up telemetry data collection infrastructure including:
 * - High-resolution timing system
 * - AS5600 encoder fast-mode configuration
 * - L6470 ADC monitoring setup
 * - Safety monitoring integration
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t optimization_telemetry_init(uint8_t motor_id);

/**
 * @brief Collect single telemetry sample with minimal overhead
 *
 * Efficiently collects one complete telemetry packet including:
 * - AS5600 position and velocity data
 * - L6470 current and status information
 * - System performance metrics
 * - Safety status checks
 *
 * Target execution time: <500µs for real-time compatibility
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @param packet Pointer to packet structure to fill
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t
optimization_telemetry_collect_sample(uint8_t motor_id,
                                      OptimizationTelemetryPacket_t *packet);

/**
 * @brief Collect telemetry dataset for characterization test
 *
 * Performs batch data collection for specified test duration and sample rate.
 * Includes safety monitoring and automatic test abort on limit violations.
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @param config Test configuration parameters
 * @param dataset Pointer to dataset structure to fill
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t optimization_telemetry_collect_dataset(
    uint8_t motor_id, const CharacterizationTestConfig_t *config,
    CharacterizationDataSet_t *dataset);

/**
 * @brief Start continuous telemetry streaming for real-time optimization
 *
 * Begins continuous telemetry collection for real-time parameter optimization.
 * Integrates with existing Phase 5A telemetry dashboard via HTTP/JSON API.
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @param sample_rate_hz Desired sample rate (Hz)
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t optimization_telemetry_start_streaming(uint8_t motor_id,
                                                     uint32_t sample_rate_hz);

/**
 * @brief Stop telemetry streaming
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t optimization_telemetry_stop_streaming(uint8_t motor_id);

/**
 * @brief Get telemetry system performance metrics
 *
 * Returns performance metrics for validation and optimization of telemetry
 * system.
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @param metrics Pointer to metrics structure to fill
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t optimization_telemetry_get_performance_metrics(
    uint8_t motor_id, TelemetryPerformanceMetrics_t *metrics);

/**
 * @brief Validate telemetry accuracy against reference measurements
 *
 * Performs validation test comparing telemetry measurements against
 * external reference (oscilloscope, external encoder, etc.).
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @param reference_data Pointer to reference measurement data
 * @param accuracy_percent Pointer to calculated accuracy percentage
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t optimization_telemetry_validate_accuracy(
    uint8_t motor_id, const CharacterizationDataSet_t *reference_data,
    float *accuracy_percent);

/**
 * @brief Calculate power efficiency from telemetry data
 *
 * Computes instantaneous and average power efficiency metrics from telemetry.
 *
 * @param packet Telemetry packet to analyze
 * @param efficiency_percent Pointer to calculated efficiency percentage
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t optimization_telemetry_calculate_efficiency(
    const OptimizationTelemetryPacket_t *packet, float *efficiency_percent);

/**
 * @brief Calculate thermal performance score from telemetry data
 *
 * Computes thermal performance metric based on current, temperature, and
 * efficiency.
 *
 * @param packet Telemetry packet to analyze
 * @param thermal_score Pointer to calculated thermal score (0.0-1.0)
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t optimization_telemetry_calculate_thermal_performance(
    const OptimizationTelemetryPacket_t *packet, float *thermal_score);

/**
 * @brief Export telemetry dataset to JSON format
 *
 * Exports characterization dataset in JSON format for external analysis.
 * Compatible with existing Phase 5A telemetry dashboard JSON API.
 *
 * @param dataset Dataset to export
 * @param json_buffer Buffer to store JSON output
 * @param buffer_size Size of JSON buffer
 * @param output_size Pointer to actual JSON output size
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t
optimization_telemetry_export_json(const CharacterizationDataSet_t *dataset,
                                   char *json_buffer, size_t buffer_size,
                                   size_t *output_size);

/**
 * @brief Emergency stop telemetry collection
 *
 * Immediately stops all telemetry collection and triggers safety systems.
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t optimization_telemetry_emergency_stop(uint8_t motor_id);

#ifdef __cplusplus
}
#endif

#endif // OPTIMIZATION_TELEMETRY_H
