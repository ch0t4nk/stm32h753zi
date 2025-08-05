---
applyTo: "**"
description: "File organization and asset placement guidelines for STM32H753ZI stepper motor project"
---

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
