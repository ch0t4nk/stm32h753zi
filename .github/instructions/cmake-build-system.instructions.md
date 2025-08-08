---
applyTo: "**/CMakeLists.txt,**/*.cmake,**/CMakePresets.json,.vscode/cmake-kits.json,**/fix_cmake.sh,**/run_host_tests.sh"
description: "STM32H753ZI build system configuration including ARM firmware and host testing compilation"
---

# Build System Instructions

## Overview
This instruction file provides comprehensive guidance for the dual build system architecture that enables both ARM firmware compilation and host testing. This system prevents wasted effort exploring toolchain issues by establishing clear separation between compilation targets.

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

## Root Cause Analysis of Recurring Issues

### Why We Keep Having CMake Problems
1. **Dev Container ARM GCC Installation**: ARM cross-compiler may not be installed or in PATH
2. **CMake Toolchain Selection**: CMake defaults to system compiler instead of ARM GCC
3. **Preset Configuration**: CMakePresets.json references may be incorrect or unused
4. **Environment Variables**: Missing or incorrect PATH/toolchain environment setup
5. **Build Directory State**: Cached CMake configuration prevents toolchain changes

## Definitive Build System Setup

### 1. Verify ARM GCC Toolchain Installation

**Check Installation:**
```bash
# MUST succeed - if this fails, ARM GCC is not installed
which arm-none-eabi-gcc
arm-none-eabi-gcc --version

# Expected output should show ARM GCC version (e.g., 10.3.1 or later)
# If missing, install with: sudo apt-get install gcc-arm-none-eabi
```

**Install ARM GCC if Missing:**
```bash
# Ubuntu/Debian installation
sudo apt-get update
sudo apt-get install gcc-arm-none-eabi gdb-arm-none-eabi

# Verify installation
arm-none-eabi-gcc --version
arm-none-eabi-gdb --version
```

### 2. CMake Toolchain Configuration

**Required Toolchain File: `cmake/gcc-arm-none-eabi.cmake`**
```cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Specify the cross compiler
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(CMAKE_SIZE arm-none-eabi-size)

# Compiler flags for STM32H753ZI (Cortex-M7)
set(CMAKE_C_FLAGS_INIT "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -fdata-sections -ffunction-sections")
set(CMAKE_CXX_FLAGS_INIT "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -fdata-sections -ffunction-sections")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections")

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# Search for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
```

### 3. CMake Presets Configuration

**Required: `CMakePresets.json`**
```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "default",
            "hidden": true,
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build/${presetName}",
            "toolchainFile": "${sourceDir}/cmake/gcc-arm-none-eabi.cmake",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug"
            }
        },
        {
            "name": "Debug",
            "inherits": "default",
            "displayName": "Debug Configuration",
            "description": "Debug build with ARM GCC toolchain",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug"
            }
        },
        {
            "name": "Release",
            "inherits": "default",
            "displayName": "Release Configuration", 
            "description": "Release build with ARM GCC toolchain",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release"
            }
        }
    ],
    "buildPresets": [
        {
            "name": "Debug",
            "configurePreset": "Debug"
        },
        {
            "name": "Release", 
            "configurePreset": "Release"
        }
    ]
}
```

## Correct Build Procedure

### Method 1: Using CMake Presets (RECOMMENDED)
```bash
# Clean start (removes any cached configuration)
rm -rf build/

# Configure with preset (automatically uses ARM GCC toolchain)
cmake --preset Debug

# Build
cmake --build build/Debug

# Alternative: Build with preset
cmake --build --preset Debug
```

### Method 2: Manual Toolchain Specification
```bash
# Clean start
rm -rf build/

# Configure with explicit toolchain
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -G Ninja

# Build
cmake --build build
```

### Method 3: Environment-Based (Fallback)
```bash
# Set environment variables
export CC=arm-none-eabi-gcc
export CXX=arm-none-eabi-g++
export AS=arm-none-eabi-as

# Clean and configure
rm -rf build/
cmake -S . -B build -G Ninja

# Build
cmake --build build
```

## VS Code Integration

### Required: `.vscode/cmake-kits.json`
```json
[
    {
        "name": "ARM GCC for STM32H753ZI",
        "compilers": {
            "C": "/usr/bin/arm-none-eabi-gcc",
            "CXX": "/usr/bin/arm-none-eabi-g++"
        },
        "toolchainFile": "${workspaceFolder}/cmake/gcc-arm-none-eabi.cmake"
    }
]
```

### VS Code CMake Extension Setup
1. **Install CMake Tools Extension** (if not already installed)
2. **Select Kit**: `Ctrl+Shift+P` → "CMake: Select a Kit" → "ARM GCC for STM32H753ZI"
3. **Select Preset**: `Ctrl+Shift+P` → "CMake: Select Configure Preset" → "Debug"
4. **Configure**: `Ctrl+Shift+P` → "CMake: Configure"
5. **Build**: `Ctrl+Shift+P` → "CMake: Build" or use build task

## Troubleshooting Guide

### Issue: "cc: error: unrecognized command-line option '-mthumb'"
**Cause**: CMake is using system compiler (gcc) instead of ARM GCC cross-compiler
**Solution**:
```bash
# Verify ARM GCC is found
which arm-none-eabi-gcc

# Clean build directory completely
rm -rf build/

# Use preset method (ensures toolchain file is used)
cmake --preset Debug
```

### Issue: "CMAKE_TOOLCHAIN_FILE was not used by the project"
**Cause**: Toolchain file specified after build directory already configured
**Solution**:
```bash
# MUST clean build directory first
rm -rf build/

# Then configure with toolchain
cmake --preset Debug
```

### Issue: "arm-none-eabi-gcc: command not found"
**Cause**: ARM GCC toolchain not installed or not in PATH
**Solution**:
```bash
# Install ARM GCC toolchain
sudo apt-get update
sudo apt-get install gcc-arm-none-eabi gdb-arm-none-eabi

# Verify installation
arm-none-eabi-gcc --version
```

### Issue: CMake finds wrong compiler version
**Cause**: Multiple GCC installations or PATH issues
**Solution**:
```bash
# Check which compiler CMake will find
which arm-none-eabi-gcc
ls -la /usr/bin/arm-none-eabi-*

# Force specific compiler in toolchain file if needed
set(CMAKE_C_COMPILER /usr/bin/arm-none-eabi-gcc)
```

### Issue: Linker errors about missing libraries
**Cause**: Wrong linker flags or missing nano.specs
**Solution**:
```bash
# Verify ARM GCC has newlib-nano
arm-none-eabi-gcc --print-search-dirs
arm-none-eabi-gcc --print-file-name=libc_nano.a

# Update linker flags in toolchain file
set(CMAKE_EXE_LINKER_FLAGS_INIT "-specs=nano.specs -specs=nosys.specs")
```

## Rapid Recovery Procedure

### When Build System is Broken - Quick Fix
```bash
#!/bin/bash
# save as scripts/fix_cmake.sh

echo "🔧 STM32H753ZI CMake Quick Fix"

# 1. Verify ARM GCC
echo "📋 Checking ARM GCC installation..."
if ! which arm-none-eabi-gcc > /dev/null; then
    echo "❌ ARM GCC not found - installing..."
    sudo apt-get update && sudo apt-get install -y gcc-arm-none-eabi gdb-arm-none-eabi
fi

arm-none-eabi-gcc --version

# 2. Clean build completely
echo "🧹 Cleaning build directory..."
rm -rf build/

# 3. Configure with preset
echo "⚙️ Configuring with Debug preset..."
cmake --preset Debug

# 4. Build
echo "🔨 Building..."
cmake --build build/Debug

echo "✅ Build system fixed!"
```

## VS Code Tasks Integration

### Required: `.vscode/tasks.json` Build Task
```json
{
    "label": "Build (CMake ARM)",
    "type": "shell", 
    "command": "cmake",
    "args": ["--build", "build/Debug"],
    "group": {
        "kind": "build",
        "isDefault": true
    },
    "dependsOrder": "sequence",
    "dependsOn": "Configure (CMake ARM)",
    "options": {
        "cwd": "${workspaceFolder}"
    }
},
{
    "label": "Configure (CMake ARM)",
    "type": "shell",
    "command": "cmake", 
    "args": ["--preset", "Debug"],
    "group": "build",
    "options": {
        "cwd": "${workspaceFolder}"
    }
}
```

## Environment Validation

### Python Virtual Environment Setup
**Required Virtual Environment**: `.venv` in workspace root
```bash
# Check if virtual environment exists
if [ ! -d ".venv" ]; then
    echo "⚠️  Creating Python virtual environment..."
    python3 -m venv .venv
    source .venv/bin/activate
    pip install --upgrade pip
    pip install chromadb requests ollama
    echo "✅ Virtual environment created and packages installed"
else
    echo "✅ Virtual environment exists"
fi

# Always activate virtual environment for development
source .venv/bin/activate
```

**VS Code Integration**: Workspace is configured to automatically activate `.venv`:
- `python.defaultInterpreterPath`: Uses `.venv/bin/python`
- `terminal.integrated.env.linux`: Automatically adds `.venv/bin` to PATH
- `terminal.integrated.profiles.linux`: Auto-activates venv in new terminals

### Pre-Build Checklist Script
```bash
#!/bin/bash
# save as scripts/validate_build_env.sh

echo "🔍 STM32H753ZI Build Environment Validation"

# Check ARM GCC
echo -n "ARM GCC: "
if which arm-none-eabi-gcc > /dev/null; then
    echo "✅ $(arm-none-eabi-gcc --version | head -1)"
else
    echo "❌ Not found"
    exit 1
fi

# Check CMake
echo -n "CMake: "
if which cmake > /dev/null; then
    echo "✅ $(cmake --version | head -1)"
else
    echo "❌ Not found"
    exit 1
fi

# Check Ninja
echo -n "Ninja: "
if which ninja > /dev/null; then
    echo "✅ $(ninja --version)"
else
    echo "❌ Not found - install with: sudo apt-get install ninja-build"
fi

# Check toolchain file
if [ -f "cmake/gcc-arm-none-eabi.cmake" ]; then
    echo "✅ Toolchain file exists"
else
    echo "❌ Toolchain file missing: cmake/gcc-arm-none-eabi.cmake"
    exit 1
fi

# Check preset file
if [ -f "CMakePresets.json" ]; then
    echo "✅ CMake presets configured"
else
    echo "❌ CMake presets missing: CMakePresets.json"
    exit 1
fi

echo "🎯 Build environment ready!"
```

## Container Integration

### Dockerfile ARM GCC Setup
```dockerfile
# Ensure ARM GCC is installed in dev container
RUN apt-get update && apt-get install -y \
    gcc-arm-none-eabi \
    gdb-arm-none-eabi \
    ninja-build \
    && rm -rf /var/lib/apt/lists/*

# Verify installation
RUN arm-none-eabi-gcc --version && ninja --version
```

## Quick Reference Commands

### Daily Development Commands
```bash
# Quick build (assumes environment is set up)
cmake --build build/Debug

# Clean rebuild
rm -rf build/ && cmake --preset Debug && cmake --build build/Debug

# Check toolchain
cmake --preset Debug && grep -A5 "The target system is" build/Debug/CMakeCache.txt

# Validate environment
./scripts/validate_build_env.sh
```

### Emergency Reset
```bash
# Complete reset when everything is broken
rm -rf build/ .vscode/cmake-tools-kits.json
cmake --preset Debug
```

## Integration with SSOT

### Build Configuration SSOT
All build-related configurations should reference:
- `cmake/gcc-arm-none-eabi.cmake` - Primary toolchain configuration
- `CMakePresets.json` - Build preset definitions
- `.vscode/cmake-kits.json` - VS Code integration
- `scripts/validate_build_env.sh` - Environment validation

Never hardcode compiler paths or flags outside these SSOT files.

## Success Indicators

### Working Build System Shows:
```bash
# Successful configure output:
-- The C compiler identification is GNU 10.3.1
-- The ASM compiler identification is GNU
-- Check for working C compiler: /usr/bin/arm-none-eabi-gcc
-- Check for working C compiler: /usr/bin/arm-none-eabi-gcc - works

# Successful build output:
[  1%] Building C object CMakeFiles/stm32h753_ihm02a1.dir/Core/Src/main.c.o
[  2%] Building C object CMakeFiles/stm32h753_ihm02a1.dir/Core/Src/stm32h7xx_hal_msp.c.o
...
[100%] Linking C executable stm32h753_ihm02a1.elf
```

Remember: **Clean build directory first, use presets, verify toolchain installation**
