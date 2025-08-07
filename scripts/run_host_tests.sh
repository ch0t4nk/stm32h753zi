#!/bin/bash
# Host-based Unit Test Builder and Runner
# This script compiles and runs unit tests using the host compiler (gcc)
# rather than the ARM cross-compiler

set -e

echo "🧪 STM32H753ZI Host-based Unit Tests"
echo "===================================="

# Configuration
WORKSPACE_ROOT="/workspaces/code"
HOST_BUILD_DIR="${WORKSPACE_ROOT}/build_host_tests"
UNITY_SOURCE="${WORKSPACE_ROOT}/external/unity"
TEST_SOURCE="${WORKSPACE_ROOT}/tests"

# Activate virtual environment if available
if [ -f "${WORKSPACE_ROOT}/.venv/bin/activate" ]; then
    source "${WORKSPACE_ROOT}/.venv/bin/activate"
    echo "✅ Virtual environment activated"
fi

# Create host build directory
mkdir -p "${HOST_BUILD_DIR}"

echo "🧹 Cleaning previous host test builds..."
rm -rf "${HOST_BUILD_DIR}"/*

echo "🔧 Compiling host-based unit tests..."

# Common include directories
INCLUDES=(
    "-I${WORKSPACE_ROOT}/src"
    "-I${WORKSPACE_ROOT}/src/config"
    "-I${WORKSPACE_ROOT}/src/common"
    "-I${WORKSPACE_ROOT}/src/safety"
    "-I${WORKSPACE_ROOT}/src/hal_abstraction"
    "-I${WORKSPACE_ROOT}/tests/mocks"
    "-I${UNITY_SOURCE}"
)

# Common compiler flags for host testing
CFLAGS=(
    "-std=gnu11"
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Wno-unused-parameter"
    "-g3"
    "-O0"
    "-DUNITY_TESTING=1"
    "-DHOST_TESTING=1"
    "-DSTM32H753xx"
    "-DUSE_HAL_DRIVER"
)

# Source files for HAL abstracted safety test
TEST_SOURCES=(
    "${TEST_SOURCE}/unit/test_safety_systems_hal_abstracted.c"
    "${TEST_SOURCE}/mocks/mock_hal_abstraction.c"
    "${WORKSPACE_ROOT}/src/safety/emergency_stop_abstracted.c"
    "${UNITY_SOURCE}/unity.c"
)

echo "📦 Building test_safety_systems_hal_abstracted..."

# Check if all source files exist
for src in "${TEST_SOURCES[@]}"; do
    if [ ! -f "$src" ]; then
        echo "❌ Source file not found: $src"
        exit 1
    fi
done

# Compile the test executable
gcc "${CFLAGS[@]}" "${INCLUDES[@]}" "${TEST_SOURCES[@]}" \
    -o "${HOST_BUILD_DIR}/test_safety_systems_hal_abstracted"

if [ $? -eq 0 ]; then
    echo "✅ Test compilation successful"
else
    echo "❌ Test compilation failed"
    exit 1
fi

echo "🏃 Running host-based unit tests..."

# Run the test
cd "${HOST_BUILD_DIR}"
if ./test_safety_systems_hal_abstracted; then
    echo "✅ All tests passed!"
else
    echo "❌ Some tests failed"
    exit 1
fi

echo ""
echo "🎉 Host-based unit tests completed successfully!"
echo "📁 Test executable: ${HOST_BUILD_DIR}/test_safety_systems_hal_abstracted"
