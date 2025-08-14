/**
 * @file workflow_config.h
 * @brief Development Workflow Configuration (SSOT)
 *
 * This file provides centralized configuration for development workflow
 * complexity levels, allowing selective enabling/disabling of validation,
 * initialization, and convenience features.
 *
 * @note This is part of the SSOT (Single Source of Truth) system
 * @see .github/instructions/ssot-config.instructions.md
 * @see .github/instructions/development-workflow.instructions.md
 */

#ifndef WORKFLOW_CONFIG_H
#define WORKFLOW_CONFIG_H

// =============================================================================
// WORKFLOW MODES (Quick Presets)
// =============================================================================

/**
 * @brief Workflow complexity modes
 *
 * These provide quick presets for different use cases:
 * - MINIMAL: Fastest startup, minimal checks (experienced users)
 * - STANDARD: Balanced validation and performance (default)
 * - THOROUGH: Full validation and convenience features (new users)
 * - DEBUG: Maximum verbosity and validation (troubleshooting)
 * - CUSTOM: Use individual flags below
 */
typedef enum {
    WORKFLOW_MODE_MINIMAL = 0,  // Fast, minimal validation
    WORKFLOW_MODE_STANDARD = 1, // Balanced (default)
    WORKFLOW_MODE_THOROUGH = 2, // Full validation
    WORKFLOW_MODE_DEBUG = 3,    // Maximum verbosity
    WORKFLOW_MODE_CUSTOM = 4    // Use individual flags
} WorkflowMode_t;

// Current active mode (can be changed via environment variable or script)
#ifndef WORKFLOW_MODE
#define WORKFLOW_MODE WORKFLOW_MODE_STANDARD
#endif

// =============================================================================
// INDIVIDUAL FEATURE FLAGS
// =============================================================================

/**
 * @defgroup EnvironmentValidation Environment Validation Flags
 * @brief Control depth of environment checking and validation
 * @{
 */

/** Enable virtual environment validation (check .venv exists and is valid) */
#ifndef WORKFLOW_VALIDATE_VENV
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_VALIDATE_VENV 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_VALIDATE_VENV 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_VALIDATE_VENV 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_VALIDATE_VENV 1
#else
#define WORKFLOW_VALIDATE_VENV 1 // Default for custom mode
#endif
#endif

/** Enable Python path detection and validation */
#ifndef WORKFLOW_VALIDATE_PYTHON
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_VALIDATE_PYTHON 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_VALIDATE_PYTHON 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_VALIDATE_PYTHON 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_VALIDATE_PYTHON 1
#else
#define WORKFLOW_VALIDATE_PYTHON 1
#endif
#endif

/** Enable cross-platform compatibility detection */
#ifndef WORKFLOW_DETECT_PLATFORM
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_DETECT_PLATFORM 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_DETECT_PLATFORM 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_DETECT_PLATFORM 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_DETECT_PLATFORM 1
#else
#define WORKFLOW_DETECT_PLATFORM 1
#endif
#endif

/** Enable build environment validation (ARM GCC, CMake, etc.) */
#ifndef WORKFLOW_VALIDATE_BUILD_ENV
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_VALIDATE_BUILD_ENV 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_VALIDATE_BUILD_ENV 0 // Skip in standard for speed
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_VALIDATE_BUILD_ENV 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_VALIDATE_BUILD_ENV 1
#else
#define WORKFLOW_VALIDATE_BUILD_ENV 0
#endif
#endif

/** @} */

/**
 * @defgroup ConvenienceFeatures Convenience Features
 * @brief Control loading of convenience functions and aliases
 * @{
 */

/** Auto-load workflow functions (workflow-before, workflow-during, etc.) */
#ifndef WORKFLOW_LOAD_FUNCTIONS
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_LOAD_FUNCTIONS 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_LOAD_FUNCTIONS 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_LOAD_FUNCTIONS 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_LOAD_FUNCTIONS 1
#else
#define WORKFLOW_LOAD_FUNCTIONS 1
#endif
#endif

/** Create convenience aliases (wf-before, wf-during, etc.) */
#ifndef WORKFLOW_CREATE_ALIASES
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_CREATE_ALIASES 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_CREATE_ALIASES 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_CREATE_ALIASES 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_CREATE_ALIASES 1
#else
#define WORKFLOW_CREATE_ALIASES 1
#endif
#endif

/** Auto-activate virtual environment in new terminals */
#ifndef WORKFLOW_AUTO_ACTIVATE_VENV
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_AUTO_ACTIVATE_VENV 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_AUTO_ACTIVATE_VENV 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_AUTO_ACTIVATE_VENV 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_AUTO_ACTIVATE_VENV 1
#else
#define WORKFLOW_AUTO_ACTIVATE_VENV 1
#endif
#endif

/** @} */

/**
 * @defgroup ValidationDepth Validation Depth Control
 * @brief Control how thoroughly validation is performed
 * @{
 */

/** Enable dependency checking (packages, tools, versions) */
#ifndef WORKFLOW_CHECK_DEPENDENCIES
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_CHECK_DEPENDENCIES 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_CHECK_DEPENDENCIES 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_CHECK_DEPENDENCIES 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_CHECK_DEPENDENCIES 1
#else
#define WORKFLOW_CHECK_DEPENDENCIES 0
#endif
#endif

/** Enable git hooks validation */
#ifndef WORKFLOW_VALIDATE_GIT_HOOKS
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_VALIDATE_GIT_HOOKS 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_VALIDATE_GIT_HOOKS 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_VALIDATE_GIT_HOOKS 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_VALIDATE_GIT_HOOKS 1
#else
#define WORKFLOW_VALIDATE_GIT_HOOKS 0
#endif
#endif

/** Enable SSOT configuration validation */
#ifndef WORKFLOW_VALIDATE_SSOT
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_VALIDATE_SSOT 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_VALIDATE_SSOT 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_VALIDATE_SSOT 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_VALIDATE_SSOT 1
#else
#define WORKFLOW_VALIDATE_SSOT 0
#endif
#endif

/** @} */

/**
 * @defgroup ErrorHandling Error Handling and Logging
 * @brief Control verbosity and error handling behavior
 * @{
 */

/** Enable verbose logging during initialization */
#ifndef WORKFLOW_VERBOSE_LOGGING
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_VERBOSE_LOGGING 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_VERBOSE_LOGGING 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_VERBOSE_LOGGING 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_VERBOSE_LOGGING 1
#else
#define WORKFLOW_VERBOSE_LOGGING 0
#endif
#endif

/** Continue on validation errors vs fail fast */
#ifndef WORKFLOW_CONTINUE_ON_ERROR
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_CONTINUE_ON_ERROR 1 // Keep going for speed
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_CONTINUE_ON_ERROR 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_CONTINUE_ON_ERROR 0 // Fail fast to catch issues
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_CONTINUE_ON_ERROR 0
#else
#define WORKFLOW_CONTINUE_ON_ERROR 1
#endif
#endif

/** Show performance timing information */
#ifndef WORKFLOW_SHOW_TIMING
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_SHOW_TIMING 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_SHOW_TIMING 0
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_SHOW_TIMING 1
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_SHOW_TIMING 1
#else
#define WORKFLOW_SHOW_TIMING 0
#endif
#endif

/** @} */

// =============================================================================
// CONFIGURATION PATHS AND CONSTANTS
// =============================================================================

/** Environment variable name to override workflow mode */
#define WORKFLOW_MODE_ENV_VAR "STM32_WORKFLOW_MODE"

/** Configuration file for custom settings */
#define WORKFLOW_CONFIG_FILE ".workflow_config"

/** Maximum time allowed for environment validation (milliseconds) */
#ifndef WORKFLOW_VALIDATION_TIMEOUT_MS
#if WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL
#define WORKFLOW_VALIDATION_TIMEOUT_MS 100
#elif WORKFLOW_MODE == WORKFLOW_MODE_STANDARD
#define WORKFLOW_VALIDATION_TIMEOUT_MS 500
#elif WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH
#define WORKFLOW_VALIDATION_TIMEOUT_MS 2000
#elif WORKFLOW_MODE == WORKFLOW_MODE_DEBUG
#define WORKFLOW_VALIDATION_TIMEOUT_MS 5000
#else
#define WORKFLOW_VALIDATION_TIMEOUT_MS 500
#endif
#endif

// =============================================================================
// CONVENIENCE MACROS
// =============================================================================

/** Check if any validation is enabled */
#define WORKFLOW_ANY_VALIDATION_ENABLED                                       \
    (WORKFLOW_VALIDATE_VENV || WORKFLOW_VALIDATE_PYTHON ||                    \
     WORKFLOW_VALIDATE_BUILD_ENV || WORKFLOW_CHECK_DEPENDENCIES)

/** Check if full validation mode is active */
#define WORKFLOW_FULL_VALIDATION_MODE                                         \
    (WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH ||                               \
     WORKFLOW_MODE == WORKFLOW_MODE_DEBUG)

/** Check if minimal mode is active */
#define WORKFLOW_MINIMAL_MODE (WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL)

/** Quick preset check macros */
#define WORKFLOW_IS_MINIMAL() (WORKFLOW_MODE == WORKFLOW_MODE_MINIMAL)
#define WORKFLOW_IS_STANDARD() (WORKFLOW_MODE == WORKFLOW_MODE_STANDARD)
#define WORKFLOW_IS_THOROUGH() (WORKFLOW_MODE == WORKFLOW_MODE_THOROUGH)
#define WORKFLOW_IS_DEBUG() (WORKFLOW_MODE == WORKFLOW_MODE_DEBUG)
#define WORKFLOW_IS_CUSTOM() (WORKFLOW_MODE == WORKFLOW_MODE_CUSTOM)

// =============================================================================
// RUNTIME CONFIGURATION SUPPORT
// =============================================================================

/**
 * @brief Configuration override structure for runtime changes
 *
 * This allows scripts to override SSOT settings at runtime without
 * recompiling or changing header files.
 */
typedef struct {
    int validate_venv;
    int validate_python;
    int detect_platform;
    int validate_build_env;
    int load_functions;
    int create_aliases;
    int auto_activate_venv;
    int check_dependencies;
    int validate_git_hooks;
    int validate_ssot;
    int verbose_logging;
    int continue_on_error;
    int show_timing;
} WorkflowRuntimeConfig_t;

// =============================================================================
// DOCUMENTATION AND USAGE
// =============================================================================

/**
 * @brief Usage Examples
 *
 * Environment Variable Override:
 *   export STM32_WORKFLOW_MODE=minimal    # Fast mode
 *   export STM32_WORKFLOW_MODE=thorough   # Full validation
 *   export STM32_WORKFLOW_MODE=debug      # Maximum verbosity
 *
 * PowerShell Examples:
 *   $env:STM32_WORKFLOW_MODE = "minimal"
 *   .\scripts\run_python.ps1 scripts\auto_update_status.py
 *
 * Custom Configuration File (.workflow_config):
 *   mode=custom
 *   validate_venv=1
 *   validate_python=0
 *   verbose_logging=1
 *
 * Script Integration:
 *   if [[ "$WORKFLOW_VALIDATE_VENV" == "1" ]]; then
 *       validate_virtual_environment
 *   fi
 */

#endif // WORKFLOW_CONFIG_H
