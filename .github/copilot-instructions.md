<!-- Workspace-specific Copilot instructions for the STM32H753ZI stepper motor project -->

# Copilot instructions — stm32h753zi repository

Purpose: give AI coding agents the minimal, immediately useful context to be productive in this repo.

- Project type: Embedded C firmware for STM32H753ZI (Nucleo-144) controlling stepper motors via X-NUCLEO-IHM02A1 and L6470 drivers; hybrid X-CUBE-SPN2 + selective MCSDK integration.
- Primary paths and domains you should read first:

  - `src/` — main firmware and drivers (look for `src/spn2/`, `src/drivers/`, `src/hal_abstraction/`).
  - `src/config/` — Single Source of Truth (SSOT) headers: `hardware_config.h`, `motor_config.h`, `comm_config.h`, `safety_config.h`, `workflow_config.h`.
  - `scripts/` — developer tooling: `auto_update_status.py`, `validate_ssot.py`, `stm32_semantic_search.py`, and `run_python.ps1`.
  - `00_reference/` — large read-only vendor docs and examples (do not modify; copy snippets if needed and reference source).
  - `CMakeLists.txt` and `cmake/` — build configuration and host test integration.

- High-level architecture and intent (read before editing code):

  - Firmware SSOT (production): `src/config/*` controls hardware pins, motor limits, and safety thresholds. Use these header values; do not hardcode hardware constants in implementation files.
  - Workflow SSOT (dev tooling): `src/config/workflow_config.h` and `scripts/*` control validation depth and developer-mode behaviours; scripts will source or read these to alter verbosity/validation.
  - Hybrid integration: X-CUBE-SPN2 is the core stepper framework under `src/spn2/`; selective MCSDK enhancements live in `src/mcsdk_selective/` and `src/drivers/adaptation/`.

- Critical developer workflows (copyable examples):

  - Build (recommended): run the `Build (CMake)` VS Code task or `powershell -NoProfile -ExecutionPolicy Bypass -File scripts\fix_cmake.ps1` which prepares and builds the project.
  - Quick build (CI/local): `cmake --build build` after configuring via presets or `scripts\fix_cmake.ps1`.
  - Flashing (Windows): use the `Flash STM32H753ZI` task which calls STM32_Programmer_CLI.exe; example args are in `.vscode/tasks` (search for task named "Flash STM32H753ZI").
  - Run host tests: `cd build && ctest` (Run Tests (CTest) task).
  - Validate SSOT: `${workspaceFolder}\.venv\Scripts\python.exe scripts/validate_ssot.py` or use the `Validate SSOT` task.
  - Update STATUS.md (automated): run `scripts/auto_update_status.py` via the `Update STATUS.md` task; CI hooks call this after builds.

- Project-specific patterns and conventions:

  - SSOT-first: All hardware and workflow constants must be read from `src/config/*.h`. Look for `_Static_assert` checks used for compile-time validation.
  - No direct use of `00_reference/` in builds: copy snippets to `src/` with attribution instead of including vendor docs directly.
  - Hybrid driver adaptation: prefer using `src/drivers/adaptation/` when bridging SPN2 and MCSDK APIs; avoid changing SPN2 core files unless necessary.
  - Tests and host-tests: host test harnesses live under `host_tests/` and `build_host_tests/`. Unit tests use Unity/GoogleTest; run via CTest.

- Integration points and external dependencies to be aware of:

  - ARM GCC toolchain (arm-none-eabi-\*), OpenOCD, STM32CubeProgrammer CLI — expected in PATH for build/flash tasks.
  - Python dev tooling (.venv) used for scripts in `scripts/` (semantic search, SSOT validation, status updates). Use repository venv at `.venv`.
  - Semantic search DB under `vector_db/` and `docs/semantic_vector_db/` — scripts `stm32_semantic_search.py` provide programmatic access.

- Useful code examples to cite when making edits:

  - Example SSOT usage: `src/drivers/...` files include `config/hardware_config.h` and `config/motor_config.h` before setting pins or L6470 parameters.
  - Build hook: `scripts/fix_cmake.ps1` and `CMakeLists.txt` contain build orchestration and `update-status` targets.
  - Status automation: `scripts/auto_update_status.py` demonstrates how STATUS.md is generated and which metadata the project expects.

- When changing behavior, enforce validation:

  - Add or update `_Static_assert` in the SSOT headers where applicable.
  - Update `scripts/validate_ssot.py` and `scripts/auto_update_status.py` if new SSOT keys are introduced.

- Quick guidance for code-generation agents (do this first):
  1. Read `src/config/*` headers. Ensure any new constant is added there and validated.
  2. Search for usages (`grep` or semantic search) before renaming or changing macros.
  3. Run `scripts/validate_ssot.py` and `cmake --build build` locally to catch integration and compile-time errors.

If anything above is unclear or you'd like more detail on a specific area (build chain, SSOT fields, or semantic search usage), tell me what to expand and I'll iterate.

<!-- Use this file to provide workspace-specific custom instructions to Copilot. For more details, visit https://code.visualstudio.com/docs/copilot/copilot-customization#_use-a-githubcopilotinstructionsmd-file -->

# STM32H753ZI Stepper Motor Control Project - Copilot Instructions

This workspace develops firmware for an **STM32H753ZI Nucleo-144** board controlling stepper motors via **X-NUCLEO-IHM02A1** shield with closed-loop feedback using **AS5600 magnetic encoders**.

## 📋 Current Project Status

**IMPORTANT**: Always check `STATUS.md` in the root directory for the most current project state, including:

- Current development phase and active work items
- Recent completions and next priorities
- Build status and key metrics
- Technical context and architecture status
- Quick reference commands and file locations

**🤖 STATUS.md Automation System (Phase 3 Complete)**: The status file is now automatically updated via Git hooks after every commit, with real-time monitoring capabilities and enhanced VS Code integration. No manual maintenance required for basic status tracking.

**Automation Features:**

- ✅ **Git Hooks**: Post-commit STATUS.md updates with intelligent loop prevention
- ✅ **Real-time Monitoring**: Live build/git status with optimized process management (<1s response)
- ✅ **VS Code Tasks**: Enhanced workflow with manual triggers and preview capabilities
- ✅ **Smart Detection**: Merge commit handling, build context analysis, session continuity tracking
- ✅ **Performance Optimization**: Filesystem-first git checking, strict timeouts, graceful fallbacks

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

### Supporting Instructions (7 Files)

- **ssot-config.instructions.md**: Single Source of Truth configuration management principles
- **status-maintenance.instructions.md**: STATUS.md maintenance rules, format requirements, and context preservation (`STATUS.md`)
- **feature-management.instructions.md**: Feature creation, management, and tracking guidelines (`features/feature_registry.json,scripts/feature_tracker.py,docs/FEATURE_TRACKING_SYSTEM.md`)
- **development-workflow.instructions.md**: **CRITICAL** development workflow ensuring feature tracking and STATUS.md integration for Copilot context (`**/*.{c,h,md,json,py,sh,yml,yaml}`)
- **file-organization.instructions.md**: File structure and asset placement guidelines
- **cmake-build-system.instructions.md**: Build system configuration including ARM firmware and host testing compilation (`**/CMakeLists.txt,**/*.cmake,**/CMakePresets.json,.vscode/cmake-kits.json,**/fix_cmake.sh,**/run_host_tests.sh`)

### MCSDK Integration Documentation

- **docs/MCSDK_INTEGRATION_PLAN.md**: Comprehensive X-CUBE-SPN2 + MCSDK 6.4.1 hybrid integration strategy and CubeMX workflow

**Key Improvements**: Reduced from 19 instruction files to 13 production-focused files (32% reduction) with archived legacy content. Eliminated redundant RTOS instructions (FreeRTOS complete), merged build system guidance, and added critical development workflow instruction for maintainable, production-ready development guidance with systematic feature tracking integration.

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

# Validate workspace markdown links
python3 scripts/link_validator.py

## Reference Assets

The `00_reference/` directory contains ST official assets (READ-ONLY):

- STM32H7 HAL drivers and examples
- X-NUCLEO-IHM02A1 board support package
- L6470 driver libraries and documentation
- Application examples and datasheets

**IMPORTANT**: Never modify `00_reference/` files. Copy needed code to appropriate `src/` locations.

## Environment Context

This workspace runs with:

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
```
