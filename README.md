# STM32H753ZI Stepper Motor Control Project

![Version](https://img.shields.io/badge/version-v0.3.0-blue)
![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![Automation](https://img.shields.io/badge/automation-phase%203%20complete-success)
![Development](https://img.shields.io/badge/phase-driver%20implementation-blue)
![Code Quality](https://img.shields.io/badge/code%20quality-lint%20free-success)

> ## ✅ Current Project Status
**IMPORTANT**: Always check `STATUS.md` in the root directory for the most current project state, including:
- Current development phase and active work items
- Recent completions and next priorities  
- Build status and key metrics
- Technical context and architecture status
- Quick reference commands and file locations

**Latest Major Achievement: HAL Abstraction Layer Complete** ✅  
Professional embedded architecture with complete hardware abstraction layer:
- **Platform-Independent Interface**: `src/hal_abstraction/hal_abstraction.h` (432 lines)
- **STM32H7 Implementation**: Production-ready hardware abstraction for STM32H753ZI
- **Mock Testing Framework**: Unity-integrated testing with 6/6 tests passing
- **Host-Based Testing**: Hardware-independent test execution
- **Dual Build Targets**: Same code compiles for ARM firmware and host testing
- **Next Phase**: Source code migration to HAL abstraction (2-3 days estimated)

## 🎉 Latest Achievement: HAL Abstraction Architecture Complete!

**Major Milestone Achieved:** Complete hardware abstraction layer enabling professional embedded development with platform-independent testing and clean architecture separation.

### ✅ What's Working
- **🏗️ HAL Abstraction Layer Complete**: Platform-independent interface, STM32H7 implementation, and comprehensive mock framework
- **🧪 Hardware-Independent Testing**: 6/6 HAL abstraction tests passing with Unity framework
- **🔄 Dual Build System**: ARM firmware compilation + host-based testing with same source code
- **📋 Clean Architecture**: Professional embedded patterns with proper error handling
- **🤖 STATUS.md Automation Phase 3**: Git hooks + optimized monitoring system with intelligent loop prevention
- **Complete peripheral initialization** for SPI2, I2C1, I2C2, TIM2, FDCAN1
- **CMake build system** with ARM GCC toolchain and automated status integration
- **Pin conflict resolution** and CubeMX integration
- **Development environment** optimized with Python tooling and semantic search
- **SSOT validation** and spell checker configuration

### 🚀 Next Phase: Complete HAL Migration
Ready to migrate remaining source files to HAL abstraction and resolve legacy test conflicts for full architecture consistency.

## Overview
Advanced stepper motor control firmware for **STM32H753ZI Nucleo-144** with **X-NUCLEO-IHM02A1** shield, featuring:
- **Dual L6470 stepper drivers** with daisy-chain SPI communication ✅ *Peripheral Ready*
- **AS5600 magnetic encoders** for closed-loop position feedback ✅ *Peripheral Ready*  
- **Multi-protocol communication**: UART, CAN-FD, Ethernet REST API ✅ *Peripheral Ready*
- **Comprehensive safety systems** with watchdog and fault recovery 🔄 *Next Phase*
- **SSOT architecture** with centralized configuration management ✅ *Complete*
- **Enhanced documentation system** with 86MB STM32H7 reference materials ✅ *Complete*
- **🤖 Automated Development Workflow** with Git hooks and real-time monitoring ✅ *Complete*

## 🤖 Enhanced Development Automation (NEW!)
This project features a **comprehensive automation system**:
- **Git Hooks**: Automatic STATUS.md updates after every commit with intelligent loop prevention
- **Real-time Monitoring**: Live build/git status tracking with optimized process management
- **VS Code Integration**: Enhanced tasks for build+update, preview, and manual triggers
- **Smart Process Management**: Filesystem-first git checking, strict timeouts, graceful fallbacks
- **Performance**: Reduced git response times from 5+ seconds to <1 second
This project features a **comprehensive documentation integration system**:
- **Semantic Search Database**: ChromaDB with Ollama mxbai-embed-large embeddings (981 documents)
- **AI-Powered Understanding**: Real embeddings for context-aware search across STM32H7/L6470/BSP domains
- **Consolidated Copilot instructions** with domain-based architecture (6 core files, 85% reduction)
- **Legacy keyword search** archived in `scripts/legacy_archive/` for reference

### Documentation Search Tools
```bash
# PRODUCTION SEMANTIC SEARCH (NEW - Preferred)
# Use wrapper script (auto-handles virtual environment)
./scripts/stm32_search.sh concept "GPIO configuration" --scope STM32H7
./scripts/stm32_search.sh function "HAL_GPIO_Init" --scope STM32H7  
./scripts/stm32_search.sh function "L6470" --scope L6470
./scripts/stm32_search.sh peripheral "SPI" --scope all

# Alternative: Direct virtual environment usage
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "GPIO configuration" --scope STM32H7
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py function "HAL_SPI_Init" --scope STM32H7
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py register "ABS_POS" --scope L6470

# Semantic concept discovery across documentation  
/workspaces/code/.venv/bin/python scripts/stm32_semantic_search.py concept "stepper motor control" --scope all
```

### 🤖 Automated Development Workflow
The project includes comprehensive automation for enhanced productivity:

```bash
# STATUS.md automatically updates after every commit via Git hooks
git commit -m "feat: add new driver"  # ← Triggers automatic STATUS.md update

# Manual status monitoring and updates
/workspaces/code/.venv/bin/python3 scripts/status_monitor.py --status-bar  # VS Code format
/workspaces/code/.venv/bin/python3 scripts/status_monitor.py --json        # JSON format
/workspaces/code/.venv/bin/python3 scripts/status_monitor.py --watch       # Real-time monitoring

# VS Code tasks for enhanced workflow
cmake --build build --target update-status         # Manual STATUS.md update
cmake --build build --target update-status-preview # Preview changes
./scripts/demo_status_integration.sh               # Complete workflow demo

# Validation and testing
python3 scripts/validate_ssot.py                   # SSOT compliance check
python3 scripts/link_validator.py                  # Documentation links
```

**Automation Features:**
- ✅ **Git Hooks**: Post-commit STATUS.md updates with intelligent loop prevention
- ✅ **Real-time Monitoring**: Live build/git status with optimized process management  
- ✅ **VS Code Integration**: Enhanced tasks for manual triggers and previews
- ✅ **Smart Detection**: Merge commit handling, build context analysis
- ✅ **Performance**: <1s git response times, filesystem-first checking

## Quickstart

### Ready to Build & Flash! 🚀
The project is fully configured and ready for development:

```bash
# 1. Build the project (all peripherals initialized)
cmake -S . -B build && cmake --build build

# 2. Flash to STM32H753ZI Nucleo board  
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg \
        -c "program build/stm32h753_ihm02a1.elf verify reset exit"

# 3. Monitor via UART (115200 baud on ST-Link Virtual COM Port)
# 4. Validate SSOT configuration
python3 scripts/validate_ssot.py
```

### Current Peripheral Status
All peripheral handles are initialized and available:
```c
extern SPI_HandleTypeDef hspi2;      // ✅ L6470 stepper drivers (PB13/14/15, PA9)
extern I2C_HandleTypeDef hi2c1;      // ✅ AS5600 encoder 1 (PB6/7)  
extern I2C_HandleTypeDef hi2c2;      // ✅ AS5600 encoder 2 (PB10/11)
extern TIM_HandleTypeDef htim2;      // ✅ 1kHz control loop timer
extern FDCAN_HandleTypeDef hfdcan1;  // ✅ MCU communication (PD0/1)
```

### Next: Implement Drivers
```bash
# Ready for driver development:
# 1. Create src/drivers/l6470_driver.c (SPI communication)
# 2. Create src/drivers/as5600_driver.c (I2C communication)  
# 3. Implement control loop in TIM2 interrupt handler
```

## Architecture
- **MCU**: STM32H753ZI (ARM Cortex-M7 @ 480MHz, 2MB Flash, 1MB RAM)
- **Motor Drivers**: 2× L6470 dSPIN (up to 7A, 1/128 microstepping)
- **Position Feedback**: 2× AS5600 magnetic encoders (12-bit resolution)
- **Communication**: UART (debug), CAN-FD, Ethernet (LAN8742 PHY)
- **Safety**: Independent watchdog, L6470 fault monitoring, thermal protection

## API References
- REST: `api/openapi.yaml`
- CAN: `api/can_api.yaml`

## Development Environment
- **Dev Container**: Ubuntu 22.04.5 LTS with complete embedded toolchain
- **Toolchain**: ARM GCC, OpenOCD, CMake, Doxygen
- **Documentation**: 91.1MB comprehensive reference (STM32H7 + L6470 + Nucleo BSP)  
- **Testing**: Unity, GoogleTest frameworks
- **Validation**: SSOT consistency checking, pre-commit hooks

## Build System & Tasks
Available VS Code tasks:
- **Build (CMake)**: `cmake -S . -B build && cmake --build build`
- **Flash STM32H753ZI**: Program firmware via OpenOCD + ST-Link
- **Run Tests (CTest)**: Execute unit and integration tests
- **Generate Docs (Doxygen)**: Create API documentation
- **Validate SSOT**: Check configuration consistency

### Example Build Commands
```bash
# Build project
cmake -S . -B build && cmake --build build

# Run tests
cd build && ctest

# Flash to hardware
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg 
        -c "program build/stm32h753_ihm02a1.elf verify reset exit"
```

## Project Structure
```
src/
├── config/              # SSOT configuration headers
├── common/              # SSOT common definitions  
├── drivers/             # Hardware abstraction layer
├── controllers/         # Control algorithms and logic
├── communication/       # Protocol implementations
├── safety/              # Safety monitoring and response
└── application/         # Main application logic

.github/instructions/    # Modular Copilot instruction system
00_reference/           # STM32H7 + L6470 + Nucleo BSP documentation (READ-ONLY)
scripts/                # Consolidated development tools
docs/                   # Project documentation and summaries
tests/                  # Unit and integration tests
```

For complete file organization guidelines, see `.github/instructions/file-organization.instructions.md`.

## Version System
- **Semantic Versioning**: MAJOR.MINOR.PATCH-PRERELEASE+BUILD
- **Auto-generation**: Git commit SHA and build metadata
- **Build Integration**: Version embedded in firmware binary

## Documentation System
This project features an **enhanced documentation integration system**:
- **Semantic Search Database**: `docs/semantic_vector_db/` (981 documents) with ChromaDB + Ollama embeddings
  * `stm32_hal` collection (763 documents): STM32H7 HAL, Nucleo BSP, reference manuals  
  * `motor_control` collection (218 documents): X-CUBE-SPN2 L6470 stepper driver documentation
- **Legacy Search Indexes**: Archived in `docs/indexes/` (32,200+ keywords for reference)
- **Consolidated Instructions**: Domain-based instruction system (6 core files, 85% reduction)
- **Production Search**: AI-powered semantic understanding vs exact keyword matching

See `.github/copilot-instructions.md` for complete semantic search system overview.

---

**Safety First, SSOT Always, Modular Design Throughout.**
