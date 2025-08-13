# STM32H753ZI Stepper Motor Control Project - GitHub Copilot Instructions

**ALWAYS FOLLOW THESE INSTRUCTIONS FIRST**. Only fallback to additional search and context gathering if the information here is incomplete or found to be in error.

## 🎯 Project Overview
This is a **safety-critical stepper motor control system** for STM32H753ZI Nucleo-144 board with X-NUCLEO-IHM02A1 expansion shield. The system controls dual L6470 stepper drivers with AS5600 magnetic encoder feedback using FreeRTOS.

## 📋 **CRITICAL**: Build System Requirements

### Essential Dependencies
Install ALL required tools before attempting to build:
```bash
# Install ARM GCC toolchain (REQUIRED - cannot build without this)
sudo apt-get update && sudo apt-get install -y gcc-arm-none-eabi gdb-arm-none-eabi

# Install build tools
sudo apt-get install -y cmake ninja-build build-essential

# Install Python virtual environment support
sudo apt-get install -y python3-venv
```

### Verify Installation
**ALWAYS verify** tools are correctly installed:
```bash
# These commands MUST succeed before building
arm-none-eabi-gcc --version  # Should show version 13.2.1 or later
cmake --version              # Should show CMake 3.22 or later
ninja --version              # Should show Ninja 1.10 or later
```

## 🚀 **VALIDATED** Build Commands

### **CRITICAL**: Python Virtual Environment Setup
**ALWAYS activate virtual environment first** - the build system requires it:
```bash
# Create virtual environment (one-time setup)
python3 -m venv .venv

# MANDATORY: Activate virtual environment (every session)
source .venv/bin/activate

# Install required Python packages
pip install chromadb requests
```

### Primary Build Process
**Validated timing expectations - NEVER CANCEL these commands:**

```bash
# 1. Configuration (takes ~1.5 seconds)
# NEVER CANCEL: CMake configuration is fast but critical
source .venv/bin/activate  # REQUIRED
cmake --preset Debug
# Expected output: "Build files have been written to: .../build/Debug"

# 2. Build firmware (takes 15-25 minutes)
# NEVER CANCEL: Build takes 15-25 minutes to complete. Set timeout to 40+ minutes.
cmake --build build/Debug
# Expected: 174 compilation units, may fail on missing register definitions (known issue)
```

### Quick Fix Script (Recommended)
**Use this automated script for fastest setup:**
```bash
# NEVER CANCEL: Script handles environment setup + build (2-3 minutes)
./scripts/fix_cmake.sh
# Handles: virtual env activation, dependency checks, CMake config, build attempt
```

### Alternative Manual Commands
If automated script fails:
```bash
# Clean start (removes cached configuration)
rm -rf build/

# Manual configuration with toolchain
source .venv/bin/activate
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build
```

## 🧪 Testing Framework

### Validation Commands
Run these to verify system state:
```bash
# Environment validation (takes ~0.2 seconds)
./scripts/validate_build_env.sh
# Should show: "✅ Build environment is correctly configured"

# SSOT configuration check (takes ~0.15 seconds)  
source .venv/bin/activate
python3 scripts/validate_ssot.py
# Should show: "🎉 SSOT validation passed!"
```

### Host-Based Testing
**IMPORTANT**: Host tests require path adjustments for non-container environments:
```bash
# Current host test script expects /workspaces path - may fail
./scripts/run_host_tests.sh
# Known issue: "cannot create directory '/workspaces': Permission denied"

# Manual host testing (alternative)
source .venv/bin/activate
cd tests && find unit -name "*.c" -exec echo "Found test: {}" \;
```

## ⚠️ **CRITICAL**: Known Build Issues

### Schema Generation Failure
**Expected Issue**: Build may fail with:
```
ninja: error: 'scripts/schema_generator.py', needed by 'src/config/as5600_registers_generated.h', missing
```

**Workaround**: Create stub files:
```bash
mkdir -p src/config
echo "// L6470 register stub file" > src/config/l6470_registers_generated.h
echo "// AS5600 register stub file" > src/config/as5600_registers_generated.h
```

### Missing Ollama Service
**Expected Issue**: Semantic search commands fail:
```
❌ Connection to search service failed
```
**Impact**: Documentation search unavailable, but build/development continues normally.

## 🔧 VS Code Integration

### Essential VS Code Tasks
Use Ctrl+Shift+P → "Tasks: Run Task":
- **"Build (CMake)"**: Primary build command (calls ./scripts/fix_cmake.sh)
- **"Run Tests (CTest)"**: Execute unit tests (may fail due to path issues)
- **"Validate SSOT"**: Configuration consistency check
- **"Flash STM32H753ZI"**: Program firmware to hardware

### CMake Kit Selection
If VS Code CMake extension is available:
1. Ctrl+Shift+P → "CMake: Select a Kit" → "ARM GCC for STM32H753ZI"
2. Ctrl+Shift+P → "CMake: Select Configure Preset" → "Debug"

## 📁 Project Navigation

### Key Source Code Locations
```
src/
├── config/              # SSOT configuration headers (hardware pins, parameters)
├── common/              # Shared definitions and error codes
├── hal_abstraction/     # Platform-independent hardware layer
├── drivers/             # L6470 stepper, AS5600 encoder drivers  
├── controllers/         # Motor control algorithms and PID
├── safety/              # Emergency stop, watchdog, fault monitoring
├── communication/       # UART, SPI, I2C, CAN protocols
├── application/         # Main application logic
└── telemetry/          # Performance monitoring and optimization
```

### Important Files for Development
- **CMakeLists.txt**: Main build configuration
- **cmake/gcc-arm-none-eabi.cmake**: ARM GCC toolchain setup
- **STATUS.md**: Auto-updated project status and context
- **scripts/fix_cmake.sh**: Automated build environment setup
- **scripts/validate_build_env.sh**: Dependency verification

### Documentation System
```
.github/instructions/     # Domain-specific Copilot guidance
00_reference/            # STM32H7 + L6470 + Nucleo BSP docs (READ-ONLY)
docs/                    # Project documentation and guides
scripts/                 # Build automation and validation tools
```

## 🛡️ Safety-Critical Development

### **MANDATORY** Safety Checks
**ALWAYS validate safety systems before hardware deployment:**
```bash
# Run safety validation
source .venv/bin/activate
python3 scripts/validate_ssot.py
# Look for: "✅ Configuration consistency checks passed"

# Check emergency stop implementation
grep -r "emergency_stop" src/safety/
# Should find: emergency stop functions and handlers
```

### Safety-Critical Features
- **Emergency Stop**: Hardware button + software triggers (<1ms response)
- **Watchdog System**: Multiple independent watchdog timers
- **Fault Monitoring**: Real-time monitoring of motor/encoder status
- **Position Limits**: Software and hardware position constraints
- **Overcurrent Protection**: Current monitoring and limiting

## 🔄 **VALIDATED** Development Workflow

### Daily Development Commands
```bash
# 1. ALWAYS start with environment setup
source .venv/bin/activate

# 2. Quick build (if already configured)
cmake --build build/Debug

# 3. Clean rebuild when needed
rm -rf build && ./scripts/fix_cmake.sh

# 4. Validate changes
python3 scripts/validate_ssot.py
./scripts/validate_build_env.sh
```

### Before Committing Changes
**ALWAYS run these validation steps:**
```bash
source .venv/bin/activate
python3 scripts/validate_ssot.py        # Configuration consistency
./scripts/validate_build_env.sh         # Environment validation
cmake --build build/Debug               # Ensure builds successfully
```

## 📊 **MEASURED** Timing Expectations

### Build Performance (Validated)
- **Environment Setup**: 1-2 minutes (first time)
- **CMake Configuration**: 1.5 seconds
- **Full Build**: 15-25 minutes (174 compilation units)
- **SSOT Validation**: 0.15 seconds  
- **Environment Check**: 0.2 seconds

### **CRITICAL**: NEVER CANCEL Operations
- **Build Process**: May take 25+ minutes - use 40+ minute timeout
- **CMake Configuration**: Usually fast but wait minimum 2 minutes
- **Schema Generation**: If available, may take 1-2 minutes
- **Testing**: Individual tests complete in seconds, full suite may take 5-10 minutes

## 🎯 **MANUAL VALIDATION** Scenarios

### After Making Code Changes
1. **Build Test**: `cmake --build build/Debug` must complete without errors
2. **SSOT Check**: `python3 scripts/validate_ssot.py` must pass
3. **Safety Validation**: Verify emergency stop and safety functions compile
4. **Configuration Check**: Ensure hardware pin assignments are correct

### Before Hardware Deployment
1. **Full Build**: Complete firmware build without warnings
2. **Size Check**: Verify memory usage is within limits (check arm-none-eabi-size output)
3. **Safety Systems**: Validate all safety-critical code paths
4. **Documentation**: Ensure all changes are documented in SSOT headers

## 🚨 **CRITICAL** Warnings

### Safety System Warnings
```
⚠️  This is a safety-critical motor control system
⚠️  Always run comprehensive tests before deployment  
⚠️  Validate all safety systems before connecting hardware
```

### Development Warnings
- **NEVER bypass safety checks** - they prevent hardware damage
- **ALWAYS use SSOT configuration** - no hardcoded hardware values
- **REQUIRED virtual environment** - build fails without proper Python setup
- **ARM GCC required** - system cannot build with standard GCC

## 📈 Current Project Status

**Check STATUS.md for live project status** - it's automatically updated with:
- Current development phase and priorities
- Build health and memory usage  
- Recent completions and next steps
- Technical context for development continuity

### Architecture Highlights
- **STM32H753ZI**: ARM Cortex-M7 @ 480MHz, 2MB Flash, 1MB RAM
- **FreeRTOS**: Real-time operating system with 1kHz control loop
- **Dual Motors**: L6470 stepper drivers with SPI daisy-chain
- **Closed-Loop**: AS5600 magnetic encoders for position feedback
- **Safety-Critical**: Multiple watchdogs, emergency stop, fault monitoring

---

**Remember: Safety first, SSOT always, validate everything before hardware deployment.**
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
# python3 archive/development_artifacts/legacy_archive/search_enhanced_docs.py [queries...]

# Validate workspace markdown links
python3 scripts/link_validator.py
```

**Semantic Search Database (NEW):**
- `docs/semantic_vector_db/` (77,938 documents): ChromaDB with Instructor-XL embeddings across 10 collections
  * `stm32_hal` collection (55,884 documents): STM32H7 HAL, peripheral drivers, reference manuals
  * `project_source` collection (15,868 documents): Project source code, headers, implementation files
  * `build_system` collection (3,006 documents): CMake, build configuration, toolchain setup
  * `motor_control` collection (1,311 documents): X-CUBE-SPN2 L6470 stepper driver documentation
  * `documentation` collection (1,062 documents): Project documentation, reports, design documents
  * `instructions` collection (643 documents): Development instruction files and guidelines
  * `scripts` collection (113 documents): Automation scripts, tools, utilities
  * `validation`, `config`, `apis` collections: Testing, configuration, and API definitions

**Legacy Search Indexes (Archived):**
- `docs/indexes/STM32H7_FULL_INDEX.json` (8.9MB): Complete STM32H7 searchable index with 31,772 keywords
- `docs/indexes/L6470_SEARCH_INDEX.json` (2.1MB): Complete L6470 documentation index with 356 keywords
- `docs/indexes/STM32H7xx_Nucleo_BSP_INDEX.json` (1MB): Nucleo-144 BSP functions with 74 keywords
- `docs/indexes/STM32H7_COPILOT_INDEX.yaml` (8KB): Copilot-optimized quick reference  
- `docs/indexes/STM32H7_COPILOT_INDEX.json` (9.9KB): Programmatic access format

**Semantic Search Capabilities (NEW):**
- **AI-Powered Understanding**: Uses Instructor-XL embeddings for context-aware search
- **Intelligent Chunking**: Smart document segmentation with STM32-specific parsing
- **Multi-Collection Search**: Targeted search across STM32H7/L6470/BSP/project domains
- **Scope filtering**: Target specific documentation sets (STM32H7/L6470/NUCLEO_BSP/all)
- **Real embeddings**: 768-dimensional vectors from Instructor-XL model
- **Production ready**: Processes 77,938 documents across 10 collections (1.53GB database)
- **GPU Acceleration**: RTX 4080 SUPER with 100% validation success rate (151.9ms avg response)

**Migration Notes:**
- **New system**: `scripts/stm32_semantic_search.py` (semantic search with AI embeddings)
- **Legacy system**: `archive/development_artifacts/legacy_archive/search_enhanced_docs.py` (archived keyword search)
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
