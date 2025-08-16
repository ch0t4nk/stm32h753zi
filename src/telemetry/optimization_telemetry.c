/**
 * @file optimization_telemetry.c
 * @brief Focused telemetry data acquisition implementation for motor
 * characterization
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
#include "hal_abstraction.h"
#include "rtos/telemetry_dashboard.h"
#include "safety/safety_system.h"

// Use HAL abstraction for timer support (enables unit testing)
// STM32 HAL includes moved to HAL abstraction implementation

// L6470 register constants for configuration (abstracted from drivers)
#include "config/l6470_registers_generated.h"
#include "config/motor_config.h"

// NOTE: Driver functions accessed through HAL abstraction layer (FTR-013
// complete)#include <math.h>
#include <stdio.h>
#include <string.h>

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

// ... (full implementation as in archive) ...
