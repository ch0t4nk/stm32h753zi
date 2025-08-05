---
applyTo: "**"
description: "SSOT configuration management for STM32H753ZI stepper motor project"
---

### Configuration Dependencies
- Hardware config defines physical constraints
- Motor config must respect hardware limits
- Communication config must match hardware capabilities
- Safety config provides bounds for all other configs
- Documentation config defines structure for all project documentation

### Documentation Links
Always include references to related instruction files:
```c
// TODO: See .github/instructions/hardware-pins.instructions.md for pin mapping details
// TODO: See .github/instructions/l6470-registers.instructions.md for register configuration
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
```r motor project"
---

# SSOT Configuration Management Instructions

## Overview
This instruction file provides guidance for maintaining the Single Source of Truth (SSOT) architecture across all configuration files in the STM32H753ZI stepper motor control project.

## SSOT Principles

### Always Reference SSOT Headers
- **NEVER** hardcode hardware constants in implementation files
- **ALWAYS** include the appropriate SSOT header from `src/config/`
- **VALIDATE** all configuration parameters at compile time where possible

### Configuration File Hierarchy
```
src/config/
├── hardware_config.h      // Pin assignments, peripheral mappings (SSOT for hardware)
├── comm_config.h          // Protocol parameters, timing, addresses (SSOT for communication)
├── motor_config.h         // L6470 settings, motion limits (SSOT for motor control)
├── safety_config.h        // Watchdog, fault thresholds (SSOT for safety systems)
├── build_config.h         // Version, compilation flags (SSOT for build system)
└── documentation_config.h // Documentation paths, indexes, tooling (SSOT for documentation)
```

## Code Generation Rules

### Correct SSOT Usage Pattern
```c
// ✅ CORRECT - Reference SSOT configuration
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

## Configuration Validation Requirements

### Runtime Parameter Validation
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

### Compile-Time Validation
```c
// Static assertions for SSOT validation
_Static_assert(MAX_MOTORS == 2, "Project designed for exactly 2 motors");
_Static_assert(UART_BAUDRATE <= 115200, "UART baudrate exceeds safe limit");
_Static_assert(CAN_BITRATE_KBPS <= 1000, "CAN bitrate exceeds specification");
```

## Cross-Reference Requirements

### Configuration Dependencies
- Hardware config defines physical constraints
- Motor config must respect hardware limits
- Communication config must match hardware capabilities
- Safety config provides bounds for all other configs

### Documentation Links
Always include references to related instruction files:
```c
// TODO: See .github/instructions/hardware-pins.instructions.md for pin mapping details
// TODO: See .github/instructions/l6470-registers.instructions.md for register configuration
```

## SSOT Validation Integration

### Automated Validation
Use the validation script to check SSOT compliance:
```bash
python3 scripts/validate_ssot.py
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
Set up validation to run automatically:
```bash
# In .git/hooks/pre-commit
#!/bin/bash
python3 scripts/validate_ssot.py || exit 1
python3 scripts/link_validator.py || exit 1
```

## Common SSOT Anti-Patterns to Avoid

### 1. Configuration Duplication
❌ Don't duplicate the same constant in multiple files
✅ Define once in appropriate SSOT header, reference everywhere

### 2. Hardcoded Magic Numbers
❌ `delay_ms(100);` 
✅ `delay_ms(MOTOR_INIT_DELAY_MS);`

### 3. Inconsistent Units
❌ Mix degrees, radians, steps without clear conversion
✅ Use consistent units defined in SSOT, provide conversion functions

### 4. Environment-Specific Values
❌ Different pin assignments for different developers
✅ Centralized hardware configuration that works for all

### 5. Hardcoded Documentation Paths
❌ `search_index = "docs/indexes/STM32H7_FULL_INDEX.json"`
✅ `search_index = DOC_INDEX_STM32H7_FULL`

### 6. Inconsistent Instruction References
❌ Mix of `.instructions/` and `.github/instructions/` paths
✅ All references use SSOT documentation configuration

## SSOT Maintenance Guidelines

### Adding New Configuration
1. Determine appropriate SSOT file (hardware, comm, motor, safety, build, documentation)
2. Add parameter with clear naming and documentation
3. Update validation script to check new parameter
4. Reference from implementation files only

### Adding New Documentation
1. Add new files to appropriate documentation directories
2. Update `documentation_config.h` with new path definitions
3. Update search indexes if adding reference documentation
4. Add references to related instruction files
5. Validate all documentation links work correctly

### Modifying Existing Configuration
1. Check all usage locations before changing
2. Ensure backward compatibility or update all references
3. Run full SSOT validation after changes
4. Update related instruction files

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
- Safety implications if applicable

Remember: **SSOT is not just about avoiding duplication - it's about ensuring system consistency, safety, and maintainability.**
