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

## Consolidated Instruction System

This project uses a **streamlined domain-based instruction system** located in `.github/instructions/` with optimized VS Code Copilot scoping:

### Domain-Based Instructions (6 Core Files)
- **project-setup.instructions.md**: Workspace setup, build configuration, testing infrastructure, optimization (`**/*.{c,h,md,cmake,txt,json,yml,yaml}`)
- **hardware.instructions.md**: Hardware configuration, GPIO control, memory management, STM32H7 HAL (`src/drivers/**/*.{c,h}`)
- **comms.instructions.md**: Communication protocols, UART/CAN/SPI/I2C interfaces, networking (`src/communication/**/*.{c,h}`)
- **core-software.instructions.md**: Data types, error handling, state management, motor control algorithms (`src/{common,controllers,drivers}/**/*.{c,h}`)
- **safety-rt.instructions.md**: Safety systems, real-time control, timing management, fail-safe mechanisms (`src/{safety,controllers}/**/*.{c,h}`)
- **api-ui.instructions.md**: API design, user interfaces, documentation generation, external integration (`src/{communication,application}/**/*.{c,h}`)

### Supporting Instructions
- **ssot-config.instructions.md**: Single Source of Truth configuration management principles
- **file-organization.instructions.md**: File structure and asset placement guidelines

### MCSDK Integration Documentation
- **docs/MCSDK_INTEGRATION_PLAN.md**: Comprehensive X-CUBE-SPN2 + MCSDK 6.4.1 hybrid integration strategy and CubeMX workflow

**Key Improvements**: Reduced from 39 granular files to 6 domain-based files (85% reduction) with robust brace-expansion `applyTo` patterns for reliable VS Code Copilot scoping. All original content consolidated and enhanced with better organization.

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

## Semantic Documentation Search System
**NEW**: Use intelligent semantic search with real AI embeddings for development with **unified STM32H7 + L6470 + Nucleo BSP coverage**:

```bash
# PRODUCTION SEMANTIC SEARCH (NEW - Preferred)

# Recommended: Use wrapper script (auto-handles virtual environment)
./scripts/stm32_search.sh concept "GPIO configuration" --scope STM32H7
./scripts/stm32_search.sh function "HAL_GPIO_Init" --scope STM32H7  
./scripts/stm32_search.sh function "L6470" --scope L6470
./scripts/stm32_search.sh peripheral "SPI" --scope all
./scripts/stm32_search.sh concept "stepper motor control" --scope all

# Alternative: Direct virtual environment usage
# Use virtual environment for proper dependencies
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "GPIO configuration" --scope STM32H7

# Semantic search for specific functions with context understanding
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py function "HAL_GPIO_Init" --scope STM32H7
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py function "L6470" --scope L6470

# Find peripheral configurations with semantic understanding
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py peripheral "SPI" --scope all
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py peripheral "UART" --scope STM32H7

# Search for register information with context
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py register "ABS_POS" --scope L6470

# Semantic concept discovery across documentation
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "stepper motor control" --scope all
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "board support package" --scope NUCLEO_BSP

# Rebuild semantic database after documentation updates
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "test" --rebuild

# LEGACY SEARCH (Archived - Available for reference)
# python3 scripts/legacy_archive/search_enhanced_docs.py [queries...]

# Validate workspace markdown links
python3 scripts/link_validator.py
```

**Semantic Search Database (NEW):**
- `docs/semantic_vector_db/` (981 documents): ChromaDB with Ollama mxbai-embed-large embeddings
  * `stm32_hal` collection (763 documents): STM32H7 HAL, Nucleo BSP, reference manuals
  * `motor_control` collection (218 documents): X-CUBE-SPN2 L6470 stepper driver documentation
  * `safety_systems`, `project_code`, `instruction_guides` collections (future expansion)

**Legacy Search Indexes (Archived):**
- `docs/indexes/STM32H7_FULL_INDEX.json` (8.9MB): Complete STM32H7 searchable index with 31,772 keywords
- `docs/indexes/L6470_SEARCH_INDEX.json` (2.1MB): Complete L6470 documentation index with 356 keywords
- `docs/indexes/STM32H7xx_Nucleo_BSP_INDEX.json` (1MB): Nucleo-144 BSP functions with 74 keywords
- `docs/indexes/STM32H7_COPILOT_INDEX.yaml` (8KB): Copilot-optimized quick reference  
- `docs/indexes/STM32H7_COPILOT_INDEX.json` (9.9KB): Programmatic access format

**Semantic Search Capabilities (NEW):**
- **AI-Powered Understanding**: Uses real Ollama embeddings for context-aware search
- **Intelligent Chunking**: Smart document segmentation with STM32-specific parsing
- **Multi-Collection Search**: Targeted search across STM32H7/L6470/BSP/project domains
- **Scope filtering**: Target specific documentation sets (STM32H7/L6470/NUCLEO_BSP/all)
- **Real embeddings**: 1024-dimensional vectors from Ollama mxbai-embed-large model
- **Production ready**: Processes 981 documents from real workspace (27 source files)
- **GPU Acceleration**: CUDA 12.1 support with automatic GPU detection

**Migration Notes:**
- **New system**: `scripts/stm32_semantic_search.py` (semantic search with AI embeddings)
- **Legacy system**: `scripts/legacy_archive/search_enhanced_docs.py` (archived keyword search)
- **Performance**: Semantic understanding vs exact keyword matching
- **Requirements**: ChromaDB + requests packages in virtual environment

**Search Pattern Best Practices:**
- **STM32H7**: Use concept searches (`concept "GPIO configuration"`) for broader results
- **L6470**: Use function patterns (`function "L6470"`) - driver functions work well semantically  
- **NUCLEO_BSP**: Use exact BSP function names (`function "BSP_LED_Init"`) for specific functions
- **Cross-platform**: Use concept searches (`concept "motor_control"`) for comprehensive results
- **Scope**: Case-insensitive (`--scope STM32H7`, `--scope nucleo_bsp`, `--scope L6470`)

**Common Search Patterns:**
```bash
# STM32H7 - Use concept and function searches (semantic understanding)
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "GPIO configuration" --scope STM32H7
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py function "HAL_GPIO_Init" --scope STM32H7

# L6470 - Use function and concept patterns (semantic context)
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py function "L6470" --scope L6470    # ✅ WORKS
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "stepper driver" --scope L6470

# Nucleo BSP - Use function names (good semantic matching)
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py function "BSP_LED_Init" --scope NUCLEO_BSP  # ✅ WORKS
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "board support" --scope NUCLEO_BSP

# Cross-platform concept searches (most effective for complex topics)
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "stepper configuration" --scope all
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "peripheral setup" --scope all
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
