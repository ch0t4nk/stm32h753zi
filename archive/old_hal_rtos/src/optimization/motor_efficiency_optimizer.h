/**
 * @file motor_efficiency_optimizer.h
 * @brief Motor Efficiency Optimization Module - FTR-009 Implementation
 * @author STM32H753ZI Project Team
 * @date 2025-08-10
 *
 * @note This module implements efficiency optimization algorithms based on
 * motor characterization data from FTR-001 (Optimization Telemetry) and
 * FTR-002 (Motor Characterization). Designed for simulation mode initially,
 * with hardware validation planned for FTR-008.
 *
 * TODO: See .github/instructions/core-software.instructions.md for
 * architecture guidance
 * TODO: See .github/instructions/safety-rt.instructions.md for safety
 * integration
 */

#ifndef MOTOR_EFFICIENCY_OPTIMIZER_H
#define MOTOR_EFFICIENCY_OPTIMIZER_H

#include "common/data_types.h"
#include "common/error_codes.h"
#include "config/motor_config.h"
#include "config/optimization_config.h"
#include "config/safety_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 */
/* Optimization Algorithm Types (SSOT)                                       */
/* ==========================================================================
 */

/**
 * @brief Efficiency optimization algorithm types
 */
typedef enum {
  EFFICIENCY_ALG_NONE = 0,          // No optimization active
  EFFICIENCY_ALG_BASIC_PID,         // Basic PID parameter optimization
  EFFICIENCY_ALG_ADAPTIVE_CURRENT,  // Adaptive current optimization
  EFFICIENCY_ALG_LOAD_COMPENSATION, // Load-based compensation
  EFFICIENCY_ALG_PREDICTIVE_TORQUE, // Predictive torque control
  EFFICIENCY_ALG_MULTI_MODAL,       // Multi-modal optimization
  EFFICIENCY_ALG_COUNT              // Number of algorithms
} EfficiencyAlgorithm_t;

/**
 * @brief Optimization target parameters
 */
typedef enum {
  OPT_TARGET_POWER_CONSUMPTION = 0, // Minimize power consumption
  OPT_TARGET_POSITIONING_ACCURACY,  // Maximize positioning accuracy
  OPT_TARGET_SETTLING_TIME,         // Minimize settling time
  OPT_TARGET_VIBRATION,             // Minimize vibration
  OPT_TARGET_THERMAL,               // Optimize thermal characteristics
  OPT_TARGET_MULTI_OBJECTIVE,       // Multi-objective optimization
  OPT_TARGET_COUNT                  // Number of targets
} OptimizationTarget_t;

/**
 * @brief Optimization state machine
 */
typedef enum {
  OPT_STATE_UNINITIALIZED = 0, // Not initialized
  OPT_STATE_IDLE,              // Ready but not optimizing
  OPT_STATE_CHARACTERIZING,    // Collecting baseline data
  OPT_STATE_OPTIMIZING,        // Active optimization
  OPT_STATE_CONVERGED,         // Optimization converged
  OPT_STATE_VALIDATION,        // Validating results
  OPT_STATE_FAULT,             // Optimization fault
  OPT_STATE_COUNT              // Number of states
} OptimizationState_t;

/* ==========================================================================
 */
/* Optimization Configuration Structures                                     */
/* ==========================================================================
 */

/**
 * @brief Optimization configuration parameters
 */
typedef struct {
  EfficiencyAlgorithm_t algorithm;   // Selected algorithm
  OptimizationTarget_t target;       // Optimization target
  float target_efficiency_pct;       // Target efficiency improvement (%)
  float convergence_threshold;       // Convergence threshold
  uint32_t max_iterations;           // Maximum optimization iterations
  uint32_t characterization_time_ms; // Time for baseline characterization
  bool safety_limits_enabled;        // Enable safety limit checking
  bool real_time_adaptation;         // Enable real-time parameter adaptation
  float adaptation_rate;             // Parameter adaptation rate (0.0-1.0)
  uint32_t validation_time_ms;       // Time for result validation
} OptimizationConfig_t;

/**
 * @brief Motor performance metrics for optimization
 */
typedef struct {
  // Power metrics
  float power_consumption_w; // Total power consumption (watts)
  float motor_current_rms_a; // RMS motor current (amperes)
  float supply_voltage_v;    // Supply voltage (volts)
  float efficiency_pct;      // Motor efficiency percentage

  // Motion metrics
  float position_error_rms_deg; // RMS position error (degrees)
  float settling_time_ms;       // Time to settle within tolerance
  float overshoot_pct;          // Position overshoot percentage
  float rise_time_ms;           // Time to reach 90% of target

  // Dynamic metrics
  float vibration_rms_g; // RMS vibration (g-force, if available)
  float noise_level_db;  // Noise level (dB, if available)
  float temperature_c;   // Motor temperature (°C)
  float load_torque_nm;  // Estimated load torque (N⋅m)

  // Timing metrics
  timestamp_ms_t measurement_time; // When metrics were collected
  bool metrics_valid;              // Data validity flag
  uint32_t sample_count;           // Number of samples in metrics
} PerformanceMetrics_t;

/**
 * @brief Optimization results and status
 */
typedef struct {
  OptimizationState_t state;              // Current optimization state
  EfficiencyAlgorithm_t active_algorithm; // Currently active algorithm

  // Baseline performance (before optimization)
  PerformanceMetrics_t baseline_metrics;

  // Current performance (during/after optimization)
  PerformanceMetrics_t current_metrics;

  // Optimization progress
  float efficiency_improvement_pct; // Achieved efficiency improvement
  uint32_t iterations_completed;    // Optimization iterations completed
  uint32_t convergence_count;       // Times convergence criteria met
  bool optimization_converged;      // Optimization has converged

  // Optimized parameters
  float optimized_kp;             // Optimized proportional gain
  float optimized_ki;             // Optimized integral gain
  float optimized_kd;             // Optimized derivative gain
  float optimized_current_pct;    // Optimized current percentage
  float optimized_microstep_mode; // Optimized microstepping setting

  // Statistics
  timestamp_ms_t optimization_start_time; // When optimization started
  uint32_t total_optimization_time_ms;    // Total optimization time
  SystemError_t last_error;               // Last error encountered
  uint16_t error_count;                   // Total error count
} OptimizationResults_t;

/* ==========================================================================
 */
/* Optimization Context Structure                                            */
/* ==========================================================================
 */

/**
 * @brief Complete optimization context for a motor
 */
typedef struct {
  motor_id_t motor_id;           // Motor identifier
  OptimizationConfig_t config;   // Optimization configuration
  OptimizationResults_t results; // Current results and status

  // Algorithm-specific data
  void *algorithm_data;       // Algorithm-specific state data
  size_t algorithm_data_size; // Size of algorithm data

  // Safety monitoring
  bool safety_override_active; // Safety override engaged
  SystemError_t safety_error;  // Safety-related error

  // Real-time control integration
  bool control_loop_active;        // Optimization control loop active
  uint32_t control_loop_period_ms; // Control loop update period
  timestamp_ms_t last_update_time; // Last optimization update

  // Telemetry integration (FTR-001)
  bool telemetry_enabled;     // Telemetry data collection enabled
  uint32_t telemetry_samples; // Number of telemetry samples

  // Motor characterization integration (FTR-002)
  bool characterization_complete; // Motor characterization complete
  void *characterization_data;    // Motor characterization results
} OptimizationContext_t;

/* ==========================================================================
 */
/* Public Function Declarations                                              */
/* ==========================================================================
 */

/**
 * @brief Initialize motor efficiency optimization system
 * @return SystemError_t Success or error code
 */
SystemError_t motor_efficiency_optimizer_init(void);

/**
 * @brief Deinitialize motor efficiency optimization system
 * @return SystemError_t Success or error code
 */
SystemError_t motor_efficiency_optimizer_deinit(void);

/**
 * @brief Configure optimization for a specific motor
 * @param motor_id Motor identifier
 * @param config Optimization configuration
 * @return SystemError_t Success or error code
 */
SystemError_t configure_motor_optimization(motor_id_t motor_id,
                                           const OptimizationConfig_t *config);

/**
 * @brief Start optimization process for a motor
 * @param motor_id Motor identifier
 * @return SystemError_t Success or error code
 */
SystemError_t start_motor_optimization(motor_id_t motor_id);

/**
 * @brief Stop optimization process for a motor
 * @param motor_id Motor identifier
 * @return SystemError_t Success or error code
 */
SystemError_t stop_motor_optimization(motor_id_t motor_id);

/**
 * @brief Update optimization algorithm (called from control loop)
 * @param motor_id Motor identifier
 * @param dt Time step since last update (seconds)
 * @return SystemError_t Success or error code
 */
SystemError_t update_motor_optimization(motor_id_t motor_id, float dt);

/**
 * @brief Get current optimization results
 * @param motor_id Motor identifier
 * @param results Pointer to results structure
 * @return SystemError_t Success or error code
 */
SystemError_t get_optimization_results(motor_id_t motor_id,
                                       OptimizationResults_t *results);

/**
 * @brief Get current performance metrics
 * @param motor_id Motor identifier
 * @param metrics Pointer to metrics structure
 * @return SystemError_t Success or error code
 */
SystemError_t get_performance_metrics(motor_id_t motor_id,
                                      PerformanceMetrics_t *metrics);

/**
 * @brief Validate optimization results against acceptance criteria
 * @param motor_id Motor identifier
 * @param expected_improvement Expected efficiency improvement (%)
 * @return SystemError_t Success or error code
 */
SystemError_t validate_optimization_results(motor_id_t motor_id,
                                            float expected_improvement);

/**
 * @brief Reset optimization to baseline parameters
 * @param motor_id Motor identifier
 * @return SystemError_t Success or error code
 */
SystemError_t reset_optimization_to_baseline(motor_id_t motor_id);

/* ==========================================================================
 */
/* Simulation Mode Functions                                                 */
/* ==========================================================================
 */

/**
 * @brief Enable simulation mode for testing without hardware
 * @param enable true to enable simulation mode
 * @return SystemError_t Success or error code
 */
SystemError_t set_simulation_mode(bool enable);

/**
 * @brief Inject simulated performance data for testing
 * @param motor_id Motor identifier
 * @param metrics Simulated performance metrics
 * @return SystemError_t Success or error code
 */
SystemError_t inject_simulated_metrics(motor_id_t motor_id,
                                       const PerformanceMetrics_t *metrics);

/**
 * @brief Run optimization simulation with predefined scenarios
 * @param motor_id Motor identifier
 * @param scenario_id Simulation scenario identifier
 * @return SystemError_t Success or error code
 */
SystemError_t run_optimization_simulation(motor_id_t motor_id,
                                          uint8_t scenario_id);

/* ==========================================================================
 */
/* Integration Functions (FTR-001, FTR-002)                                 */
/* ==========================================================================
 */

/**
 * @brief Integrate with optimization telemetry system (FTR-001)
 * @param motor_id Motor identifier
 * @param enable Enable telemetry integration
 * @return SystemError_t Success or error code
 */
SystemError_t integrate_optimization_telemetry(motor_id_t motor_id,
                                               bool enable);

/**
 * @brief Integrate with motor characterization module (FTR-002)
 * @param motor_id Motor identifier
 * @param characterization_data Motor characterization results
 * @return SystemError_t Success or error code
 */
SystemError_t
integrate_motor_characterization(motor_id_t motor_id,
                                 const void *characterization_data);

/* ==========================================================================
 */
/* Configuration Constants (SSOT)                                            */
/* ==========================================================================
 */

// Default optimization configuration
#define DEFAULT_EFFICIENCY_TARGET_PCT 20.0f   // Target 20% improvement
#define DEFAULT_CONVERGENCE_THRESHOLD 0.01f   // 1% convergence threshold
#define DEFAULT_MAX_ITERATIONS 100            // Maximum 100 iterations
#define DEFAULT_CHARACTERIZATION_TIME_MS 5000 // 5 seconds characterization
#define DEFAULT_VALIDATION_TIME_MS 2000       // 2 seconds validation
#define DEFAULT_ADAPTATION_RATE 0.1f          // 10% adaptation rate

// Safety limits for optimization
#define MAX_CURRENT_INCREASE_PCT 10.0f  // Max 10% current increase
#define MAX_SPEED_INCREASE_PCT 5.0f     // Max 5% speed increase
#define MIN_POSITION_ACCURACY_DEG 0.05f // Min 0.05° accuracy
#define MAX_OPTIMIZATION_TIME_MIN 10    // Max 10 minutes optimization

// Performance thresholds
#define MIN_EFFICIENCY_IMPROVEMENT_PCT 5.0f  // Minimum meaningful improvement
#define MAX_EFFICIENCY_IMPROVEMENT_PCT 50.0f // Maximum realistic improvement
#define CONVERGENCE_STABILITY_COUNT 5 // Stable iterations for convergence

#endif /* MOTOR_EFFICIENCY_OPTIMIZER_H */

/**
 * @note FTR-009 Implementation Notes:
 * 1. Designed for simulation mode initially (FTR-008 hardware dependency)
 * 2. Integrates with FTR-001 (telemetry) and FTR-002 (characterization)
 * 3. Follows core-software.instructions.md architecture patterns
 * 4. Uses HAL abstraction for hardware independence
 * 5. Implements comprehensive error handling and safety limits
 * 6. Target: 15-25% efficiency improvement as per acceptance criteria
 */
