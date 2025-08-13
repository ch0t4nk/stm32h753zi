/**
 * @file dynamic_task_tuning.h
 * @brief Dynamic Task Period Adjustment and CPU Utilization Optimization
 * @author STM32H753ZI Project Team
 * @date August 2025
 * @version 1.0
 *
 * This module provides intelligent task period adjustment based on CPU
 * utilization, system load, and performance metrics with industrial-grade
 * optimization.
 */

#ifndef DYNAMIC_TASK_TUNING_H
#define DYNAMIC_TASK_TUNING_H

/* ============================================================================
 */
/* Includes */
/* ============================================================================
 */

#include "FreeRTOS.h"
#include "common/error_codes.h"
#include "config/freertos_config_ssot.h"
#include "task.h"

#include <stdbool.h>
#include <stdint.h>

/* ============================================================================
 */
/* Constants */
/* ============================================================================
 */

/// @brief History size for dynamic tuning calculations
#define DYNAMIC_TUNING_HISTORY_SIZE 10

/// @brief Maximum number of tasks that can be tuned
#define MAX_TUNABLE_TASKS 8

/* ============================================================================
 */
/* Type Definitions */
/* ============================================================================
 */

/// @brief Task tuning parameters for each task
typedef struct {
  TaskHandle_t task_handle;        ///< FreeRTOS task handle
  const char *task_name;           ///< Task name for identification
  uint32_t base_period_ms;         ///< Base period in milliseconds
  uint32_t current_period_ms;      ///< Current adjusted period
  uint32_t min_period_ms;          ///< Minimum allowed period
  uint32_t max_period_ms;          ///< Maximum allowed period
  uint32_t adjustment_step_ms;     ///< Period adjustment step size
  bool enabled;                    ///< Tuning enabled for this task
  uint32_t last_execution_time_us; ///< Last execution time in microseconds
  uint32_t max_execution_time_us;  ///< Maximum execution time observed
  uint32_t avg_execution_time_us;  ///< Average execution time
  uint32_t execution_count;        ///< Number of executions counted
  uint32_t overrun_count;          ///< Number of timing overruns
} TaskTuningParams_t;

/// @brief System performance metrics
typedef struct {
  uint32_t cpu_utilization_percent;  ///< Current CPU utilization (0-100%)
  uint32_t idle_time_percent;        ///< Idle task percentage
  uint32_t peak_cpu_utilization;     ///< Peak CPU utilization observed
  uint32_t avg_cpu_utilization;      ///< Average CPU utilization
  uint32_t total_tasks;              ///< Total number of tasks
  uint32_t active_tasks;             ///< Number of active tasks
  uint32_t context_switches_per_sec; ///< Context switches per second
  uint32_t stack_high_water_mark;    ///< Minimum stack space across all tasks
  uint32_t heap_free_bytes;          ///< Available heap memory
  uint32_t measurement_period_ms;    ///< Measurement period for metrics
} SystemPerformanceMetrics_t;

/// @brief Tuning algorithm configuration
typedef struct {
  uint32_t measurement_window_ms;     ///< Performance measurement window
  uint32_t tuning_interval_ms;        ///< Interval between tuning adjustments
  uint32_t target_cpu_utilization;    ///< Target CPU utilization percentage
  uint32_t cpu_utilization_tolerance; ///< Tolerance around target
  uint32_t aggressive_threshold;      ///< Threshold for aggressive tuning
  bool adaptive_tuning_enabled;       ///< Enable adaptive algorithm
  bool conservative_mode;             ///< Conservative tuning mode
} TuningConfiguration_t;

/// @brief Task performance history for predictive tuning
typedef struct {
  uint32_t execution_history[DYNAMIC_TUNING_HISTORY_SIZE]; ///< Execution
                                                           ///< time history
  uint32_t period_history[DYNAMIC_TUNING_HISTORY_SIZE]; ///< Period adjustment
                                                        ///< history
  uint32_t
      utilization_history[DYNAMIC_TUNING_HISTORY_SIZE]; ///< CPU utilization
                                                        ///< history
  uint8_t history_index;    ///< Current history index
  bool history_full;        ///< History buffer is full
  uint32_t trend_direction; ///< Performance trend (0=stable, 1=improving,
                            ///< 2=degrading)
} TaskPerformanceHistory_t;

/* ============================================================================
 */
/* Constants */
/* ============================================================================
 */

/// @brief Default tuning configuration values
#define DEFAULT_MEASUREMENT_WINDOW_MS 5000 ///< 5 second measurement window
#define DEFAULT_TUNING_INTERVAL_MS 10000   ///< 10 second tuning interval
#define DEFAULT_TARGET_CPU_UTIL 60         ///< Target 60% CPU utilization
#define DEFAULT_CPU_UTIL_TOLERANCE 10      ///< ±10% tolerance

/* ============================================================================
 */
/* Public API */
/* ============================================================================
 */

/**
 * @brief Initialize dynamic task tuning system
 * @return SYSTEM_OK on success, error code on failure
 * @note MISRA-C: Initialization function with clear error handling
 * @note IEC 61508: SIL-2 - Safe initialization with validation
 */
SystemError_t dynamic_tuning_init(void);

/**
 * @brief Register task for dynamic tuning
 * @param task_handle FreeRTOS task handle
 * @param task_name Task name for identification
 * @param base_period_ms Base period in milliseconds
 * @param min_period_ms Minimum allowed period
 * @param max_period_ms Maximum allowed period
 * @return SYSTEM_OK on success, error code on failure
 * @note IEC 61508: SIL-2 - Parameter validation for safety
 */
SystemError_t dynamic_tuning_register_task(TaskHandle_t task_handle,
                                           const char *task_name,
                                           uint32_t base_period_ms,
                                           uint32_t min_period_ms,
                                           uint32_t max_period_ms);

/**
 * @brief Update performance metrics and execute tuning algorithm
 * @return SYSTEM_OK on success, error code on failure
 * @note Called periodically from system monitoring task
 * @note MISRA-C: Pure function with well-defined behavior
 */
SystemError_t dynamic_tuning_update(void);

/**
 * @brief Record task execution time for performance analysis
 * @param task_handle Task handle
 * @param execution_time_us Execution time in microseconds
 * @return SYSTEM_OK on success, error code on failure
 * @note Called from task completion hooks
 * @note High-frequency function - optimized for minimal overhead
 */
SystemError_t dynamic_tuning_record_execution(TaskHandle_t task_handle,
                                              uint32_t execution_time_us);

/**
 * @brief Get current performance metrics
 * @param metrics Pointer to metrics structure to fill
 * @return SYSTEM_OK on success, error code on failure
 * @note MISRA-C: Input validation and null pointer checks
 */
SystemError_t dynamic_tuning_get_metrics(SystemPerformanceMetrics_t *metrics);

/**
 * @brief Get task tuning parameters
 * @param task_handle Task handle
 * @param params Pointer to parameters structure to fill
 * @return SYSTEM_OK on success, error code on failure
 * @note Used for monitoring and debugging task performance
 */
SystemError_t dynamic_tuning_get_task_params(TaskHandle_t task_handle,
                                             TaskTuningParams_t *params);

/**
 * @brief Configure tuning algorithm parameters
 * @param config Pointer to configuration structure
 * @return SYSTEM_OK on success, error code on failure
 * @note IEC 61508: SIL-2 - Configuration validation and bounds checking
 */
SystemError_t dynamic_tuning_configure(const TuningConfiguration_t *config);

/**
 * @brief Enable or disable tuning for specific task
 * @param task_handle Task handle
 * @param enabled Enable (true) or disable (false) tuning
 * @return SYSTEM_OK on success, error code on failure
 * @note Used to temporarily disable tuning for critical tasks
 */
SystemError_t dynamic_tuning_set_task_enabled(TaskHandle_t task_handle,
                                              bool enabled);

/**
 * @brief Force specific period for task (override tuning)
 * @param task_handle Task handle
 * @param period_ms Forced period in milliseconds
 * @return SYSTEM_OK on success, error code on failure
 * @note Used for manual override or emergency situations
 * @note IEC 61508: SIL-2 - Manual override with safety validation
 */
SystemError_t dynamic_tuning_force_task_period(TaskHandle_t task_handle,
                                               uint32_t period_ms);

/**
 * @brief Reset tuning algorithm and performance history
 * @return SYSTEM_OK on success, error code on failure
 * @note Used after system configuration changes
 */
SystemError_t dynamic_tuning_reset(void);

/**
 * @brief Generate comprehensive performance report
 * @param buffer Buffer to write report to
 * @param buffer_size Size of buffer
 * @return Number of bytes written, or 0 on error
 * @note Generates human-readable performance analysis
 * @note MISRA-C: Bounded buffer operations
 */
uint32_t dynamic_tuning_generate_report(char *buffer, uint32_t buffer_size);

/**
 * @brief Get predictive performance estimate
 * @param task_handle Task handle
 * @param future_period_ms Proposed future period
 * @param estimated_cpu_impact Pointer to store estimated CPU impact
 * @return SYSTEM_OK on success, error code on failure
 * @note Uses performance history for predictive analysis
 * @note Advanced feature for AI-driven optimization
 */
SystemError_t dynamic_tuning_predict_impact(TaskHandle_t task_handle,
                                            uint32_t future_period_ms,
                                            uint32_t *estimated_cpu_impact);

/**
 * @brief Emergency CPU load reduction
 * @return SYSTEM_OK on success, error code on failure
 * @note Increases all task periods to reduce CPU load
 * @note IEC 61508: SIL-2 - Emergency function maintaining safety
 */
SystemError_t dynamic_tuning_emergency_load_reduction(void);

/* ============================================================================
 */
/* Callback Functions */
/* ============================================================================
 */

/**
 * @brief Task execution start callback
 * @param task_handle Task handle
 * @note Called when task begins execution (optional hook)
 */
extern void dynamic_tuning_task_start_hook(TaskHandle_t task_handle);

/**
 * @brief Task execution end callback
 * @param task_handle Task handle
 * @param execution_time_us Execution time in microseconds
 * @note Called when task completes execution (optional hook)
 */
extern void dynamic_tuning_task_end_hook(TaskHandle_t task_handle,
                                         uint32_t execution_time_us);

/* ============================================================================
 */
/* Debugging and Diagnostics                                                 */
/* ============================================================================
 */

/**
 * @brief Print current tuning status to console
 * @note Debugging function for development
 */
void dynamic_tuning_print_status(void);

/**
 * @brief Validate tuning algorithm state consistency
 * @return true if state is consistent, false if corruption detected
 * @note IEC 61508: SIL-2 - Self-diagnostic function
 */
bool dynamic_tuning_validate_state(void);

/**
 * @brief Get tuning algorithm statistics
 * @param total_adjustments Pointer to store total adjustments made
 * @param successful_adjustments Pointer to store successful adjustments
 * @param rejected_adjustments Pointer to store rejected adjustments
 * @return SYSTEM_OK on success, error code on failure
 * @note Statistics for algorithm performance analysis
 */
SystemError_t dynamic_tuning_get_statistics(uint32_t *total_adjustments,
                                            uint32_t *successful_adjustments,
                                            uint32_t *rejected_adjustments);

#endif // DYNAMIC_TASK_TUNING_H
