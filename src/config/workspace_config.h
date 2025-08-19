/**
 * @file workspace_config.h
 * @brief Workspace and Toolchain Configuration - Single Source of Truth (SSOT)
 * @author STM32H753ZI Project Team
 * @date 2025-08-18
 *
 * @note This file contains workspace setup, toolchain paths, build environment
 * configuration, and development workflow settings. All build scripts, CMake
 * configuration, and VS Code settings should reference this SSOT.
 *
 * TODO: See .github/instructions/cmake-build-system.instructions.md for usage
 * TODO: See .github/instructions/development-workflow.instructions.md for
 * workflow
 */

#ifndef WORKSPACE_CONFIG_H
#define WORKSPACE_CONFIG_H

#include <stdint.h>

// If present, include generated workspace overrides (created by
// scripts/generate_workspace_config.py)
#ifdef __has_include
#if __has_include("workspace_config.generated.h")
#include "workspace_config.generated.h"
#endif
#endif

/* ==========================================================================
 */
/* Toolchain Configuration (SSOT)                                            */
/* ==========================================================================
 */

/**
 * @brief STM32CubeCLT Version and Paths
 */
#define STM32CUBECLT_VERSION "1.19.0"
#define STM32CUBECLT_BASE_PATH "C:/ST/STM32CubeCLT_1.19.0"

// ARM GCC Toolchain Paths
#define ARM_TOOLCHAIN_BASE_PATH                                               \
    STM32CUBECLT_BASE_PATH "/GNU-tools-for-STM32/bin"
#define ARM_GCC_EXECUTABLE ARM_TOOLCHAIN_BASE_PATH "/arm-none-eabi-gcc.exe"
#define ARM_GPP_EXECUTABLE ARM_TOOLCHAIN_BASE_PATH "/arm-none-eabi-g++.exe"
#define ARM_AS_EXECUTABLE ARM_TOOLCHAIN_BASE_PATH "/arm-none-eabi-gcc.exe"
#define ARM_AR_EXECUTABLE ARM_TOOLCHAIN_BASE_PATH "/arm-none-eabi-ar.exe"
#define ARM_OBJCOPY_EXECUTABLE                                                \
    ARM_TOOLCHAIN_BASE_PATH "/arm-none-eabi-objcopy.exe"
#define ARM_SIZE_EXECUTABLE ARM_TOOLCHAIN_BASE_PATH "/arm-none-eabi-size.exe"
#define ARM_GDB_EXECUTABLE ARM_TOOLCHAIN_BASE_PATH "/arm-none-eabi-gdb.exe"

// STM32 Programmer Paths
#define STM32PROG_CLI_PATH                                                    \
    STM32CUBECLT_BASE_PATH "/STM32CubeProgrammer/bin/"                        \
                           "STM32_Programmer_CLI.exe"

// Target MCU Configuration
#define TARGET_MCU "STM32H753xx"
#define TARGET_CPU "cortex-m7"
#define TARGET_FPU "fpv5-d16"
#define TARGET_FLOAT_ABI "hard"

// Compiler Flags (from SSOT build_config.h)
#include "build_config.h"

#define TARGET_CFLAGS                                                         \
    "-mcpu=" TARGET_CPU " -mfpu=" TARGET_FPU " -mfloat-abi=" TARGET_FLOAT_ABI
#define TARGET_LDFLAGS TARGET_CFLAGS " --specs=nano.specs --specs=nosys.specs"

/* ==========================================================================
 */
/* CMake Configuration (SSOT)                                                */
/* ==========================================================================
 */

/**
 * @brief CMake Generator and Build System
 */
#define CMAKE_GENERATOR_PREFERRED "Ninja"         // Primary generator
#define CMAKE_GENERATOR_FALLBACK "Unix Makefiles" // Fallback generator
#define CMAKE_BUILD_TYPE_DEFAULT "Debug"          // Default build type
#define CMAKE_TOOLCHAIN_FILE "cmake/gcc-arm-none-eabi.cmake"

// Build Directories
#define BUILD_DIR_BASE "build"
#define BUILD_DIR_HOST_TESTS "build_host_tests"
#define BUILD_DIR_COVERAGE "build_coverage"

// CMake Presets
#define CMAKE_PRESET_DEBUG "Debug"
#define CMAKE_PRESET_RELEASE "Release"
#define CMAKE_PRESET_DEBUG_MAKE "Debug-Make"
#define CMAKE_PRESET_RELEASE_MAKE "Release-Make"

/* ==========================================================================
 */
/* Workspace Environment (SSOT)                                              */
/* ==========================================================================
 */

/**
 * @brief Development Environment Paths
 */
#define WORKSPACE_ROOT_DEFAULT "C:/repos/Nucleo-H753ZI Project/code"
#define PYTHON_VENV_PATH ".venv"
#define PYTHON_VENV_SCRIPTS_PATH PYTHON_VENV_PATH "/Scripts"
#define PYTHON_EXECUTABLE PYTHON_VENV_SCRIPTS_PATH "/python.exe"

// Documentation and References
#define DOCS_DIR "docs"
#define REFERENCE_DIR "00_reference"
#define SEMANTIC_DB_DIR DOCS_DIR "/semantic_vector_db"
#define INDEXES_DIR DOCS_DIR "/indexes"

// Scripts and Tools
#define SCRIPTS_DIR "scripts"
#define CMAKE_DIR "cmake"
#define ARCHIVE_DIR "archive"

/* ==========================================================================
 */
/* VS Code Configuration (SSOT)                                              */
/* ==========================================================================
 */

/**
 * @brief VS Code Settings and Tasks
 */
#define VSCODE_CONFIG_DIR ".vscode"
#define VSCODE_TASKS_FILE VSCODE_CONFIG_DIR "/tasks.json"
#define VSCODE_LAUNCH_FILE VSCODE_CONFIG_DIR "/launch.json"
#define VSCODE_SETTINGS_FILE VSCODE_CONFIG_DIR "/settings.json"
#define VSCODE_CMAKE_KITS_FILE VSCODE_CONFIG_DIR "/cmake-kits.json"

// Terminal Configuration
#define DEFAULT_SHELL "pwsh.exe"
#define TERMINAL_VENV_ACTIVATION "source " PYTHON_VENV_PATH "/bin/activate"

/* ==========================================================================
 */
/* Development Workflow (SSOT)                                               */
/* ==========================================================================
 */

/**
 * @brief Workflow Mode Settings
 */
typedef enum {
    WORKFLOW_MODE_MINIMAL = 0,  // Fast mode for CI/CD
    WORKFLOW_MODE_STANDARD = 1, // Default development
    WORKFLOW_MODE_THOROUGH = 2, // Full guidance for complex work
    WORKFLOW_MODE_DEBUG = 3     // Maximum verbosity for troubleshooting
} WorkflowMode_t;

#define WORKFLOW_MODE_DEFAULT WORKFLOW_MODE_STANDARD

// Feature Flags for Workflow
#define WORKFLOW_VALIDATE_VENV 1      // Validate virtual environment
#define WORKFLOW_VALIDATE_BUILD_ENV 1 // Validate build environment
#define WORKFLOW_VERBOSE_LOGGING 1    // Enable verbose logging
#define WORKFLOW_AUTO_STATUS_UPDATE 1 // Automatic STATUS.md updates
#define WORKFLOW_GIT_HOOKS_ENABLED 1  // Enable git hooks
#define WORKFLOW_SEMANTIC_SEARCH 1    // Enable semantic search tools

/* ==========================================================================
 */
/* Build Environment Validation (SSOT)                                       */
/* ==========================================================================
 */

/**
 * @brief Required Tools and Versions
 */
#define REQUIRED_CMAKE_VERSION "3.20.0"
#define REQUIRED_NINJA_VERSION "1.8.0"
#define REQUIRED_PYTHON_VERSION "3.8.0"
#define REQUIRED_GCC_ARM_VERSION "10.3.0"

// Validation Timeouts
#define VALIDATION_TIMEOUT_SECONDS 30
#define BUILD_TIMEOUT_MINUTES 10
#define TEST_TIMEOUT_MINUTES 5

/* ==========================================================================
 */
/* Cross-Platform Compatibility (SSOT)                                       */
/* ==========================================================================
 */

/**
 * @brief Platform Detection and Paths
 */
#ifdef _WIN32
#define PLATFORM_WINDOWS 1
#define PLATFORM_LINUX 0
#define PLATFORM_MACOS 0
#define PATH_SEPARATOR "\\"
#define EXECUTABLE_EXTENSION ".exe"
#define SCRIPT_EXTENSION ".ps1"
#elif defined(__linux__)
#define PLATFORM_WINDOWS 0
#define PLATFORM_LINUX 1
#define PLATFORM_MACOS 0
#define PATH_SEPARATOR "/"
#define EXECUTABLE_EXTENSION ""
#define SCRIPT_EXTENSION ".sh"
#elif defined(__APPLE__)
#define PLATFORM_WINDOWS 0
#define PLATFORM_LINUX 0
#define PLATFORM_MACOS 1
#define PATH_SEPARATOR "/"
#define EXECUTABLE_EXTENSION ""
#define SCRIPT_EXTENSION ".sh"
#endif

// Platform-specific ARM GCC paths
#if PLATFORM_WINDOWS
#define ARM_GCC_PATH                                                          \
    STM32CUBECLT_BASE_PATH "/GNU-tools-for-STM32/bin/arm-none-eabi-gcc.exe"
#else
#define ARM_GCC_PATH "arm-none-eabi-gcc" // Assume in PATH on Linux/macOS
#endif

/* ==========================================================================
 */
/* Status and Documentation (SSOT)                                           */
/* ==========================================================================
 */

/**
 * @brief Status Tracking Configuration
 */
#define STATUS_FILE "STATUS.md"
#define STATUS_UPDATE_SCRIPT SCRIPTS_DIR "/auto_update_status.py"
#define STATUS_TEMPLATE_PATH DOCS_DIR "/templates/STATUS_template.md"

// Feature Tracking
#define FEATURE_REGISTRY_FILE "features/feature_registry.json"
#define FEATURE_TRACKER_SCRIPT SCRIPTS_DIR "/feature_tracker.py"

// Documentation Generation
#define DOXYGEN_CONFIG_FILE DOCS_DIR "/Doxyfile"
#define DOXYGEN_OUTPUT_DIR DOCS_DIR "/generated"

/* ==========================================================================
 */
/* Build Artifacts and Output (SSOT)                                         */
/* ==========================================================================
 */

/**
 * @brief Build Output Configuration
 */
#define BUILD_OUTPUT_ELF "stm32h753_ihm02a1.elf"
#define BUILD_OUTPUT_HEX "stm32h753_ihm02a1.hex"
#define BUILD_OUTPUT_BIN "stm32h753_ihm02a1.bin"
#define BUILD_OUTPUT_MAP "stm32h753_ihm02a1.map"

// Linker Script
#define LINKER_SCRIPT "STM32H753XX_FLASH.ld"

// Memory Layout
#define FLASH_SIZE_BYTES (2 * 1024 * 1024) // 2MB Flash
#define SRAM_SIZE_BYTES (1024 * 1024)      // 1MB SRAM
#define DTCM_SIZE_BYTES (128 * 1024)       // 128KB DTCM
#define ITCM_SIZE_BYTES (64 * 1024)        // 64KB ITCM

/* ==========================================================================
 */
/* Testing and Validation (SSOT)                                             */
/* ==========================================================================
 */

/**
 * @brief Test Framework Configuration
 */
#define TEST_FRAMEWORK_DIR "tests"
#define TEST_UNITY_DIR "external/Unity"
#define TEST_MOCKS_DIR "tests/mocks"
#define TEST_FIXTURES_DIR "tests/fixtures"

// Test Execution
#define TEST_RUNNER_TIMEOUT_MS 10000
#define TEST_MAX_CONCURRENT 4
#define TEST_COVERAGE_TARGET_PCT 80

/* ==========================================================================
 */
/* Validation Macros and Utilities (SSOT)                                    */
/* ==========================================================================
 */

// Build-time validation
#if !defined(STM32H753xx)
#error "Target MCU not properly defined - check build configuration"
#endif

// String conversion for CMake integration
#define STRINGIFY(x) #x
#define EXPAND_STRINGIFY(x) STRINGIFY(x)

// Environment variable helpers
#define ENV_WORKSPACE_ROOT "WORKSPACE_ROOT"
#define ENV_BUILD_TYPE "BUILD_TYPE"
#define ENV_TOOLCHAIN_PATH "TOOLCHAIN_PATH"

/**
 * @brief Workspace Configuration Structure
 */
typedef struct {
    const char *workspace_root;
    const char *toolchain_path;
    const char *build_dir;
    const char *python_venv;
    WorkflowMode_t workflow_mode;
    uint32_t validation_flags;
} WorkspaceConfig_t;

// Global workspace configuration (defined in main.c or config.c)
extern const WorkspaceConfig_t g_workspace_config;

#endif /* WORKSPACE_CONFIG_H */

/**
 * @note Workspace Configuration SSOT Rules:
 * 1. All toolchain paths MUST be defined here
 * 2. CMake files MUST reference these definitions
 * 3. VS Code settings MUST use these paths
 * 4. Scripts MUST validate against these requirements
 * 5. Cross-platform compatibility centralized here
 * 6. Workflow behavior controlled from single location
 */
