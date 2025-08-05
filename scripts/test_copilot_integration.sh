#!/bin/bash

# VS Code Copilot Integration Test - Instruction File Scoping Validation
# This script validates that instruction files correctly match their target patterns

echo "🧪 VS Code Copilot Integration Test - Instruction File Scoping"
echo "=============================================================="
echo ""

# Function to test file pattern matching
test_pattern_match() {
    local instruction_file="$1"
    local pattern="$2"
    local test_file="$3"
    local expected="$4"
    
    echo "📋 Testing: $instruction_file"
    echo "   Pattern: $pattern"
    echo "   Test File: $test_file"
    echo "   Expected: $expected"
    
    # Check if file exists
    if [[ -f "$test_file" ]]; then
        echo "   ✅ Test file exists"
        
        # Simulate pattern matching (basic shell globbing test)
        # Note: This is a simplified test - VS Code uses more sophisticated matching
        case "$test_file" in
            $pattern) echo "   ✅ Pattern matches" ;;
            *) echo "   ❌ Pattern does NOT match" ;;
        esac
    else
        echo "   ❌ Test file does NOT exist"
    fi
    echo ""
}

echo "🔍 Testing Instruction File Patterns:"
echo ""

# Test 1: Hardware Instructions
test_pattern_match \
    "hardware.instructions.md" \
    "src/drivers/**/*.{c,h}" \
    "src/drivers/l6470/test_hardware_scope.c" \
    "GPIO control, memory management, STM32H7 HAL"

# Test 2: Communication Instructions  
test_pattern_match \
    "comms.instructions.md" \
    "src/communication/**/*.{c,h}" \
    "src/communication/test_comms_scope.c" \
    "Communication protocols, UART/CAN/SPI/I2C"

# Test 3: Core Software Instructions
test_pattern_match \
    "core-software.instructions.md" \
    "src/{common,controllers,drivers}/**/*.{c,h}" \
    "src/controllers/test_core_scope.h" \
    "Data types, error handling, state management"

# Test 4: Safety Real-Time Instructions
test_pattern_match \
    "safety-rt.instructions.md" \
    "src/{safety,controllers}/**/*.{c,h}" \
    "src/safety/test_safety_scope.c" \
    "Safety systems, real-time control, timing"

# Test 5: API/UI Instructions
test_pattern_match \
    "api-ui.instructions.md" \
    "src/{communication,application}/**/*.{c,h}" \
    "src/application/test_api_scope.h" \
    "API design, user interfaces, documentation"

# Test 6: Project Setup Instructions
test_pattern_match \
    "project-setup.instructions.md" \
    "**/*.{c,h,md,cmake,txt,json,yml,yaml}" \
    "test_project_scope.c" \
    "Workspace setup, build configuration, testing"

echo "🔄 Testing Overlapping Patterns:"
echo ""

echo "📋 Controllers Directory Overlap Test:"
echo "   File: src/controllers/test_overlap_controllers.c"
echo "   Should match BOTH:"
echo "   - core-software.instructions.md (src/{common,controllers,drivers}/**/*.{c,h})"
echo "   - safety-rt.instructions.md (src/{safety,controllers}/**/*.{c,h})"
echo "   Expected: Combined guidance from both files"
echo ""

echo "📋 Communication Directory Overlap Test:"
echo "   File: src/communication/test_overlap_communication.c" 
echo "   Should match BOTH:"
echo "   - comms.instructions.md (src/communication/**/*.{c,h})"
echo "   - api-ui.instructions.md (src/{communication,application}/**/*.{c,h})"
echo "   Expected: Combined guidance from both files"
echo ""

echo "🎯 Manual Validation Steps:"
echo "1. Open each test file in VS Code"
echo "2. Add Copilot comments (// Copilot: <request>)"
echo "3. Observe suggestions to verify instruction file application"
echo "4. Document which instruction guidance appears"
echo "5. Verify overlapping patterns combine appropriately"
echo ""

echo "📊 Test File Summary:"
find . -name "test_*_scope.*" -o -name "test_overlap_*" | sort

echo ""
echo "✅ Test setup complete! Open test files in VS Code to validate Copilot integration."
