# STM32CubeMX CLI Integration Guide

## Overview

This guide provides multiple approaches for programmatically configuring STM32CubeMX for the STM32H753ZI motor control project, allowing you to add all required peripherals without manual GUI interaction.

## Available Tools

### 1. STM32CubeMX CLI Script (`cubemx_config.script`)
**Direct CubeMX CLI scripting for complete automation**

### 2. Bash Automation (`run_cubemx_cli.sh`) 
**Wrapper script for Windows STM32CubeMX execution with validation**

### 3. Python Configuration Generator (`cubemx_config_generator.py`)
**Pure Python approach that directly modifies .ioc files**

## Method 1: STM32CubeMX CLI (Recommended)

### Prerequisites
- STM32CubeMX installed at: `C:\STMicroelectronics\STM32Cube\STM32CubeMX`
- WSL/dev container with Windows access
- Current `code.ioc` file in workspace

### Quick Execution

#### Option A: Automated Script
```bash
# Run the automated CLI wrapper
./scripts/run_cubemx_cli.sh

# Or with specific commands
./scripts/run_cubemx_cli.sh run      # Execute CubeMX CLI
./scripts/run_cubemx_cli.sh manual   # Show manual instructions  
./scripts/run_cubemx_cli.sh validate # Validate generated code
./scripts/run_cubemx_cli.sh help     # Show help
```

#### Option B: Direct Windows Execution
```cmd
# Windows Command Prompt
cd "C:\STMicroelectronics\STM32Cube\STM32CubeMX"
STM32CubeMX.exe -q "\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_config.script"
```

#### Option C: PowerShell
```powershell
# Windows PowerShell
& "C:\STMicroelectronics\STM32Cube\STM32CubeMX\STM32CubeMX.exe" -q "\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_config.script"
```

### What the CLI Script Configures

#### SPI2 - L6470 Stepper Drivers
```
Pins: PB13(SCK), PB14(MISO), PB15(MOSI), PA9(CS)
Mode: Master, 8-bit, Mode 3 (CPOL=1, CPHA=1)
Speed: ~7.5MHz (prescaler 32 from 240MHz APB)
Control: PA10(FAULT), PA11(BUSY) with pull-ups
```

#### I2C1 & I2C2 - AS5600 Encoders
```
I2C1: PB6(SCL), PB7(SDA) - 400kHz Fast Mode
I2C2: PB10(SCL), PB11(SDA) - 400kHz Fast Mode
Addresses: Both 0x36 (requires separate buses)
```

#### TIM2 - Control Loop Timer
```
Frequency: 1kHz (120MHz/12000/10)
Interrupt: Enabled with priority 1
Purpose: Real-time motor control loop
```

#### FDCAN1 - MCU Communication
```
Pins: PD0(RX), PD1(TX)
Bitrate: 500kbps nominal, 2Mbps data
Mode: FD with bit rate switching
```

## Method 2: Python Configuration Generator

### Direct .ioc File Modification
```bash
# Run Python configurator
python3 scripts/cubemx_config_generator.py

# This approach:
# 1. Creates backup of current code.ioc
# 2. Programmatically adds peripheral configurations
# 3. Resolves pin conflicts automatically
# 4. Validates configuration
# 5. Provides detailed summary
```

### Advantages of Python Approach
- ✅ **No CubeMX GUI required**
- ✅ **Works entirely in dev container**
- ✅ **Automatic conflict resolution**
- ✅ **Built-in validation**
- ✅ **Detailed logging and summaries**

### Python Script Features
```python
# Key capabilities:
- Backup creation with timestamps
- Pin conflict detection and resolution
- SSOT compliance validation
- Comprehensive error checking
- Detailed configuration summary
```

## Configuration Details

### Peripheral Additions

#### Phase 1: Core Motor Control (CRITICAL)
1. **SPI2**: L6470 stepper driver communication
2. **I2C1/I2C2**: AS5600 encoder feedback (dual buses)
3. **TIM2**: 1kHz control loop timing

#### Phase 2: Communication (HIGH PRIORITY)
4. **FDCAN1**: Inter-MCU communication protocol

### Pin Conflict Resolution

#### Automatic Conflict Handling
Both scripts automatically resolve these conflicts:
```
PB14: GPIO_Output (LED) → SPI2_MISO
PB7:  GPIO_Output (LED) → I2C1_SDA
```

#### Alternative LED Assignments
```
PE1: STATUS_LED_1 (replaces PB14)
PC1: STATUS_LED_2 (replaces PB7)
```

## Post-Configuration Steps

### 1. Code Generation
```bash
# If using CubeMX CLI - code is auto-generated
# If using Python approach - open CubeMX to generate:
stm32cubemx code.ioc  # Then generate code
```

### 2. Build System Validation
```bash
# Test build system
cmake --build build

# If build fails, reconfigure CMake
cmake -S . -B build -G Ninja
```

### 3. SSOT Compliance Check
```bash
# Validate against SSOT configuration
python3 scripts/validate_ssot.py

# Check instruction file compliance
python3 scripts/link_validator.py
```

### 4. Peripheral Validation
```bash
# Check generated pin assignments
grep -E "(SPI2|I2C[12]|TIM2|FDCAN1)" Core/Inc/main.h

# Verify peripheral instances
grep -E "hspi2|hi2c[12]|htim2|hfdcan1" Core/Src/main.c
```

## Troubleshooting

### Common Issues

#### WSL Path Access
```bash
# If WSL paths don't work, use UNC paths:
\\wsl$\Ubuntu\workspaces\code\scripts\cubemx_config.script

# Or copy to Windows temp:
cp scripts/cubemx_config.script /mnt/c/temp/
# Then run: STM32CubeMX.exe -q "C:\temp\cubemx_config.script"
```

#### Permission Issues
```bash
# Ensure scripts are executable
chmod +x scripts/run_cubemx_cli.sh
chmod +x scripts/cubemx_config_generator.py

# Check file access
ls -la scripts/cubemx_config.script
```

#### CubeMX Not Found
```bash
# Verify CubeMX installation
ls /mnt/c/STMicroelectronics/STM32Cube/STM32CubeMX/

# Alternative installation paths:
# C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeMX
# C:\ST\STM32CubeMX
```

### Validation Failures

#### Pin Conflicts
```bash
# Check for remaining conflicts
python3 scripts/cubemx_config_generator.py
# Review conflict resolution messages
```

#### Build Issues
```bash
# Clean rebuild
rm -rf build
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi-gcc.cmake
cmake --build build
```

## Expected Results

### Successful Configuration Output
```
✅ SPI2 configuration added (L6470 drivers)
✅ I2C1 & I2C2 configuration added (AS5600 encoders)  
✅ TIM2 configuration added (1kHz control loop)
✅ FDCAN1 configuration added (MCU communication)
✅ Pin conflicts resolved (PB14, PB7)
✅ IP count updated to 11
✅ Code generation successful
✅ Build system validated
✅ SSOT compliance verified
```

### Generated Files
```
Core/Inc/main.h          # Pin definitions and peripheral handles
Core/Src/main.c          # Peripheral initialization code  
Core/Src/stm32h7xx_it.c  # Interrupt handlers (TIM2, FDCAN1)
Core/Inc/stm32h7xx_hal_conf.h  # HAL module configuration
```

## Integration with Instruction Files

### Alignment Verification
After configuration, verify alignment with:
- `.github/instructions/hardware-pins.instructions.md` - Pin assignments
- `.github/instructions/stm32h7-spi-l6470.instructions.md` - SPI implementation
- `.github/instructions/stm32h7-i2c-as5600.instructions.md` - I2C implementation  
- `.github/instructions/time-handling.instructions.md` - Timer configuration
- `src/config/hardware_config.h` - SSOT configuration

### Next Development Steps
1. **Test peripheral initialization** in main.c
2. **Implement driver layers** using instruction files
3. **Add safety systems** per safety-systems.instructions.md
4. **Integrate control algorithms** per control-timing.instructions.md

## Performance Notes

### CubeMX CLI Approach
- **Speed**: Fast (5-10 seconds)
- **Reliability**: High (official ST tool)
- **Requirements**: Windows STM32CubeMX installation

### Python Approach  
- **Speed**: Very fast (1-2 seconds)
- **Reliability**: High (direct file manipulation)
- **Requirements**: Python 3 only (container-native)

Both approaches produce identical functional results and maintain SSOT compliance with the comprehensive instruction system.
