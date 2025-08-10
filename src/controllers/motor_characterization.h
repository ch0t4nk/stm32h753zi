#ifndef MOTOR_CHARACTERIZATION_H
#define MOTOR_CHARACTERIZATION_H

#include "common/error_codes.h"
#include "config/motor_config.h"
#include "hal_abstraction.h"
#include "telemetry/optimization_telemetry.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file motor_characterization.h
 * @brief Motor parameter identification and characterization framework
 *
 * This module provides comprehensive motor characterization capabilities for
 * the STM32H753ZI stepper motor control system using focused telemetry data
 * acquisition.
 *
 * Key Features:
 * - Automated motor parameter identification (inertia, friction, electrical
 * constants)
 * - Real-time performance analysis and optimization recommendations
 * - Load-dependent parameter characterization for adaptive control
 * - Thermal characterization for temperature compensation
 * - Manufacturer-compliant testing protocols following ST L6470 guidelines
 * - Integration with existing safety systems and emergency stops
 */

// ================================================================================================
// CONFIGURATION AND CONSTANTS
// ================================================================================================

// Characterization test limits (safety-bounded)
#define CHAR_MAX_TEST_AMPLITUDE_DEG (45.0f) // Maximum test motion amplitude
#define CHAR_MAX_TEST_FREQUENCY_HZ (50.0f)  // Maximum test frequency
#define CHAR_MAX_TEST_DURATION_MS (30000) // Maximum test duration (30 seconds)
#define CHAR_MIN_TEST_SAMPLES                                                 \
    (100) // Minimum samples for valid characterization

// Parameter identification accuracy targets
#define CHAR_INERTIA_ACCURACY_TARGET                                          \
    (90.0f) // Inertia identification accuracy (%)
#define CHAR_FRICTION_ACCURACY_TARGET                                         \
    (85.0f) // Friction identification accuracy (%)
#define CHAR_ELECTRICAL_ACCURACY_TARGET                                       \
    (95.0f) // Electrical parameter accuracy (%)
#define CHAR_THERMAL_ACCURACY_TARGET                                          \
    (80.0f) // Thermal characterization accuracy (%)

// Optimization improvement targets (based on ST L6470 documentation)
#define CHAR_EFFICIENCY_IMPROVEMENT_TARGET                                    \
    (15.0f) // Target efficiency improvement (%)
#define CHAR_SETTLING_TIME_IMPROVEMENT_TARGET                                 \
    (25.0f) // Target settling time improvement (%)
#define CHAR_OVERSHOOT_REDUCTION_TARGET                                       \
    (50.0f) // Target overshoot reduction (%)

// ================================================================================================
// DATA STRUCTURES
// ================================================================================================

/**
 * @brief Motor physical parameters identified through characterization
 *
 * Contains all identified motor parameters with confidence intervals and
 * validity indicators for use in control optimization algorithms.
 */
typedef struct {
    // === Mechanical Parameters ===
    float rotor_inertia_kg_m2;    ///< Rotor moment of inertia (kg⋅m²)
    float viscous_friction_coeff; ///< Viscous friction coefficient (N⋅m⋅s/rad)
    float static_friction_torque; ///< Static friction torque (N⋅m)
    float coulomb_friction_torque;  ///< Coulomb friction torque (N⋅m)
    float mechanical_time_constant; ///< Mechanical time constant (seconds)

    // === Electrical Parameters ===
    float winding_resistance_ohm;   ///< Phase winding resistance (Ω)
    float winding_inductance_h;     ///< Phase winding inductance (H)
    float back_emf_constant;        ///< Back-EMF constant (V⋅s/rad)
    float electrical_time_constant; ///< Electrical time constant (seconds)

    // === Torque Characteristics ===
    float torque_constant_nm_a;  ///< Torque constant (N⋅m/A)
    float holding_torque_nm;     ///< Maximum holding torque (N⋅m)
    float detent_torque_nm;      ///< Detent torque (N⋅m)
    float max_no_load_speed_rps; ///< Maximum no-load speed (rev/s)

    // === Load-Dependent Parameters ===
    float load_inertia_estimate_kg_m2; ///< Estimated external load inertia
                                       ///< (kg⋅m²)
    float load_friction_estimate;      ///< Estimated external load friction
    float load_compliance_rad_nm;      ///< Load compliance (rad/N⋅m)

    // === Thermal Parameters ===
    float thermal_resistance_c_w;  ///< Thermal resistance (°C/W)
    float thermal_time_constant_s; ///< Thermal time constant (seconds)
    float
        temperature_derating_factor; ///< Performance derating with temperature

    // === Quality and Confidence Metrics ===
    float parameter_confidence_percent; ///< Overall parameter confidence (%)
    float identification_accuracy_percent; ///< Parameter identification
                                           ///< accuracy (%)
    uint32_t characterization_timestamp;   ///< Timestamp of characterization
    bool parameters_valid; ///< Parameters are valid and reliable

} MotorPhysicalParameters_t;

/**
 * @brief Optimal control parameters derived from characterization
 *
 * Contains optimized control parameters calculated from motor characterization
 * for improved performance compared to default SSOT configuration.
 */
typedef struct {
    // === L6470 Optimized Parameters ===
    uint16_t optimal_kval_hold;    ///< Optimized KVAL_HOLD (0-255)
    uint16_t optimal_kval_run;     ///< Optimized KVAL_RUN (0-255)
    uint16_t optimal_kval_acc;     ///< Optimized KVAL_ACC (0-255)
    uint16_t optimal_kval_dec;     ///< Optimized KVAL_DEC (0-255)
    uint16_t optimal_acceleration; ///< Optimized acceleration rate
    uint16_t optimal_deceleration; ///< Optimized deceleration rate
    uint16_t optimal_max_speed;    ///< Optimized maximum speed
    uint16_t optimal_min_speed;    ///< Optimized minimum speed

    // === Motion Profile Parameters ===
    float optimal_jerk_limit; ///< Optimized jerk limitation (deg/s³)
    float
        optimal_settling_tolerance; ///< Optimized settling tolerance (degrees)
    float optimal_velocity_feedforward;     ///< Velocity feedforward gain
    float optimal_acceleration_feedforward; ///< Acceleration feedforward gain

    // === Adaptive Control Parameters ===
    bool enable_load_adaptation;      ///< Enable load-adaptive control
    float load_adaptation_gain;       ///< Load adaptation learning rate
    bool enable_thermal_compensation; ///< Enable thermal compensation
    float thermal_compensation_gain;  ///< Thermal compensation gain

    // === Performance Predictions ===
    float predicted_efficiency_improvement;    ///< Predicted efficiency
                                               ///< improvement (%)
    float predicted_settling_time_improvement; ///< Predicted settling time
                                               ///< improvement (%)
    float predicted_overshoot_reduction; ///< Predicted overshoot reduction (%)
    float predicted_power_reduction; ///< Predicted power consumption reduction
                                     ///< (%)

    // === Validation Metrics ===
    bool parameters_validated; ///< Parameters validated through testing
    float validation_accuracy_percent; ///< Validation test accuracy (%)
    uint32_t optimization_timestamp;   ///< Timestamp of optimization

} OptimalControlParameters_t;

/**
 * @brief Characterization test results and analysis
 *
 * Complete results from motor characterization including raw data,
 * identified parameters, and optimization recommendations.
 */
typedef struct {
    // === Test Information ===
    uint8_t motor_id; ///< Motor identifier
    CharacterizationTestType_t
        test_type;             ///< Type of characterization performed
    uint32_t test_timestamp;   ///< Test execution timestamp
    uint32_t test_duration_ms; ///< Actual test duration

    // === Raw Data ===
    CharacterizationDataSet_t raw_dataset; ///< Complete telemetry dataset
    uint32_t valid_samples_count; ///< Number of valid samples analyzed
    float data_quality_average;   ///< Average data quality score

    // === Identified Parameters ===
    MotorPhysicalParameters_t physical_params; ///< Identified motor parameters
    OptimalControlParameters_t
        optimal_params; ///< Optimized control parameters

    // === Performance Analysis ===
    float current_efficiency_percent;  ///< Current system efficiency
    float current_settling_time_ms;    ///< Current settling time
    float current_overshoot_percent;   ///< Current overshoot percentage
    float current_power_consumption_w; ///< Current power consumption

    // === Optimization Results ===
    float efficiency_improvement_actual; ///< Actual efficiency improvement
                                         ///< achieved
    float settling_improvement_actual;   ///< Actual settling time improvement
                                         ///< achieved
    float overshoot_reduction_actual; ///< Actual overshoot reduction achieved
    float power_reduction_actual;     ///< Actual power reduction achieved

    // === Recommendations ===
    char optimization_summary[512]; ///< Human-readable optimization summary
    char safety_notes[256];         ///< Safety considerations and notes
    bool ready_for_deployment;      ///< Optimized parameters ready for use

} MotorCharacterizationResults_t;

/**
 * @brief Characterization configuration for automated testing
 *
 * Configuration structure for setting up automated motor characterization
 * with safety limits and test protocols.
 */
typedef struct {
    // === Test Selection ===
    bool enable_step_response_test; ///< Enable step response characterization
    bool enable_frequency_sweep_test; ///< Enable frequency response
                                      ///< characterization
    bool
        enable_load_variation_test; ///< Enable load variation characterization
    bool enable_thermal_test;       ///< Enable thermal characterization
    bool enable_efficiency_mapping; ///< Enable efficiency mapping

    // === Test Parameters ===
    float step_amplitude_deg;           ///< Step test amplitude (degrees)
    float frequency_sweep_start_hz;     ///< Frequency sweep start frequency
    float frequency_sweep_end_hz;       ///< Frequency sweep end frequency
    uint32_t test_duration_per_type_ms; ///< Duration per test type
    uint32_t sample_rate_hz;            ///< Data collection sample rate

    // === Safety Configuration ===
    float safety_current_limit_a;     ///< Safety current limit
    float safety_speed_limit_dps;     ///< Safety speed limit
    float safety_temperature_limit_c; ///< Safety temperature limit
    bool abort_on_first_violation; ///< Abort testing on first safety violation

    // === Analysis Configuration ===
    float parameter_confidence_threshold; ///< Minimum confidence for parameter
                                          ///< acceptance
    bool validate_parameters;             ///< Validate identified parameters
    bool generate_optimization_report;    ///< Generate detailed optimization
                                          ///< report

} MotorCharacterizationConfig_t;

// ================================================================================================
// PUBLIC API FUNCTIONS
// ================================================================================================

/**
 * @brief Initialize motor characterization system
 *
 * Initializes the characterization framework including telemetry integration,
 * analysis algorithms, and safety monitoring for specified motor.
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_characterization_init(uint8_t motor_id);

/**
 * @brief Perform comprehensive motor characterization
 *
 * Executes complete motor characterization sequence including:
 * - Step response analysis for time constants
 * - Frequency response analysis for system identification
 * - Load variation analysis for adaptive parameters
 * - Thermal analysis for temperature compensation
 * - Efficiency mapping for power optimization
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @param config Characterization configuration
 * @param results Pointer to results structure to fill
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_characterization_run_comprehensive(
    uint8_t motor_id, const MotorCharacterizationConfig_t *config,
    MotorCharacterizationResults_t *results);

/**
 * @brief Perform step response characterization test
 *
 * Executes step response test for time constant identification and
 * transient response analysis.
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @param step_amplitude_deg Step amplitude in degrees
 * @param results Pointer to results structure to update
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t
motor_characterization_step_response(uint8_t motor_id,
                                     float step_amplitude_deg,
                                     MotorCharacterizationResults_t *results);

/**
 * @brief Perform frequency response characterization test
 *
 * Executes frequency sweep test for system identification and
 * frequency domain analysis.
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @param freq_start_hz Start frequency for sweep
 * @param freq_end_hz End frequency for sweep
 * @param results Pointer to results structure to update
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_characterization_frequency_response(
    uint8_t motor_id, float freq_start_hz, float freq_end_hz,
    MotorCharacterizationResults_t *results);

/**
 * @brief Identify motor physical parameters from telemetry data
 *
 * Analyzes telemetry data to identify motor physical parameters using
 * system identification algorithms.
 *
 * @param dataset Telemetry dataset from characterization tests
 * @param parameters Pointer to parameters structure to fill
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_characterization_identify_parameters(
    const CharacterizationDataSet_t *dataset,
    MotorPhysicalParameters_t *parameters);

/**
 * @brief Calculate optimal control parameters from motor characterization
 *
 * Calculates optimized L6470 parameters and control settings based on
 * identified motor characteristics for improved performance.
 *
 * @param physical_params Identified motor physical parameters
 * @param optimal_params Pointer to optimal parameters structure to fill
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_characterization_calculate_optimal_parameters(
    const MotorPhysicalParameters_t *physical_params,
    OptimalControlParameters_t *optimal_params);

/**
 * @brief Validate optimized parameters through testing
 *
 * Validates optimized control parameters by comparing predicted vs actual
 * performance improvements through controlled testing.
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @param optimal_params Optimized parameters to validate
 * @param validation_accuracy Pointer to calculated validation accuracy
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_characterization_validate_parameters(
    uint8_t motor_id, const OptimalControlParameters_t *optimal_params,
    float *validation_accuracy);

/**
 * @brief Apply optimized parameters to motor control system
 *
 * Safely applies validated optimal parameters to L6470 driver and control
 * system with rollback capability if performance degrades.
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @param optimal_params Validated optimal parameters
 * @param enable_monitoring Enable performance monitoring after application
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_characterization_apply_parameters(
    uint8_t motor_id, const OptimalControlParameters_t *optimal_params,
    bool enable_monitoring);

/**
 * @brief Get current motor characterization status
 *
 * Returns current characterization status and last results for specified
 * motor.
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @param status_summary Buffer for status summary string
 * @param buffer_size Size of status summary buffer
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_characterization_get_status(uint8_t motor_id,
                                                char *status_summary,
                                                size_t buffer_size);

/**
 * @brief Generate optimization report in JSON format
 *
 * Generates comprehensive optimization report including all characterization
 * results, identified parameters, and performance improvements.
 *
 * @param results Characterization results to export
 * @param json_buffer Buffer for JSON output
 * @param buffer_size Size of JSON buffer
 * @param output_size Pointer to actual JSON output size
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_characterization_generate_report(
    const MotorCharacterizationResults_t *results, char *json_buffer,
    size_t buffer_size, size_t *output_size);

/**
 * @brief Reset motor to default SSOT parameters
 *
 * Safely resets motor to original SSOT configuration parameters,
 * discarding any applied optimizations.
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_characterization_reset_to_defaults(uint8_t motor_id);

/**
 * @brief Emergency stop characterization process
 *
 * Immediately stops all characterization testing and triggers safety systems.
 *
 * @param motor_id Motor identifier (0 to MAX_MOTORS-1)
 * @return SystemError_t SYSTEM_OK on success, error code on failure
 */
SystemError_t motor_characterization_emergency_stop(uint8_t motor_id);

#ifdef __cplusplus
}
#endif

#endif // MOTOR_CHARACTERIZATION_H
