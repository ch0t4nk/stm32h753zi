/**
 * @file optimization_config.h
 * @brief SSOT configuration for motor efficiency optimization system (FTR-009)
 *
 * This file provides the Single Source of Truth (SSOT) for all
 * optimization-related configurations including algorithm parameters,
 * performance targets, safety limits, and integration settings.
 *
 * CRITICAL: This is the SSOT for optimization configuration. All optimization
 * modules must reference these values. Never hardcode optimization parameters
 * elsewhere.
 *
 * @author STM32H753ZI Motor Control Project
 * @date 2025-08-10
 * @version 1.0.0
 *
 * @see .github/instructions/ssot-config.instructions.md for SSOT principles
 * @see .github/instructions/core-software.instructions.md for optimization
 * algorithms
 */

#ifndef OPTIMIZATION_CONFIG_H
#define OPTIMIZATION_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

// Reference dependent SSOT configurations
#include "motor_config.h"
#include "safety_config.h"
#include "system_config.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// MOTOR ID TYPE DEFINITION (SSOT)
// =============================================================================

/**
 * @brief Motor ID type for optimization system consistency
 * @note Consistent with system-wide motor identification using uint8_t
 */
typedef uint8_t motor_id_t;

// =============================================================================
// OPTIMIZATION TARGETS AND PERFORMANCE
// =============================================================================

/**
 * @brief Target efficiency improvement over baseline (FTR-009 acceptance
 * criteria)
 *
 * Range: 15.0% to 25.0% improvement as specified in FTR-009
 * This is the primary success metric for the optimization feature.
 */
#define OPTIMIZATION_TARGET_EFFICIENCY_PERCENT 85.0f
#define OPTIMIZATION_MIN_IMPROVEMENT_PERCENT 15.0f
#define OPTIMIZATION_MAX_IMPROVEMENT_PERCENT 25.0f
#define OPTIMIZATION_BASELINE_EFFICIENCY_PERCENT 70.0f

/**
 * @brief Convergence criteria for optimization algorithms
 *
 * These parameters define when the optimization algorithm has converged
 * to a stable solution and can transition to maintenance mode.
 */
#define OPTIMIZATION_CONVERGENCE_THRESHOLD_PERCENT                            \
    0.5f                                    // Efficiency change < 0.5%
#define OPTIMIZATION_CONVERGENCE_SAMPLES 10 // Must be stable for 10 samples
#define OPTIMIZATION_MAX_ITERATIONS 100     // Maximum optimization iterations
#define OPTIMIZATION_CONVERGENCE_TIMEOUT_MS 30000 // 30 second timeout

// =============================================================================
// ALGORITHM CONFIGURATION
// =============================================================================

/**
 * @brief Optimization algorithm modes
 *
 * Defines the available optimization algorithms. Each algorithm optimizes
 * different motor parameters to achieve efficiency improvements.
 */
typedef enum {
    OPTIMIZATION_ALGORITHM_ADAPTIVE_CURRENT =
        0, ///< Optimize motor current based on load
    OPTIMIZATION_ALGORITHM_SPEED_PROFILE, ///< Optimize speed profiles for
                                          ///< efficiency
    OPTIMIZATION_ALGORITHM_HYBRID, ///< Combined current + speed optimization
    OPTIMIZATION_ALGORITHM_ML_PREDICTIVE, ///< ML-based predictive optimization
                                          ///< (Phase 3.0)
    OPTIMIZATION_ALGORITHM_COUNT
} OptimizationAlgorithm_t;

/**
 * @brief Default optimization algorithm for Phase 1.4
 *
 * Adaptive current optimization is the primary algorithm for Phase 1.4
 * as it provides reliable efficiency gains with minimal complexity.
 */
#define OPTIMIZATION_ALGORITHM_DEFAULT OPTIMIZATION_ALGORITHM_ADAPTIVE_CURRENT

/**
 * @brief Adaptation rate parameters
 *
 * Controls how quickly the optimization algorithm adapts to changing
 * conditions. Faster adaptation responds quicker but may be less stable.
 */
#define OPTIMIZATION_ADAPTATION_RATE_DEFAULT 0.1f // Conservative adaptation
#define OPTIMIZATION_ADAPTATION_RATE_FAST 0.3f    // Fast adaptation
#define OPTIMIZATION_ADAPTATION_RATE_SLOW 0.05f   // Slow, stable adaptation
#define OPTIMIZATION_ADAPTATION_RATE_MIN 0.01f    // Minimum allowed rate
#define OPTIMIZATION_ADAPTATION_RATE_MAX 0.5f     // Maximum allowed rate

// =============================================================================
// ADAPTIVE CURRENT OPTIMIZATION
// =============================================================================

/**
 * @brief Adaptive current optimization algorithm parameters
 *
 * These parameters control the adaptive current optimization algorithm,
 * which adjusts motor current based on load conditions and efficiency
 * feedback.
 */
#define ADAPTIVE_CURRENT_STEP_SIZE_A 0.05f // Current adjustment step (Amperes)
#define ADAPTIVE_CURRENT_MIN_CURRENT_A 0.3f // Minimum allowed current
#define ADAPTIVE_CURRENT_MAX_CURRENT_A                                        \
    MOTOR_MAX_CURRENT_A // Reference SSOT motor config
#define ADAPTIVE_CURRENT_EFFICIENCY_WINDOW_SIZE 5 // Moving average window
#define ADAPTIVE_CURRENT_DIRECTION_HOLD_COUNT                                 \
    3 // Samples before direction change

/**
 * @brief Current optimization safety margins
 *
 * Safety margins to prevent current optimization from exceeding safe operating
 * limits. These provide additional safety beyond the SSOT motor limits.
 */
#define ADAPTIVE_CURRENT_SAFETY_MARGIN_PERCENT 10.0f // 10% safety margin
#define ADAPTIVE_CURRENT_THERMAL_DERATING_TEMP_C                              \
    MOTOR_MAX_TEMPERATURE_C - 10.0f

// =============================================================================
// SPEED PROFILE OPTIMIZATION
// =============================================================================

/**
 * @brief Speed profile optimization parameters
 *
 * Parameters for optimizing motor speed profiles to reduce mechanical losses
 * and improve overall efficiency during motion sequences.
 */
#define SPEED_PROFILE_MIN_SPEED_RPM 50.0f // Minimum optimization speed
#define SPEED_PROFILE_MAX_SPEED_RPM                                           \
    MOTOR_MAX_SPEED_RPM // Reference SSOT motor config
#define SPEED_PROFILE_ACCELERATION_FACTOR                                     \
    0.8f // Conservative acceleration factor
#define SPEED_PROFILE_DECELERATION_FACTOR                                     \
    0.9f                                    // Conservative deceleration factor
#define SPEED_PROFILE_SMOOTHING_FACTOR 0.1f // Profile smoothing coefficient

// =============================================================================
// TELEMETRY INTEGRATION
// =============================================================================

/**
 * @brief Telemetry integration configuration
 *
 * Parameters controlling integration with the telemetry system (FTR-001)
 * for real-time efficiency monitoring and feedback.
 */
#define OPTIMIZATION_TELEMETRY_UPDATE_RATE_HZ 100 // 100 Hz telemetry updates
#define OPTIMIZATION_TELEMETRY_BUFFER_SIZE 50 // Circular buffer for history
#define OPTIMIZATION_TELEMETRY_TIMEOUT_MS 100 // Telemetry timeout
#define OPTIMIZATION_TELEMETRY_REQUIRED_SAMPLES                               \
    5 // Min samples before optimization

/**
 * @brief Performance metrics collection
 *
 * Configuration for collecting and storing optimization performance metrics
 * to validate the 15-25% efficiency improvement requirement.
 */
#define OPTIMIZATION_METRICS_HISTORY_SIZE 100 // Performance history samples
#define OPTIMIZATION_METRICS_UPDATE_INTERVAL_MS 1000 // 1 second metric updates
#define OPTIMIZATION_BASELINE_COLLECTION_TIME_MS                              \
    10000 // 10 second baseline collection

// =============================================================================
// SAFETY AND FAULT HANDLING
// =============================================================================

/**
 * @brief Optimization safety limits
 *
 * Safety limits specific to optimization algorithms. These work in conjunction
 * with the SSOT safety configuration to prevent unsafe optimization behavior.
 */
#define OPTIMIZATION_MAX_CURRENT_CHANGE_PER_STEP_A                            \
    0.2f // Maximum current change per iteration
#define OPTIMIZATION_MAX_SPEED_CHANGE_PER_STEP_RPM                            \
    100.0f // Maximum speed change per iteration
#define OPTIMIZATION_MIN_EFFICIENCY_THRESHOLD_PERCENT                         \
    50.0f                                      // Minimum acceptable efficiency
#define OPTIMIZATION_FAULT_DETECTION_SAMPLES 3 // Consecutive fault samples

/**
 * @brief Optimization fault conditions
 *
 * Defines fault conditions that will cause optimization to stop
 * and revert to safe default parameters.
 */
typedef enum {
    OPTIMIZATION_FAULT_NONE = 0,               ///< No fault detected
    OPTIMIZATION_FAULT_EFFICIENCY_DEGRADATION, ///< Efficiency getting worse
    OPTIMIZATION_FAULT_CURRENT_LIMIT_EXCEEDED, ///< Current safety limit
                                               ///< exceeded
    OPTIMIZATION_FAULT_SPEED_LIMIT_EXCEEDED,   ///< Speed safety limit exceeded
    OPTIMIZATION_FAULT_TEMPERATURE_LIMIT,      ///< Temperature limit exceeded
    OPTIMIZATION_FAULT_TELEMETRY_TIMEOUT,      ///< Telemetry data timeout
    OPTIMIZATION_FAULT_CONVERGENCE_TIMEOUT,    ///< Failed to converge in time
    OPTIMIZATION_FAULT_INVALID_PARAMETERS, ///< Invalid optimization parameters
    OPTIMIZATION_FAULT_COUNT
} OptimizationFault_t;

/**
 * @brief Fault recovery configuration
 *
 * Parameters controlling automatic fault recovery and fallback behavior
 * when optimization faults are detected.
 */
#define OPTIMIZATION_FAULT_RECOVERY_ATTEMPTS 3 // Number of recovery attempts
#define OPTIMIZATION_FAULT_RECOVERY_DELAY_MS                                  \
    5000                                     // Delay between recovery attempts
#define OPTIMIZATION_FALLBACK_CURRENT_A 1.0f // Safe fallback current
#define OPTIMIZATION_FALLBACK_SPEED_RPM 500.0f // Safe fallback speed

// =============================================================================
// SIMULATION AND TESTING
// =============================================================================

/**
 * @brief Simulation mode configuration
 *
 * Parameters specific to simulation mode operation for testing optimization
 * algorithms without hardware dependencies.
 */
#define OPTIMIZATION_SIMULATION_NOISE_FACTOR 0.02f  // 2% measurement noise
#define OPTIMIZATION_SIMULATION_UPDATE_RATE_HZ 1000 // 1 kHz simulation rate
#define OPTIMIZATION_SIMULATION_THERMAL_TIME_CONSTANT                         \
    30.0f // 30 second thermal time constant
#define OPTIMIZATION_SIMULATION_LOAD_VARIATION_PERCENT                        \
    10.0f // 10% load variation

/**
 * @brief Mock data generation for testing
 *
 * Parameters for generating realistic mock telemetry and characterization
 * data during unit testing and simulation.
 */
#define OPTIMIZATION_MOCK_BASELINE_CURRENT_A 1.5f    // Mock baseline current
#define OPTIMIZATION_MOCK_BASELINE_SPEED_RPM 1000.0f // Mock baseline speed
#define OPTIMIZATION_MOCK_BASELINE_EFFICIENCY_PERCENT                         \
    72.0f // Mock baseline efficiency
#define OPTIMIZATION_MOCK_TEMPERATURE_AMBIENT_C                               \
    25.0f // Mock ambient temperature

// =============================================================================
// INTEGRATION WITH OTHER SYSTEMS
// =============================================================================

/**
 * @brief Motor characterization integration
 *
 * Parameters for integrating with the motor characterization system (FTR-002)
 * to use characterized motor parameters in optimization algorithms.
 */
#define OPTIMIZATION_CHARACTERIZATION_REQUIRED                                \
    true // Characterization data required
#define OPTIMIZATION_CHARACTERIZATION_TIMEOUT_MS                              \
    5000 // Timeout waiting for characterization
#define OPTIMIZATION_USE_CHARACTERIZED_CURRENT                                \
    true // Use optimal current from characterization
#define OPTIMIZATION_CHARACTERIZATION_CONFIDENCE_MIN                          \
    0.8f // Minimum characterization confidence

/**
 * @brief FreeRTOS integration
 *
 * Task priority and timing parameters for optimization task integration
 * with the FreeRTOS system (FTR-005).
 */
#define OPTIMIZATION_TASK_PRIORITY                                            \
    (RTOS_PRIORITY_MOTOR_CONTROL - 1)     // Lower than motor control
#define OPTIMIZATION_TASK_STACK_SIZE 2048 // Task stack size in words
#define OPTIMIZATION_TASK_PERIOD_MS 10    // 100 Hz optimization rate
#define OPTIMIZATION_QUEUE_SIZE 10        // Command queue size

// =============================================================================
// VALIDATION MACROS
// =============================================================================

/**
 * @brief Compile-time validation of optimization configuration
 *
 * Static assertions to validate SSOT configuration consistency at compile
 * time.
 * NOTE: Float comparisons cannot be used in static assertions,
 * runtime validation is implemented in optimization module init functions.
 */
// _Static_assert(OPTIMIZATION_TARGET_EFFICIENCY_PERCENT >
//                   OPTIMIZATION_BASELINE_EFFICIENCY_PERCENT,
//               "Target efficiency must be greater than baseline efficiency");

// _Static_assert(OPTIMIZATION_MIN_IMPROVEMENT_PERCENT <
//                   OPTIMIZATION_MAX_IMPROVEMENT_PERCENT,
//               "Minimum improvement must be less than maximum improvement");

// _Static_assert(
//    ADAPTIVE_CURRENT_MAX_CURRENT_A <= MOTOR_MAX_CURRENT_A,
//    "Adaptive current limit must not exceed SSOT motor current limit");

// _Static_assert(SPEED_PROFILE_MAX_SPEED_RPM <= MOTOR_MAX_SPEED_RPM,
//               "Speed profile limit must not exceed SSOT motor speed limit");

_Static_assert(OPTIMIZATION_TELEMETRY_UPDATE_RATE_HZ <= 1000,
               "Telemetry update rate must not exceed 1 kHz");

_Static_assert(OPTIMIZATION_CONVERGENCE_TIMEOUT_MS >= 10000,
               "Convergence timeout must be at least 10 seconds");

#ifdef __cplusplus
}
#endif

#endif // OPTIMIZATION_CONFIG_H

/**
 * @brief SSOT Integration Notes
 *
 * This configuration file integrates with the following SSOT configuration
 * files:
 * - motor_config.h: Motor physical limits and characteristics
 * - safety_config.h: Safety thresholds and fault handling
 * - system_config.h: System-wide configuration parameters
 *
 * All optimization algorithms must reference values from this file to maintain
 * SSOT compliance and ensure consistent behavior across the system.
 */
