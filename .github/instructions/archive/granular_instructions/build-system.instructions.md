---
applyTo: "CMakeLists.txt"
description: "Build configuration and CMake toolchain setup for STM32H753ZI stepper motor project"
---

# Build Configuration Instructions

## Overview
This instruction file provides comprehensive guidance for configuring the build system, CMake toolchain, and VS Code integration for the STM32H753ZI stepper motor control project. **Follow these instructions to avoid 20+ minute configuration cycles.**

## Critical Build System Architecture

### ARM GCC Toolchain Configuration
**Location**: `cmake/arm-none-eabi-gcc.cmake`

**Key Settings**:
- **Compiler**: `/usr/bin/arm-none-eabi-gcc` (v10.3.1)
- **Target**: `Generic` system, `arm` processor
- **CPU Parameters**: `-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard`
- **Build System**: Ninja (faster than Make)
- **Cross-compilation**: `CMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY`

```cmake
# Essential STM32H753ZI settings
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CPU_PARAMETERS "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard")

# Critical: Disable test compilation for cross-compilation
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
```

### VS Code CMake Kit Configuration
**Location**: `.vscode/cmake-kits.json`

**Available Kits**:
1. **ARM GCC for STM32H753ZI** (Debug) - Primary development kit
2. **ARM GCC Release** - Optimized release builds

**Kit Selection in VS Code**:
```
Ctrl+Shift+P → "CMake: Select a Kit" → "ARM GCC for STM32H753ZI"
```

### VS Code Settings Integration
**Location**: `.vscode/settings.json`

**Critical CMake Settings**:
```json
{
    "cmake.generator": "Ninja",
    "cmake.configureOnOpen": false,
    "cmake.configureOnEdit": false,
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
    "C_Cpp.default.compilerPath": "/usr/bin/arm-none-eabi-gcc",
    "C_Cpp.default.intelliSenseMode": "gcc-arm"
}
```

## Build Process Workflow

### Initial Configuration (One-time setup)
```bash
# 1. Install dependencies (if not present)
apt install -y ninja-build

# 2. Verify ARM toolchain
arm-none-eabi-gcc --version  # Should show v10.3.1

# 3. Configure CMake with toolchain
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake -DCMAKE_BUILD_TYPE=Debug
```

### Standard Build Commands
```bash
# Configure and build
cmake -S . -B build && cmake --build build

# Clean rebuild
rm -rf build && cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake

# Using VS Code tasks
Ctrl+Shift+P → "Tasks: Run Task" → "Build (CMake)"
```

### Debug vs Release Builds
```bash
# Debug (default)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake

# Release (optimized)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake
```

## STM32H753ZI Specific Definitions

### Preprocessor Definitions (Automatic)
```c
#define STM32H753xx
#define USE_HAL_DRIVER
#define HSE_VALUE=8000000
#define LSE_VALUE=32768
#define VDD_VALUE=3300
#define PREFETCH_ENABLE=1
#define INSTRUCTION_CACHE_ENABLE=1
#define DATA_CACHE_ENABLE=1
```

### Compiler Flags
```
-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard
-Wall -Wextra -Wpedantic -fdata-sections -ffunction-sections
```

### Debug vs Release Flags
- **Debug**: `-Og -g3 -ggdb` (debug info + optimization for debugging)
- **Release**: `-Os -DNDEBUG` (size optimization + no debug assertions)

## Common Build Issues and Solutions

### Issue: "CMake can't compile a simple test program"
**Cause**: Missing linker script or incorrect CPU parameters
**Solution**: Ensure `CMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY` is set

### Issue: "Kit not found" in VS Code
**Cause**: CMake kit not properly configured
**Solution**: 
1. Check `.vscode/cmake-kits.json` exists
2. Select kit: `Ctrl+Shift+P` → `CMake: Select a Kit`
3. Choose "ARM GCC for STM32H753ZI"

### Issue: IntelliSense errors with ARM headers
**Cause**: Wrong compiler path or include paths
**Solution**: Verify settings.json has:
```json
{
    "C_Cpp.default.compilerPath": "/usr/bin/arm-none-eabi-gcc",
    "C_Cpp.default.intelliSenseMode": "gcc-arm"
}
```

### Issue: Build fails with "ninja: command not found"
**Solution**: `apt install -y ninja-build`

## Build System Integration with SSOT

### Configuration Dependencies
All build configuration must reference SSOT headers:
```cmake
# Include SSOT configuration paths
target_include_directories(${PROJECT_NAME} PRIVATE
    src/config
    src/common
)
```

### Build Configuration SSOT
**Location**: `src/config/build_config.h`
```c
// Build metadata (auto-generated)
#define BUILD_VERSION_MAJOR 1
#define BUILD_VERSION_MINOR 0
#define BUILD_TIMESTAMP __TIMESTAMP__
#define BUILD_COMPILER_VERSION __VERSION__
```

## VS Code Debugging Configuration

### Launch Configuration
**Location**: `.vscode/launch.json`
- **Debug Config**: Uses OpenOCD + ST-Link
- **Target**: STM32H753ZI with SVD file for register viewing
- **Executable**: `build/stm32h753_ihm02a1.elf`

### Debugging Workflow
1. Build project: `cmake --build build`
2. Start debugging: `F5` or Debug panel
3. OpenOCD connects via ST-Link to target
4. GDB loads ELF file and starts debugging session

## Performance Considerations

### Build Speed Optimization
- **Ninja**: ~3x faster than Make for incremental builds
- **Parallel Builds**: Ninja automatically uses available CPU cores
- **ccache**: Consider for repeated clean builds (not currently configured)

### Binary Size Optimization (Release builds)
- **Link-time optimization**: `-flto` (not enabled by default)
- **Dead code elimination**: `-fdata-sections -ffunction-sections` + `--gc-sections`
- **Size optimization**: `-Os` (enabled in Release)

## Toolchain Validation Commands

### Quick Verification Script
```bash
#!/bin/bash
echo "=== STM32H753ZI Build System Verification ==="
echo "ARM GCC Version: $(arm-none-eabi-gcc --version | head -n1)"
echo "Ninja Version: $(ninja --version)"
echo "CMake Version: $(cmake --version | head -n1)"
echo "Build Directory: $(ls -la build 2>/dev/null || echo 'Not configured')"
echo "Active Kit: Check VS Code status bar"
```

### Environment Check
```bash
# Verify all required tools
which arm-none-eabi-gcc  # Should be /usr/bin/arm-none-eabi-gcc
which ninja              # Should be /usr/bin/ninja  
which cmake              # Should be /usr/bin/cmake
which openocd            # Should be /usr/bin/openocd
```

## Documentation References

### Related Instructions
- **hardware-pins.instructions.md**: Pin assignments and peripheral setup
- **ssot-config.instructions.md**: Configuration management
- **workspace-setup.instructions.md**: Overall project structure

### External Documentation
- **STM32H7 Reference Manual**: Hardware specifications
- **ARM GCC Manual**: Compiler options and flags
- **CMake Documentation**: Build system configuration
- **OpenOCD Manual**: Debugging and programming

Remember: **Consistent build configuration prevents development time waste and ensures reproducible builds across environments.**
