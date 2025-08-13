/**
 * @file timing_precision.c
 * @brief High-Precision Timing System Implementation
 * @author STM32H753ZI Project Team
 * @date 2025-08-12
 *
 * @details Implementation of microsecond precision timing system for safety
 *          validation and real-time control loop monitoring. Uses existing
 *          HAL abstraction layer for hardware independence.
 */

#include "timing_precision.h"
#include "config/hardware_config.h"
#include "config/motor_config.h"
#include "hal_abstraction/hal_abstraction.h"
#include <string.h>

/* ==========================================================================
 */
/* Private Variables                                                         */
/* ==========================================================================
 */

static TimingPrecisionConfig_t timing_config = {0};
static SafetyTimingResults_t safety_results = {0};
static bool timing_system_initialized = false;

/* ==========================================================================
 */
/* Public Function Implementation                                            */
/* ==========================================================================
 */

/**
 * @brief Initialize high-precision timing system
 */
SystemError_t timing_precision_init(void) {
    if (timing_system_initialized) {
        return SYSTEM_OK; // Already initialized
    }

    // Initialize timing configuration
    memset(&timing_config, 0, sizeof(timing_config));
    timing_config.timer_frequency_hz = TIMING_TIMER_FREQUENCY_HZ;
    timing_config.max_measurement_us =
        UINT32_MAX / timing_config.timer_frequency_hz;
    timing_config.measurement_count = 0;
    timing_config.overrun_count = 0;

    // Initialize safety timing results
    memset(&safety_results, 0, sizeof(safety_results));
    safety_results.emergency_stop_compliant = true;
    safety_results.control_loop_compliant = true;
    safety_results.safety_check_compliant = true;

    // Verify that HAL abstraction microsecond timing is available
    uint32_t test_time = HAL_Abstraction_GetMicroseconds();
    if (test_time == 0) {
        return ERROR_TIMER_INIT_FAILED;
    }

    timing_config.initialized = true;
    timing_system_initialized = true;

    return SYSTEM_OK;
}

/**
 * @brief Deinitialize timing system
 */
SystemError_t timing_precision_deinit(void) {
    if (!timing_system_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Reset configuration
    memset(&timing_config, 0, sizeof(timing_config));
    memset(&safety_results, 0, sizeof(safety_results));

    timing_system_initialized = false;

    return SYSTEM_OK;
}

/**
 * @brief Get high-resolution timestamp in microseconds
 */
uint32_t get_microsecond_timer(void) {
    if (!timing_system_initialized) {
        return 0;
    }

    // Use HAL abstraction for hardware-independent microsecond timing
    return HAL_Abstraction_GetMicroseconds();
}

/**
 * @brief Precise delay in microseconds
 */
void delay_microseconds(uint32_t microseconds) {
    if (!timing_system_initialized || microseconds == 0) {
        return;
    }

    uint32_t start_time = get_microsecond_timer();
    while ((get_microsecond_timer() - start_time) < microseconds) {
        // Busy wait for precise timing
        __NOP();
    }
}

/**
 * @brief Start timing measurement
 */
SystemError_t timing_start_measurement(TimingMeasurement_t *measurement) {
    if (!timing_system_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    if (measurement == NULL) {
        return ERROR_NULL_POINTER;
    }

    // Initialize measurement structure
    memset(measurement, 0, sizeof(TimingMeasurement_t));
    measurement->start_time_us = get_microsecond_timer();
    measurement->valid = true;

    timing_config.measurement_count++;

    return SYSTEM_OK;
}

/**
 * @brief End timing measurement and calculate duration
 */
SystemError_t timing_end_measurement(TimingMeasurement_t *measurement) {
    if (!timing_system_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    if (measurement == NULL) {
        return ERROR_NULL_POINTER;
    }

    if (!measurement->valid) {
        return ERROR_INVALID_STATE;
    }

    measurement->end_time_us = get_microsecond_timer();

    // Calculate duration, handling timer overflow
    if (measurement->end_time_us >= measurement->start_time_us) {
        measurement->duration_us =
            measurement->end_time_us - measurement->start_time_us;
    } else {
        // Timer overflow occurred
        measurement->duration_us = (UINT32_MAX - measurement->start_time_us) +
                                   measurement->end_time_us;
        measurement->overrun = true;
        timing_config.overrun_count++;
    }

    // Check for measurement overrun
    if (measurement->duration_us > timing_config.max_measurement_us) {
        measurement->overrun = true;
        timing_config.overrun_count++;
    }

    return SYSTEM_OK;
}

/**
 * @brief Validate emergency stop response time
 */
bool timing_validate_emergency_stop(uint32_t response_time_us) {
    if (!timing_system_initialized) {
        return false;
    }

    // Update safety results
    safety_results.emergency_stop_time_us = response_time_us;
    if (response_time_us > safety_results.max_emergency_stop_time_us) {
        safety_results.max_emergency_stop_time_us = response_time_us;
    }

    // Check compliance
    bool compliant = TIMING_VALIDATE_EMERGENCY_STOP(response_time_us);
    safety_results.emergency_stop_compliant = compliant;

    return compliant;
}

/**
 * @brief Validate control loop execution time
 */
bool timing_validate_control_loop(uint32_t execution_time_us) {
    if (!timing_system_initialized) {
        return false;
    }

    // Update safety results
    safety_results.control_loop_time_us = execution_time_us;
    if (execution_time_us > safety_results.max_control_loop_time_us) {
        safety_results.max_control_loop_time_us = execution_time_us;
    }

    // Check compliance
    bool compliant = TIMING_VALIDATE_CONTROL_LOOP(execution_time_us);
    safety_results.control_loop_compliant = compliant;

    return compliant;
}

/**
 * @brief Validate safety check execution time
 */
bool timing_validate_safety_check(uint32_t check_time_us) {
    if (!timing_system_initialized) {
        return false;
    }

    // Update safety results
    safety_results.safety_check_time_us = check_time_us;
    if (check_time_us > safety_results.max_safety_check_time_us) {
        safety_results.max_safety_check_time_us = check_time_us;
    }

    // Check compliance
    bool compliant = TIMING_VALIDATE_SAFETY_CHECK(check_time_us);
    safety_results.safety_check_compliant = compliant;

    return compliant;
}

/**
 * @brief Get safety timing validation results
 */
SystemError_t timing_get_safety_results(SafetyTimingResults_t *results) {
    if (!timing_system_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    if (results == NULL) {
        return ERROR_NULL_POINTER;
    }

    // Copy current results
    memcpy(results, &safety_results, sizeof(SafetyTimingResults_t));

    return SYSTEM_OK;
}

/**
 * @brief Reset timing statistics
 */
SystemError_t timing_reset_statistics(void) {
    if (!timing_system_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    // Reset counters but preserve configuration
    timing_config.measurement_count = 0;
    timing_config.overrun_count = 0;

    // Reset safety results
    memset(&safety_results, 0, sizeof(safety_results));
    safety_results.emergency_stop_compliant = true;
    safety_results.control_loop_compliant = true;
    safety_results.safety_check_compliant = true;

    return SYSTEM_OK;
}

/**
 * @brief Get timing system configuration
 */
SystemError_t timing_get_config(TimingPrecisionConfig_t *config) {
    if (!timing_system_initialized) {
        return ERROR_NOT_INITIALIZED;
    }

    if (config == NULL) {
        return ERROR_NULL_POINTER;
    }

    // Copy current configuration
    memcpy(config, &timing_config, sizeof(TimingPrecisionConfig_t));

    return SYSTEM_OK;
}
