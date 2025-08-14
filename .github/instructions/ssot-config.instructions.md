---
applyTo: "**"
description: "Dual SSOT configuration management for STM32H753ZI stepper motor project"
---

# Dual SSOT Configuration Management Instructions

## Overview

This instruction file provides guidance for maintaining the **Dual Single Source of Truth (SSOT) architecture** across all configuration files in the STM32H753ZI stepper motor control project. This architecture separates firmware behavior from development environment management for optimal flexibility and maintainability.

## Dual SSOT Architecture Principles

### 1. 🔧 Firmware SSOT (`src/config/`)

**Purpose**: Controls STM32H753ZI hardware behavior and motor control parameters  
**Scope**: Production firmware configuration, safety-critical settings, hardware abstraction  
**Target**: ARM Cortex-M7 microcontroller behavior and embedded system operation

### 2. ⚙️ Workflow SSOT (`src/config/workflow_config.h`)

**Purpose**: Controls development environment complexity and validation behavior  
**Scope**: Developer experience, cross-platform compatibility, validation depth control  
**Target**: Development toolchain, environment validation, workflow automation

### Always Reference SSOT Headers

- **NEVER** hardcode hardware constants in implementation files
- **NEVER** hardcode workflow behavior in scripts without SSOT reference
- **ALWAYS** include the appropriate SSOT header from `src/config/`
- **VALIDATE** all configuration parameters at compile time where possible

### Configuration File Hierarchy

```
src/config/
├── hardware_config.h      // Pin assignments, peripheral mappings (Firmware SSOT)
├── comm_config.h          // Protocol parameters, timing, addresses (Firmware SSOT)
├── motor_config.h         // L6470 settings, motion limits (Firmware SSOT)
├── safety_config.h        // Watchdog, fault thresholds (Firmware SSOT)
├── build_config.h         // Version, compilation flags (Firmware SSOT)
├── documentation_config.h // Documentation paths, indexes (Firmware SSOT)
└── workflow_config.h      // Development environment complexity control (Workflow SSOT)
```

## Firmware SSOT Implementation

### Correct Firmware SSOT Usage Pattern

```c
// ✅ CORRECT - Reference Firmware SSOT configuration
#include "config/hardware_config.h"
#include "config/motor_config.h"

void motor_init(void) {
    HAL_GPIO_WritePin(MOTOR1_CS_PORT, MOTOR1_CS_PIN, GPIO_PIN_SET);

    // Configure L6470 with SSOT values
    l6470_set_acceleration(MOTOR1_ACCELERATION);
    l6470_set_max_speed(MOTOR1_MAX_SPEED);
}
```

### Incorrect Hardcoded Values

```c
// ❌ INCORRECT - Hardcoded values
void motor_init(void) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);  // Hardcoded pin

    l6470_set_acceleration(0x500);  // Magic number
    l6470_set_max_speed(2000);      // Hardcoded speed
}
```

## Workflow SSOT Implementation

### Correct Workflow SSOT Usage Pattern

```bash
#!/bin/bash
# ✅ CORRECT - Reference Workflow SSOT configuration

# Use SSOT-aware script execution
source scripts/load_workflow_config.sh

if [[ "$WORKFLOW_VALIDATE_VENV" == "1" ]]; then
    validate_virtual_environment
fi

if [[ "$WORKFLOW_VERBOSE_LOGGING" == "1" ]]; then
    echo "🔍 Validating build environment..."
fi
```

```powershell
# ✅ CORRECT - PowerShell Workflow SSOT usage
# Use configurable script wrapper
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --verbose

# Check workflow mode
python scripts/workflow_config.py show
```

### Incorrect Hardcoded Workflow Behavior

```bash
# ❌ INCORRECT - Hardcoded workflow patterns
#!/bin/bash
# Always validate everything (no user choice)
validate_virtual_environment  # Forced validation
validate_build_environment   # No complexity control
echo "Verbose output enabled" # No verbosity control
```

## Configuration Validation Requirements

### Firmware SSOT Runtime Validation

```c
SystemError_t validate_motor_config(uint8_t motor_id) {
    if (motor_id >= MAX_MOTORS) {
        return ERROR_MOTOR_INVALID_ID;
    }

    if (MOTOR_MAX_SPEED > L6470_MAX_SPEED_LIMIT) {
        return ERROR_MOTOR_SPEED_OUT_OF_RANGE;
    }

    return SYSTEM_OK;
}
```

### Workflow SSOT Runtime Validation

```bash
# Validate workflow configuration consistency
python scripts/workflow_config.py validate

# Check individual workflow settings
if ! python scripts/workflow_config.py get validate_venv; then
    echo "Virtual environment validation disabled"
fi
```

### Compile-Time Validation

```c
// Static assertions for Firmware SSOT validation
_Static_assert(MAX_MOTORS == 2, "Project designed for exactly 2 motors");
_Static_assert(UART_BAUDRATE <= 115200, "UART baudrate exceeds safe limit");
_Static_assert(CAN_BITRATE_KBPS <= 1000, "CAN bitrate exceeds specification");

// Workflow SSOT validation (preprocessor checks)
#if WORKFLOW_MODE > WORKFLOW_MODE_DEBUG
#error "Invalid workflow mode specified"
#endif
```

## Cross-Reference Requirements

## Cross-Reference Requirements

### Firmware SSOT Dependencies

- Hardware config defines physical constraints
- Motor config must respect hardware limits
- Communication config must match hardware capabilities
- Safety config provides bounds for all other configs
- Documentation config defines structure for all project documentation

### Workflow SSOT Dependencies

- Environment validation depends on platform detection
- Convenience features depend on validation settings
- Error handling behavior depends on workflow mode
- Cross-platform compatibility depends on detection flags

### Documentation Links

Always include references to related instruction files:

```c
// Firmware SSOT references
// TODO: See .github/instructions/hardware.instructions.md for pin mapping details
// TODO: See .github/instructions/core-software.instructions.md for register configuration

// Workflow SSOT references
// TODO: See .github/instructions/development-workflow.instructions.md for workflow patterns
```

### Documentation SSOT Integration

All documentation tools and references must use SSOT definitions:

```c
#include "config/documentation_config.h"

// ✅ CORRECT - Use SSOT documentation paths
char* search_index = DOC_INDEX_STM32H7_FULL;
char* instruction_file = INSTRUCTION_HARDWARE_PINS;

// ❌ INCORRECT - Hardcoded documentation paths
char* search_index = "docs/indexes/STM32H7_FULL_INDEX.json";
char* instruction_file = ".github/instructions/hardware-pins.instructions.md";
```

## Dual SSOT Validation Integration

### Automated Validation

Use validation scripts to check both SSOT systems:

```bash
# Firmware SSOT validation
python3 scripts/validate_ssot.py

# Workflow SSOT validation
python3 scripts/workflow_config.py validate

# Combined validation
python3 scripts/validate_ssot.py --include-workflow
```

### Workflow SSOT Mode Switching

```powershell
# PowerShell (Windows)
python scripts/workflow_config.py mode minimal    # Fast mode for CI/CD
python scripts/workflow_config.py mode standard   # Default development
python scripts/workflow_config.py mode thorough   # Full guidance for complex work
python scripts/workflow_config.py mode debug      # Maximum verbosity for troubleshooting

# Use SSOT-aware execution for all Python operations
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --verbose
```

```bash
# Bash (Linux/macOS)
.venv/bin/python scripts/workflow_config.py mode standard
.venv/bin/python scripts/auto_update_status.py --verbose
```

### Documentation SSOT Validation

Validate documentation structure and search indexes:

```bash
# Validate all SSOT configurations including documentation
python3 scripts/validate_ssot.py --include-docs

# Validate instruction file references
python3 scripts/fix_instruction_references.py

# Validate documentation links
python3 scripts/link_validator.py

# Test enhanced documentation search with Copilot optimization
python3 scripts/search_enhanced_docs.py concept "safety_systems" --scope STM32H7
python3 scripts/search_enhanced_docs.py concept "motor_control" --scope all
python3 scripts/search_enhanced_docs.py function BSP_LED --scope NUCLEO_BSP
python3 scripts/search_enhanced_docs.py test --scope all
```

### Pre-commit Hooks

Set up validation to run automatically for both SSOT systems:

```bash
# In .git/hooks/pre-commit
#!/bin/bash
python3 scripts/validate_ssot.py || exit 1
python3 scripts/workflow_config.py validate || exit 1
python3 scripts/link_validator.py || exit 1
```

## Common Dual SSOT Anti-Patterns to Avoid

### 1. Configuration Duplication (Firmware SSOT)

❌ Don't duplicate the same constant in multiple files
✅ Define once in appropriate SSOT header, reference everywhere

### 2. Hardcoded Magic Numbers (Firmware SSOT)

❌ `delay_ms(100);`
✅ `delay_ms(MOTOR_INIT_DELAY_MS);`

### 3. Inconsistent Units (Firmware SSOT)

❌ Mix degrees, radians, steps without clear conversion
✅ Use consistent units defined in SSOT, provide conversion functions

### 4. Environment-Specific Values (Firmware SSOT)

❌ Different pin assignments for different developers
✅ Centralized hardware configuration that works for all

### 5. Hardcoded Workflow Behavior (Workflow SSOT)

❌ Always validate everything regardless of user experience level
✅ Use workflow mode and individual flags for adaptive behavior

### 6. Platform-Specific Scripts (Workflow SSOT)

❌ Separate scripts for Windows/Linux with different behavior
✅ SSOT-aware cross-platform scripts that detect and adapt

### 7. Hardcoded Documentation Paths (Documentation SSOT)

❌ `search_index = "docs/indexes/STM32H7_FULL_INDEX.json"`
✅ `search_index = DOC_INDEX_STM32H7_FULL`

### 8. Inconsistent Instruction References (Documentation SSOT)

❌ Mix of `.instructions/` and `.github/instructions/` paths
✅ All references use SSOT documentation configuration

## Dual SSOT Maintenance Guidelines

### Adding New Firmware Configuration

1. Determine appropriate SSOT file (hardware, comm, motor, safety, build, documentation)
2. Add parameter with clear naming and documentation
3. Update validation script to check new parameter
4. Reference from implementation files only

### Adding New Workflow Configuration

1. Determine if it's a mode-specific setting or individual flag
2. Add to `workflow_config.h` with proper mode defaults
3. Update `workflow_config.py` script to handle new setting
4. Test all workflow modes with new configuration

### Adding New Documentation

1. Add new files to appropriate documentation directories
2. Update `documentation_config.h` with new path definitions
3. Update search indexes if adding reference documentation
4. Add references to related instruction files
5. Validate all documentation links work correctly

### Modifying Existing Firmware Configuration

1. Check all usage locations before changing
2. Ensure backward compatibility or update all references
3. Run full SSOT validation after changes
4. Update related instruction files

### Modifying Existing Workflow Configuration

1. Test impact on all workflow modes (minimal/standard/thorough/debug)
2. Ensure cross-platform compatibility (Windows/Linux/macOS)
3. Update documentation and examples
4. Validate with different user experience scenarios

### Modifying Documentation Structure

1. Update `documentation_config.h` first with new paths
2. Update all tools and scripts that reference changed paths
3. Regenerate search indexes if reference docs change (Copilot-optimized)
4. Validate instruction file references still work
5. Update workspace setup and copilot instructions
6. Test semantic categorization for new documentation

### Copilot-Optimized Documentation Workflow

When adding documentation that affects Copilot assistance:

```bash
# Regenerate Copilot-optimized indexes after documentation changes
python3 scripts/documentation_manager.py index recreate --scope STM32H7
python3 scripts/documentation_manager.py index recreate --scope L6470

# Test search functionality with semantic categories
python3 scripts/search_enhanced_docs.py concept "new_concept" --scope all

# Validate token efficiency improvements
python3 scripts/search_enhanced_docs.py test --scope all
```

### Configuration Documentation

Each SSOT parameter must include:

- Clear description of purpose
- Valid range/values
- Units of measurement
- Related parameters or constraints
- Safety implications if applicable (Firmware SSOT)
- User experience implications if applicable (Workflow SSOT)

## Dual SSOT Integration Examples

### Firmware + Workflow SSOT Integration

```c
// Firmware SSOT configuration
#include "config/hardware_config.h"
#include "config/workflow_config.h"

void debug_pin_toggle(void) {
    #if WORKFLOW_VERBOSE_LOGGING
    // Only toggle debug pin in verbose workflow modes
    HAL_GPIO_TogglePin(DEBUG_LED_PORT, DEBUG_LED_PIN);
    #endif
}
```

### Script Integration Example

```bash
#!/bin/bash
# Script that uses both SSOT systems

# Load workflow configuration
source scripts/load_workflow_config.sh

# Use firmware SSOT for hardware validation
if [[ "$WORKFLOW_VALIDATE_BUILD_ENV" == "1" ]]; then
    echo "Validating ARM GCC toolchain..."
    arm-none-eabi-gcc --version || exit 1
fi

# Execute with appropriate verbosity
if [[ "$WORKFLOW_VERBOSE_LOGGING" == "1" ]]; then
    echo "🔧 Building with firmware SSOT configuration..."
fi

cmake --build build
```

Remember: **Dual SSOT is not just about avoiding duplication - it's about ensuring system consistency, safety, maintainability, and adaptive developer experience based on context and user needs.**
