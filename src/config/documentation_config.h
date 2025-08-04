/**
 * @file documentation_config.h
 * @brief Documentation Structure Configuration - SSOT for Documentation
 * Management
 *
 * This file defines the Single Source of Truth for all documentation paths,
 * search indexes, instruction files, and documentation tooling configuration.
 *
 * @author STM32H753ZI Stepper Motor Project Team
 * @date 2025-01-04
 * @version 1.0.0
 *
 * SSOT Compliance:
 * - All documentation paths must reference these definitions
 * - All search tools must use these index locations
 * - All instruction references must use these path patterns
 * - All generated documentation must follow these structure rules
 *
 * Related Files:
 * - .github/instructions/ssot-config.instructions.md - SSOT architecture
 * guidance
 * - .github/instructions/workspace-setup.instructions.md - Project overview
 * - scripts/search_enhanced_docs.py - Uses DOC_INDEX_* definitions
 * - scripts/validate_ssot.py - Validates these configurations
 */

#ifndef DOCUMENTATION_CONFIG_H
#define DOCUMENTATION_CONFIG_H

/* =============================================================================
 * DOCUMENTATION STRUCTURE CONFIGURATION
 * =============================================================================
 * These paths define the official documentation structure for the project.
 * ALL documentation tools and scripts must reference these definitions.
 */

/* Documentation Root Directories */
#define DOC_ROOT_DIR "docs"
#define DOC_INDEXES_DIR "docs/indexes"
#define DOC_DESIGN_DIR "docs/design"
#define DOC_REQUIREMENTS_DIR "docs/requirements"

/* Instruction System Configuration */
#define INSTRUCTION_ROOT_DIR ".github/instructions"
#define INSTRUCTION_FILE_SUFFIX ".instructions.md"
#define INSTRUCTION_README_FILE "README.md"

/* Reference Documentation Directories */
#define REFERENCE_ROOT_DIR "00_reference"
#define REFERENCE_STM32H7_DIR "00_reference/stm32h753xx_User_Manual_md"
#define REFERENCE_L6470_DIR "00_reference/x_cube_spn2_markdown_docs"
#define REFERENCE_ST_ASSETS_DIR "00_reference/ST_Assets"

/* =============================================================================
 * SEARCH INDEX CONFIGURATION
 * =============================================================================
 * These definitions specify the locations and formats of all search indexes
 * used by the enhanced documentation search system.
 */

/* STM32H7 Documentation Indexes */
#define DOC_INDEX_STM32H7_FULL "docs/indexes/STM32H7_FULL_INDEX.json"
#define DOC_INDEX_STM32H7_COPILOT_JSON "docs/indexes/STM32H7_COPILOT_INDEX.json"
#define DOC_INDEX_STM32H7_COPILOT_YAML "docs/indexes/STM32H7_COPILOT_INDEX.yaml"

/* L6470 Documentation Indexes */
#define DOC_INDEX_L6470_SEARCH "docs/indexes/L6470_SEARCH_INDEX.json"

/* Index Size and Performance Metrics (for validation) */
#define DOC_INDEX_STM32H7_SIZE_MB 8.9  // Expected size in megabytes
#define DOC_INDEX_L6470_SIZE_MB 2.1    // Expected size in megabytes
#define DOC_INDEX_TOTAL_FILES 4185     // Total files in all indexes
#define DOC_INDEX_TOTAL_KEYWORDS 32128 // Total searchable keywords

/* =============================================================================
 * INSTRUCTION FILE CONFIGURATION
 * =============================================================================
 * These definitions specify the complete instruction file structure and
 * categorization for the modular instruction system.
 */

/* Core System Instructions */
#define INSTRUCTION_SSOT_CONFIG                                                \
  ".github/instructions/ssot-config.instructions.md"
#define INSTRUCTION_WORKSPACE_SETUP                                            \
  ".github/instructions/workspace-setup.instructions.md"
#define INSTRUCTION_DATA_TYPES ".github/instructions/data-types.instructions.md"
#define INSTRUCTION_ERROR_HANDLING                                             \
  ".github/instructions/error-handling.instructions.md"
#define INSTRUCTION_BUILD_CONFIG                                               \
  ".github/instructions/build-config.instructions.md"
#define INSTRUCTION_BUILD_SYSTEM                                               \
  ".github/instructions/build-system.instructions.md"

/* Hardware Configuration Instructions */
#define INSTRUCTION_HARDWARE_PINS                                              \
  ".github/instructions/hardware-pins.instructions.md"
#define INSTRUCTION_HARDWARE_VARIANTS                                          \
  ".github/instructions/hardware-variants.instructions.md"
#define INSTRUCTION_MEMORY_CONFIG                                              \
  ".github/instructions/memory-config.instructions.md"

/* Motor Control Instructions */
#define INSTRUCTION_L6470_REGISTERS                                            \
  ".github/instructions/l6470-registers.instructions.md"
#define INSTRUCTION_SAFETY_SYSTEMS                                             \
  ".github/instructions/safety-systems.instructions.md"
#define INSTRUCTION_TIME_HANDLING                                              \
  ".github/instructions/time-handling.instructions.md"
#define INSTRUCTION_UNITS_MEASUREMENTS                                         \
  ".github/instructions/units-measurements.instructions.md"
#define INSTRUCTION_CONTROL_TIMING                                             \
  ".github/instructions/control-timing.instructions.md"

/* Communication Instructions */
#define INSTRUCTION_COMM_PROTOCOLS                                             \
  ".github/instructions/comm-protocols.instructions.md"
#define INSTRUCTION_STM32H7_UART                                               \
  ".github/instructions/stm32h7-uart-protocol.instructions.md"
#define INSTRUCTION_STM32H7_SPI_L6470                                          \
  ".github/instructions/stm32h7-spi-l6470.instructions.md"
#define INSTRUCTION_STM32H7_I2C_AS5600                                         \
  ".github/instructions/stm32h7-i2c-as5600.instructions.md"
#define INSTRUCTION_STM32H7_GPIO_HAL                                           \
  ".github/instructions/stm32h7-gpio-hal.instructions.md"

/* Development Process Instructions */
#define INSTRUCTION_BUILD_VALIDATION                                           \
  ".github/instructions/build-validation.instructions.md"
#define INSTRUCTION_BUILD_VERSION                                              \
  ".github/instructions/build-version.instructions.md"
#define INSTRUCTION_TESTING_CONFIG                                             \
  ".github/instructions/testing-config.instructions.md"
#define INSTRUCTION_DEBUG_CONFIG                                               \
  ".github/instructions/debug-config.instructions.md"
#define INSTRUCTION_OPTIMIZATION                                               \
  ".github/instructions/optimization.instructions.md"

/* System Architecture Instructions */
#define INSTRUCTION_STATE_MANAGEMENT                                           \
  ".github/instructions/state-management.instructions.md"
#define INSTRUCTION_STATE_ACCESS                                               \
  ".github/instructions/state-access.instructions.md"
#define INSTRUCTION_SYSTEM_STATE                                               \
  ".github/instructions/system-state.instructions.md"
#define INSTRUCTION_THREAD_SAFETY                                              \
  ".github/instructions/thread-safety.instructions.md"
#define INSTRUCTION_BUFFER_MANAGEMENT                                          \
  ".github/instructions/buffer-management.instructions.md"

/* Advanced Topics Instructions */
#define INSTRUCTION_FEATURE_FLAGS                                              \
  ".github/instructions/feature-flags.instructions.md"
#define INSTRUCTION_PORTABILITY                                                \
  ".github/instructions/portability.instructions.md"
#define INSTRUCTION_USER_INTERFACE                                             \
  ".github/instructions/user-interface.instructions.md"

/* =============================================================================
 * DOCUMENTATION TOOLING CONFIGURATION
 * =============================================================================
 * Configuration for documentation generation, validation, and search tools.
 */

/* Doxygen Configuration */
#define DOXYGEN_CONFIG_FILE "docs/Doxyfile"
#define DOXYGEN_OUTPUT_DIR "docs/doxygen"
#define DOXYGEN_HTML_DIR "docs/doxygen/html"
#define DOXYGEN_LATEX_DIR "docs/doxygen/latex"

/* Search Tool Configuration */
#define SEARCH_SCRIPT_PATH "scripts/search_enhanced_docs.py"
#define SEARCH_SCOPE_STM32H7 "STM32H7"
#define SEARCH_SCOPE_L6470 "L6470"
#define SEARCH_SCOPE_ALL "all"

/* Validation Tool Configuration */
#define VALIDATION_SCRIPT_PATH "scripts/validate_ssot.py"
#define LINK_VALIDATOR_SCRIPT_PATH "scripts/link_validator.py"
#define INSTRUCTION_VALIDATOR_PATH "scripts/fix_instruction_references.py"
#define CONSOLIDATION_SCRIPT_PATH                                              \
  "scripts/consolidate_instruction_references.py"

/* Documentation Output Paths */
#define DOC_GENERATED_DIR "docs/generated"
#define DOC_API_DIR "docs/api"
#define DOC_REPORTS_DIR "docs/reports"

/* Project Configuration Guides */
#define DOC_CMAKE_KIT_SETUP "docs/CMAKE_KIT_SETUP.md"
#define DOC_TOOLCHAIN_CONFIG "docs/TOOLCHAIN_CONFIGURATION.md"

/* =============================================================================
 * DOCUMENTATION STANDARDS AND VALIDATION
 * =============================================================================
 * Standards and validation criteria for documentation consistency.
 */

/* File Naming Standards */
#define DOC_MARKDOWN_EXTENSION ".md"
#define DOC_INSTRUCTION_EXTENSION ".instructions.md"
#define DOC_CONFIG_EXTENSION ".h"
#define DOC_INDEX_EXTENSION ".json"

/* Documentation Quality Metrics */
#define DOC_MIN_INSTRUCTION_SIZE_BYTES 1000 // Minimum instruction file size
#define DOC_MAX_BROKEN_LINKS 0              // Zero tolerance for broken links
#define DOC_MIN_COVERAGE_PERCENT 95         // Minimum documentation coverage

/* Search Performance Standards */
#define SEARCH_MAX_RESPONSE_TIME_MS 50  // Maximum search response time
#define SEARCH_MIN_PRECISION_PERCENT 90 // Minimum search precision

/* =============================================================================
 * DOCUMENTATION VALIDATION FUNCTIONS
 * =============================================================================
 * These macros provide compile-time validation for documentation configuration.
 */

/* Static Assertions for Documentation Structure */
#ifndef __ASSEMBLER__
#include <assert.h>

/* Validate instruction file count matches expected */
#define EXPECTED_INSTRUCTION_FILES 31
#define EXPECTED_INDEX_FILES 4
#define EXPECTED_CORE_INSTRUCTIONS 5

/* Compile-time validation macros */
#define VALIDATE_DOC_PATH(path)                                                \
  _Static_assert(sizeof(path) > 1, "Documentation path cannot be empty")

#define VALIDATE_EXTENSION(filename, expected_ext)                             \
  _Static_assert(strstr(filename, expected_ext) != NULL,                       \
                 "File does not have expected extension")

/* Apply validations */
VALIDATE_DOC_PATH(DOC_ROOT_DIR);
VALIDATE_DOC_PATH(INSTRUCTION_ROOT_DIR);
VALIDATE_DOC_PATH(REFERENCE_ROOT_DIR);

#endif /* __ASSEMBLER__ */

/* =============================================================================
 * USAGE EXAMPLES AND INTEGRATION GUIDANCE
 * =============================================================================
 *
 * This configuration should be used as follows:
 *
 * 1. In Search Scripts:
 *    #include "config/documentation_config.h"
 *    index_file = DOC_INDEX_STM32H7_FULL;
 *
 * 2. In Validation Scripts:
 *    #include "config/documentation_config.h"
 *    check_file_exists(INSTRUCTION_SSOT_CONFIG);
 *
 * 3. In Source Files:
 *    // TODO: See INSTRUCTION_L6470_REGISTERS for register configuration
 *    // TODO: See INSTRUCTION_HARDWARE_PINS for pin mapping details
 *
 * 4. In Build Scripts:
 *    #include "config/documentation_config.h"
 *    doxygen_cmd = f"doxygen {DOXYGEN_CONFIG_FILE}"
 *
 * IMPORTANT: Never hardcode documentation paths - always reference these
 * SSOT definitions to ensure consistency across the entire project.
 */

#endif /* DOCUMENTATION_CONFIG_H */

/* TODO: See .github/instructions/ssot-config.instructions.md for SSOT
 * architecture guidance */
/* TODO: See .github/instructions/workspace-setup.instructions.md for complete
 * project overview */
