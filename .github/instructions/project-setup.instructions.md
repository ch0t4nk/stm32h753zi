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
├── drivers/                # Hardware abstraction layer
├── controllers/            # Control algorithms
├── communication/          # Protocol implementations
├── safety/                 # Safety systems
└── application/            # Main application logic
```

### SSOT Configuration Management
- **NEVER** hardcode hardware constants in implementation files
- **ALWAYS** include the appropriate SSOT header from `src/config/`
- **VALIDATE** all configuration parameters at compile time where possible

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
**HAL Mocking for Unit Tests:**
```c
// Mock HAL functions for testing
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

### Status File Maintenance
**STATUS.md** in the root directory provides current project context for GitHub Copilot:

**Update Frequency Guidelines:**
- **After major milestones**: Update progress sections and next steps
- **Weekly**: Review current work items and priorities
- **On phase transitions**: Update current phase and architecture status
- **Build changes**: Update build status and metrics

**Status Structure:**
```markdown
## 🔄 Current Work
- [ ] Task in progress
- [x] Completed task
- [ ] Upcoming task

## ⏭️ Next Steps
1. Immediate priority
2. Secondary priority
3. Future consideration
```

**Automated Updates:**
- Build status from CMake results
- Flash/RAM usage from build output
- Git commit references for recent work

**Manual Maintenance:**
- Current work focus areas
- Next phase priorities
- Technical observations and notes
- Architecture progress tracking

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
