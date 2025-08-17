#ifndef TELEMETRY_CONFIG_H
#define TELEMETRY_CONFIG_H

/**
 * @file telemetry_config.h
 * @brief SSOT: Telemetry configuration for STM32H753ZI stepper motor project
 *
 * All telemetry-related configuration parameters for optimization, reporting,
 * and safety monitoring. Do not hardcode values in implementation files—always
 * reference these macros. See .github/instructions/ssot-config.instructions.md
 * for SSOT policy and update requirements.
 */

// Telemetry buffer sizes (optimized for STM32H7 memory)
#define TELEMETRY_CHARACTERIZATION_BUFFER_SIZE (2000) // 2s @ 1kHz
#define TELEMETRY_SAMPLE_RATE_MAX_HZ (1000)           // 1kHz max
#define TELEMETRY_SAMPLE_RATE_DEFAULT_HZ (500)        // 500Hz default
#define TELEMETRY_MEMORY_POOL_SIZE_KB (8)             // 8KB pool

// Performance monitoring constants
#define TELEMETRY_CPU_OVERHEAD_TARGET_PCT (2.0f) // <2% CPU
#define TELEMETRY_TIMING_TOLERANCE_US (100)      // ±100µs
#define TELEMETRY_SAFETY_CHECK_INTERVAL_MS (10)  // 10ms

// Validation and calibration constants
#define TELEMETRY_CALIBRATION_SAMPLES (100)
#define TELEMETRY_VALIDATION_DURATION_MS (5000) // 5s
#define TELEMETRY_ACCURACY_TARGET_PCT (95.0f)

#endif // TELEMETRY_CONFIG_H
