<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# STM32H753ZI Stepper Motor Control Project - Copilot Instructions

This workspace develops firmware for an **STM32H753ZI Nucleo-144** board controlling stepper motors via **X-NUCLEO-IHM02A1** shield with closed-loop feedback using **AS5600 magnetic encoders**.

## X-CUBE-SPN2 + MCSDK Hybrid Integration

This project uses a **hybrid approach** combining **X-CUBE-SPN2** (stepper-specific) with **X-CUBE-MCSDK 6.4.1** (selective enhancements):

### Primary Framework: X-CUBE-SPN2
- **Core Architecture**: X-CUBE-SPN2 stepper motor expansion package
- **Hardware Support**: X-NUCLEO-IHM02A1 shield with dual L6470 drivers
- **Reference Documentation**: `00_reference/x_cube_spn2_markdown_docs` (2.1MB, 197 files)
- **Advantages**: Native stepper support, no algorithm limitations, perfect IHM02A1 integration

### Enhancement Layer: MCSDK 6.4.1 (Selective)
- **Advanced Algorithms**: Position control, motion profiling, safety systems
- **Development Tools**: Motor Control Workbench, real-time monitoring
- **Enhanced Features**: Dual sensor support, speed overshoot mitigation
- **Integration**: Selective adoption of MCSDK capabilities within SPN2 framework

### File Organization
- **Primary Code**: `src/spn2/` (X-CUBE-SPN2 components)
- **Enhancement Code**: `src/mcsdk_selective/` (selected MCSDK components)
- **Integration Layer**: `src/drivers/adaptation/` (SPN2-MCSDK bridge)
- **CubeMX Project**: `stm32h753zi_spn2_mcsdk.ioc`

## Modular Instruction System

This project uses a **modular instruction system** located in `.github/instructions/` with domain-specific guidance:

### Core System Instructions
- **ssot-config.instructions.md**: Single Source of Truth configuration management
- **hardware-pins.instructions.md**: STM32H753ZI pin assignments and peripheral configuration
- **data-types.instructions.md**: Consistent data types and type safety
- **error-handling.instructions.md**: Error codes, fault recovery, and diagnostics
- **build-config.instructions.md**: Build configuration and version management
- **build-system.instructions.md**: CMake toolchain, VS Code setup, and build troubleshooting

### Motor Control Instructions
- **l6470-registers.instructions.md**: L6470 stepper driver configuration and control
- **safety-systems.instructions.md**: Safety systems, watchdog, and fail-safe mechanisms
- **time-handling.instructions.md**: Timing systems and control loop management
- **units-measurements.instructions.md**: Physical units and measurement handling

### STM32H7 HAL Implementation (Enhanced with Comprehensive Documentation)
- **stm32h7-gpio-hal.instructions.md**: STM32H7 GPIO HAL with 198 documentation files referenced
- **stm32h7-spi-l6470.instructions.md**: STM32H7 SPI HAL for L6470 daisy-chain communication  
- **stm32h7-i2c-as5600.instructions.md**: STM32H7 I2C HAL for AS5600 dual-encoder configuration
- **stm32h7-uart-protocol.instructions.md**: STM32H7 UART HAL for command interface and debugging

### Communication Instructions
- **comm-protocols.instructions.md**: UART, CAN, SPI, I2C, and Ethernet protocols
- **workspace-setup.instructions.md**: Workspace-level project guidance

### MCSDK Integration Documentation
- **docs/MCSDK_INTEGRATION_PLAN.md**: Comprehensive X-CUBE-SPN2 + MCSDK 6.4.1 hybrid integration strategy and CubeMX workflow

Each instruction file targets specific source directories and provides comprehensive technical guidance for that domain.

## Critical Design Principles

### Single Source of Truth (SSOT)
**CRITICAL**: All configuration must reference centralized SSOT sources:
- Hardware configurations in `src/config/hardware_config.h`
- Communication parameters in `src/config/comm_config.h`
- Motor settings in `src/config/motor_config.h`
- Safety thresholds in `src/config/safety_config.h`
- Build metadata in `src/config/build_config.h`

### Safety First
**CRITICAL**: Always implement fail-safe behavior:
- Monitor L6470 fault flags continuously
- Implement watchdog timer for fault recovery
- Validate all inputs and sensor readings
- Stop motors immediately on any fault condition
- Use controlled motion profiles to prevent mechanical stress

## Reference Assets and Documentation System
The `00_reference/` directory contains comprehensive ST official documentation:
- **STM32H7 HAL Documentation**: 86MB, 3,988 markdown files with complete peripheral coverage
- **X-CUBE-SPN2 L6470 Documentation**: 2.1MB stepper driver specific reference with 197 files
- **STM32H7xx Nucleo BSP Documentation**: 824KB, 42 markdown files with board support package functions
- **CMSIS Documentation**: 49MB, 2,268 HTML files (reference only, not converted)
- **STM32H7 HAL drivers and examples**: Complete implementation examples
- **X-NUCLEO-IHM02A1 board support package**: Shield-specific guidance
- **Application examples and datasheets**: Real-world implementation patterns

**IMPORTANT**: Never modify `00_reference/` files. Copy needed code to appropriate `src/` locations.

## Enhanced Documentation Search System
Use comprehensive documentation search tools for development with **unified STM32H7 + L6470 + Nucleo BSP coverage**:

```bash
# Search for peripheral-specific documentation (STM32H7 + L6470)
python3 scripts/search_enhanced_docs.py peripheral SPI --scope all

# Find L6470 function implementation details (NOTE: functions are concatenated)
python3 scripts/search_enhanced_docs.py function L6470 --scope L6470

# Find STM32H7 HAL functions
python3 scripts/search_enhanced_docs.py function HAL_GPIO_Init --scope STM32H7

# Search Nucleo BSP board functions
python3 scripts/search_enhanced_docs.py function BSP_LED --scope NUCLEO_BSP
python3 scripts/search_enhanced_docs.py function BSP_PB --scope NUCLEO_BSP

# Search Nucleo BSP board functions
python3 scripts/search_enhanced_docs.py function BSP_LED --scope NUCLEO_BSP
python3 scripts/search_enhanced_docs.py function BSP_PB --scope NUCLEO_BSP

# Search L6470 registers
python3 scripts/search_enhanced_docs.py register ABS_POS --scope L6470

# Discover concept-related files across all documentation
python3 scripts/search_enhanced_docs.py concept "stepper motor" --scope all
python3 scripts/search_enhanced_docs.py concept board_support --scope NUCLEO_BSP

# Validate workspace markdown links
python3 scripts/link_validator.py
```

**Available Indexes:**
- `docs/indexes/STM32H7_FULL_INDEX.json` (8.9MB): Complete STM32H7 searchable index with 31,772 keywords
- `docs/indexes/L6470_SEARCH_INDEX.json` (2.1MB): Complete L6470 documentation index with 356 keywords
- `docs/indexes/STM32H7xx_Nucleo_BSP_INDEX.json` (1MB): Nucleo-144 BSP functions with 74 keywords
- `docs/indexes/STM32H7_COPILOT_INDEX.yaml` (8KB): Copilot-optimized quick reference  
- `docs/indexes/STM32H7_COPILOT_INDEX.json` (9.9KB): Programmatic access format

**Search Capabilities:**
- **Unified search**: Single tool for STM32H7, L6470, and Nucleo BSP documentation
- **Scope filtering**: Target specific documentation sets (STM32H7/L6470/NUCLEO_BSP/all)
- **Performance**: 7x faster than HTML-based searches (0.03s vs 0.22s)
- **Coverage**: 91.1MB total documentation, 4,227 files, 32,200+ keywords

**Search Pattern Best Practices:**
- **STM32H7**: Use exact names (`HAL_GPIO_Init`, `peripheral SPI`)  
- **L6470**: Use broad patterns (`function L6470`, not `L6470_Init`) - functions are concatenated in index
- **NUCLEO_BSP**: Use exact BSP function names (`BSP_LED_Init`, `BSP_PB_Init`)
- **Cross-platform**: Use concept searches (`concept "motor_control"`) for comprehensive results
- **Scope**: Case-insensitive (`--scope STM32H7`, `--scope nucleo_bsp`, `--scope L6470`)

**Common Search Patterns:**
```bash
# STM32H7 - Use exact function names (work well)
python3 scripts/search_enhanced_docs.py function HAL_GPIO_Init --scope STM32H7
python3 scripts/search_enhanced_docs.py peripheral SPI --scope STM32H7

# L6470 - Use broad patterns (functions are concatenated)
python3 scripts/search_enhanced_docs.py function L6470 --scope L6470    # ✅ CORRECT
python3 scripts/search_enhanced_docs.py function L6470_Init --scope L6470  # ❌ WILL FAIL

# Nucleo BSP - Use exact BSP function names (work well)
python3 scripts/search_enhanced_docs.py function BSP_LED_Init --scope NUCLEO_BSP  # ✅ CORRECT
python3 scripts/search_enhanced_docs.py function BSP_BUTTON --scope NUCLEO_BSP    # ✅ CORRECT

# Cross-platform concept searches (most effective for complex topics)
python3 scripts/search_enhanced_docs.py concept "stepper configuration" --scope all
python3 scripts/search_enhanced_docs.py concept "board support" --scope all
```

## Reference Assets
The `00_reference/` directory contains ST official assets (READ-ONLY):
- STM32H7 HAL drivers and examples
- X-NUCLEO-IHM02A1 board support package
- L6470 driver libraries and documentation
- Application examples and datasheets

**IMPORTANT**: Never modify `00_reference/` files. Copy needed code to appropriate `src/` locations.

## Environment Context
This workspace runs in a dev container with:
- ARM GCC toolchain, OpenOCD, Doxygen with Graphviz
- Unity/GoogleTest frameworks
- Python tools: CAN, Sphinx, Breathe for docs
- SSOT validation tools: jq, pre-commit hooks
- All development tools for embedded C/C++

## Reference Material Usage
When referencing `00_reference/` assets:
- **Copy** needed header files to `src/` with proper attribution
- **Extract** relevant code snippets with modifications for project needs
- **Document** the source of copied/adapted code in SSOT headers
- **Never** directly include from `00_reference/` in build system

Remember: **Safety first, SSOT always, modular design throughout.**
