/**
 * @file adaptive_current_algorithm.h
 * @brief Adaptive Current Optimization Algorithm - FTR-009 Component
 * @author STM32H753ZI Project Team
 * @date 2025-08-10
 *
 * @note Implements adaptive current optimization algorithm that dynamically
 * adjusts motor current based on efficiency feedback. Part of FTR-009
 * Motor Efficiency Optimization module.
 *
 * TODO: See .github/instructions/core-software.instructions.md for algorithm
 * patterns
 * TODO: See .github/instructions/safety-rt.instructions.md for real-time
 * constraints
 */

#ifndef ADAPTIVE_CURRENT_ALGORITHM_H
#define ADAPTIVE_CURRENT_ALGORITHM_H

#include "common/data_types.h"
#include "common/error_codes.h"
#include "config/optimization_config.h"
#include "motor_efficiency_optimizer.h"
#include <stdbool.h>
#include <stdint.h>

/* ==========================================================================
 */
/* Adaptive Current Algorithm Configuration                                  */
/* ==========================================================================
 */

/**
 * @brief Adaptive current algorithm configuration
 */
typedef struct {
  float initial_current_pct;      // Initial current percentage (50-120%)
  float learning_rate;            // Algorithm learning rate (0.01-0.5)
  float exploration_factor;       // Exploration vs exploitation (0.0-1.0)
  float stability_threshold;      // Stability threshold for convergence
  uint32_t convergence_window;    // Window size for convergence detection
  bool enable_safety_constraints; // Enable safety constraint checking
  float max_current_change_pct;   // Maximum current change per iteration
  float min_efficiency_threshold; // Minimum efficiency before abort
} AdaptiveCurrentConfig_t;

/**
 * @brief Adaptive current algorithm state
 */
typedef struct {
  float current_setpoint_pct;   // Current percentage setpoint
  float previous_efficiency;    // Previous efficiency measurement
  float efficiency_gradient;    // Estimated efficiency gradient
  float learning_rate_adaptive; // Adaptive learning rate

  // Exploration state
  bool exploration_mode;       // Currently exploring vs exploiting
  uint32_t exploration_count;  // Number of exploration steps
  float exploration_direction; // Current exploration direction

  // Convergence tracking
  float efficiency_history[10]; // Recent efficiency history
  uint8_t history_index;        // Current history index
  uint32_t stable_iterations;   // Iterations meeting stability criteria
  bool converged;               // Algorithm convergence status

  // Adaptation statistics
  uint32_t adaptation_count;  // Total adaptations performed
  uint32_t improvement_count; // Successful improvements
  uint32_t degradation_count; // Performance degradations
  float best_efficiency;      // Best efficiency achieved
  float best_current_pct;     // Current setting for best efficiency

  // Safety monitoring
  bool safety_override_active;   // Safety override engaged
  uint32_t safety_violations;    // Number of safety violations
  float min_observed_efficiency; // Minimum efficiency observed
} AdaptiveCurrentState_t;

/* ==========================================================================
 */
/* Algorithm Performance Metrics                                             */
/* ==========================================================================
 */

/**
 * @brief Adaptive current algorithm performance metrics
 */
typedef struct {
  // Convergence metrics
  uint32_t iterations_to_converge;  // Iterations required for convergence
  float convergence_efficiency;     // Efficiency at convergence
  float efficiency_improvement_pct; // Total efficiency improvement

  // Stability metrics
  float efficiency_variance;  // Variance in efficiency measurements
  float current_variance;     // Variance in current settings
  uint32_t oscillation_count; // Number of oscillations detected

  // Exploration metrics
  float exploration_ratio;         // Ratio of exploration vs exploitation
  uint32_t exploration_successes;  // Successful exploration steps
  uint32_t exploitation_successes; // Successful exploitation steps

  // Learning metrics
  float average_learning_rate;     // Average learning rate used
  float gradient_estimation_error; // Error in gradient estimation
  uint32_t gradient_reversals;     // Number of gradient reversals

  // Safety metrics
  uint32_t safety_interventions;  // Number of safety interventions
  float max_current_violation;    // Maximum current violation detected
  uint32_t efficiency_violations; // Efficiency threshold violations

  // Timing metrics
  uint32_t total_execution_time_ms;   // Total algorithm execution time
  uint32_t average_iteration_time_us; // Average iteration time
  uint32_t max_iteration_time_us;     // Maximum iteration time
} AdaptiveCurrentMetrics_t;

/* ==========================================================================
 */
/* Public Function Declarations                                              */
/* ==========================================================================
 */

/**
 * @brief Initialize adaptive current optimization algorithm
 * @param motor_id Motor identifier
 * @param config Algorithm configuration
 * @return SystemError_t Success or error code
 */
SystemError_t adaptive_current_init(motor_id_t motor_id,
                                    const AdaptiveCurrentConfig_t *config);

/**
 * @brief Deinitialize adaptive current optimization algorithm
 * @param motor_id Motor identifier
 * @return SystemError_t Success or error code
 */
SystemError_t adaptive_current_deinit(motor_id_t motor_id);

/**
 * @brief Execute single iteration of adaptive current algorithm
 * @param motor_id Motor identifier
 * @param current_metrics Current performance metrics
 * @param dt Time step since last iteration (seconds)
 * @param optimal_current_pct Output: Optimal current percentage
 * @return SystemError_t Success or error code
 */
SystemError_t adaptive_current_step(motor_id_t motor_id,
                                    const PerformanceMetrics_t *current_metrics,
                                    float dt, float *optimal_current_pct);

/**
 * @brief Check if algorithm has converged
 * @param motor_id Motor identifier
 * @param converged Output: Convergence status
 * @return SystemError_t Success or error code
 */
SystemError_t adaptive_current_check_convergence(motor_id_t motor_id,
                                                 bool *converged);

/**
 * @brief Get algorithm state and metrics
 * @param motor_id Motor identifier
 * @param state Output: Algorithm state
 * @param metrics Output: Algorithm metrics
 * @return SystemError_t Success or error code
 */
SystemError_t adaptive_current_get_status(motor_id_t motor_id,
                                          AdaptiveCurrentState_t *state,
                                          AdaptiveCurrentMetrics_t *metrics);

/**
 * @brief Reset algorithm to initial state
 * @param motor_id Motor identifier
 * @return SystemError_t Success or error code
 */
SystemError_t adaptive_current_reset(motor_id_t motor_id);

/**
 * @brief Update algorithm configuration
 * @param motor_id Motor identifier
 * @param config New configuration
 * @return SystemError_t Success or error code
 */
SystemError_t
adaptive_current_update_config(motor_id_t motor_id,
                               const AdaptiveCurrentConfig_t *config);

/* ==========================================================================
 */
/* Simulation and Testing Functions                                          */
/* ==========================================================================
 */

/**
 * @brief Run algorithm simulation with synthetic data
 * @param motor_id Motor identifier
 * @param scenario_id Simulation scenario
 * @param iterations Number of iterations to simulate
 * @return SystemError_t Success or error code
 */
SystemError_t adaptive_current_simulate(motor_id_t motor_id,
                                        uint8_t scenario_id,
                                        uint32_t iterations);

/**
 * @brief Inject synthetic performance metrics for testing
 * @param motor_id Motor identifier
 * @param metrics Synthetic metrics
 * @return SystemError_t Success or error code
 */
SystemError_t
adaptive_current_inject_metrics(motor_id_t motor_id,
                                const PerformanceMetrics_t *metrics);

/**
 * @brief Generate algorithm performance report
 * @param motor_id Motor identifier
 * @param report_buffer Buffer for report text
 * @param buffer_size Size of report buffer
 * @return SystemError_t Success or error code
 */
SystemError_t adaptive_current_generate_report(motor_id_t motor_id,
                                               char *report_buffer,
                                               size_t buffer_size);

/* ==========================================================================
 */
/* Configuration Constants and Defaults                                      */
/* ==========================================================================
 */

// Default algorithm configuration
#define ADAPTIVE_CURRENT_DEFAULT_INITIAL_PCT 80.0f  // Start at 80% current
#define ADAPTIVE_CURRENT_DEFAULT_LEARNING_RATE 0.1f // 10% learning rate
#define ADAPTIVE_CURRENT_DEFAULT_EXPLORATION 0.2f   // 20% exploration
#define ADAPTIVE_CURRENT_DEFAULT_STABILITY 0.5f     // 0.5% stability threshold
#define ADAPTIVE_CURRENT_DEFAULT_CONVERGENCE_WINDOW 10 // 10 iteration window

// Safety limits
#define ADAPTIVE_CURRENT_MIN_CURRENT_PCT 30.0f    // Minimum 30% current
#define ADAPTIVE_CURRENT_MAX_CURRENT_PCT 120.0f   // Maximum 120% current
#define ADAPTIVE_CURRENT_MAX_CHANGE_PCT 5.0f      // Max 5% change per step
#define ADAPTIVE_CURRENT_MIN_EFFICIENCY_PCT 60.0f // Minimum 60% efficiency

// Performance thresholds
#define ADAPTIVE_CURRENT_MAX_ITERATIONS 200         // Maximum iterations
#define ADAPTIVE_CURRENT_CONVERGENCE_THRESHOLD 0.1f // Convergence threshold
#define ADAPTIVE_CURRENT_OSCILLATION_THRESHOLD 3    // Oscillation detection
#define ADAPTIVE_CURRENT_SAFETY_MARGIN 5.0f         // Safety margin percentage

/* ==========================================================================
 */
/* Algorithm-Specific Error Codes                                           */
/* ==========================================================================
 */

// Extend system error codes for algorithm-specific errors
#define ERROR_ADAPTIVE_CURRENT_BASE 0x7000
#define ERROR_ADAPTIVE_CURRENT_NOT_CONVERGED (ERROR_ADAPTIVE_CURRENT_BASE + 1)
#define ERROR_ADAPTIVE_CURRENT_OSCILLATING (ERROR_ADAPTIVE_CURRENT_BASE + 2)
#define ERROR_ADAPTIVE_CURRENT_SAFETY_VIOLATION                                \
  (ERROR_ADAPTIVE_CURRENT_BASE + 3)
#define ERROR_ADAPTIVE_CURRENT_INVALID_GRADIENT                                \
  (ERROR_ADAPTIVE_CURRENT_BASE + 4)
#define ERROR_ADAPTIVE_CURRENT_LEARNING_RATE_TOO_LOW                           \
  (ERROR_ADAPTIVE_CURRENT_BASE + 5)
#define ERROR_ADAPTIVE_CURRENT_EFFICIENCY_DEGRADED                             \
  (ERROR_ADAPTIVE_CURRENT_BASE + 6)

#endif /* ADAPTIVE_CURRENT_ALGORITHM_H */

/**
 * @note Adaptive Current Algorithm Design Notes:
 * 1. Uses gradient-based optimization with adaptive learning rate
 * 2. Balances exploration vs exploitation for robust convergence
 * 3. Implements safety constraints to prevent motor damage
 * 4. Tracks convergence with stability analysis
 * 5. Designed for real-time operation with minimal computational overhead
 * 6. Supports simulation mode for testing and development
 */
