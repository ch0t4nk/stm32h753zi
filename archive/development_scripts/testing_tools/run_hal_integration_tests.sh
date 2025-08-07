#!/bin/bash
# HAL Abstraction Host Tests Compilation Script
# Builds host-based integration tests for HAL abstraction with mock implementation

set -e  # Exit on any error

WORKSPACE_ROOT="/workspaces/code"
TEST_DIR="${WORKSPACE_ROOT}/tests"
HAL_DIR="${WORKSPACE_ROOT}/src/hal_abstraction"
COMMON_DIR="${WORKSPACE_ROOT}/src/common"
UNITY_DIR="${WORKSPACE_ROOT}/external/unity"

# Ensure Unity is available
if [ ! -d "$UNITY_DIR" ]; then
    echo "❌ Unity framework not found at $UNITY_DIR"
    echo "Please install Unity testing framework first"
    exit 1
fi

# Include directories for compilation
INCLUDE_DIRS="-I${WORKSPACE_ROOT}/src"
INCLUDE_DIRS="$INCLUDE_DIRS -I${WORKSPACE_ROOT}/src/config"
INCLUDE_DIRS="$INCLUDE_DIRS -I${WORKSPACE_ROOT}/src/common"
INCLUDE_DIRS="$INCLUDE_DIRS -I${UNITY_DIR}"

# Compiler flags for host testing
CFLAGS="-Wall -Wextra -DUNITY_TESTING -std=c99 -g -O0"
CFLAGS="$CFLAGS -Wno-unused-parameter"  # Suppress warnings for mock unused parameters

echo "==== HAL Abstraction Integration Tests ===="
echo "Workspace: $WORKSPACE_ROOT"
echo "Unity: $UNITY_DIR"
echo "Include dirs: $INCLUDE_DIRS"
echo "Compiler flags: $CFLAGS"
echo ""

# Create temporary build directory
BUILD_DIR="/tmp/hal_tests_build"
mkdir -p "$BUILD_DIR"

echo "🔧 Compiling Unity framework..."
gcc $CFLAGS $INCLUDE_DIRS -c "${UNITY_DIR}/unity.c" -o "${BUILD_DIR}/unity.o"

echo "🔧 Compiling HAL abstraction mock..."
gcc $CFLAGS $INCLUDE_DIRS -c "${HAL_DIR}/hal_abstraction_mock.c" -o "${BUILD_DIR}/hal_abstraction_mock.o"

echo "🔧 Compiling integration tests..."
gcc $CFLAGS $INCLUDE_DIRS -c "${TEST_DIR}/test_hal_abstraction_integration.c" -o "${BUILD_DIR}/test_hal_abstraction_integration.o"

echo "🔗 Linking test executable..."
gcc "${BUILD_DIR}/unity.o" \
    "${BUILD_DIR}/hal_abstraction_mock.o" \
    "${BUILD_DIR}/test_hal_abstraction_integration.o" \
    -o "${BUILD_DIR}/test_hal_abstraction_integration"

if [ $? -eq 0 ]; then
    echo "✅ Integration tests compiled successfully"
    
    # Run the tests
    echo ""
    echo "🚀 Running HAL abstraction integration tests..."
    "${BUILD_DIR}/test_hal_abstraction_integration"
    
    test_result=$?
    if [ $test_result -eq 0 ]; then
        echo ""
        echo "🎉 All HAL abstraction integration tests passed!"
    else
        echo ""
        echo "❌ Some integration tests failed (exit code: $test_result)"
        exit $test_result
    fi
else
    echo "❌ Integration tests compilation failed"
    exit 1
fi

# Clean up build directory
echo ""
echo "🧹 Cleaning up..."
rm -rf "$BUILD_DIR"

echo "✅ HAL abstraction integration testing complete!"
