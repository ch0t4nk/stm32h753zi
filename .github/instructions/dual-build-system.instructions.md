---
applyTo: "**/CMakeLists.txt,**/*.cmake,**/CMakePresets.json,.vscode/cmake-kits.json,**/fix_cmake.sh,**/run_host_tests.sh"
description: "Dual build system configuration for ARM firmware and host testing compilation on STM32H753ZI stepper motor project"
---

# Dual Build System Instructions

## Overview
This instruction file provides comprehensive guidance for the dual build system architecture that enables both ARM firmware compilation and host testing without configuration conflicts. This system prevents wasted effort exploring toolchain issues by establishing clear separation between compilation targets.

## Dual Build Architecture

### ARM Firmware Compilation (Primary Target)
**Purpose**: Build actual STM32H753ZI firmware for hardware deployment
**Toolchain**: ARM GCC cross-compiler (`arm-none-eabi-gcc`)
**Configuration**: CMakePresets.json with ARM-specific settings
**Build Directory**: `build/` (primary build artifacts)

### Host Testing Compilation (Secondary Target)  
**Purpose**: Build drivers and logic for local testing with mocks
**Toolchain**: Local GCC (`gcc`, system compiler)
**Configuration**: Separate `host_tests/CMakeLists.txt`
**Build Directory**: `build_host_tests/` (isolated test artifacts)

## Build System Components

### Primary ARM Build System
```bash
# ARM firmware compilation workflow
./scripts/fix_cmake.sh              # Configure + build ARM firmware
cmake -S . -B build --preset Debug  # Manual ARM configuration
cmake --build build                 # Build ARM firmware
```

**Key Files:**
- `CMakeLists.txt` (root): Main ARM firmware build configuration
- `CMakePresets.json`: ARM toolchain presets (Debug/Release)
- `cmake/gcc-arm-none-eabi.cmake`: ARM GCC toolchain configuration
- `scripts/fix_cmake.sh`: Automated ARM build with virtual environment setup

### Host Testing Build System
```bash
# Host testing compilation workflow
./scripts/run_host_tests.sh         # Build and run host tests
cd host_tests && cmake -S . -B ../build_host_tests  # Manual host config
cmake --build build_host_tests      # Build host tests only
```

**Key Files:**
- `host_tests/CMakeLists.txt`: Separate host testing configuration
- `scripts/run_host_tests.sh`: Automated host testing with local GCC
- `build_host_tests/`: Isolated host test build directory

## Critical Separation Principles

### 1. Toolchain Isolation
**ARM Compilation:**
```cmake
# CMakeLists.txt (ARM target)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
```

**Host Compilation:**
```cmake
# host_tests/CMakeLists.txt (Host target)
# Uses system default gcc/g++ (no cross-compilation)
set(CMAKE_BUILD_TYPE Debug)
```

### 2. Include Path Separation
**ARM Firmware Includes:**
- STM32H7 HAL libraries (`STM32Cube_FW_H7_V1.12.0/`)
- Hardware-specific headers
- ARM CMSIS libraries
- Platform-dependent implementations

**Host Testing Includes:**
- Mock implementations (`src/hal_abstraction/hal_abstraction_mock.c`)
- Unity testing framework
- Standard C library (no embedded constraints)
- Platform-independent interfaces only

### 3. Compilation Flag Separation
**ARM Firmware Flags:**
```cmake
set(CMAKE_C_FLAGS 
    "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard"
    "-Wall -Wextra -Wpedantic -Og -g3"
    "-fdata-sections -ffunction-sections"
)
```

**Host Testing Flags:**
```cmake
set(CMAKE_C_FLAGS 
    "-Wall -Wextra -Wpedantic -g -O0"
    "-DSIMULATION_ENABLED=1"
    "-DTEST_MODE=1"
)
```

## HAL Abstraction Integration

### Platform-Independent Driver Code
All drivers use HAL abstraction layer to enable dual compilation:

```c
// ✅ CORRECT - Works for both ARM and host
#include "hal_abstraction/hal_abstraction.h"

SystemError_t driver_function(void) {
    // Platform-independent HAL calls
    SystemError_t result = HAL_Abstraction_GPIO_WritePin(
        MOTOR1_CS_PORT, MOTOR1_CS_PIN, HAL_GPIO_PIN_SET);
    
    HAL_I2C_Transaction_t transaction = { /* ... */ };
    return HAL_Abstraction_I2C_Transaction(HAL_I2C_INSTANCE_1, &transaction);
}
```

### Implementation Selection
**ARM Firmware**: Links with `hal_abstraction_stm32h7.c` (real hardware)
**Host Testing**: Links with `hal_abstraction_mock.c` (Unity mocks)

## Simulation Framework Integration

### Conditional Compilation
```c
// Driver code supports both real hardware and simulation
#if SIMULATION_ENABLED
    if (simulation_mode_active()) {
        // Use simulation backend
        return simulation_function(parameters);
    }
#endif
    // Use real hardware via HAL abstraction
    return HAL_Abstraction_Hardware_Function(parameters);
```

### Build System Integration
**ARM Builds**: `SIMULATION_ENABLED=0` (real hardware)
**Host Builds**: `SIMULATION_ENABLED=1` (mock/simulation)

## Common Build Issues and Solutions

### Issue 1: Toolchain Conflicts
**Problem**: ARM and host toolchains interfere with each other
**Solution**: Use completely separate build directories and CMake configurations

### Issue 2: Include Path Conflicts  
**Problem**: ARM-specific headers included in host builds
**Solution**: HAL abstraction layer provides platform-independent interfaces

### Issue 3: Simulation Header Dependencies
**Problem**: Simulation headers may have host-specific dependencies
**Solution**: Conditional compilation with `#if SIMULATION_ENABLED` guards

### Issue 4: Library Linking Conflicts
**Problem**: ARM libraries linked in host builds or vice versa
**Solution**: Separate CMakeLists.txt files with target-specific dependencies

## Workflow Best Practices

### Development Workflow
1. **Write platform-independent driver code** using HAL abstraction
2. **Test with host builds first** using simulation/mock framework
3. **Validate with ARM builds** for real hardware compatibility
4. **Deploy to hardware** only after both builds pass

### Debugging Workflow
1. **Host testing**: Use GDB, Valgrind, AddressSanitizer for quick debugging
2. **ARM debugging**: Use OpenOCD + GDB for hardware-specific issues
3. **Simulation**: Test complex scenarios safely without hardware risk

### Integration Testing
```bash
# Complete validation workflow
./scripts/run_host_tests.sh         # Test with mocks/simulation
./scripts/fix_cmake.sh              # Build ARM firmware
cmake --build build --target tests  # Run embedded unit tests
```

## Virtual Environment Integration

### Python Automation Support
Both build systems integrate with Python virtual environment for automation:

```bash
# Virtual environment setup (done automatically)
source .venv/bin/activate
.venv/bin/python scripts/auto_update_status.py

# Build scripts use virtual environment
VIRTUAL_ENV="${workspaceFolder}/.venv"
PATH="${workspaceFolder}/.venv/bin:${env:PATH}"
```

### Automation Scripts
- `scripts/auto_update_status.py`: Post-build STATUS.md updates
- `scripts/validate_ssot.py`: Configuration validation
- `scripts/stm32_semantic_search.py`: Documentation search

## VS Code Integration

### CMake Kit Configuration
```json
// .vscode/cmake-kits.json
{
  "name": "ARM GCC for STM32H753ZI",
  "toolchainFile": "cmake/gcc-arm-none-eabi.cmake",
  "description": "ARM GCC cross-compiler for STM32H753ZI"
}
```

### Tasks Configuration
- **Build (CMake)**: ARM firmware build with virtual environment
- **Run Tests (CTest)**: Host testing with local GCC
- **Flash STM32H753ZI**: Hardware deployment via OpenOCD

### Debug Configuration
**ARM Debugging:**
```json
{
  "name": "Debug STM32H753ZI",
  "type": "cortex-debug",
  "servertype": "openocd",
  "executable": "${workspaceFolder}/build/stm32h753_ihm02a1.elf"
}
```

**Host Debugging:**
```json
{
  "name": "Debug Host Tests",
  "type": "cppdbg",
  "program": "${workspaceFolder}/build_host_tests/test_executable"
}
```

## Memory and Performance

### ARM Firmware Optimization
- **Flash Usage**: Optimized for 2MB STM32H753ZI flash
- **RAM Usage**: Efficient use of 1MB SRAM
- **Real-time Performance**: Cortex-M7 optimizations enabled

### Host Testing Performance
- **Fast Iteration**: No cross-compilation overhead
- **Debug Information**: Full debugging symbols and sanitizers
- **Memory Analysis**: Valgrind and AddressSanitizer support

## Troubleshooting Guide

### Build Failures
1. **Check toolchain installation**: `arm-none-eabi-gcc --version`
2. **Verify virtual environment**: `which python3` should show `.venv/bin/python3`
3. **Clean builds**: `rm -rf build build_host_tests`
4. **Check CMake configuration**: Look for toolchain file errors

### Runtime Issues
1. **Host tests failing**: Check mock implementations and simulation setup
2. **ARM firmware issues**: Use OpenOCD debugging and hardware validation
3. **Memory errors**: Use host builds with AddressSanitizer first

### Integration Problems
1. **HAL abstraction errors**: Ensure all drivers use abstraction layer
2. **Simulation conflicts**: Check conditional compilation guards
3. **SSOT violations**: Run `python3 scripts/validate_ssot.py`

## Related Instruction Files

### Cross-References
- **project-setup.instructions.md**: Overall project configuration and testing framework
- **hardware.instructions.md**: STM32H7 HAL configuration and GPIO control
- **core-software.instructions.md**: HAL abstraction layer and error handling
- **ssot-config.instructions.md**: Single Source of Truth configuration management

### Build System Dependencies
- All SSOT configuration headers must be accessible to both build systems
- HAL abstraction layer enables platform-independent driver development
- Simulation framework provides safe testing without hardware risk
- Virtual environment ensures consistent Python automation across both builds

Remember: **Separate build systems enable safe development, HAL abstraction enables dual compilation, simulation enables risk-free testing.**
