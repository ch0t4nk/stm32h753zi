````instructions
---
applyTo: "**"
description: "File organization and asset placement guidelines for STM32H753ZI stepper motor project with complete mature architecture"
---

# File Organization Instructions

## Overview
This instruction file provides comprehensive guidance for where to place different types of files and assets in the STM32H753ZI stepper motor control project. Our project has evolved to a **mature, production-ready architecture** with comprehensive documentation, automation, and professional development infrastructure.

## Project Maturity Status
**Current Status**: 🎉 **PROJECT COMPLETE** - Professional architecture implemented  
**Phase**: Enhanced development with FreeRTOS integration roadmap

## Complete Directory Structure

### Root Directory - Professional Project Layout
**Rule**: Root contains only essential configuration and entry points. Complex assets organized in dedicated directories.

**✅ Production Root Structure:**
```
├── CMakeLists.txt              # Main build configuration
├── CMakePresets.json           # CMake build presets
├── README.md                   # Project overview and quick start  
├── STATUS.md                   # Auto-updated project status (✨ AI-friendly)
├── LICENSE                     # Project license
├── .gitignore                  # Git ignore rules
├── pyproject.toml              # Python project configuration
├── code.ioc                    # STM32CubeMX project file
├── STM32H753XX_FLASH.ld        # Linker script for STM32H753ZI
├── startup_stm32h753xx.s       # Startup assembly code
├── .env.template               # Environment template (no secrets)
└── .status_context.json        # Status automation context (auto-generated)
```

### Source Code Architecture - Professional Multi-Layer
```
src/
├── config/                     # 🏆 SSOT Configuration (Single Source of Truth)
│   ├── hardware_config.h       # Pin assignments, clock settings
│   ├── motor_config.h          # Motor control parameters
│   ├── safety_config.h         # Safety thresholds and limits
│   ├── comm_config.h           # Communication settings
│   ├── system_config.h         # System-wide settings
│   ├── build_config.h          # Build metadata and versioning
│   ├── documentation_config.h  # Documentation paths (SSOT)
│   ├── l6470_registers_generated.{c,h}    # Auto-generated register definitions
│   └── as5600_registers_generated.{c,h}   # Auto-generated register definitions
├── common/                     # Shared definitions and utilities
│   ├── error_codes.h           # System error definitions (SSOT)
│   ├── data_types.h            # Common data structures
│   ├── system_state.h          # System state management
│   └── syscalls.c              # System call implementations
├── hal_abstraction/            # 🎯 Hardware Abstraction Layer (Testing-Ready)
│   ├── hal_abstraction.h       # Platform-independent interface
│   ├── hal_abstraction_stm32h7.c  # STM32H7 implementation
│   └── hal_abstraction_mock.c  # Mock implementation for testing
├── drivers/                    # Hardware drivers with professional structure
│   ├── adaptation/             # Driver adaptation layer
│   ├── l6470/                  # L6470 stepper driver
│   ├── as5600/                 # AS5600 encoder driver
│   ├── bsp/                    # Board Support Package
│   └── hal/                    # STM32 HAL drivers
├── controllers/                # Control algorithms and PID
├── communication/              # Protocol implementations (UART, CAN, SPI, I2C)
├── safety/                     # Safety systems and monitoring
├── application/                # Main application logic
├── demo/                       # Demonstration and example code
└── simulation/                 # Hardware-free simulation framework
```

### Reference Documentation System - Comprehensive Knowledge Base
```
00_reference/                   # 📚 Complete Reference Library
├── AN4838-introduction-to-Memory-Protection-unit-management-on-stm32-mcus-stmicroelectronics.md
├── AN4908-getting-started-with-usart-Automatic-Baud-Rater-Detection-for-stm32-mcus-stmicroelectronics.md
├── AN5342--how-to-use-Error-Correction-Code-ECC-management-for-internal-memories-protection-on-stm32-mcus-stmicroelectronics.md
├── AN5348-introduction-to-FDCAN-Peripherals-for-stm32-mcus-stmicroelectronics.md
├── ES0392-stm32h742xig-stm32h743xig-stm32h750xb-stm32h753xi-device-ERRATA-stmicroelectronics.md
├── OM0253-stm32f7-series-and-stm32h7-series-cortexm7-processor-Programming-Manual-stmicroelectronics.md
├── RM0433-stm32h742-stm32h743753-and-stm32h750-value-line-advanced-armbased-32bit-mcus-stmicroelectronics.md
├── STM32H72x_73x_74x_75x_system_architecture_overview.md
├── UM2285-development-guidelines-for-stm32cube-expansion-packages-stmicroelectronics.md
├── CMSIS_docs/                 # CMSIS documentation
├── ST_Assets/                  # STMicroelectronics assets
├── STM32Cube_FW_H7_V1.12.0/   # STM32 firmware package documentation
├── stm32h753xx_User_Manual_md/ # STM32H753ZI user manual
├── stm32h7xx_nucleo_bsp_md/    # Nucleo BSP documentation  
├── STM32H7xx_Nucleo_BSP_User_Manual/ # Nucleo user manual
└── x_cube_spn2_markdown_docs/  # X-CUBE-SPN2 documentation
```

### API Specifications and Interface Definitions
```
api/                           # 📋 API Specifications
├── openapi.yaml               # OpenAPI/Swagger REST API specification
└── can_api.yaml               # CAN protocol API specification
```

### Professional Documentation System
```
docs/                          # 📖 Project Documentation Hub
├── API_AUDIT_REPORT.json      # API compliance audit results
├── changelog.md               # Version history and changes
├── CODEBASE_ANALYSIS_REPORT.md # Code quality analysis
├── Doxyfile                   # Doxygen configuration
├── GPU_ACCELERATION_SETUP.md  # Advanced development setup
├── meeting_notes.md           # Development meeting records
├── PROJECT_COMPLETION_REPORT.md # Project milestone documentation
├── README.md                  # Documentation navigation guide
├── SEMANTIC_SEARCH_ENGINE_DESIGN.md # Search system architecture
├── SSOT_COMPLIANCE_CORRECTION.md # SSOT implementation details
├── STATUS_AUTOMATION_DESIGN.md # Status automation architecture
├── STATUS_AUTOMATION_README.md # Status system user guide
└── [auto-generated]/          # Generated documentation (Doxygen, etc.)
```

### Professional Development Infrastructure
```
scripts/                       # 🔧 Automation and Development Tools
├── auto_update_status.py      # Automated STATUS.md maintenance
├── status_monitor.py          # Real-time status monitoring
├── validate_ssot.py           # SSOT configuration validation
├── validate_build_env.sh      # Build environment validation
├── fix_cmake.sh               # CMake build automation
├── demo_status_integration.sh # Status automation demonstration
├── semantic_search.py         # Documentation search engine
├── link_validator.py          # Documentation link validation
├── install/                   # Development environment setup
│   ├── install_ceedling.sh
│   ├── install_googletest.sh
│   └── install_unity.sh
└── validation/                # Quality assurance scripts
```

### Archive and Historical Development
```
archive/                       # 🗃️ Development History Archive
├── README.md                  # Archive navigation
├── development_artifacts/     # Historical development files
├── development_scripts/       # Legacy development scripts
├── outdated_docs/            # Superseded documentation
├── phase_reports/            # Development phase reports
└── validation_programs/      # Legacy validation programs
```

### Advanced Development Systems
```
rtos/                         # 🚀 FreeRTOS Integration (Phase 5A)
├── README.md                 # FreeRTOS implementation roadmap
└── [implementation files]    # FreeRTOS task and scheduler code

bootloader/                   # 🔄 Custom Bootloader System
├── README.md                 # Bootloader documentation
└── [bootloader source]      # Bootloader implementation files

middleware/                   # 🔌 Middleware Components
└── [middleware files]        # Custom middleware implementations

external/                     # 📦 External Dependencies
└── [third-party code]        # External libraries and dependencies

examples/                     # 💡 Usage Examples and Demos
└── [example projects]        # Demonstration code and tutorials

schemas/                      # 📊 Data Schema Definitions
└── [schema files]           # JSON schemas and data validation
```

### Testing Infrastructure
```
tests/                        # 🧪 Comprehensive Testing Framework
├── unit/                     # Unit tests with mocks
├── integration/              # Integration testing
├── fixtures/                 # Test data and fixtures
└── mocks/                    # Mock object implementations
```

### Build and Development Artifacts
```
build/                        # ARM firmware build (git-ignored)
build_host_tests/             # Host-based testing build (git-ignored)  
logs/                         # 📊 Automation and build logs
├── status_automation.log     # Status update automation log
└── [other logs]              # Build and automation logs

.venv/                        # Python virtual environment (git-ignored)
.vscode/                      # VS Code workspace configuration
.devcontainer/                # Development container configuration
```

### CMake Build System Configuration
```
cmake/                        # ⚙️ Advanced CMake Configuration
├── arm-none-eabi-gcc.cmake   # ARM GCC toolchain
├── gcc-arm-none-eabi.cmake   # Alternative toolchain config
├── starm-clang.cmake         # Clang toolchain support
└── stm32cubemx/              # STM32CubeMX integration
```

## File Placement Rules by Category

### ✅ **Configuration Files → `src/config/`**
- All SSOT configuration headers
- Hardware pin assignments  
- Communication parameters
- Safety thresholds
- Auto-generated register definitions

### ✅ **Implementation Code → `src/[domain]/`**
- Application logic → `src/application/`
- Driver code → `src/drivers/[driver_name]/`
- Communication protocols → `src/communication/`
- Control algorithms → `src/controllers/`
- Safety systems → `src/safety/`

### ✅ **Documentation → `docs/` or `00_reference/`**
- Project documentation → `docs/`
- Reference manuals → `00_reference/`
- API specifications → `api/`
- Status and reports → `docs/` (STATUS.md stays in root for AI visibility)

### ✅ **Scripts and Tools → `scripts/`**
- Python automation scripts
- Shell build scripts
- Validation tools
- Installation scripts → `scripts/install/`

### ✅ **Development Infrastructure → Dedicated Directories**
- CMake configuration → `cmake/`
- VS Code settings → `.vscode/`
- Container setup → `.devcontainer/`
- Testing framework → `tests/`

### ❌ **PROHIBITED Locations**
- **Root directory**: No summary files, scripts, or documentation (except STATUS.md for AI)
- **src/ mixing**: No mixing test code with production code
- **docs/ scripts**: No executable scripts in documentation directories
- **Random placement**: No files outside their designated category directories

## Automated Maintenance
- **STATUS.md**: Auto-updated by git hooks and automation scripts
- **Build artifacts**: Auto-cleaned by CMake targets
- **Documentation**: Auto-generated by Doxygen and custom scripts
- **Logs**: Centralized in `logs/` directory with rotation

**Key Principle**: Every file has a clear, designated location based on its purpose and lifecycle. This enables efficient development, automated tooling, and professional project management.

````

# File Organization Instructions

## Overview
This instruction file provides comprehensive guidance for where to place different types of files and assets in the STM32H753ZI stepper motor control project. Proper file organization ensures maintainability, consistency, and follows the SSOT principle.

## Directory Structure and File Placement Rules

### Root Directory - MINIMAL CONTENT ONLY
**Rule**: Keep root directory minimal and organized. Only essential project files belong here.

**✅ ALLOWED in root:**
```
├── CMakeLists.txt          # Main build configuration
├── README.md               # Project overview and quick start
├── LICENSE                 # Project license
├── .gitignore             # Git ignore rules
├── .env.template          # Environment template (no secrets)
└── build/                 # Build artifacts directory (git-ignored)
```

**❌ PROHIBITED in root:**
- Documentation files (belongs in `docs/`)
- Shell scripts (belongs in `scripts/`)
- Summary/status files (belongs in `docs/`)
- Configuration summaries (belongs in `docs/`)
- Installation scripts (belongs in `scripts/`)
- Build artifacts as individual files (use `build/` directory)

### Source Code Organization
```
src/
├── config/                 # SSOT configuration headers ONLY
│   ├── hardware_config.h   # Hardware pin assignments
│   ├── comm_config.h       # Communication parameters  
│   ├── motor_config.h      # Motor control settings
│   ├── safety_config.h     # Safety thresholds
│   ├── build_config.h      # Build/version metadata
│   └── documentation_config.h # Documentation paths (SSOT)
├── common/                 # Common definitions and utilities
├── drivers/                # Hardware abstraction layer
├── controllers/            # Control algorithms
├── communication/          # Protocol implementations
├── safety/                 # Safety systems
└── application/            # Main application logic
```

### Documentation Organization
```
docs/
├── design/                 # Design documents, architecture
├── requirements/           # Requirements specifications
├── indexes/                # Search indexes for documentation
├── *.md                   # Project summaries, status reports
├── Doxyfile               # Doxygen configuration
└── [generated]/           # Generated documentation (git-ignored)
```

**✅ Documentation files belong in `docs/`:**
- `ENHANCED_DOCUMENTATION_SUMMARY.md`
- `INSTRUCTION_UPDATE_SUMMARY.md`
- `WORKSPACE_STATUS.md`
- `SCRIPT_CONSOLIDATION_SUMMARY.md`
- Any `*_SUMMARY.md` or `*_STATUS.md` files

### Scripts and Tools Organization
```
scripts/
├── *.py                   # Python automation scripts
├── *.sh                   # Shell scripts for build/install
├── install/               # Installation scripts subdirectory
│   ├── install_ceedling.sh
│   ├── install_googletest.sh
│   └── install_unity.sh
└── validation/            # Validation and testing scripts
```

**✅ All scripts belong in `scripts/`:**
- Installation scripts (`install_*.sh`)
- Build automation scripts
- Validation scripts (`validate_*.py`)
- Documentation tools (`search_*.py`)
- Container setup scripts (`fix_container.sh`)

### Test Organization
```
tests/
├── unit/                  # Unit tests
├── integration/           # Integration tests
├── fixtures/              # Test fixtures and data
└── mocks/                 # Mock objects
```

### Build Artifacts Organization
```
build/                     # Git-ignored build directory
├── CMakeCache.txt         # CMake cache
├── CMakeFiles/            # CMake generated files
├── Makefile              # Generated makefiles
├── *.ninja               # Ninja build files
├── src/                  # Compiled objects
└── stm32h753_ihm02a1.elf # Final binary
```

**✅ Build artifacts belong in `build/`:**
- `CMakeCache.txt`
- `CMakeFiles/`
- `build.ninja`
- `cmake_install.cmake`
- Any compiled binaries or objects

### Reference Materials Organization
```
00_reference/             # Read-only reference materials
├── STM32Cube_FW_H7_V1.12.0/  # ST HAL libraries
├── stm32h753xx_User_Manual_md/ # STM32H7 documentation
├── x_cube_spn2_markdown_docs/  # L6470 documentation
└── ST_Assets/            # ST software installers
```

## File Naming Conventions

### Documentation Files
- Project summaries: `PROJECT_SUMMARY.md`
- Status reports: `WORKSPACE_STATUS.md`
- Technical analysis: `TECHNICAL_ANALYSIS.md`
- Meeting notes: `meeting_notes.md` (lowercase for recurring files)

### Script Files
- Installation: `install_<component>.sh`
- Validation: `validate_<target>.py`
- Build tools: `build_<target>.py`
- Search tools: `search_<domain>.py`

### Configuration Files
- Hardware: `hardware_config.h`
- Communication: `comm_config.h`
- SSOT headers: `<domain>_config.h`

## Anti-Patterns to Avoid

### ❌ Root Directory Pollution
```bash
# WRONG - clutters root directory
/workspaces/code/SUMMARY.md
/workspaces/code/install_something.sh
/workspaces/code/validation_report.md
/workspaces/code/build_artifact.o
```

```bash
# CORRECT - organized placement
/workspaces/code/docs/SUMMARY.md
/workspaces/code/scripts/install_something.sh
/workspaces/code/docs/validation_report.md
/workspaces/code/build/build_artifact.o
```

### ❌ Mixed File Types in Single Directory
```bash
# WRONG - mixed content
src/
├── motor_driver.c
├── BUILD_NOTES.md         # Documentation in source
├── install_deps.sh        # Script in source
└── test_results.txt       # Test output in source
```

```bash
# CORRECT - separated by type
src/motor_driver.c         # Source code only
docs/BUILD_NOTES.md        # Documentation in docs
scripts/install_deps.sh    # Scripts in scripts
tests/test_results.txt     # Test outputs in tests
```

### ❌ Hardcoded Paths in Scripts
```bash
# WRONG - hardcoded paths
cp summary.md /workspaces/code/SUMMARY.md
```

```bash
# CORRECT - use SSOT configuration
cp summary.md "${WORKSPACE_ROOT}/docs/SUMMARY.md"
```

## File Movement Guidelines

### Moving Misplaced Files
1. **Identify misplaced files**: Check root for non-essential files
2. **Determine correct location**: Use rules above
3. **Update references**: Search for hardcoded paths
4. **Validate after move**: Ensure scripts still work

### Example Corrections
```bash
# Documentation files to docs/
mv ENHANCED_DOCUMENTATION_SUMMARY.md docs/
mv INSTRUCTION_UPDATE_SUMMARY.md docs/
mv WORKSPACE_STATUS.md docs/

# Scripts to scripts/
mv fix_container.sh scripts/
mv install_*.sh scripts/install/

# Build artifacts to build/
mv CMakeCache.txt build/ 2>/dev/null || true
mv build.ninja build/ 2>/dev/null || true
```

## Validation Rules

### Pre-commit Checks
1. Root directory contains only allowed files
2. All scripts are in `scripts/` directory
3. All documentation is in `docs/` directory
4. Build artifacts are in `build/` directory (git-ignored)
5. No hardcoded file paths in scripts

### SSOT Compliance
- Use `documentation_config.h` for all documentation paths
- Reference script locations through environment variables
- Build paths should be configurable

## Integration with Existing Instructions

### Related Instruction Files
- **project-setup.instructions.md**: Workspace setup, build system, testing infrastructure, optimization
- **hardware.instructions.md**: Hardware configuration, GPIO control, memory management, STM32H7 HAL
- **comms.instructions.md**: Communication protocols, UART/CAN/SPI/I2C interfaces, networking
- **core-software.instructions.md**: Data types, error handling, state management, motor control algorithms
- **safety-rt.instructions.md**: Safety systems, real-time control, timing management, fail-safe mechanisms
- **api-ui.instructions.md**: API design, user interfaces, documentation generation, external integration

### Cross-References
- All file paths must follow SSOT configuration
- Scripts must use centralized path definitions
- Documentation structure supports enhanced search system

Remember: **Organized files enable maintainable code, clear separation supports SSOT principles.**
