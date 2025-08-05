# STM32H753ZI Toolchain Configuration Guide

## Overview
This document provides comprehensive guidance for the ARM GCC toolchain configuration used in the STM32H753ZI stepper motor control project. **This configuration prevents 20+ minute build setup cycles.**

## Toolchain Architecture

### Core Components
- **Compiler**: ARM Embedded GCC 10.3.1
- **Build System**: CMake with Ninja generator
- **IDE Integration**: VS Code with CMake Tools extension
- **Debugger**: OpenOCD + GDB with ST-Link interface
- **Target**: STM32H753ZI (ARM Cortex-M7, 480MHz)

### Directory Structure
```
├── cmake/
│   └── arm-none-eabi-gcc.cmake      # CMake toolchain file
├── .vscode/
│   ├── cmake-kits.json              # VS Code kit definitions
│   ├── settings.json                # IDE configuration
│   └── launch.json                  # Debug configurations
└── build/                           # Generated build files
```

## CMake Toolchain Configuration

### File: `cmake/arm-none-eabi-gcc.cmake`

#### Cross-Compilation Setup
```cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Compiler definitions
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
```

#### STM32H753ZI CPU Configuration
```cmake
set(CPU_PARAMETERS "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard")
```

**Parameter Explanation**:
- `-mcpu=cortex-m7`: Target ARM Cortex-M7 processor
- `-mthumb`: Use Thumb instruction set (16/32-bit mix)
- `-mfpu=fpv5-d16`: Use FPv5 FPU with 16 double-precision registers
- `-mfloat-abi=hard`: Hardware floating-point calling convention

#### Compiler Flags
```cmake
set(CMAKE_C_FLAGS "${CPU_PARAMETERS} -Wall -Wextra -Wpedantic -Wno-unused-parameter -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wjump-misses-init -Wdouble-promotion -Wshadow -fdata-sections -ffunction-sections")
```

**Flag Breakdown**:
- **Warning Flags**: `-Wall -Wextra -Wpedantic` - Comprehensive warnings
- **Optimization Flags**: `-fdata-sections -ffunction-sections` - Enable dead code elimination
- **Safety Flags**: `-Wnull-dereference -Wshadow` - Detect potential issues

#### Build Type Configurations
```cmake
# Debug: Optimize for debugging
set(CMAKE_C_FLAGS_DEBUG "-Og -g3 -ggdb")

# Release: Optimize for size
set(CMAKE_C_FLAGS_RELEASE "-Os -DNDEBUG")
```

#### Critical Cross-Compilation Setting
```cmake
# Prevents CMake from trying to run cross-compiled test programs
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
```

## VS Code Integration

### File: `.vscode/cmake-kits.json`

#### Kit Definitions
```json
[
  {
    "name": "ARM GCC for STM32H753ZI",
    "description": "ARM Embedded GCC toolchain for STM32H7 development",
    "compilers": {
      "C": "/usr/bin/arm-none-eabi-gcc",
      "CXX": "/usr/bin/arm-none-eabi-g++"
    },
    "toolchainFile": "/workspaces/code/cmake/arm-none-eabi-gcc.cmake",
    "preferredGenerator": {
      "name": "Ninja"
    },
    "environmentVariables": {
      "CMAKE_BUILD_TYPE": "Debug",
      "ARM_TOOLCHAIN_DIR": "/usr/bin"
    }
  }
]
```

### File: `.vscode/settings.json` (Key Sections)

#### CMake Configuration
```json
{
    "cmake.generator": "Ninja",
    "cmake.configureOnOpen": false,
    "cmake.configureOnEdit": false,
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

#### IntelliSense Configuration
```json
{
    "C_Cpp.default.compilerPath": "/usr/bin/arm-none-eabi-gcc",
    "C_Cpp.default.intelliSenseMode": "gcc-arm",
    "C_Cpp.default.cStandard": "c11",
    "C_Cpp.default.cppStandard": "c++17"
}
```

## Build Commands Reference

### Initial Setup
```bash
# Install dependencies
apt install -y ninja-build

# Verify toolchain
arm-none-eabi-gcc --version
ninja --version
```

### CMake Configuration
```bash
# Debug build
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake \
  -DCMAKE_BUILD_TYPE=Debug

# Release build
cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake \
  -DCMAKE_BUILD_TYPE=Release
```

### Build Operations
```bash
# Standard build
cmake --build build

# Parallel build (explicit)
cmake --build build -- -j$(nproc)

# Clean rebuild
rm -rf build && cmake -S . -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake
```

### VS Code Operations
```
# Kit selection
Ctrl+Shift+P → "CMake: Select a Kit" → "ARM GCC for STM32H753ZI"

# Configure
Ctrl+Shift+P → "CMake: Configure"

# Build
Ctrl+Shift+P → "CMake: Build" or use build tasks
```

## Debugging Configuration

### File: `.vscode/launch.json`

```json
{
    "name": "Debug STM32H753ZI (OpenOCD)",
    "type": "cortex-debug",
    "request": "launch",
    "servertype": "openocd",
    "executable": "${workspaceFolder}/build/stm32h753_ihm02a1.elf",
    "configFiles": [
        "interface/stlink.cfg",
        "target/stm32h7x.cfg"
    ],
    "svdFile": "${workspaceFolder}/00_reference/ST_Assets/stm32h7-svd_2.8/STM32H753.svd"
}
```

### Debug Workflow
1. **Build**: Generate ELF file with debug symbols
2. **Connect**: OpenOCD establishes ST-Link connection
3. **Program**: Flash ELF to target memory
4. **Debug**: GDB attaches and starts debug session

## Performance Considerations

### Build Speed
- **Ninja vs Make**: ~3x faster incremental builds
- **Parallel Compilation**: Automatic with Ninja
- **Incremental Builds**: Only changed files recompiled

### Binary Optimization
- **Debug**: `-Og` - Optimize for debugging experience
- **Release**: `-Os` - Optimize for code size
- **Dead Code Removal**: `-fdata-sections -ffunction-sections` + `--gc-sections`

## Troubleshooting Guide

### Common Issues

#### "Kit not found" in VS Code
**Symptoms**: CMake extension shows "No kit selected"
**Solution**:
1. Verify `.vscode/cmake-kits.json` exists
2. `Ctrl+Shift+P` → `CMake: Select a Kit`
3. Choose "ARM GCC for STM32H753ZI"

#### "CMake can't compile test program"
**Symptoms**: CMake fails during configuration
**Root Cause**: Missing `CMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY`
**Solution**: Verify toolchain file has this setting

#### IntelliSense errors with ARM headers
**Symptoms**: Red squiggles on valid ARM-specific code
**Solution**: Check settings.json:
```json
{
    "C_Cpp.default.compilerPath": "/usr/bin/arm-none-eabi-gcc",
    "C_Cpp.default.intelliSenseMode": "gcc-arm"
}
```

#### Build fails with "ninja: command not found"
**Solution**: `apt install -y ninja-build`

#### Linker errors about missing startup files
**Symptoms**: Undefined references to startup code
**Solution**: Ensure proper startup files are included in CMakeLists.txt

### Verification Commands
```bash
# Toolchain check
which arm-none-eabi-gcc  # /usr/bin/arm-none-eabi-gcc
which ninja              # /usr/bin/ninja

# Version check
arm-none-eabi-gcc --version  # Should show 10.3.1
ninja --version              # Should show 1.10.1+

# Build verification
ls build/*.elf              # Should show compiled ELF file
```

## Integration with Project Architecture

### SSOT Configuration
The toolchain integrates with the Single Source of Truth (SSOT) configuration system:
- **Build metadata**: `src/config/build_config.h`
- **Hardware definitions**: `src/config/hardware_config.h`
- **Compiler flags**: Automatically applied via toolchain

### Cross-References
- **Build Configuration**: `.github/instructions/build-config.instructions.md`
- **SSOT Management**: `.github/instructions/ssot-config.instructions.md`
- **Workspace Setup**: `.github/instructions/workspace-setup.instructions.md`

## Best Practices

### Development Workflow
1. **One-time setup**: Configure kit in VS Code
2. **Daily development**: Use VS Code build tasks
3. **Clean builds**: Use terminal commands when needed
4. **Debugging**: Use VS Code debug configurations

### Configuration Management
- **Version control**: Include `.vscode/cmake-kits.json` and `cmake/` directory
- **Environment independence**: Use relative paths where possible
- **Documentation**: Keep this guide updated with changes

### Performance Tips
- **Incremental builds**: Prefer `cmake --build build` over clean rebuilds
- **Parallel compilation**: Ninja handles this automatically
- **Debug vs Release**: Use Debug for development, Release for final testing

Remember: **Proper toolchain configuration is a one-time investment that saves hours of development time.**
