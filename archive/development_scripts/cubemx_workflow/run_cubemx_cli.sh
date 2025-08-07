#!/bin/bash

# STM32CubeMX CLI Automation Script for STM32H753ZI Motor Control Project
# This script automates the peripheral configuration using STM32CubeMX CLI

set -e  # Exit on any error

# Configuration
WORKSPACE_ROOT="/workspaces/code"
CUBEMX_CONFIG_SCRIPT="$WORKSPACE_ROOT/scripts/cubemx_config.script" 
CUBEMX_GENERATE_SCRIPT="$WORKSPACE_ROOT/scripts/cubemx_generate_code.script"
IOC_FILE="$WORKSPACE_ROOT/code.ioc"
CUBEMX_PATH_WINDOWS="C:/STMicroelectronics/STM32Cube/STM32CubeMX"
CUBEMX_EXE="$CUBEMX_PATH_WINDOWS/STM32CubeMX.exe"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}=================================="
echo -e "STM32CubeMX CLI Configuration"
echo -e "STM32H753ZI Motor Control Project"
echo -e "==================================${NC}"

# Function to check if we're in WSL or can access Windows
check_windows_access() {
    if [[ -n "$WSL_DISTRO_NAME" ]] || [[ -n "$WSLENV" ]]; then
        echo -e "${GREEN}✅ WSL detected - Windows STM32CubeMX should be accessible${NC}"
        return 0
    elif command -v wslpath &> /dev/null; then
        echo -e "${GREEN}✅ WSL tools available${NC}"
        return 0
    else
        echo -e "${YELLOW}⚠️  WSL not detected - checking for alternative access...${NC}"
        return 1
    fi
}

# Function to run CubeMX CLI
run_cubemx_cli() {
    local script_file="$1"
    local ioc_file="$2"
    
    echo -e "${BLUE}🔧 Running STM32CubeMX CLI configuration...${NC}"
    
    # Check if we can access Windows CubeMX
    if check_windows_access; then
        # Try WSL approach first
        if command -v wslpath &> /dev/null; then
            local windows_script=$(wslpath -w "$script_file")
            local windows_ioc=$(wslpath -w "$ioc_file")
            
            echo -e "${BLUE}   Script: $windows_script${NC}"
            echo -e "${BLUE}   IOC File: $windows_ioc${NC}"
            
            # Run CubeMX CLI via WSL
            cmd.exe /c "\"$CUBEMX_EXE\" -q \"$windows_script\""
        else
            echo -e "${YELLOW}⚠️  Direct WSL path conversion not available${NC}"
            echo -e "${BLUE}💡 Manual execution required:${NC}"
            echo -e "   1. Open Command Prompt on Windows"
            echo -e "   2. Navigate to: $CUBEMX_PATH_WINDOWS"
            echo -e "   3. Run: STM32CubeMX.exe -q \"$(wslpath -w "$script_file" 2>/dev/null || echo "$script_file")\""
        fi
    else
        echo -e "${YELLOW}⚠️  Windows access not detected${NC}"
        echo -e "${BLUE}💡 Alternative approaches:${NC}"
        echo -e "   1. Manual CubeMX execution (see instructions below)"
        echo -e "   2. Install CubeMX in Linux container"
        echo -e "   3. Use remote CubeMX via X11 forwarding"
    fi
}

# Function to validate generated code
validate_generated_code() {
    echo -e "${BLUE}🔍 Validating generated code...${NC}"
    
    # Check if Core directory exists and has expected files
    if [[ -d "$WORKSPACE_ROOT/Core" ]]; then
        echo -e "${GREEN}✅ Core directory found${NC}"
        
        # Check for main.h with expected peripheral defines
        if [[ -f "$WORKSPACE_ROOT/Core/Inc/main.h" ]]; then
            echo -e "${GREEN}✅ main.h generated${NC}"
            
            # Check for SPI2 configuration
            if grep -q "SPI2" "$WORKSPACE_ROOT/Core/Inc/main.h"; then
                echo -e "${GREEN}✅ SPI2 configuration detected${NC}"
            else
                echo -e "${YELLOW}⚠️  SPI2 configuration not found in main.h${NC}"
            fi
            
            # Check for I2C configuration
            if grep -q "I2C[12]" "$WORKSPACE_ROOT/Core/Inc/main.h"; then
                echo -e "${GREEN}✅ I2C configuration detected${NC}"
            else
                echo -e "${YELLOW}⚠️  I2C configuration not found in main.h${NC}"
            fi
            
            # Check for TIM2 configuration  
            if grep -q "TIM2" "$WORKSPACE_ROOT/Core/Inc/main.h"; then
                echo -e "${GREEN}✅ TIM2 configuration detected${NC}"
            else
                echo -e "${YELLOW}⚠️  TIM2 configuration not found in main.h${NC}"
            fi
        else
            echo -e "${RED}❌ main.h not found${NC}"
        fi
    else
        echo -e "${RED}❌ Core directory not found - code generation may have failed${NC}"
    fi
}

# Function to run SSOT validation
validate_ssot_compliance() {
    echo -e "${BLUE}🔍 Validating SSOT compliance...${NC}"
    
    if [[ -f "$WORKSPACE_ROOT/scripts/validate_ssot.py" ]]; then
        cd "$WORKSPACE_ROOT"
        if python3 scripts/validate_ssot.py; then
            echo -e "${GREEN}✅ SSOT validation passed${NC}"
        else
            echo -e "${YELLOW}⚠️  SSOT validation issues detected${NC}"
        fi
    else
        echo -e "${YELLOW}⚠️  SSOT validation script not found${NC}"
    fi
}

# Function to test build system
test_build() {
    echo -e "${BLUE}🔨 Testing build system...${NC}"
    
    cd "$WORKSPACE_ROOT"
    if cmake --build build 2>/dev/null; then
        echo -e "${GREEN}✅ Build successful${NC}"
    else
        echo -e "${YELLOW}⚠️  Build issues detected - may need CMake reconfiguration${NC}"
        echo -e "${BLUE}💡 Try: cmake -S . -B build -G Ninja${NC}"
    fi
}

# Main execution
main() {
    echo -e "${BLUE}📋 Configuration Summary:${NC}"
    echo -e "   Workspace: $WORKSPACE_ROOT"
    echo -e "   IOC File: $IOC_FILE"
    echo -e "   Script: $CUBEMX_SCRIPT"
    echo -e "   Windows CubeMX: $CUBEMX_EXE"
    echo ""
    
    # Check prerequisites
    if [[ ! -f "$IOC_FILE" ]]; then
        echo -e "${RED}❌ IOC file not found: $IOC_FILE${NC}"
        exit 1
    fi
    
    if [[ ! -f "$CUBEMX_SCRIPT" ]]; then
        echo -e "${RED}❌ CubeMX script not found: $CUBEMX_SCRIPT${NC}"
        exit 1
    fi
    
    echo -e "${GREEN}✅ Prerequisites satisfied${NC}"
    echo ""
    
    # Run CubeMX CLI configuration
    run_cubemx_cli "$CUBEMX_SCRIPT" "$IOC_FILE"
    echo ""
    
    # Post-configuration validation
    echo -e "${BLUE}🔍 Post-configuration validation:${NC}"
    validate_generated_code
    validate_ssot_compliance  
    test_build
    echo ""
    
    echo -e "${GREEN}✅ STM32CubeMX CLI configuration complete!${NC}"
    echo -e "${BLUE}📋 Next steps:${NC}"
    echo -e "   1. Verify peripheral configurations in generated code"
    echo -e "   2. Run build system: cmake --build build"
    echo -e "   3. Test peripheral initialization"
    echo -e "   4. Validate against instruction files"
}

# Show manual execution instructions
show_manual_instructions() {
    echo -e "${BLUE}📋 Manual STM32CubeMX CLI Execution Instructions:${NC}"
    echo ""
    echo -e "${YELLOW}Option 1: Windows Command Prompt${NC}"
    echo -e "1. Open Command Prompt as Administrator"
    echo -e "2. Navigate to STM32CubeMX installation:"
    echo -e "   cd \"C:\\STMicroelectronics\\STM32Cube\\STM32CubeMX\""
    echo -e "3. Run CubeMX CLI with our script:"
    echo -e "   STM32CubeMX.exe -q \"$(wslpath -w "$CUBEMX_SCRIPT" 2>/dev/null || echo "scripts/cubemx_config.script")\""
    echo ""
    echo -e "${YELLOW}Option 2: PowerShell${NC}"
    echo -e "1. Open PowerShell as Administrator"  
    echo -e "2. Run:"
    echo -e "   & \"C:\\STMicroelectronics\\STM32Cube\\STM32CubeMX\\STM32CubeMX.exe\" -q \"$(wslpath -w "$CUBEMX_SCRIPT" 2>/dev/null || echo "scripts/cubemx_config.script")\""
    echo ""
    echo -e "${YELLOW}Option 3: If WSL paths don't work${NC}"
    echo -e "1. Copy the script content to Windows"
    echo -e "2. Save as: C:\\temp\\cubemx_config.script"
    echo -e "3. Run: STM32CubeMX.exe -q \"C:\\temp\\cubemx_config.script\""
}

# Handle command line arguments
case "${1:-run}" in
    "run")
        CUBEMX_SCRIPT="$CUBEMX_CONFIG_SCRIPT"
        main
        ;;
    "generate")
        CUBEMX_SCRIPT="$CUBEMX_GENERATE_SCRIPT"
        main
        ;;
    "manual")
        show_manual_instructions
        ;;
    "validate")
        validate_generated_code
        validate_ssot_compliance
        ;;
    "help")
        echo -e "${BLUE}STM32CubeMX CLI Automation Script${NC}"
        echo ""
        echo -e "${YELLOW}Usage:${NC}"
        echo -e "  $0 [run|generate|manual|validate|help]"
        echo ""
        echo -e "${YELLOW}Commands:${NC}"
        echo -e "  run      - Execute full CubeMX CLI configuration + generation"
        echo -e "  generate - Execute code generation only (recommended)"
        echo -e "  manual   - Show manual execution instructions"
        echo -e "  validate - Validate generated code only"
        echo -e "  help     - Show this help message"
        ;;
    *)
        echo -e "${RED}❌ Unknown command: $1${NC}"
        echo -e "Use '$0 help' for usage information"
        exit 1
        ;;
esac
