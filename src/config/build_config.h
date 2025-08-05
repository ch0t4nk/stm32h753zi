/**
 * @file build_config.h
 * @brief Build Configuration and Version Info - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-01-29
 *
 * @note This file contains build-time configuration, version information, and
 * feature flags. Values are automatically generated during build process where
 * possible.
 *
 * TODO: See .github/instructions/build-config.instructions.md for automated
 * version generation
 * TODO: See .instructions/feature-flags.md for conditional compilation setup
 */

#ifndef BUILD_CONFIG_H
#define BUILD_CONFIG_H

#include <stdint.h>

/* ========================================================================== */
/* Version Information (SSOT)                                                */
/* ========================================================================== */
// TODO: See .instructions/build-version.md for automatic version generation
// from git

#define VERSION_MAJOR 1 // Major version number
#define VERSION_MINOR 0 // Minor version number
#define VERSION_PATCH 0 // Patch version number
#define VERSION_BUILD 1 // Build number (auto-incremented)

// Version string format: "v1.0.0-build1"
#define VERSION_STRING                                                         \
  "v" STRINGIFY(VERSION_MAJOR) "." STRINGIFY(VERSION_MINOR) "." STRINGIFY(     \
      VERSION_PATCH) "-build" STRINGIFY(VERSION_BUILD)

// Build information (to be populated by build system)
#ifndef BUILD_DATE
#define BUILD_DATE __DATE__ // Build date string
#endif

#ifndef BUILD_TIME
#define BUILD_TIME __TIME__ // Build time string
#endif

#ifndef GIT_HASH
#define GIT_HASH "unknown" // Git commit hash (8 chars)
#endif

#ifndef GIT_BRANCH
#define GIT_BRANCH "unknown" // Git branch name
#endif

// Build configuration identification
#define BUILD_CONFIG_DEBUG 0   // Debug build flag
#define BUILD_CONFIG_RELEASE 1 // Release build flag
#define BUILD_CONFIG_TEST 0    // Test build flag

/* ========================================================================== */
/* Feature Enable/Disable Flags (SSOT)                                       */
/* ========================================================================== */
// TODO: See .instructions/feature-flags.md for conditional compilation strategy

// Motor Control Features
#define FEATURE_DUAL_MOTOR 1     // Enable dual motor control
#define FEATURE_CLOSED_LOOP 1    // Enable closed-loop control with encoders
#define FEATURE_AUTO_HOMING 1    // Enable automatic homing on startup
#define FEATURE_MOTION_PROFILE 1 // Enable advanced motion profiling

// Communication Features
#define FEATURE_UART_INTERFACE 1 // Enable UART command interface
#define FEATURE_CAN_INTERFACE 1  // Enable CAN bus communication
#define FEATURE_ETHERNET 1       // Enable Ethernet/TCP interface
#define FEATURE_HTTP_SERVER 0    // Enable HTTP server (disabled for now)

// Safety Features
#define FEATURE_EMERGENCY_STOP 1 // Enable emergency stop functionality
#define FEATURE_WATCHDOG 1       // Enable watchdog monitoring
#define FEATURE_FAULT_LOGGING 1  // Enable fault event logging
#define FEATURE_SAFE_RECOVERY 1  // Enable automatic fault recovery

// Diagnostic Features
#define FEATURE_PERFORMANCE_MON 1 // Enable performance monitoring
#define FEATURE_DEBUG_OUTPUT 1    // Enable debug output via UART
#define FEATURE_STATISTICS 1      // Enable runtime statistics collection
#define FEATURE_CALIBRATION 1     // Enable calibration routines

// Development/Test Features (disabled in release builds)
#if BUILD_CONFIG_DEBUG
#define FEATURE_SIMULATOR_MODE 1 // Enable simulator mode (no hardware)
#define FEATURE_UNIT_TESTS 1     // Enable unit test framework
#define FEATURE_MEMORY_DEBUG 1   // Enable memory usage debugging
#define FEATURE_TIMING_DEBUG 1   // Enable timing analysis
#else
#define FEATURE_SIMULATOR_MODE 0 // Disable in release
#define FEATURE_UNIT_TESTS 0     // Disable in release
#define FEATURE_MEMORY_DEBUG 0   // Disable in release
#define FEATURE_TIMING_DEBUG 0   // Disable in release
#endif

/* ========================================================================== */
/* Hardware Configuration Selection (SSOT)                                   */
/* ========================================================================== */
// TODO: See .instructions/hardware-variants.md for multi-board support

// Target Hardware Platform
#define HW_PLATFORM_STM32H753ZI 1 // STM32H753ZI Nucleo-144 board
#define HW_PLATFORM_CUSTOM 0      // Custom hardware variant

// Shield/Expansion Board
#define HW_SHIELD_IHM02A1 1 // X-NUCLEO-IHM02A1 stepper driver shield
#define HW_SHIELD_CUSTOM 0  // Custom shield configuration

// Motor Configuration
#define HW_MOTOR_NEMA17 1 // NEMA 17 stepper motors
#define HW_MOTOR_NEMA23 0 // NEMA 23 stepper motors (higher power)

// Encoder Configuration
#define HW_ENCODER_AS5600 1      // AS5600 magnetic encoders
#define HW_ENCODER_INCREMENTAL 0 // Incremental optical encoders

/* ========================================================================== */
/* Memory and Performance Configuration (SSOT)                               */
/* ========================================================================== */
// TODO: See .instructions/memory-config.md for memory optimization settings

// Memory Allocation Limits
#define MAX_HEAP_SIZE_BYTES (32 * 1024) // Maximum heap allocation
#define MAX_STACK_SIZE_BYTES (8 * 1024) // Maximum stack size per task
#define DMA_BUFFER_COUNT 8              // Number of DMA buffers
#define DMA_BUFFER_SIZE_BYTES 512       // Size of each DMA buffer

// Performance Settings
#define MAX_INTERRUPT_LATENCY_US 50    // Maximum interrupt latency
#define CONTROL_LOOP_JITTER_US 10      // Maximum control loop jitter
#define COMMUNICATION_BUFFER_SIZE 2048 // Communication buffer size

// Cache Configuration (STM32H7 specific)
#define ENABLE_INSTRUCTION_CACHE 1 // Enable I-Cache
#define ENABLE_DATA_CACHE 1        // Enable D-Cache
#define CACHE_COHERENCY_CHECK 1    // Enable cache coherency checks

/* ========================================================================== */
/* Debugging and Logging Configuration (SSOT)                                */
/* ========================================================================== */
// TODO: See .instructions/debug-config.md for debug output configuration

// Debug Output Levels
#define DEBUG_LEVEL_NONE 0
#define DEBUG_LEVEL_ERROR 1
#define DEBUG_LEVEL_WARNING 2
#define DEBUG_LEVEL_INFO 3
#define DEBUG_LEVEL_DEBUG 4
#define DEBUG_LEVEL_VERBOSE 5

// Current debug level (set based on build type)
#if BUILD_CONFIG_DEBUG
#define DEBUG_LEVEL DEBUG_LEVEL_DEBUG
#elif BUILD_CONFIG_TEST
#define DEBUG_LEVEL DEBUG_LEVEL_INFO
#else
#define DEBUG_LEVEL DEBUG_LEVEL_WARNING
#endif

// Debug Output Destinations
#define DEBUG_OUTPUT_UART 1 // Send debug output to UART
#define DEBUG_OUTPUT_ITM 0  // Send debug output to ITM (SWO)
#define DEBUG_OUTPUT_FILE 0 // Send debug output to file (test builds)

// Logging Configuration
#define LOG_BUFFER_SIZE 1024    // Size of log message buffer
#define LOG_MAX_MESSAGE_LEN 128 // Maximum length of single log message
#define LOG_TIMESTAMP_ENABLE 1  // Include timestamps in log messages

/* ========================================================================== */
/* Build Optimization Settings (SSOT)                                        */
/* ========================================================================== */
// TODO: See .instructions/optimization.md for compiler optimization settings

// Compiler Optimization Preferences
#if BUILD_CONFIG_DEBUG
#define OPTIMIZATION_LEVEL "-O0" // No optimization for debugging
#define INLINE_FUNCTIONS 0       // Disable function inlining
#define DEAD_CODE_ELIMINATION 0  // Disable dead code elimination
#else
#define OPTIMIZATION_LEVEL "-O2" // Optimize for speed and size
#define INLINE_FUNCTIONS 1       // Enable function inlining
#define DEAD_CODE_ELIMINATION 1  // Enable dead code elimination
#endif

// Link Time Optimization
#define ENABLE_LTO 1           // Enable Link Time Optimization
#define ENABLE_STRIP_SYMBOLS 1 // Strip debug symbols in release

/* ========================================================================== */
/* Testing and Validation Configuration (SSOT)                               */
/* ========================================================================== */
// TODO: See .instructions/testing-config.md for test framework configuration

#if FEATURE_UNIT_TESTS
// Unit Test Framework
#define UNIT_TEST_FRAMEWORK "Unity" // Unity testing framework
#define MAX_TEST_CASES 100          // Maximum number of test cases
#define TEST_TIMEOUT_MS 5000        // Test case timeout
#define TEST_BUFFER_SIZE 512        // Test output buffer size

// Test Coverage
#define ENABLE_CODE_COVERAGE 1 // Enable code coverage analysis
#define TARGET_COVERAGE_PCT 80 // Target code coverage percentage
#endif

/* ========================================================================== */
/* Build Validation and Checks (SSOT)                                        */
/* ========================================================================== */
// TODO: See .instructions/build-validation.md for compile-time checks

// Compile-time configuration validation
#if !FEATURE_DUAL_MOTOR && FEATURE_CLOSED_LOOP
#warning "Closed-loop control requires dual motor feature"
#endif

#if FEATURE_HTTP_SERVER && !FEATURE_ETHERNET
#error "HTTP server requires Ethernet feature to be enabled"
#endif

#if FEATURE_WATCHDOG && BUILD_CONFIG_TEST
#warning "Watchdog enabled in test build - may interfere with testing"
#endif

// Resource requirement checks
#if MAX_HEAP_SIZE_BYTES > (64 * 1024)
#error "Heap size exceeds available SRAM"
#endif

#if (UART_TX_BUFFER_SIZE + UART_RX_BUFFER_SIZE) > (8 * 1024)
#error "UART buffers too large"
#endif

/* ========================================================================== */
/* Build Utility Macros (SSOT)                                               */
/* ========================================================================== */

// String conversion macros
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Compiler attribute macros
#define PACKED __attribute__((packed))
#define ALIGNED(n) __attribute__((aligned(n)))
#define WEAK __attribute__((weak))
#define INLINE __attribute__((always_inline)) inline

// Memory section placement macros
#define PLACE_IN_DTCM __attribute__((section(".dtcm")))
#define PLACE_IN_ITCM __attribute__((section(".itcm")))
#define PLACE_IN_SRAM1 __attribute__((section(".sram1")))
#define PLACE_IN_SRAM2 __attribute__((section(".sram2")))

// Build information structure
typedef struct {
  uint32_t version_major;
  uint32_t version_minor;
  uint32_t version_patch;
  uint32_t version_build;
  const char *version_string;
  const char *build_date;
  const char *build_time;
  const char *git_hash;
  const char *git_branch;
  uint32_t feature_flags;
} BuildInfo_t;

// Global build information instance (defined in main.c)
extern const BuildInfo_t g_build_info;

#endif /* BUILD_CONFIG_H */

/**
 * @note Build Configuration SSOT Rules:
 * 1. All build-time settings MUST be defined here
 * 2. Feature flags control conditional compilation
 * 3. Version information centralized and auto-generated
 * 4. Build validation checks prevent invalid configurations
 * 5. Memory and performance settings in one location
 */
