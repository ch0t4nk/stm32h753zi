#!/bin/bash
# Host-based Unit Test Builder and Runner
# This script compiles and runs unit tests using the host compiler (gcc)
# rather than the ARM cross-compiler
# 
# Part of the dual-build system - handles host testing compilation separately
# from ARM firmware compilation

set -e

echo "🧪 STM32H753ZI Host-based Unit Tests (Dual-Build System)"
echo "========================================================"

# Configuration
WORKSPACE_ROOT="/workspaces/code"
HOST_BUILD_DIR="${WORKSPACE_ROOT}/build_host_tests"
HOST_TESTS_DIR="${WORKSPACE_ROOT}/host_tests"
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

# Check if we should use CMake or direct compilation
if [ -f "${HOST_TESTS_DIR}/CMakeLists.txt" ]; then
    echo "🔧 Using CMake-based host testing (dual-build system)..."
    
    # Configure host tests with CMake
    cd "${HOST_TESTS_DIR}"
    cmake -S . -B "${HOST_BUILD_DIR}" -DCMAKE_BUILD_TYPE=Debug
    
    if [ $? -ne 0 ]; then
        echo "❌ CMake configuration failed"
        exit 1
    fi
    
    # Build host tests
    cmake --build "${HOST_BUILD_DIR}"
    
    if [ $? -ne 0 ]; then
        echo "❌ CMake build failed"
        exit 1
    fi
    
    echo "🏃 Running host-based unit tests via CTest..."
    cd "${HOST_BUILD_DIR}"
    
    # Run tests with CTest
    ctest --output-on-failure --verbose
    
    if [ $? -eq 0 ]; then
        echo "✅ All CMake-based tests passed!"
    else
        echo "❌ Some CMake-based tests failed"
        exit 1
    fi

else
    echo "🔧 Using direct GCC compilation (legacy mode)..."

    # Common include directories
    INCLUDES=(
        "-I${WORKSPACE_ROOT}/src"
        "-I${WORKSPACE_ROOT}/src/config"
        "-I${WORKSPACE_ROOT}/src/common"
        "-I${WORKSPACE_ROOT}/src/safety"
        "-I${WORKSPACE_ROOT}/src/controllers"
        "-I${WORKSPACE_ROOT}/src/telemetry"
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
        "-DSIMULATION_ENABLED=1"
        "-DTEST_MODE=1"
        "-DSTM32H753xx"
        "-DUSE_HAL_DRIVER"
    )

    # Function to compile and run a test
    compile_and_run_test() {
        local test_name="$1"
        shift
        local test_sources=("$@")
        
        echo "📦 Building ${test_name}..."
        
        # Check if all source files exist
        for src in "${test_sources[@]}"; do
            if [ ! -f "$src" ]; then
                echo "❌ Source file not found: $src"
                return 1
            fi
        done
        
        # Compile the test executable
        gcc "${CFLAGS[@]}" "${INCLUDES[@]}" "${test_sources[@]}" \
            -o "${HOST_BUILD_DIR}/${test_name}" -lm
        
        if [ $? -eq 0 ]; then
            echo "✅ ${test_name} compilation successful"
        else
            echo "❌ ${test_name} compilation failed"
            return 1
        fi
        
        echo "🏃 Running ${test_name}..."
        cd "${HOST_BUILD_DIR}"
        if "./${test_name}"; then
            echo "✅ ${test_name} passed!"
            return 0
        else
            echo "❌ ${test_name} failed"
            return 1
        fi
    }

    # Legacy safety test
    if [ -f "${TEST_SOURCE}/unit/test_safety_systems_hal_abstracted.c" ]; then
        TEST_SOURCES_SAFETY=(
            "${TEST_SOURCE}/unit/test_safety_systems_hal_abstracted.c"
            "${TEST_SOURCE}/mocks/mock_hal_abstraction.c"
            "${WORKSPACE_ROOT}/src/safety/emergency_stop_abstracted.c"
            "${UNITY_SOURCE}/unity.c"
        )
        compile_and_run_test "test_safety_systems_hal_abstracted" "${TEST_SOURCES_SAFETY[@]}"
    fi

    # Phase 1.3 Simplified Validation test
    if [ -f "${TEST_SOURCE}/unit/test_phase1_3_validation.c" ]; then
        TEST_SOURCES_PHASE1_3=(
            "${TEST_SOURCE}/unit/test_phase1_3_validation.c"
            "${TEST_SOURCE}/mocks/mock_hal_abstraction.c"
            "${UNITY_SOURCE}/unity.c"
        )
        compile_and_run_test "test_phase1_3_validation" "${TEST_SOURCES_PHASE1_3[@]}"
    fi

    # Phase 1.3 Full Telemetry test (disabled until dependencies ready)
    # if [ -f "${TEST_SOURCE}/unit/test_optimization_telemetry.c" ]; then
    #     TEST_SOURCES_TELEMETRY=(
    #         "${TEST_SOURCE}/unit/test_optimization_telemetry.c"
    #         "${TEST_SOURCE}/mocks/mock_hal_abstraction.c"
    #         "${WORKSPACE_ROOT}/src/telemetry/optimization_telemetry.c"
    #         "${UNITY_SOURCE}/unity.c"
    #     )
    #     compile_and_run_test "test_optimization_telemetry" "${TEST_SOURCES_TELEMETRY[@]}"
    # fi

    # Phase 1.3 Full Motor Characterization test (disabled until dependencies ready)
    # if [ -f "${TEST_SOURCE}/unit/test_motor_characterization.c" ]; then
    #     TEST_SOURCES_CHARACTERIZATION=(
    #         "${TEST_SOURCE}/unit/test_motor_characterization.c"
    #         "${TEST_SOURCE}/mocks/mock_hal_abstraction.c"
    #         "${WORKSPACE_ROOT}/src/controllers/motor_characterization.c"
    #         "${WORKSPACE_ROOT}/src/telemetry/optimization_telemetry.c"
    #         "${UNITY_SOURCE}/unity.c"
    #     )
    #     compile_and_run_test "test_motor_characterization" "${TEST_SOURCES_CHARACTERIZATION[@]}"
    # fi

fi

echo ""
echo "🎉 Host-based unit tests completed successfully!"
echo "📁 Test executables: ${HOST_BUILD_DIR}/"
echo "🔄 Dual-build system: Host testing complete, ARM firmware build separate"
