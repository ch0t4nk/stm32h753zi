#!/bin/bash
# STM32H753ZI Build Environment Validation Script
# Purpose: Verify all build dependencies and configuration are correct
# Usage: ./scripts/validate_build_env.sh

echo "🔍 STM32H753ZI Build Environment Validation"
echo "==========================================="

EXIT_CODE=0

# CRITICAL: Check virtual environment first
echo "🐍 Python Virtual Environment:"
if [ -z "$VIRTUAL_ENV" ]; then
    echo "❌ Virtual environment not activated!"
    echo "   Run: source .venv/bin/activate"
    echo "   Or use: ./scripts/fix_cmake.sh (auto-activates)"
    EXIT_CODE=1
else
    echo "✅ Virtual environment active: $VIRTUAL_ENV"
    
    # Check Python packages
    echo -n "ChromaDB: "
    if python -c "import chromadb" 2>/dev/null; then
        echo "✅ Available"
    else
        echo "❌ Missing (run: pip install chromadb)"
        EXIT_CODE=1
    fi
    
    echo -n "Requests: "
    if python -c "import requests" 2>/dev/null; then
        echo "✅ Available"
    else
        echo "❌ Missing (run: pip install requests)"
        EXIT_CODE=1
    fi
fi

echo ""

# Helper function to check command availability
check_command() {
    local cmd=$1
    local package=$2
    
    echo -n "$cmd: "
    if which "$cmd" > /dev/null 2>&1; then
        echo "✅ Available"
        return 0
    else
        echo "❌ Not found"
        if [ -n "$package" ]; then
            echo "   Install with: sudo apt-get install $package"
        fi
        return 1
    fi
}

# Check ARM GCC toolchain
echo "📋 ARM GCC Toolchain:"
if which arm-none-eabi-gcc > /dev/null 2>&1; then
    echo "✅ arm-none-eabi-gcc: $(arm-none-eabi-gcc --version | head -1)"
    
    # Check specific tools
    check_command "arm-none-eabi-g++" || EXIT_CODE=1
    check_command "arm-none-eabi-gdb" || EXIT_CODE=1
    check_command "arm-none-eabi-objcopy" || EXIT_CODE=1
    check_command "arm-none-eabi-size" || EXIT_CODE=1
    
    # Check if newlib-nano is available
    echo -n "newlib-nano: "
    if find /usr -name "*nano.specs" 2>/dev/null | grep -q nano.specs; then
        echo "✅ Available"
    else
        echo "❌ Not found (specs may fail)"
        EXIT_CODE=1
    fi
else
    echo "❌ arm-none-eabi-gcc: Not found"
    echo "   Install with: sudo apt-get install gcc-arm-none-eabi gdb-arm-none-eabi"
    EXIT_CODE=1
fi

echo ""

# Check build tools
echo "📋 Build Tools:"
check_command "cmake" "cmake" || EXIT_CODE=1
check_command "ninja" "ninja-build" || EXIT_CODE=1
check_command "make" "build-essential" || EXIT_CODE=1

echo ""

# Check workspace configuration files
echo "📋 Workspace Configuration:"
REQUIRED_FILES=(
    "CMakeLists.txt:Main build configuration"
    "CMakePresets.json:CMake presets"
    "cmake/gcc-arm-none-eabi.cmake:ARM GCC toolchain file"
    ".vscode/cmake-kits.json:VS Code CMake kits"
    ".vscode/tasks.json:VS Code build tasks"
    "STM32H753XX_FLASH.ld:Linker script"
    "startup_stm32h753xx.s:Startup assembly"
)

for file_desc in "${REQUIRED_FILES[@]}"; do
    file="${file_desc%%:*}"
    desc="${file_desc##*:}"
    
    echo -n "$file: "
    if [ -f "$file" ]; then
        echo "✅ Present ($desc)"
    else
        echo "❌ Missing ($desc)"
        EXIT_CODE=1
    fi
done

echo ""

# Check VS Code CMake Tools extension requirements
echo "📋 VS Code Integration:"
if [ -f ".vscode/cmake-kits.json" ]; then
    echo -n "CMake Kits: "
    if grep -q "ARM GCC for STM32H753ZI" .vscode/cmake-kits.json; then
        echo "✅ ARM GCC kit configured"
    else
        echo "❌ ARM GCC kit not found in cmake-kits.json"
        EXIT_CODE=1
    fi
else
    echo "❌ .vscode/cmake-kits.json: Missing (VS Code CMake integration will not work)"
    EXIT_CODE=1
fi

if [ -f "CMakePresets.json" ]; then
    echo -n "CMake Presets: "
    if grep -q "gcc-arm-none-eabi.cmake" CMakePresets.json; then
        echo "✅ ARM GCC toolchain preset configured"
    else
        echo "❌ ARM GCC toolchain not referenced in presets"
        EXIT_CODE=1
    fi
fi

echo ""

# Check source code structure
echo "📋 Source Code Structure:"
REQUIRED_DIRS=(
    "src:Source code directory"
    "Core/Inc:STM32 HAL includes"
    "Core/Src:STM32 HAL sources"
    "tests:Test directory"
)

for dir_desc in "${REQUIRED_DIRS[@]}"; do
    dir="${dir_desc%%:*}"
    desc="${dir_desc##*:}"
    
    echo -n "$dir/: "
    if [ -d "$dir" ]; then
        echo "✅ Present ($desc)"
    else
        echo "⚠️  Missing ($desc) - may be optional"
    fi
done

echo ""

# Environment summary
echo "📋 Environment Summary:"
if [ $EXIT_CODE -eq 0 ]; then
    echo "✅ Build environment is correctly configured"
    echo "🎯 Ready to build STM32H753ZI firmware!"
    echo ""
    echo "💡 Quick start commands:"
    echo "   cmake --preset Debug        # Configure for debug build"
    echo "   cmake --build build/Debug   # Build firmware"
    echo "   ./scripts/fix_cmake.sh      # Fix any configuration issues"
else
    echo "❌ Build environment has issues that need to be resolved"
    echo "🔧 Run './scripts/fix_cmake.sh' to attempt automatic fixes"
fi

echo ""

# Additional diagnostics if requested
if [ "$1" = "--verbose" ] || [ "$1" = "-v" ]; then
    echo "📋 Detailed Diagnostics:"
    echo "Working directory: $(pwd)"
    echo "PATH: $PATH"
    echo ""
    
    if which arm-none-eabi-gcc > /dev/null; then
        echo "ARM GCC search paths:"
        arm-none-eabi-gcc --print-search-dirs | head -5
        echo ""
    fi
    
    if which cmake > /dev/null; then
        echo "CMake version details:"
        cmake --version
        echo ""
    fi
    
    echo "Git status:"
    git status --porcelain | head -10
fi

exit $EXIT_CODE
