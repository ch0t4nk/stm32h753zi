#!/bin/bash
# Host Test Compilation and Execution Script
# Builds and runs HAL abstraction tests using host compiler (gcc)

set -e  # Exit on any error

WORKSPACE_ROOT="/workspaces/code"
TEST_DIR="${WORKSPACE_ROOT}/tests/mocks"
HAL_DIR="${WORKSPACE_ROOT}/src/hal_abstraction"
COMMON_DIR="${WORKSPACE_ROOT}/src/common"

# Ensure test directory exists
mkdir -p "$TEST_DIR"

# Include directories for compilation
INCLUDE_DIRS="-I${WORKSPACE_ROOT}/src -I${WORKSPACE_ROOT}/src/config -I${WORKSPACE_ROOT}/src/common"

# Compiler flags for host testing
CFLAGS="-Wall -Wextra -DUNITY_TESTING -std=c99 -g -O0"

echo "==== HAL Abstraction Mock Compilation Test ===="
echo "Workspace: $WORKSPACE_ROOT"
echo "Include dirs: $INCLUDE_DIRS"
echo "Compiler flags: $CFLAGS"
echo ""

# Test 1: Compile just the mock implementation
echo "🔧 Testing mock HAL abstraction compilation..."
gcc $CFLAGS $INCLUDE_DIRS -c "${HAL_DIR}/hal_abstraction_mock.c" -o /tmp/hal_mock.o

if [ $? -eq 0 ]; then
    echo "✅ Mock HAL abstraction compiles successfully"
else
    echo "❌ Mock HAL abstraction compilation failed"
    exit 1
fi

# Test 2: Check if we can create a simple test program
echo ""
echo "🧪 Creating simple test program..."

cat > /tmp/test_hal_mock.c << 'EOF'
#include "hal_abstraction/hal_abstraction.h"
#include <stdio.h>
#include <assert.h>

// Forward declare mock control functions
void HAL_Abstraction_Mock_Reset(void);
void HAL_Abstraction_Mock_SetGPIOState(HAL_GPIO_Port_t port, uint32_t pin, HAL_GPIO_State_t state);

int main() {
    printf("Testing HAL Abstraction Mock...\n");
    
    // Reset mock state
    HAL_Abstraction_Mock_Reset();
    
    // Test GPIO write - using pin number 0 instead of GPIO_PIN_0 constant
    SystemError_t result = HAL_Abstraction_GPIO_Write(HAL_GPIO_PORT_A, 0, HAL_GPIO_STATE_SET);
    assert(result == SYSTEM_OK);
    
    // Test GPIO read
    HAL_GPIO_State_t state;
    HAL_Abstraction_Mock_SetGPIOState(HAL_GPIO_PORT_A, 0, HAL_GPIO_STATE_SET);
    result = HAL_Abstraction_GPIO_Read(HAL_GPIO_PORT_A, 0, &state);
    assert(result == SYSTEM_OK);
    assert(state == HAL_GPIO_STATE_SET);
    
    // Test timing functions
    uint32_t tick = HAL_Abstraction_GetTick();
    printf("Current tick: %u\n", tick);
    
    HAL_Abstraction_Delay(10);
    uint32_t new_tick = HAL_Abstraction_GetTick();
    assert(new_tick >= tick + 10);
    
    printf("✅ All basic HAL mock tests passed!\n");
    return 0;
}
EOF

# Compile the test program
echo "🔨 Compiling test program..."
gcc $CFLAGS $INCLUDE_DIRS /tmp/test_hal_mock.c "${HAL_DIR}/hal_abstraction_mock.c" -o /tmp/test_hal_mock

if [ $? -eq 0 ]; then
    echo "✅ Test program compiles successfully"
    
    # Run the test
    echo ""
    echo "🚀 Running test program..."
    /tmp/test_hal_mock
    
    if [ $? -eq 0 ]; then
        echo "✅ Test program runs successfully"
    else
        echo "❌ Test program failed during execution"
        exit 1
    fi
else
    echo "❌ Test program compilation failed"
    exit 1
fi

# Clean up
rm -f /tmp/hal_mock.o /tmp/test_hal_mock.c /tmp/test_hal_mock

echo ""
echo "🎉 HAL Abstraction Mock validation complete!"
echo "    - Mock compiles without errors"
echo "    - Basic functionality works"
echo "    - Ready for full test integration"
