#!/bin/bash

# Permanent IWDG Build Issue Fix
# Addresses CMake dependency file generation problems

echo "🔧 Permanent IWDG Build Issue Fix"
echo "=================================="

# 1. Clean all build artifacts completely
echo "🧹 Cleaning all build artifacts..."
rm -rf build/ build_host_tests/
rm -rf CMakeCache.txt cmake_install.cmake
find . -name "*.obj" -delete 2>/dev/null || true
find . -name "*.d" -delete 2>/dev/null || true

# 2. Create comprehensive build directory structure
echo "📁 Creating comprehensive build directory structure..."
mkdir -p build/CMakeFiles/stm32h753_ihm02a1.dir/Core/Src
mkdir -p build/CMakeFiles/stm32h753_ihm02a1.dir/drivers/BSP/STM32H7xx_Nucleo
mkdir -p build/CMakeFiles/stm32h753_ihm02a1.dir/drivers/STM32H7xx_HAL_Driver/Src
mkdir -p build/CMakeFiles/stm32h753_ihm02a1.dir/src
mkdir -p build/CMakeFiles/unity.dir
mkdir -p build/CMakeFiles/test_safety_systems_hal_abstracted.dir
mkdir -p build/CMakeFiles/test_optimization_telemetry.dir

# 3. Set proper permissions recursively
echo "🔐 Setting proper permissions..."
chmod -R 755 build/
chmod 755 . 2>/dev/null || true

# 4. Fix CMake generator issues
echo "⚙️ Configuring CMake with proper settings..."
export CC=arm-none-eabi-gcc
export CXX=arm-none-eabi-g++
export AR=arm-none-eabi-ar

# 5. Configure with specific flags to avoid dependency issues
cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_VERBOSE_MAKEFILE=OFF \
    -DCMAKE_DEPENDS_USE_COMPILER=OFF \
    -G "Unix Makefiles"

if [ $? -eq 0 ]; then
    echo "✅ CMake configuration successful"
    
    echo "🔨 Building with dependency tracking fixes..."
    cmake --build build -- -j1  # Single-threaded to avoid race conditions
    
    if [ $? -eq 0 ]; then
        echo "🎉 Build successful! IWDG issue resolved."
        
        # Verify the problematic file was built
        if [ -f "build/CMakeFiles/stm32h753_ihm02a1.dir/drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.c.obj" ]; then
            echo "✅ IWDG object file created successfully"
        fi
        
        if [ -f "build/CMakeFiles/stm32h753_ihm02a1.dir/drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.c.obj.d" ]; then
            echo "✅ IWDG dependency file created successfully"
        fi
        
    else
        echo "❌ Build failed - investigating further..."
        echo "📋 Checking build directory structure:"
        ls -la build/CMakeFiles/stm32h753_ihm02a1.dir/drivers/STM32H7xx_HAL_Driver/Src/ 2>/dev/null || echo "Directory not found"
    fi
else
    echo "❌ CMake configuration failed"
fi

echo ""
echo "🎯 IWDG Build Fix Complete"
