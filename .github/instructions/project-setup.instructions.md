---
applyTo: "**/*.{c,h,md,cmake,txt,json,yml,yaml}"
description: "Project setup, build configuration, testing, and optimization guidance for STM32H753ZI stepper motor project"
---

# Project Setup Instructions

## Overview

This instruction file provides comprehensive guidance for project setup, build configuration, testing infrastructure, and optimization for the STM32H753ZI stepper motor control project.

**MAJOR UPDATES**: This project now includes advanced automation, semantic search, HAL abstraction testing, and FreeRTOS integration roadmap (Phase 5). All examples use our mature architecture.

## Modern Project Features (2025 Update)

### 🤖 STATUS Automation System

**Automated Project Context for AI Assistance:**

- **Auto-updating STATUS.md**: Git hooks automatically update project status after commits
- **Real-time monitoring**: Build status, memory usage, git state tracking
- **AI-friendly context**: GitHub Copilot gets current project state automatically
- **Performance optimized**: 1-2 second git operations with graceful fallbacks

```bash
# Automation is already active - STATUS.md updates automatically after commits
# Manual control available:
/workspaces/code/.venv/bin/python3 scripts/auto_update_status.py --verbose
/workspaces/code/.venv/bin/python3 scripts/status_monitor.py --status-bar  # VS Code format

# VS Code tasks for enhanced control:
cmake --build build --target update-status         # Manual STATUS.md update
cmake --build build --target update-status-preview # Preview changes
./scripts/demo_status_integration.sh               # Full demo
```

### 🔍 Semantic Search Engine

**Professional Documentation Search:**

- **Intelligent search**: Natural language queries across all documentation
- **Multi-modal**: Searches code, markdown, reference manuals, API specs
- **Context-aware**: Understands project-specific terminology
- **Integration-ready**: Available via scripts and Python API

```bash
# Search across entire project knowledge base
/workspaces/code/.venv/bin/python3 scripts/semantic_search.py "motor control PID tuning"
/workspaces/code/.venv/bin/python3 scripts/semantic_search.py "L6470 SPI communication error"
/workspaces/code/.venv/bin/python3 scripts/semantic_search.py "FreeRTOS task priority configuration"
```

### 🧪 HAL Abstraction Testing Framework

**Hardware-Independent Development:**

- **Mock hardware**: All drivers work without actual hardware
- **Clean testing**: Unit tests run on host machine using mock HAL
- **Professional architecture**: Application code isolated from hardware
- **Copilot compatible**: All examples work with our actual codebase

```c
// ✅ CORRECT - Using HAL abstraction (testable, professional)
#include "hal_abstraction.h"

SystemError_t motor_init(void) {
    SystemError_t result = HAL_Abstraction_GPIO_Write(MOTOR_ENABLE_PORT,
                                                      MOTOR_ENABLE_PIN,
                                                      HAL_GPIO_STATE_SET);
    return result;
}

// ❌ INCORRECT - Direct HAL (not testable, outdated pattern)
#include "stm32h7xx_hal.h"
void bad_motor_init(void) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);  // Hard to test!
}
```

### 🔄 Dual Build System

**ARM Firmware + Host Testing:**

- **ARM build**: `build/` - STM32H753ZI firmware with arm-none-eabi-gcc
- **Host build**: `build_host_tests/` - Unit tests with native gcc/ninja
- **Parallel development**: Test algorithms on host, deploy to ARM
- **CI/CD ready**: Both builds validated automatically

```bash
# ARM firmware build (for STM32H753ZI)
cmake --preset=default
cmake --build build

# Host testing build (for development machine)
cmake --preset=host-tests
cmake --build build_host_tests
cd build_host_tests && ctest
```

## Workspace Setup and Project Structure

### Development Environment

This workspace runs in a **pre-configured dev container** with:

- **ARM GCC Toolchain**: Cross-compilation for STM32H7 (arm-none-eabi-gcc 10.3.1+)
- **OpenOCD**: On-chip debugging and programming
- **STM32CubeMX Integration**: HAL configuration and code generation
- **Testing Frameworks**: Unity for embedded testing, GoogleTest for host tests
- **Python Environment**: CAN utilities, semantic search, STATUS automation (.venv)
- **SSOT Validation**: Pre-commit hooks, configuration validation scripts
- **Documentation**: Doxygen with Graphviz for API documentation
- **Code Quality**: clang-format, clang-tidy, cppcheck pre-installed

### Core Toolchain Verification

**All tools pre-installed and ready:**

```bash
# ARM Cross-compilation toolchain
arm-none-eabi-gcc --version     # ARM GCC 10.3.1+
arm-none-eabi-gdb --version     # ARM GDB debugger

# STM32 Development tools
openocd --version               # OpenOCD for programming/debugging
st-flash --version              # ST-Link utilities

# Code quality and analysis
clang-format --version          # Code formatting (Google style)
clang-tidy --version           # Static analysis
cppcheck --version             # Additional linting
doxygen --version              # Documentation generation

# Modern Python environment (/.venv)
python3 --version              # Python 3.11+ for automation
/workspaces/code/.venv/bin/python3 -m pip list | grep -E "(semantic|status)"  # Our custom tools
```

### VS Code Task Integration

**One-Click Development with Professional Tasks:**

```bash
# Primary development tasks (Ctrl+Shift+P → "Tasks: Run Task")
"Build (CMake)"                    # Main ARM firmware build
"Run Tests (CTest)"               # Host-based unit tests
"STATUS: Build + Update"          # Build + auto-update STATUS.md
"Flash STM32H753ZI"              # Program to hardware
"Generate Docs (Doxygen)"        # API documentation

# Development quality tasks
"Format Code"                     # clang-format all source
"Lint (clang-tidy/cppcheck)"     # Static analysis
"Validate SSOT"                  # Config consistency check
"Validate Build Environment"      # Toolchain verification

# STATUS automation tasks
"STATUS: Preview Changes"         # Dry-run status updates
"STATUS: Force Full Update"      # Complete regeneration
"STATUS: Quick Update"           # Fast manual update
```

### File Organization Rules

**Root Directory - MINIMAL CONTENT ONLY**

```
├── CMakeLists.txt          # Main build configuration
├── README.md               # Project overview and quick start
├── LICENSE                 # Project license
├── .gitignore             # Git ignore rules
└── build/                 # Build artifacts directory (git-ignored)
```

**Source Code Organization**

```
src/
├── config/                 # SSOT configuration headers ONLY
├── common/                 # Common definitions and utilities
├── hal_abstraction/        # Hardware Abstraction Layer (HAL)
│   ├── hal_abstraction.h   # Platform-independent HAL interface
│   ├── hal_abstraction_stm32h7.c  # STM32H7 implementation
│   └── hal_abstraction_mock.c     # Mock implementation for testing
├── drivers/                # Hardware-specific drivers
├── controllers/            # Control algorithms
├── communication/          # Protocol implementations
├── safety/                 # Safety systems (with HAL abstraction)
└── application/            # Main application logic
```

### SSOT Configuration Management

- **NEVER** hardcode hardware constants in implementation files
- **ALWAYS** include the appropriate SSOT header from `src/config/`
- **USE HAL ABSTRACTION**: All hardware access must go through `hal_abstraction.h` interface
- **VALIDATE** all configuration parameters at compile time where possible

**HAL Abstraction Layer Benefits:**

- **Platform Independence**: Code compiles for multiple targets (STM32H7, mocks, future platforms)
- **Hardware-Independent Testing**: Unit tests run without actual hardware using mock implementations
- **Clean Architecture**: Application code isolated from hardware-specific details
- **Easy Debugging**: Mock layer enables controlled fault injection and state monitoring

## Build Configuration and System

### CMake Configuration

**Primary Build Configuration:**

```cmake
# STM32H753ZI-specific settings
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# Compiler settings for Cortex-M7
set(CMAKE_C_FLAGS
    "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard"
    "-Wall -Wextra -Wpedantic -Og -g3"
)

# Memory configuration
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32H753XX_FLASH.ld")
```

**Build Targets:**

- **stm32h753_ihm02a1.elf**: Main firmware binary
- **tests**: Unit test suite
- **docs**: Doxygen documentation

### ARM GCC Toolchain Setup

**Required Tools:**

- `arm-none-eabi-gcc` (version 10.3.1 or later)
- `arm-none-eabi-gdb` for debugging
- `openocd` for flashing and debugging

**VS Code Integration:**

1. Configure CMake kit: `Ctrl+Shift+P` → `CMake: Select a Kit` → `ARM GCC for STM32H753ZI`
2. Set build variant: Debug for development, Release for production
3. Configure launch.json for OpenOCD debugging

### Build Troubleshooting

**Common Issues and Solutions:**

1. **Toolchain Not Found**

   ```bash
   # Verify ARM GCC installation
   arm-none-eabi-gcc --version
   which arm-none-eabi-gcc
   ```

2. **Memory Overflow**

   - Check linker script memory regions
   - Enable garbage collection: `-fdata-sections -ffunction-sections`
   - Use link-time optimization for Release builds

3. **HAL Library Conflicts**
   - Ensure single HAL version across project
   - Check for conflicting `#define` statements
   - Verify `stm32h7xx_hal_conf.h` configuration

## Testing Infrastructure

### Unit Testing Framework

**Unity Framework for Embedded C:**

```c
// Example test structure
#include "unity.h"
#include "motor_controller.h"

void setUp(void) {
    // Test setup code
}

void tearDown(void) {
    // Test cleanup code
}

void test_motor_controller_init(void) {
    SystemError_t result = motor_controller_init();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_motor_controller_init);
    return UNITY_END();
}
```

**Test Organization:**

```
tests/
├── unit/                  # Unit tests
│   ├── test_motor_controller.c
│   └── test_l6470_driver.c
├── integration/           # Integration tests
├── fixtures/              # Test fixtures and data
└── mocks/                 # Mock objects
```

### Test Configuration

- **CTest Integration**: Run tests with `ctest` command
- **Continuous Integration**: Automated testing on commits
- **Coverage Reports**: Track test coverage metrics
- **Hardware-in-Loop**: Testing with actual STM32H753ZI hardware

### Mock Objects and Stubs

**HAL Abstraction Mock Testing:**

```c
// Example using HAL abstraction for testable code
#include "hal_abstraction.h"

SystemError_t motor_controller_test_function(void) {
    // This code works with both real hardware and mocks
    SystemError_t result = HAL_Abstraction_GPIO_WritePin(
        MOTOR1_CS_PORT, MOTOR1_CS_PIN, HAL_GPIO_PIN_SET);

    if (result != SYSTEM_OK) {
        return result;
    }

    // SPI transaction using abstraction
    HAL_SPI_Transaction_t transaction = {
        .tx_data = command_buffer,
        .tx_size = sizeof(command_buffer),
        .rx_data = response_buffer,
        .rx_size = sizeof(response_buffer),
        .timeout_ms = 100
    };

    return HAL_Abstraction_SPI_Transaction(SPI_MOTOR_BUS, &transaction);
}

// Test using mock implementation
void test_motor_controller_with_mocks(void) {
    // Configure mock behavior
    MockHAL_SetGPIOState(MOTOR1_CS_PORT, MOTOR1_CS_PIN, HAL_GPIO_PIN_SET);
    MockHAL_SetSPIResponse(SPI_MOTOR_BUS, expected_response, sizeof(expected_response));

    // Test the function - runs without hardware
    SystemError_t result = motor_controller_test_function();
    TEST_ASSERT_EQUAL(SYSTEM_OK, result);

    // Verify mock interactions
    TEST_ASSERT_TRUE(MockHAL_WasGPIOPinSet(MOTOR1_CS_PORT, MOTOR1_CS_PIN));
}
```

**HAL Mocking for Unit Tests:**

```c
// Legacy example - prefer HAL abstraction above
HAL_StatusTypeDef HAL_SPI_Transmit_Mock(SPI_HandleTypeDef *hspi,
                                       uint8_t *pData,
                                       uint16_t Size,
                                       uint32_t Timeout) {
    // Mock implementation for testing
    return HAL_OK;
}
```

## Optimization Guidelines

### Performance Optimization

**Compiler Optimization Levels:**

- **Debug (-Og)**: Optimized for debugging experience
- **Release (-O2)**: Balanced optimization for production
- **Size (-Os)**: Optimize for code size when flash is limited

**STM32H7-Specific Optimizations:**

1. **Cache Configuration**

   ```c
   // Enable I-Cache and D-Cache for performance
   SCB_EnableICache();
   SCB_EnableDCache();
   ```

### 📄 Automated Doc Regeneration (Self-hosted runner)

We provide a safe automated workflow to regenerate full domain documentation using a local LLM (recommended: Ollama) on a self-hosted runner.

- Purpose: run `.copilot-tasks/generate_docs.md` logic and produce updated `docs/*.md` and `docs/figures/*.svg` artifacts.
- Trigger: manual (`workflow_dispatch`) on a trusted self-hosted runner only. Do NOT run on public shared runners.
- Requirements on the runner:
  - Ollama installed and the model available locally (example: `gpt-OSS:20b`).
  - Node.js (for mermaid-cli) and npm available.
  - Python 3.11 and project `.venv` (or system Python) with `requirements.txt` installed.
  - Sufficient memory (we recommend 64GB RAM and GPU/VRAM as appropriate for the model).

How it works (high level):

1. Checkout repository
2. Install Node/Python deps
3. Prepare local context and prompts (scripts/generate_docs_runner.py --prompt .copilot-tasks/generate_docs.md --dry-run)
4. Run generation on Ollama (scripts/generate_docs_runner.py --execute)
5. Validate outputs (scripts/validate_ssot.py) and render mermaid diagrams
6. Create a local branch with generated docs and open a PR for review

Security notes:

- The workflow will NOT push to `main` automatically; it creates a branch and opens a PR for review.
- Do not place cloud API keys in this workflow; it is designed for local Ollama usage on trusted hardware.

2. **DMA Usage**

   - Use DMA for SPI/I2C transfers to reduce CPU load
   - Configure circular DMA for continuous data streams

3. **Memory Placement**

   ```c
   // Place time-critical code in ITCM RAM
   __attribute__((section(".itcmram")))
   void critical_interrupt_handler(void);

   // Place frequently accessed data in DTCM RAM
   __attribute__((section(".dtcmram")))
   volatile uint32_t motor_control_data[1024];
   ```

### Memory Optimization

**Flash Memory Management:**

- Use const for read-only data
- Implement data compression for large lookup tables
- Store calibration data in external QSPI flash if needed

**RAM Optimization:**

- Use stack for temporary variables
- Implement memory pools for dynamic allocation
- Monitor stack usage with `-fstack-usage` flag

**DTCM/ITCM Usage:**

- DTCM: Fast data access for control loops
- ITCM: Fast instruction execution for time-critical code

### Real-Time Performance

**Interrupt Priorities:**

```c
// Configure NVIC priorities for real-time performance
HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 0, 0);  // Highest priority
HAL_NVIC_SetPriority(SPI2_IRQn, 1, 0);           // SPI communication
HAL_NVIC_SetPriority(I2C1_EV_IRQn, 2, 0);       // I2C communication
```

**Timing Analysis:**

- Use SysTick for precise timing measurements
- Profile interrupt service routines
- Monitor worst-case execution times

## Version Management and Build Metadata

### Version Control Integration

**Automated Version Generation:**

```cmake
# Generate version from git
execute_process(
    COMMAND git describe --tags --dirty
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

add_definitions(-DFIRMWARE_VERSION="${GIT_VERSION}")
```

**Build Information Structure:**

```c
typedef struct {
    const char* version;
    const char* build_date;
    const char* build_time;
    const char* git_hash;
    uint32_t feature_flags;
} BuildInfo_t;

extern const BuildInfo_t build_info;
```

### Configuration Validation

**Pre-build Validation Scripts:**

```bash
#!/bin/bash
# validate_config.sh - Run before each build

# Check SSOT configuration consistency
python3 scripts/validate_ssot.py

# Validate instruction file references
python3 scripts/link_validator.py

# Check memory configuration
python3 scripts/validate_memory_config.py
```

**Static Analysis Integration:**

- **cppcheck**: Static code analysis
- **clang-tidy**: Modern C++ best practices
- **PC-lint**: MISRA C compliance checking

### Debugging and Development Tools

**Debug Configuration:**

```json
// launch.json for VS Code
{
  "name": "Debug STM32H753ZI",
  "type": "cortex-debug",
  "servertype": "openocd",
  "configFiles": ["interface/stlink.cfg", "target/stm32h7x.cfg"],
  "executable": "${workspaceFolder}/build/stm32h753_ihm02a1.elf",
  "runToEntryPoint": "main"
}
```

**Development Workflow:**

1. **Configure Workflow**: `python scripts/workflow_config.py mode standard` (or minimal/thorough/debug)
2. **Configure Build**: `cmake -S . -B build -G Ninja`
3. **Build**: `cmake --build build`
4. **Test**: `cd build && ctest`
5. **Flash**: `openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "program build/stm32h753_ihm02a1.elf verify reset exit"`
6. **Debug**: Use VS Code integrated debugger

**SSOT Workflow Configuration:**

```powershell
# Configure workflow complexity based on use case
python scripts/workflow_config.py mode minimal    # CI/CD, experienced developers
python scripts/workflow_config.py mode standard   # Daily development
python scripts/workflow_config.py mode thorough   # Critical development phases
python scripts/workflow_config.py mode debug      # Troubleshooting, investigation

# Use SSOT-aware script execution
scripts/run_python_configurable.ps1 scripts/auto_update_status.py --verbose
scripts/run_python_configurable.ps1 scripts/feature_tracker.py list --status IN_PROGRESS

# Individual workflow setting control
python scripts/workflow_config.py set verbose_logging false
python scripts/workflow_config.py set validate_venv true
```

### Feature Flags and Conditional Compilation

**Feature Toggle System:**

```c
// Feature flags for conditional compilation
#define FEATURE_ADVANCED_CONTROL    (1 << 0)
#define FEATURE_CAN_COMMUNICATION   (1 << 1)
#define FEATURE_ETHERNET_SUPPORT    (1 << 2)
#define FEATURE_DEBUG_LOGGING       (1 << 3)

// Compile-time feature selection
#ifndef ENABLED_FEATURES
#define ENABLED_FEATURES (FEATURE_ADVANCED_CONTROL | FEATURE_DEBUG_LOGGING)
#endif

#if (ENABLED_FEATURES & FEATURE_ADVANCED_CONTROL)
    // Advanced control algorithms
#endif
```

## Integration Guidelines

### Continuous Integration Setup

**GitHub Actions Workflow:**

```yaml
name: STM32H753ZI Build and Test
on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Setup ARM GCC
        run: sudo apt-get install gcc-arm-none-eabi
      - name: Configure CMake
        run: cmake -S . -B build -G Ninja
      - name: Build
        run: cmake --build build
      - name: Test
        run: cd build && ctest
```

### Quality Gates

**Pre-commit Hooks:**

- SSOT validation
- Code formatting (clang-format)
- Static analysis (cppcheck)
- Unit test execution
- Documentation link validation

**Release Criteria:**

- All tests pass
- Static analysis clean
- Memory usage within limits
- Performance benchmarks met
- Safety validation complete

## Project Status Management

### Status File Maintenance & Automation

**STATUS.md** in the root directory provides current project context for GitHub Copilot with **comprehensive automation system**:

**🤖 Automated Features (Phase 3 Complete):**

- **Git Hooks**: Automatic post-commit STATUS.md updates with intelligent loop prevention
- **Real-time Monitoring**: Live build/git status tracking with optimized process management
- **VS Code Integration**: Enhanced tasks for manual triggers, previews, and build+update workflows
- **Smart Detection**: Merge commit handling, build context analysis, performance optimization
- **Centralized Logging**: All automation events tracked in `logs/status_automation.log`

**Automation Workflow:**

```bash
# Automatic updates (no user action required)
git commit -m "feat: add new driver"  # ← Triggers STATUS.md update automatically

# Manual monitoring and control
/workspaces/code/.venv/bin/python3 scripts/status_monitor.py --status-bar  # VS Code format
/workspaces/code/.venv/bin/python3 scripts/status_monitor.py --json        # JSON format
/workspaces/code/.venv/bin/python3 scripts/status_monitor.py --watch       # Real-time monitoring

# VS Code tasks for enhanced control
cmake --build build --target update-status         # Manual STATUS.md update
cmake --build build --target update-status-preview # Preview changes without commit
./scripts/demo_status_integration.sh               # Complete workflow demonstration
```

**Update Frequency (Automated + Manual):**

- **Automatic**: After every git commit via post-commit hook
- **Build Integration**: Real-time build status and memory usage tracking
- **Manual Override**: For immediate updates or preview before committing
- **Validation**: Pre-commit hooks ensure SSOT compliance and format checking

**Status Structure (Enhanced):**

```markdown
## 🔄 Current Work

- [ ] Task in progress
- [x] Completed task
- [ ] Upcoming task

## ⏭️ Next Steps

1. Immediate priority
2. Secondary priority
3. Future consideration

## 📝 Recent Session Context (Last Conversation)

- **Last Session**: [Auto-generated from git history]
- **Git State**: [Auto-generated commit reference]
- **Build Status**: [Auto-generated from build artifacts]
```

**Automated Data Sources:**

- **Build Status**: CMake results, memory usage from arm-none-eabi-size
- **Git Context**: Recent commits, branch status, working directory state
- **Development Metrics**: Flash/RAM usage, build timing, error detection
- **Session Continuity**: AI conversation handoff context, recent activities

**Performance Optimizations:**

- **Filesystem-first Git Checking**: Avoids subprocess overhead for clean repositories
- **Strict Timeouts**: 1-2 second git operation limits with graceful fallbacks
- **Smart Caching**: Recent git index monitoring to optimize status checks
- **Process Management**: Proper subprocess cleanup to prevent git lock conflicts

### Copilot Context Integration

The status file enables GitHub Copilot to:

- Understand current development phase
- Suggest relevant code patterns
- Provide context-appropriate assistance
- Reference recent progress and next priorities

**Best Practices:**

- Keep status current for optimal Copilot assistance
- Use emoji sections for quick visual parsing
- Include metrics and concrete progress indicators
- Reference key commands and file locations

Remember: **Organized setup enables reliable development, comprehensive testing ensures quality, optimized builds enable performance, current status enables effective AI assistance.**
