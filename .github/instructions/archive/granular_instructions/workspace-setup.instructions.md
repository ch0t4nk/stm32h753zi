---
applyTo: "*"  
description: "Workspace-level guidance and project overview for STM32H753ZI stepper motor project"
---

# STM32H753ZI Stepper Motor Project - Workspace Overview

## Project Overview

This project uses an **STM32H753ZI Nucleo-144** board as the main controller to drive two stepper motors in closed-loop control. The motors are driven by an **X-NUCLEO-IHM02A1** expansion shield containing two ST **L6470** stepper driver ICs controlled via SPI. Each motor's position is monitored by an **AS5600 magnetic encoder** for feedback.

**Key Components:**

| Component | Role |
|-----------|------|
| **STM32H753ZI MCU** | Main controller (ARM Cortex-M7, 480MHz) |
| **X-NUCLEO-IHM02A1 Shield** | Dual stepper driver (2x L6470 dSPIN ICs) |
| **Stepper Motors (2×)** | Bipolar stepper motors for actuation |
| **AS5600 Encoders (2×)** | 12-bit magnetic rotary sensors for feedback |
| **ST-Link V3** | Debugger + USB Virtual COM Port |
| **Communication** | UART, CAN-FD, Ethernet (LAN8742 PHY) |

## Modular Instruction System

This project uses a **comprehensive modular instruction system** in `.github/instructions/`:

### Core System Instructions
- **ssot-config.instructions.md**: Single Source of Truth configuration management
- **hardware-pins.instructions.md**: STM32H753ZI pin assignments and peripherals
- **data-types.instructions.md**: Consistent data types and type safety
- **error-handling.instructions.md**: Error codes, fault recovery, and diagnostics
- **build-config.instructions.md**: Build configuration and version management
- **build-system.instructions.md**: CMake toolchain, VS Code setup, and build troubleshooting

### Motor Control Instructions
- **l6470-registers.instructions.md**: L6470 stepper driver configuration and control
- **safety-systems.instructions.md**: Safety systems, watchdog, and fail-safe mechanisms
- **time-handling.instructions.md**: Timing systems and control loop management
- **units-measurements.instructions.md**: Physical units and measurement handling

### Communication Instructions
- **comm-protocols.instructions.md**: UART, CAN, SPI, I2C, and Ethernet protocols

Each instruction file provides comprehensive technical guidance targeting specific source directories.

## Critical Design Philosophy

### Safety First
- **Fail-safe design**: Motors stop rather than run uncontrolled
- **Continuous monitoring**: L6470 fault flags, encoder validation, communication timeouts
- **Watchdog protection**: Independent watchdog timer for fault recovery
- **Controlled motion**: Acceleration/deceleration profiles prevent mechanical stress

### SOLID Architecture  
- **Single Responsibility**: Separate classes for motor control, drivers, encoders, communication
- **Open/Closed**: Abstract interfaces for extensibility
- **Liskov Substitution**: Implementations are substitutable
- **Interface Segregation**: Focused, specific interfaces
- **Dependency Inversion**: Depend on abstractions, not implementations

### Single Source of Truth (SSOT)
Configuration is centralized in `src/config/` - see `ssot-config.instructions.md` for details.

## Development Environment

This workspace runs in a dev container with:
- **ARM GCC Toolchain**: Cross-compilation for STM32H7
- **OpenOCD**: On-chip debugging and programming
- **STM32CubeMX Integration**: HAL configuration and code generation
- **Testing Frameworks**: Unity and GoogleTest for embedded testing
- **Python Tools**: CAN utilities, documentation generation (Sphinx/Breathe)
- **SSOT Validation**: Pre-commit hooks, configuration validation scripts
- **Documentation**: Doxygen with Graphviz for API documentation
- **Enhanced Search System**: Copilot-optimized unified STM32H7 + L6470 + Nucleo BSP documentation search (91.1MB coverage, 32,200+ keywords)

## Reference Assets

The `00_reference/` directory contains official ST assets (READ-ONLY):
- **STM32H7 HAL Documentation**: 86MB, 3,988 markdown files with complete peripheral coverage and semantic categorization
- **L6470 Documentation**: 2.1MB X-CUBE-SPN2 stepper driver documentation with 356 searchable keywords
- **STM32H7xx Nucleo BSP Documentation**: 824KB, 42 markdown files with board support package functions
- **CMSIS Documentation**: 49MB, 2,268 HTML files (reference only, available for manual consultation)
- **STM32H7 HAL drivers and examples**: Complete implementation examples
- **X-NUCLEO-IHM02A1 board support package**: Shield-specific guidance
- **Application examples and datasheets**: Real-world implementation patterns

**Enhanced Documentation Search:**
```bash
# Search STM32H7 peripherals with semantic categorization
python3 scripts/search_enhanced_docs.py peripheral SPI --scope STM32H7

# Search L6470 functions and registers with token efficiency
# NOTE: L6470 functions are concatenated - use broad searches
python3 scripts/search_enhanced_docs.py function L6470 --scope L6470
python3 scripts/search_enhanced_docs.py register ABS_POS --scope L6470

# Search Nucleo BSP board functions
python3 scripts/search_enhanced_docs.py function BSP_LED --scope NUCLEO_BSP
python3 scripts/search_enhanced_docs.py function BSP_PB --scope NUCLEO_BSP

# Unified search across all documentation with Copilot optimization
python3 scripts/search_enhanced_docs.py concept "motor_control" --scope all

# Search specific technical concepts with semantic targeting
python3 scripts/search_enhanced_docs.py concept "safety_systems" --scope STM32H7
python3 scripts/search_enhanced_docs.py concept "stepper_configuration" --scope L6470
python3 scripts/search_enhanced_docs.py concept board_support --scope NUCLEO_BSP

# Validate workspace links
python3 scripts/link_validator.py
```

**Search Pattern Best Practices:**
- **STM32H7**: Use exact names (`HAL_GPIO_Init`, `peripheral SPI`)  
- **L6470**: Use broad patterns (`function L6470`, not `L6470_Init`) - functions are concatenated in index
- **NUCLEO_BSP**: Use exact BSP function names (`BSP_LED_Init`, `BSP_PB_Init`)
- **Cross-platform**: Use concept searches (`concept "motor_control"`) for comprehensive results
- **Scope**: Case-insensitive (`--scope STM32H7`, `--scope nucleo_bsp`, `--scope L6470`)

**Search Index Locations:**
- `docs/indexes/STM32H7_FULL_INDEX.json` - Complete STM32H7 documentation index (8.9MB, 31,772 keywords)
- `docs/indexes/L6470_SEARCH_INDEX.json` - Complete L6470 documentation index (356 keywords)
- `docs/indexes/STM32H7xx_Nucleo_BSP_INDEX.json` - Nucleo-144 BSP functions index (74 keywords)
- `docs/indexes/STM32H7_COPILOT_INDEX.*` - Copilot-optimized quick reference formats (token-efficient)

**IMPORTANT**: Never modify `00_reference/` files. Copy needed code to appropriate `src/` locations.

## Quick Start Guide

1. **Initialize Development Environment**
   ```bash
   # The dev container provides all necessary tools
   # STM32CubeMX, ARM GCC, OpenOCD are pre-installed
   ```

2. **Configure Build System**
   - VS Code: `Ctrl+Shift+P` → `CMake: Select a Kit` → `ARM GCC for STM32H753ZI`
   - See `build-system.instructions.md` for detailed CMake/toolchain setup
   - Verify: ARM GCC toolchain and Ninja build system are configured

3. **Review Modular Instructions**
   - Each `.instructions.md` file provides domain-specific guidance
   - Start with `ssot-config.instructions.md` for configuration principles
   - Reference `build-system.instructions.md` for build troubleshooting
   - Reference `hardware-pins.instructions.md` for STM32H753ZI specifics

4. **Build and Test**
   ```bash
   # Configure CMake with ARM toolchain (first time)
   cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake
   
   # Build project
   cmake --build build
   
   # Run tests
   cd build && ctest
   
   # Flash to hardware
   openocd -f interface/stlink.cfg -f target/stm32h7x.cfg 
           -c "program build/stm32h753_ihm02a1.elf verify reset exit"
   ```

## Architecture Overview

```
/workspaces/code/
├── src/                     # Source code and SSOT configuration
│   ├── config/             # SSOT configuration headers
│   ├── common/             # SSOT common definitions  
│   ├── drivers/            # Hardware abstraction layer
│   ├── controllers/        # Control algorithms and logic
│   ├── communication/      # Protocol implementations
│   ├── safety/             # Safety monitoring and response
│   └── application/        # Main application logic
├── docs/                   # Documentation and project summaries
│   ├── indexes/            # Search indexes (STM32H7, L6470)
│   ├── design/             # Design documentation
│   └── requirements/       # Requirements and specifications
├── scripts/                # Development and automation tools
├── tests/                  # Test code and test fixtures
├── 00_reference/           # Read-only ST reference materials
└── [build system files]    # CMake, build artifacts, etc.
```

This modular structure ensures clear separation of concerns while maintaining centralized configuration through the SSOT principle.

Remember: **Safety first, SSOT always, modular design throughout.**
