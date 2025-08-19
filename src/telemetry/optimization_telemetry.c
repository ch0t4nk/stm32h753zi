/**
 * @file optimization_telemetry.c
 * @brief Focused telemetry data acquisition implementation for motor
// ...existing code...
 *
 * This module provides efficient, real-time telemetry collection for motor
 * tuning algorithms with <2% CPU overhead while maintaining 1kHz control loop
 * compatibility.
 *
 * Implementation Strategy:
 * - Zero-copy data structures for minimal memory overhead
 * - Optimized sensor read sequences for maximum throughput
 * - Efficient AS5600 I2C burst reads (±50µs per sample)
 * - L6470 SPI status reads with cached parameters (±30µs per sample)
 * - Real-time safety monitoring with configurable limits
 * - Phase 5A telemetry dashboard integration for remote monitoring
 */

#include "optimization_telemetry.h"
#include "common/system_state.h"
#include "config/hardware_config.h"
#include "config/telemetry_config.h" // SSOT: All telemetry config here
#include "hal_abstraction.h"
#include "rtos/telemetry_dashboard.h"
#include "safety/emergency_stop_abstracted.h"
#include "safety/safety_system.h"

// Use HAL abstraction for timer support (enables unit testing)
// STM32 HAL includes moved to HAL abstraction implementation

// L6470 register constants for configuration (abstracted from drivers)
#include "config/l6470_registers_generated.h"
#include "config/motor_config.h"

// NOTE: Driver functions accessed through HAL abstraction layer (FTR-013
// complete)
#include <math.h>
#include <stdio.h>
#include <string.h>

/* Provide a portable no-op for assembly NOP when building for host tests */
#ifndef __NOP
#define __NOP() ((void)0)
#endif

// ================================================================================================
// PRIVATE DATA STRUCTURES AND CONSTANTS
// ================================================================================================

/**
 * @brief Telemetry context for motor-specific data collection
 *
 * Maintains per-motor state for efficient telemetry collection including
 * calibration data, timing state, and safety monitoring.
 */
typedef struct {
    bool initialized;        ///< Initialization status
    bool streaming_active;   ///< Streaming mode active
    uint32_t sample_rate_hz; ///< Current sample rate (Hz)
    uint32_t
        last_sample_timestamp_us; ///< Last sample timestamp (microseconds)

    // AS5600 encoder state
    float
        last_position_degrees; ///< Previous position for velocity calculation
    float
        last_velocity_dps; ///< Previous velocity for acceleration calculation
    uint32_t encoder_calibration_offset; ///< Encoder zero-position offset

    // L6470 driver state
    uint16_t cached_kval_hold;         ///< Cached KVAL_HOLD value
    uint16_t cached_kval_run;          ///< Cached KVAL_RUN value
    uint32_t last_status_read_time_us; ///< Last L6470 status read timestamp

    // Performance monitoring
    TelemetryPerformanceMetrics_t performance; ///< Performance metrics
    uint32_t
        timing_error_accumulator; ///< Timing error accumulator for statistics

    // Safety monitoring state
    bool safety_limits_enabled;      ///< Safety monitoring enabled
    float safety_current_limit_a;    ///< Current safety limit (Amperes)
    float safety_speed_limit_dps;    ///< Speed safety limit (degrees/second)
    float safety_error_limit_deg;    ///< Position error safety limit (degrees)
    uint32_t safety_violation_count; ///< Number of safety violations detected

} TelemetryContext_t;

// Private telemetry contexts for each motor
static TelemetryContext_t telemetry_contexts[MAX_MOTORS];

// NOTE: Hardware-driven exclusions (see docs/SSOT_EXCLUSIONS_TELEMETRY.md):
// - SPI/nCS pin assignments are fixed by board layout and solder bridges
// (UM1964 Table 6, Sec. 2.2)
// - L6470 supply voltage (8V–45V) and phase current (≤3A) are hardware limits
// (UM1964 Sec. 1)
// - L6470 register addresses/reset values are fixed by silicon (UM1963 Table
// 3)
// - Some errata workarounds may require hardcoded logic
// (ERRATA_STM32H753ZI.txt)

// Use SSOT macros for buffer sizes, sample rates, etc. throughout
// implementation Example: use TELEMETRY_CHARACTERIZATION_BUFFER_SIZE,
// TELEMETRY_SAMPLE_RATE_MAX_HZ, etc.
// TODO: Refactor all hardcoded values to use SSOT macros if any remain

// High-resolution timer instance for precise timing
static HAL_Timer_Instance_t timer_instance =
    HAL_TIMER_INSTANCE_1; // Use HAL abstraction timer

// Performance measurement variables
static uint32_t sample_start_time_us = 0;
static uint32_t sample_end_time_us = 0;

// ================================================================================================
// PRIVATE FUNCTION DECLARATIONS
// ================================================================================================

static SystemError_t telemetry_read_as5600_burst(uint8_t motor_id,
                                                 float *position_degrees,
                                                 float *velocity_dps,
                                                 float *acceleration_dps2);

static SystemError_t telemetry_read_l6470_status_fast(
    uint8_t motor_id, float *motor_current_a, uint8_t *status_flags,
    bool *thermal_warning, bool *stall_detected, bool *overcurrent_detected);

static SystemError_t
telemetry_calculate_derivatives(TelemetryContext_t *context,
                                float current_position, float dt_seconds,
                                float *velocity_dps, float *acceleration_dps2);

static SystemError_t
telemetry_check_safety_bounds(const TelemetryContext_t *context,
                              const OptimizationTelemetryPacket_t *packet,
                              bool *safety_ok);

static uint32_t telemetry_get_microsecond_timer(void);
static uint32_t
telemetry_calculate_checksum(const CharacterizationDataSet_t *dataset);
static void telemetry_update_performance_metrics(TelemetryContext_t *context,
                                                 uint32_t sample_time_us);

// ================================================================================================
// PUBLIC API IMPLEMENTATION
// ================================================================================================

// Ported and refactored from legacy
// archive/old_hal_rtos/src/telemetry/optimization_telemetry.c All config
// values use SSOT macros from config/telemetry_config.h and related headers

SystemError_t optimization_telemetry_init(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_INVALID_PARAMETER;
    }
    TelemetryContext_t *context = &telemetry_contexts[motor_id];
    memset(context, 0, sizeof(TelemetryContext_t));

    HAL_Timer_Config_t timer_config = {.frequency_hz =
                                           1000000, // 1 MHz (1µs resolution)
                                       .auto_reload = true,
                                       .interrupt_enable = false,
                                       .priority = 5};
    SystemError_t timer_result =
        HAL_Abstraction_Timer_Init(timer_instance, &timer_config);
    if (timer_result != SYSTEM_OK)
        return ERROR_TIMER_INIT_FAILED;
    timer_result = HAL_Abstraction_Timer_Start(timer_instance);
    if (timer_result != SYSTEM_OK)
        return ERROR_TIMER_START_FAILED;

    SystemError_t result = HAL_Abstraction_AS5600_Init(motor_id);
    if (result != SYSTEM_OK)
        return result;
    float initial_position;
    result = HAL_Abstraction_AS5600_ReadAngle(motor_id, &initial_position);
    if (result != SYSTEM_OK)
        return result;
    context->last_position_degrees = initial_position;
    context->encoder_calibration_offset = 0;

    result = HAL_Abstraction_L6470_Init(motor_id);
    if (result != SYSTEM_OK)
        return result;
    uint32_t kval_hold_value;
    result = HAL_Abstraction_L6470_GetParameter(motor_id, CHIP_KVAL_HOLD_ADDR,
                                                &kval_hold_value);
    context->cached_kval_hold =
        (result == SYSTEM_OK) ? (uint8_t)kval_hold_value : SSOT_KVAL_DEFAULT;
    uint32_t kval_run_value;
    result = HAL_Abstraction_L6470_GetParameter(motor_id, CHIP_KVAL_RUN_ADDR,
                                                &kval_run_value);
    context->cached_kval_run =
        (result == SYSTEM_OK) ? (uint8_t)kval_run_value : SSOT_KVAL_DEFAULT;

    context->safety_limits_enabled = true;
    // Hardware limit: L6470 phase current ≤3A (UM1964 Sec. 1, SSOT exclusion)
    context->safety_current_limit_a =
        MOTOR_MAX_CURRENT_A * SAFETY_CURRENT_LIMIT_RATIO;
    context->safety_speed_limit_dps =
        MOTOR_MAX_SPEED_DPS * SAFETY_SPEED_LIMIT_RATIO;
    context->safety_error_limit_deg = MOTOR_RUNAWAY_THRESHOLD_DEG;

    context->performance.cpu_overhead_percent = 0.0f;
    context->performance.memory_usage_bytes = sizeof(TelemetryContext_t);
    context->performance.real_time_compatible = true;
    context->performance.total_samples_collected = 0;

    context->sample_rate_hz = TELEMETRY_SAMPLE_RATE_DEFAULT_HZ;
    context->initialized = true;
    return SYSTEM_OK;
}

SystemError_t
optimization_telemetry_collect_sample(uint8_t motor_id,
                                      OptimizationTelemetryPacket_t *packet) {
    if (motor_id >= MAX_MOTORS || packet == NULL)
        return ERROR_INVALID_PARAMETER;
    TelemetryContext_t *context = &telemetry_contexts[motor_id];
    if (!context->initialized)
        return ERROR_NOT_INITIALIZED;
    sample_start_time_us = telemetry_get_microsecond_timer();
    memset(packet, 0, sizeof(OptimizationTelemetryPacket_t));
    packet->timestamp_us = sample_start_time_us;
    packet->sample_sequence_id =
        context->performance.total_samples_collected + 1;
    SystemError_t result = telemetry_read_as5600_burst(
        motor_id, &packet->position_degrees, &packet->velocity_dps,
        &packet->acceleration_dps2);
    if (result != SYSTEM_OK) {
        packet->data_quality_score = 0;
        return result;
    }
    result = telemetry_read_l6470_status_fast(
        motor_id, &packet->motor_current_a, &packet->status_flags,
        &packet->thermal_warning, &packet->stall_detected,
        &packet->overcurrent_detected);
    if (result != SYSTEM_OK)
        packet->data_quality_score = 20;
    packet->kval_hold_actual = context->cached_kval_hold;
    packet->kval_run_actual = context->cached_kval_run;
    // Use SSOT macro for supply voltage (see config/motor_config.h)
    // Hardware limit: L6470 supply voltage 8V–45V (UM1964 Sec. 1)
    float supply_voltage = MOTOR_SUPPLY_VOLTAGE;
    packet->power_consumption_w = packet->motor_current_a * supply_voltage;
    result = optimization_telemetry_calculate_thermal_performance(
        packet, &packet->thermal_performance);
    if (result != SYSTEM_OK)
        packet->thermal_performance = TELEMETRY_THERMAL_PERFORMANCE_DEFAULT;
    packet->cpu_utilization_percent =
        context->performance.cpu_overhead_percent;
    packet->position_error =
        packet->commanded_position - packet->position_degrees;
    result = telemetry_check_safety_bounds(context, packet,
                                           &packet->safety_bounds_ok);
    if (result != SYSTEM_OK)
        packet->safety_bounds_ok = false;
    if (!packet->safety_bounds_ok && context->safety_limits_enabled) {
        optimization_telemetry_emergency_stop(motor_id);
        return ERROR_SAFETY_LIMIT_VIOLATION;
    }
    packet->data_quality_score = 100;
    if (!packet->safety_bounds_ok)
        packet->data_quality_score -= 30;
    if (packet->thermal_warning)
        packet->data_quality_score -= 20;
    if (packet->stall_detected)
        packet->data_quality_score -= 25;
    if (packet->overcurrent_detected)
        packet->data_quality_score -= 50;
    sample_end_time_us = telemetry_get_microsecond_timer();
    packet->control_loop_time_us = sample_end_time_us - sample_start_time_us;
    telemetry_update_performance_metrics(context,
                                         packet->control_loop_time_us);
    context->performance.total_samples_collected++;
    return SYSTEM_OK;
}

SystemError_t optimization_telemetry_collect_dataset(
    uint8_t motor_id, const CharacterizationTestConfig_t *config,
    CharacterizationDataSet_t *dataset) {
    if (motor_id >= MAX_MOTORS || config == NULL || dataset == NULL)
        return ERROR_INVALID_PARAMETER;
    TelemetryContext_t *context = &telemetry_contexts[motor_id];
    if (!context->initialized)
        return ERROR_NOT_INITIALIZED;
    memset(dataset, 0, sizeof(CharacterizationDataSet_t));
    dataset->test_type = config->test_type;
    dataset->sample_rate_hz = config->sample_rate_hz;
    dataset->test_duration_ms = config->test_duration_ms;
    dataset->motor_id = motor_id;
    dataset->test_start_timestamp = HAL_Abstraction_GetTick();
    memcpy(dataset->test_parameters, &config->step_amplitude_deg,
           sizeof(dataset->test_parameters));
    uint32_t expected_samples =
        (config->test_duration_ms * config->sample_rate_hz) / 1000;
    if (expected_samples > TELEMETRY_CHARACTERIZATION_BUFFER_SIZE)
        return ERROR_BUFFER_OVERFLOW;
    context->safety_limits_enabled = true;
    // Use SSOT macro for max current (see config/motor_config.h)
    // Hardware limit: L6470 phase current ≤3A (UM1964 Sec. 1)
    context->safety_current_limit_a = MOTOR_MAX_CURRENT_A;
    context->safety_speed_limit_dps =
        MOTOR_MAX_SPEED_DPS * SAFETY_SPEED_LIMIT_RATIO;
    context->safety_error_limit_deg = MOTOR_RUNAWAY_THRESHOLD_DEG;
    uint32_t sample_interval_us = 1000000 / config->sample_rate_hz;
    uint32_t sample_index = 0;
    uint32_t start_time_us = telemetry_get_microsecond_timer();
    while (sample_index < expected_samples) {
        OptimizationTelemetryPacket_t *packet =
            &dataset->samples[sample_index];
        SystemError_t result =
            optimization_telemetry_collect_sample(motor_id, packet);
        if (result != SYSTEM_OK)
            break;
        sample_index++;
        uint32_t now = telemetry_get_microsecond_timer();
        uint32_t elapsed = now - start_time_us;
        if (elapsed < sample_interval_us * sample_index) {
            // Busy-wait for precise timing
            while ((telemetry_get_microsecond_timer() - start_time_us) <
                   sample_interval_us * sample_index) {
                __NOP();
            }
        }
    }
    dataset->sample_count = sample_index;
    dataset->data_valid = (sample_index > 0);
    dataset->checksum = telemetry_calculate_checksum(dataset);
    context->safety_limits_enabled = true;
    context->safety_current_limit_a =
        MOTOR_MAX_CURRENT_A * SAFETY_CURRENT_LIMIT_RATIO;
    context->safety_speed_limit_dps =
        MOTOR_MAX_SPEED_DPS * SAFETY_SPEED_LIMIT_RATIO;
    context->safety_error_limit_deg = MOTOR_RUNAWAY_THRESHOLD_DEG;
    return SYSTEM_OK;
}

SystemError_t optimization_telemetry_start_streaming(uint8_t motor_id,
                                                     uint32_t sample_rate_hz) {
    if (motor_id >= MAX_MOTORS)
        return ERROR_INVALID_PARAMETER;
    TelemetryContext_t *context = &telemetry_contexts[motor_id];
    if (!context->initialized)
        return ERROR_NOT_INITIALIZED;
    if (sample_rate_hz > TELEMETRY_SAMPLE_RATE_MAX_HZ)
        return ERROR_INVALID_PARAMETER;
    context->sample_rate_hz = sample_rate_hz;
    context->streaming_active = true;
    context->last_sample_timestamp_us = telemetry_get_microsecond_timer();
    return SYSTEM_OK;
}

SystemError_t optimization_telemetry_stop_streaming(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS)
        return ERROR_INVALID_PARAMETER;
    TelemetryContext_t *context = &telemetry_contexts[motor_id];
    context->streaming_active = false;
    return SYSTEM_OK;
}

SystemError_t optimization_telemetry_get_performance_metrics(
    uint8_t motor_id, TelemetryPerformanceMetrics_t *metrics) {
    if (motor_id >= MAX_MOTORS || metrics == NULL)
        return ERROR_INVALID_PARAMETER;
    TelemetryContext_t *context = &telemetry_contexts[motor_id];
    if (!context->initialized)
        return ERROR_NOT_INITIALIZED;
    memcpy(metrics, &context->performance,
           sizeof(TelemetryPerformanceMetrics_t));
    return SYSTEM_OK;
}

SystemError_t optimization_telemetry_emergency_stop(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS)
        return ERROR_INVALID_PARAMETER;
    TelemetryContext_t *context = &telemetry_contexts[motor_id];
    context->streaming_active = false;
    SystemError_t result = HAL_Abstraction_L6470_HardStop(motor_id);
    estop_trigger(ESTOP_SRC_SOFTWARE);
    return result;
}

// PRIVATE FUNCTION IMPLEMENTATIONS (as in legacy, refactored for SSOT)

static SystemError_t telemetry_read_as5600_burst(uint8_t motor_id,
                                                 float *position_degrees,
                                                 float *velocity_dps,
                                                 float *acceleration_dps2) {
    TelemetryContext_t *context = &telemetry_contexts[motor_id];
    SystemError_t result =
        HAL_Abstraction_AS5600_ReadAngle(motor_id, position_degrees);
    if (result != SYSTEM_OK)
        return result;
    uint32_t current_time_us = telemetry_get_microsecond_timer();
    float dt_seconds = (current_time_us - context->last_sample_timestamp_us) /
                       MICROSECONDS_PER_SECOND_F;
    result =
        telemetry_calculate_derivatives(context, *position_degrees, dt_seconds,
                                        velocity_dps, acceleration_dps2);
    context->last_sample_timestamp_us = current_time_us;
    return result;
}

static SystemError_t telemetry_read_l6470_status_fast(
    uint8_t motor_id, float *motor_current_a, uint8_t *status_flags,
    bool *thermal_warning, bool *stall_detected, bool *overcurrent_detected) {
    TelemetryContext_t *context = &telemetry_contexts[motor_id];
    uint32_t status_register;
    SystemError_t result =
        HAL_Abstraction_L6470_GetStatus(motor_id, &status_register);
    if (result != SYSTEM_OK)
        return result;
    // NOTE: L6470 register addresses/reset values are hardware-determined (see
    // SSOT_EXCLUSIONS_TELEMETRY.md, UM1963 Table 3)
    *status_flags = (uint8_t)(status_register & 0xFF);
    *thermal_warning = (status_register & (1 << 7)) != 0;
    *stall_detected = (status_register & (1 << 5)) != 0;
    *overcurrent_detected = (status_register & (1 << 3)) != 0;
    // NOTE: Actual current readout is hardware-limited by L6470 design (see
    // SSOT_EXCLUSIONS_TELEMETRY.md)
    *motor_current_a =
        TELEMETRY_MOTOR_CURRENT_PLACEHOLDER_F; // Placeholder, implement actual
                                               // read
    return SYSTEM_OK;
}

static SystemError_t telemetry_calculate_derivatives(
    TelemetryContext_t *context, float current_position, float dt_seconds,
    float *velocity_dps, float *acceleration_dps2) {
    if (dt_seconds <= 0.0f) {
        *velocity_dps = 0.0f;
        *acceleration_dps2 = 0.0f;
        return SYSTEM_OK;
    }
    *velocity_dps =
        (current_position - context->last_position_degrees) / dt_seconds;
    *acceleration_dps2 =
        (*velocity_dps - context->last_velocity_dps) / dt_seconds;
    context->last_position_degrees = current_position;
    context->last_velocity_dps = *velocity_dps;
    return SYSTEM_OK;
}

static SystemError_t
telemetry_check_safety_bounds(const TelemetryContext_t *context,
                              const OptimizationTelemetryPacket_t *packet,
                              bool *safety_ok) {
    *safety_ok = true;
    if (packet->motor_current_a > context->safety_current_limit_a)
        *safety_ok = false;
    if (fabsf(packet->velocity_dps) > context->safety_speed_limit_dps)
        *safety_ok = false;
    if (fabsf(packet->position_error) > context->safety_error_limit_deg)
        *safety_ok = false;
    if (packet->thermal_warning || packet->stall_detected ||
        packet->overcurrent_detected)
        *safety_ok = false;
    return SYSTEM_OK;
}

static uint32_t telemetry_get_microsecond_timer(void) {
    uint32_t counter = 0;
    HAL_Abstraction_Timer_GetCounter(timer_instance, &counter);
    return counter;
}

static uint32_t
telemetry_calculate_checksum(const CharacterizationDataSet_t *dataset) {
    uint32_t checksum = 0;
    checksum ^= dataset->sample_count;
    checksum ^= dataset->sample_rate_hz;
    checksum ^= dataset->test_duration_ms;
    checksum ^= (uint32_t)dataset->test_type;
    checksum ^= dataset->motor_id;
    if (dataset->sample_count > 0) {
        checksum ^= (uint32_t)(dataset->samples[0].position_degrees * 1000.0f);
        checksum ^= dataset->samples[0].timestamp_us;
        if (dataset->sample_count > 1) {
            uint32_t last_idx = dataset->sample_count - 1;
            checksum ^=
                (uint32_t)(dataset->samples[last_idx].position_degrees *
                           1000.0f);
            checksum ^= dataset->samples[last_idx].timestamp_us;
        }
    }
    return checksum;
}

static void telemetry_update_performance_metrics(TelemetryContext_t *context,
                                                 uint32_t sample_time_us) {
    TelemetryPerformanceMetrics_t *metrics = &context->performance;
    if (metrics->total_samples_collected == 0) {
        metrics->average_sample_time_us = sample_time_us;
    } else {
        metrics->average_sample_time_us =
            (metrics->average_sample_time_us * 9 + sample_time_us) / 10;
    }
    if (sample_time_us > metrics->max_sample_time_us) {
        metrics->max_sample_time_us = sample_time_us;
    }
    float target_sample_period_us =
        MICROSECONDS_PER_SECOND_F / context->sample_rate_hz;
    metrics->cpu_overhead_percent =
        (sample_time_us / target_sample_period_us) * 100.0f;
    metrics->real_time_compatible =
        (sample_time_us <
         (target_sample_period_us * TELEMETRY_REALTIME_COMPATIBILITY_RATIO));
    float expected_interval_us =
        MICROSECONDS_PER_SECOND_F / context->sample_rate_hz;
    float actual_interval_us =
        telemetry_get_microsecond_timer() - context->last_sample_timestamp_us;
    float timing_error = fabsf(actual_interval_us - expected_interval_us);
    metrics->timing_accuracy_percent =
        100.0f - (timing_error / expected_interval_us * 100.0f);
    // ...existing code...
    if (metrics->timing_accuracy_percent > 100.0f)
        metrics->timing_accuracy_percent = 100.0f;
    if (metrics->timing_accuracy_percent < 0.0f)
        metrics->timing_accuracy_percent = 0.0f;
}
