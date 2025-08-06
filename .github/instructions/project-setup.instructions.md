---
applyTo: "**/*.{c,h,md,cmake,txt,json,yml,yaml}"
description: "Project setup, build configuration, testing, and optimization guidance for STM32H753ZI stepper motor project"
---

# Project Setup Instructions

## Overview
This instruction file provides comprehensive guidance for project setup, build configuration, testing infrastructure, and optimization for the STM32H753ZI stepper motor control project.

## Workspace Setup and Project Structure

### Development Environment
This workspace runs in a dev container with:
- **ARM GCC Toolchain**: Cross-compilation for STM32H7
- **OpenOCD**: On-chip debugging and programming
- **STM32CubeMX Integration**: HAL configuration and code generation
- **Testing Frameworks**: Unity and GoogleTest for embedded testing
- **Python Tools**: CAN utilities, documentation generation (Sphinx/Breathe)
- **SSOT Validation**: Pre-commit hooks, configuration validation scripts
- **Documentation**: Doxygen with Graphviz for API documentation

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
1. **Configure**: `cmake -S . -B build -G Ninja`
2. **Build**: `cmake --build build`
3. **Test**: `cd build && ctest`
4. **Flash**: `openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "program build/stm32h753_ihm02a1.elf verify reset exit"`
5. **Debug**: Use VS Code integrated debugger

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
