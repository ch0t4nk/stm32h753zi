#!/bin/bash
# STM32H753ZI CMake Quick Fix Script
# Purpose: Rapidly recover from CMake configuration issues
# Usage: ./scripts/fix_cmake.sh

set -e  # Exit on error

# 0. OS Detection - Auto-redirect to PowerShell version if on Windows
if [[ "$OSTYPE" == "msys"* ]] || [[ "$OSTYPE" == "cygwin"* ]] || [[ -n "$WINDIR" ]] || [[ -n "$SYSTEMROOT" ]]; then
    echo "🪟 Windows environment detected - redirecting to PowerShell version..."
    if [ -f "scripts/fix_cmake.ps1" ]; then
        powershell.exe -ExecutionPolicy Bypass -File "scripts/fix_cmake.ps1"
        exit $?
    else
        echo "❌ PowerShell version not found at scripts/fix_cmake.ps1"
        echo "💡 Running Linux version on Windows (may have path issues)"
    fi
fi

echo "🔧 STM32H753ZI CMake Quick Fix"
echo "=============================="

# 0. CRITICAL: Ensure virtual environment is activated
echo "🐍 Checking Python virtual environment..."
if [ -z "$VIRTUAL_ENV" ]; then
    echo "❌ Virtual environment not activated!"
    echo "📋 Activating virtual environment..."
    if [ -f ".venv/bin/activate" ]; then
        source .venv/bin/activate
        echo "✅ Virtual environment activated: $VIRTUAL_ENV"
    else
        echo "❌ Virtual environment not found at .venv/"
        echo "📋 Creating virtual environment..."
        python3 -m venv .venv
        source .venv/bin/activate
        echo "✅ Virtual environment created and activated: $VIRTUAL_ENV"
    fi
else
    echo "✅ Virtual environment already active: $VIRTUAL_ENV"
fi

# Ensure Python dependencies are installed
echo "📦 Checking Python dependencies..."
if ! python -c "import chromadb, requests" 2>/dev/null; then
    echo "📦 Installing required Python packages..."
    pip install --quiet chromadb requests ollama
    echo "✅ Python dependencies installed"
fi

# Setup Ollama service and model
echo "🤖 Checking Ollama service and model setup..."
if ! pgrep -f "ollama serve" > /dev/null; then
    echo "🚀 Starting Ollama service..."
    ollama serve > /dev/null 2>&1 &
    sleep 3
    echo "✅ Ollama service started"
else
    echo "✅ Ollama service already running"
fi

# Check if required model is available
echo "📥 Checking for mxbai-embed-large model..."
if ! ollama list | grep -q "mxbai-embed-large"; then
    echo "⬇️  Downloading mxbai-embed-large model (this may take a few minutes)..."
    ollama pull mxbai-embed-large
    echo "✅ Model downloaded successfully"
else
    echo "✅ mxbai-embed-large model already available"
fi

# Test Ollama connection
echo "🔍 Testing Ollama connection..."
if curl -s http://localhost:11434/api/tags > /dev/null; then
    echo "✅ Ollama API accessible"
else
    echo "⚠️  Ollama API not accessible - semantic search will use mock embeddings"
fi

# 1. Verify ARM GCC installation
echo "📋 Checking ARM GCC installation..."
if ! which arm-none-eabi-gcc > /dev/null; then
    echo "❌ ARM GCC not found - installing..."
    sudo apt-get update && sudo apt-get install -y gcc-arm-none-eabi gdb-arm-none-eabi ninja-build
    echo "✅ ARM GCC installed"
else
    echo "✅ ARM GCC found: $(arm-none-eabi-gcc --version | head -1)"
fi

# 2. Verify required tools
echo "📋 Checking required build tools..."
MISSING_TOOLS=()

if ! which cmake > /dev/null; then
    MISSING_TOOLS+=("cmake")
fi

if ! which ninja > /dev/null; then
    MISSING_TOOLS+=("ninja-build")
fi

if [ ${#MISSING_TOOLS[@]} -ne 0 ]; then
    echo "❌ Missing tools: ${MISSING_TOOLS[*]}"
    
    # Special handling for ninja - we can use Make as fallback
    CRITICAL_MISSING=()
    for tool in "${MISSING_TOOLS[@]}"; do
        if [ "$tool" != "ninja-build" ]; then
            CRITICAL_MISSING+=("$tool")
        fi
    done
    
    if [ ${#CRITICAL_MISSING[@]} -ne 0 ]; then
        echo "❌ Critical tools missing: ${CRITICAL_MISSING[*]}"
        if which sudo > /dev/null; then
            echo "🔧 Installing critical tools..."
            sudo apt-get update && sudo apt-get install -y "${CRITICAL_MISSING[@]}"
            echo "✅ Critical tools installed"
        else
            echo "💀 FATAL: Critical tools missing and cannot install (no sudo access)"
            echo "📋 Please install manually: ${CRITICAL_MISSING[*]}"
            exit 1
        fi
    fi
    
    # Handle ninja specially - optional but preferred
    if [[ " ${MISSING_TOOLS[*]} " =~ " ninja-build " ]]; then
        echo "⚠️  Ninja not available - will use Make generator as fallback"
        USE_MAKE_FALLBACK=true
    fi
else
    echo "✅ All required tools available"
    USE_MAKE_FALLBACK=false
fi

# 3. Verify workspace files
echo "📋 Checking workspace configuration..."
REQUIRED_FILES=(
    "cmake/gcc-arm-none-eabi.cmake"
    "CMakePresets.json"
    "CMakeLists.txt"
)

for file in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        echo "❌ Required file missing: $file"
        echo "   Please ensure you're in the correct workspace directory"
        exit 1
    fi
done
echo "✅ All required workspace files present"

# 4. Clean build directory completely
echo "🧹 Cleaning build directory..."
if [ -d "build" ]; then
    rm -rf build/
    echo "✅ Build directory cleaned"
else
    echo "✅ Build directory already clean"
fi

# 5. Remove VS Code CMake cache (if present)
if [ -f ".vscode/cmake-tools-kits.json" ]; then
    echo "🧹 Cleaning VS Code CMake cache..."
    rm -f .vscode/cmake-tools-kits.json
    echo "✅ VS Code CMake cache cleaned"
fi

# 6. Configure with appropriate generator
echo "⚙️ Configuring CMake..."
if [ "$USE_MAKE_FALLBACK" = "true" ]; then
    echo "📋 Using Make generator (ninja not available)..."
    if cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug; then
        echo "✅ CMake configuration (Make) successful"
        BUILD_DIR="build"
    else
        echo "❌ CMake configuration failed"
        exit 1
    fi
else
    echo "📋 Trying Ninja generator first..."
    if cmake --preset Debug; then
        echo "✅ CMake configuration (Ninja) successful"
        BUILD_DIR="build/Debug"
    else
        echo "❌ Ninja preset failed, trying Make fallback..."
        if cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug; then
            echo "✅ CMake configuration (Make fallback) successful"
            BUILD_DIR="build"
        else
            echo "❌ Both configuration methods failed"
            echo "📋 Environment details:"
            echo "   ARM GCC: $(which arm-none-eabi-gcc)"
            echo "   CMake: $(which cmake)"
            echo "   Ninja: $(which ninja 2>/dev/null || echo 'not found')"
            echo "   Make: $(which make 2>/dev/null || echo 'not found')"
            echo "   Working directory: $(pwd)"
            echo "   Virtual env: $VIRTUAL_ENV"
            exit 1
        fi
    fi
fi

# 7. Build
echo "🔨 Building..."
if cmake --build "$BUILD_DIR"; then
    echo "✅ Build successful!"
    
    # Show build artifact
    if [ -f "$BUILD_DIR/stm32h753_ihm02a1.elf" ]; then
        echo "📦 Generated: $BUILD_DIR/stm32h753_ihm02a1.elf"
        arm-none-eabi-size "$BUILD_DIR/stm32h753_ihm02a1.elf"
    fi
else
    echo "❌ Build failed"
    echo "📋 Check build output above for specific errors"
    exit 1
fi

echo ""
echo "🎉 CMake build system successfully fixed and working!"
echo "🐍 Virtual environment: $VIRTUAL_ENV"
echo "� Build directory: $BUILD_DIR"
echo ""
echo "�💡 To use in VS Code:"
echo "   1. Ctrl+Shift+P → 'CMake: Select a Kit' → 'ARM GCC for STM32H753ZI'"
echo "   2. Ctrl+Shift+P → 'CMake: Select Configure Preset' → 'Debug'"
echo "   3. Use VS Code build tasks or Ctrl+Shift+P → 'CMake: Build'"
echo ""
echo "🔧 Daily commands:"
echo "   cmake --build $BUILD_DIR           # Quick build"
if [ "$USE_MAKE_FALLBACK" = "true" ]; then
    echo "   cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake -G 'Unix Makefiles' && cmake --build build  # Clean rebuild"
else
    echo "   cmake --preset Debug && cmake --build build/Debug  # Clean rebuild"
fi
