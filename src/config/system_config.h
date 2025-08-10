/**
 * @file system_config.h
 * @brief System-wide configuration parameters and feature flags
 * @details This file contains system-level configuration that affects
 *          multiple subsystems and feature compilation flags.
 *
 * @note This is part of the SSOT (Single Source of Truth) configuration
 * system. All system-wide parameters must be defined here.
 *
 * @author STM32H753ZI Stepper Motor Control Project
 * @date 2024
 */

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "build_config.h"
#include <stdbool.h>
#include <stdint.h>

//==============================================================================
// System Clock Configuration
//==============================================================================

/// System clock frequency in Hz (480 MHz for STM32H753ZI)
#define SYSTEM_CLOCK_FREQ_HZ 480000000U

/// AHB clock frequency in Hz (240 MHz)
#define AHB_CLOCK_FREQ_HZ 240000000U

/// APB1 clock frequency in Hz (120 MHz)
#define APB1_CLOCK_FREQ_HZ 120000000U

/// APB2 clock frequency in Hz (120 MHz)
#define APB2_CLOCK_FREQ_HZ 120000000U

/// SysTick timer frequency in Hz (1 kHz for 1ms ticks)
#define SYSTICK_FREQ_HZ 1000U

//==============================================================================
// Feature Flags and Conditional Compilation
//==============================================================================

/// Feature flags for conditional compilation
#define FEATURE_MOTOR_CONTROL (1 << 0)     ///< Motor control algorithms
#define FEATURE_CAN_COMMUNICATION (1 << 1) ///< CAN-FD communication
#define FEATURE_ETHERNET_SUPPORT (1 << 2)  ///< Ethernet and REST API
#define FEATURE_DEBUG_LOGGING (1 << 3)     ///< Debug output and logging
#define FEATURE_SAFETY_MONITORING (1 << 4) ///< Safety and fault monitoring
#define FEATURE_ENCODER_FEEDBACK (1 << 5)  ///< AS5600 encoder support
#define FEATURE_MOTOR_SIMULATION (1 << 6)  ///< Motor simulation for testing

/// Compile-time feature selection
#ifndef ENABLED_FEATURES
#define ENABLED_FEATURES                                                      \
    (FEATURE_MOTOR_CONTROL | FEATURE_SAFETY_MONITORING |                      \
     FEATURE_ENCODER_FEEDBACK | FEATURE_DEBUG_LOGGING)
#endif

/// Check if a feature is enabled
#define IS_FEATURE_ENABLED(feature) ((ENABLED_FEATURES & (feature)) != 0)

//==============================================================================
// Memory Configuration
//==============================================================================

/// Total available RAM in bytes (1 MB for STM32H753ZI)
#define TOTAL_RAM_SIZE_BYTES (1024 * 1024)

/// DTCM RAM size in bytes (128 KB fast access)
#define DTCM_RAM_SIZE_BYTES (128 * 1024)

/// ITCM RAM size in bytes (64 KB fast instruction access)
#define ITCM_RAM_SIZE_BYTES (64 * 1024)

/// Stack size for main task
#define MAIN_TASK_STACK_SIZE 4096U

/// Maximum interrupt nesting level
#define MAX_INTERRUPT_NESTING 8U

//==============================================================================
// System Timing Configuration
//==============================================================================

/// Watchdog timeout in milliseconds
#define SYSTEM_WATCHDOG_TIMEOUT_MS 5000U

/// Maximum system initialization timeout in milliseconds
#define SYSTEM_INIT_TIMEOUT_MS 10000U

//==============================================================================
// Debug and Development Configuration
//==============================================================================

#ifdef DEBUG
/// Enable debug assertions
#define ENABLE_DEBUG_ASSERTIONS 1

/// Enable performance profiling
#define ENABLE_PERFORMANCE_PROFILING 1

/// Enable detailed error logging
#define ENABLE_DETAILED_ERROR_LOGGING 1
#else
#define ENABLE_DEBUG_ASSERTIONS 0
#define ENABLE_PERFORMANCE_PROFILING 0
#define ENABLE_DETAILED_ERROR_LOGGING 0
#endif

/// Maximum debug message length
#define MAX_DEBUG_MESSAGE_LENGTH 256U

//==============================================================================
// Performance and Optimization Settings
//==============================================================================

/// Enable cache usage
#define ENABLE_CACHE_USAGE 1

/// Enable DMA for data transfers
#define ENABLE_DMA_TRANSFERS 1

/// Enable compiler optimizations for critical sections
#define ENABLE_CRITICAL_OPTIMIZATIONS 1

//==============================================================================
// System Validation and Constraints
//==============================================================================

/// Compile-time system validation
_Static_assert(CONTROL_LOOP_FREQ_HZ <= 10000,
               "Control loop frequency too high");
_Static_assert(CONTROL_LOOP_PERIOD_MS >= 1, "Control loop period too short");
_Static_assert(MAIN_TASK_STACK_SIZE >= 2048, "Main task stack too small");
_Static_assert(SYSTEM_WATCHDOG_TIMEOUT_MS >= 1000,
               "Watchdog timeout too short");

//==============================================================================
// Conditional Feature Validation
//==============================================================================

#if IS_FEATURE_ENABLED(FEATURE_MOTOR_CONTROL)
/// Motor control is enabled
#define MOTOR_CONTROL_AVAILABLE 1
#else
#define MOTOR_CONTROL_AVAILABLE 0
#endif

#if IS_FEATURE_ENABLED(FEATURE_MOTOR_SIMULATION)
/// Motor simulation parameters
#define MOTOR_SIM_UPDATE_FREQ_HZ 10000U ///< Simulation update rate
#define MOTOR_SIM_DAMPING_FACTOR 0.95f  ///< Simulation damping
#define MOTOR_SIM_INERTIA_FACTOR 0.1f   ///< Simulation inertia
#endif

//==============================================================================
// System Information Structure
//==============================================================================

/// System information structure
typedef struct {
    uint32_t system_clock_hz;  ///< System clock frequency
    uint32_t control_freq_hz;  ///< Control loop frequency
    uint32_t enabled_features; ///< Enabled feature flags
    uint32_t total_ram_bytes;  ///< Total available RAM
    const char *build_version; ///< Build version string
    const char *build_date;    ///< Build date string
} SystemInfo_t;

/// Get system information
const SystemInfo_t *system_get_info(void);

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_CONFIG_H

/**
 * @example
 * @code
 * // Using feature flags
 * #if IS_FEATURE_ENABLED(FEATURE_MOTOR_CONTROL)
 *     result = motor_controller_init();
 * #endif
 *
 * // Accessing system information
 * const SystemInfo_t* info = system_get_info();
 * printf("System clock: %lu Hz\n", info->system_clock_hz);
 * @endcode
 */
