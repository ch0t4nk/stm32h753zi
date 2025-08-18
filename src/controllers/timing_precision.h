/**
 * @file timing_precision.h
 * @brief High-Precision Timing System for Safety Validation
 * @author STM32H753ZI Project Team
 * @date 2025-08-12
 *
 * @details Microsecond precision timing system for safety-critical response
 * time validation and real-time control loop monitoring following
 *          .github/instructions/safety-rt.instructions.md
 *
 * @note SAFETY-CRITICAL: This system provides timing validation for emergency
 *       stop response times and real-time control loop performance monitoring
 */

#ifndef TIMING_PRECISION_H
#define TIMING_PRECISION_H

#include "common/system_state.h"
#include "config/safety_config.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 */
/* Type Definitions                                                          */
/* ==========================================================================
 */

/**
 * @brief High-precision timing configuration
 */
typedef struct {
  bool initialized;            ///< Timing system initialized
  uint32_t timer_frequency_hz; ///< Timer frequency in Hz
  uint32_t max_measurement_us; ///< Maximum measurable time in microseconds
  uint32_t measurement_count;  ///< Total measurements taken
  uint32_t overrun_count;      ///< Timing overrun count
} TimingPrecisionConfig_t;

/**
 * @brief Timing measurement result
 */
typedef struct {
  uint32_t start_time_us; ///< Start timestamp in microseconds
  uint32_t end_time_us;   ///< End timestamp in microseconds
  uint32_t duration_us;   ///< Measured duration in microseconds
  bool valid;             ///< Measurement is valid
  bool overrun;           ///< Timing overrun detected
} TimingMeasurement_t;

/**
 * @brief Safety timing validation results
 */
typedef struct {
  uint32_t emergency_stop_time_us;     ///< Last emergency stop response time
  uint32_t max_emergency_stop_time_us; ///< Maximum emergency stop time seen
  uint32_t control_loop_time_us;       ///< Last control loop execution time
  uint32_t max_control_loop_time_us;   ///< Maximum control loop time seen
  uint32_t safety_check_time_us;       ///< Last safety check time
  uint32_t max_safety_check_time_us;   ///< Maximum safety check time seen
  bool emergency_stop_compliant;       ///< Emergency stop within limits
  bool control_loop_compliant;         ///< Control loop within limits
  bool safety_check_compliant;         ///< Safety checks within limits
} SafetyTimingResults_t;

/* ==========================================================================
 */
/* Public Function Declarations                                              */
/* ==========================================================================
 */

/**
 * @brief Initialize high-precision timing system
 * @return SystemError_t Success or error code
 *
 * @details Configures TIM2 for microsecond precision timing and initializes
 *          the timing validation system for safety-critical measurements
 */
SystemError_t timing_precision_init(void);

/**
 * @brief Deinitialize timing system
 * @return SystemError_t Success or error code
 */
SystemError_t timing_precision_deinit(void);

/**
 * @brief Get high-resolution timestamp in microseconds
 * @return uint32_t Timestamp in microseconds
 *
 * @details Provides microsecond precision timing for safety validation.
 *          Uses HAL abstraction layer for hardware independence.
 */
uint32_t get_microsecond_timer(void);

/**
 * @brief Precise delay in microseconds
 * @param microseconds Delay time in microseconds
 *
 * @note This is a busy-wait delay for precise timing requirements
 */
void delay_microseconds(uint32_t microseconds);

/**
 * @brief Start timing measurement
 * @param measurement Measurement structure to initialize
 * @return SystemError_t Success or error code
 */
SystemError_t timing_start_measurement(TimingMeasurement_t *measurement);

/**
 * @brief End timing measurement and calculate duration
 * @param measurement Measurement structure to complete
 * @return SystemError_t Success or error code
 */
SystemError_t timing_end_measurement(TimingMeasurement_t *measurement);

/**
 * @brief Validate emergency stop response time
 * @param response_time_us Response time in microseconds
 * @return bool True if within safety limits, false otherwise
 */
bool timing_validate_emergency_stop(uint32_t response_time_us);

/**
 * @brief Validate control loop execution time
 * @param execution_time_us Execution time in microseconds
 * @return bool True if within real-time limits, false otherwise
 */
bool timing_validate_control_loop(uint32_t execution_time_us);

/**
 * @brief Validate safety check execution time
 * @param check_time_us Safety check time in microseconds
 * @return bool True if within limits, false otherwise
 */
bool timing_validate_safety_check(uint32_t check_time_us);

/**
 * @brief Get safety timing validation results
 * @param results Pointer to results structure
 * @return SystemError_t Success or error code
 */
SystemError_t timing_get_safety_results(SafetyTimingResults_t *results);

/**
 * @brief Reset timing statistics
 * @return SystemError_t Success or error code
 */
SystemError_t timing_reset_statistics(void);

/**
 * @brief Get timing system configuration
 * @param config Pointer to configuration structure
 * @return SystemError_t Success or error code
 */
SystemError_t timing_get_config(TimingPrecisionConfig_t *config);

/* ==========================================================================
 */
/* Safety Timing Validation Macros                                          */
/* ==========================================================================
 */

/**
 * @brief Validate emergency stop timing compliance
 * @param time_us Time in microseconds
 */
#define TIMING_VALIDATE_EMERGENCY_STOP(time_us)                                \
  ((time_us) <= TIMING_EMERGENCY_STOP_MAX_US)

/**
 * @brief Validate control loop timing compliance
 * @param time_us Time in microseconds
 */
#define TIMING_VALIDATE_CONTROL_LOOP(time_us)                                  \
  ((time_us) <= TIMING_CONTROL_LOOP_MAX_US)

/**
 * @brief Validate safety check timing compliance
 * @param time_us Time in microseconds
 */
#define TIMING_VALIDATE_SAFETY_CHECK(time_us)                                  \
  ((time_us) <= TIMING_SAFETY_CHECK_MAX_US)

/* ==========================================================================
 */
/* Configuration Constants (SSOT Reference)                                 */
/* ==========================================================================
 */

/// @brief Maximum allowed emergency stop response time (microseconds)
#define TIMING_EMERGENCY_STOP_MAX_US (ESTOP_REACTION_TIME_MS * 1000)

/// @brief Maximum allowed control loop execution time (microseconds)
#define TIMING_CONTROL_LOOP_MAX_US (MOTOR_CONTROL_LOOP_PERIOD_TICKS * 800)

/// @brief Maximum allowed safety check execution time (microseconds)
#define TIMING_SAFETY_CHECK_MAX_US (SAFETY_MONITOR_PERIOD_TICKS * 500)

/// @brief Timer frequency for timing calculations
#define TIMING_TIMER_FREQUENCY_HZ 1000000U // 1 MHz for microsecond precision

#ifdef __cplusplus
}
#endif

#endif /* TIMING_PRECISION_H */
