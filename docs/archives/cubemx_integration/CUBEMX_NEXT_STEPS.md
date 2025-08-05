# CubeMX Integration Status & Next Steps

## Current Status Assessment

### ✅ Successfully Implemented
You've made excellent progress on the CubeMX project setup:

1. **CubeMX Project Created**: `code.ioc` with STM32H753ZI configuration
2. **HSE Clock Configured**: 25MHz external crystal properly set
3. **System Clock**: 480MHz PLL configuration established
4. **Nucleo BSP Integration**: Board support package enabled
5. **USART3 VCP**: Virtual COM Port on PD8/PD9 for ST-Link
6. **Basic GPIO**: LEDs and user button configured
7. **Generated Code**: Core/ directory with HAL initialization

### 📋 Ready for Next Phase: Critical Peripherals

Based on your updated `.instructions` files, here are the **immediate next steps** for CubeMX:

## Phase 1: Motor Control Peripherals (DO NEXT)

### 1. Add SPI2 for L6470 Stepper Drivers
**CubeMX Configuration Steps:**
1. **Peripherals → SPI → SPI2**: Enable as Master
2. **Configuration Settings**:
   - Mode: Master
   - Hardware NSS: Disabled (software CS)
   - Data Size: 8 bits
   - First Bit: MSB First
   - Prescaler: 32 (gives ~7.5MHz from 240MHz APB)
   - Clock Polarity: High (CPOL=1)
   - Clock Phase: 2 Edge (CPHA=1)

3. **Pin Assignment**:
   - **PB13**: SPI2_SCK
   - **PB14**: SPI2_MISO 
   - **PB15**: SPI2_MOSI
   - **PA9**: GPIO_Output (CS - software controlled)

### 2. Add I2C1 & I2C2 for AS5600 Encoders  
**CubeMX Configuration Steps:**
1. **Peripherals → I2C → I2C1**: Enable Master mode
   - **PB6**: I2C1_SCL
   - **PB7**: I2C1_SDA
   - Speed: 400kHz (Fast Mode)

2. **Peripherals → I2C → I2C2**: Enable Master mode
   - **PB10**: I2C2_SCL
   - **PB11**: I2C2_SDA  
   - Speed: 400kHz (Fast Mode)

### 3. Add TIM2 for Control Loop
**CubeMX Configuration Steps:**
1. **Peripherals → Timers → TIM2**: Enable Internal Clock
2. **Configuration**:
   - Prescaler: 12000-1 (for 10kHz intermediate)
   - Period: 10-1 (for final 1kHz)
   - Enable Update Interrupt
3. **NVIC Settings**: Enable TIM2 global interrupt

## Phase 2: Communication Peripherals (AFTER PHASE 1)

### 4. Add FDCAN1 for MCU Communication
**CubeMX Configuration Steps:**
1. **Peripherals → FDCAN → FDCAN1**: Enable
2. **Pin Assignment**:
   - **PD0**: FDCAN1_RX
   - **PD1**: FDCAN1_TX
3. **Configuration**: 500kbps nominal bitrate

## Critical Pin Conflict Resolution

### ⚠️ **IMPORTANT**: Resolve Pin Conflicts First
Your current `code.ioc` shows:
- **PB14**: Currently GPIO_Output (LED) → **CONFLICTS** with SPI2_MISO
- **PB7**: Currently GPIO_Output (LED) → **CONFLICTS** with I2C1_SDA

**Fix in CubeMX:**
1. **Change PB14**: Remove GPIO_Output, assign to SPI2_MISO
2. **Change PB7**: Remove GPIO_Output, assign to I2C1_SDA  
3. **Reassign LEDs**: Use PE1 and PC1 for additional LEDs if needed

## Specific CubeMX Actions

### Step-by-Step Process:
1. **Open CubeMX**: Load your current `code.ioc`
2. **Resolve Conflicts**: 
   - Remove PB14 and PB7 as GPIO outputs
   - These will be reassigned to SPI2 and I2C1
3. **Add SPI2**: Configure as described above
4. **Add I2C1/I2C2**: Configure as described above  
5. **Add TIM2**: Configure for 1kHz control loop
6. **Generate Code**: Keep user code, regenerate
7. **Validate**: Build and test

### Expected Pin Configuration After Changes:
```
SPI2 (L6470 Motor Drivers):
  PB13 → SPI2_SCK
  PB14 → SPI2_MISO  (was LED, now SPI)
  PB15 → SPI2_MOSI
  PA9  → GPIO_Output (CS)

I2C1 (Encoder 1):
  PB6  → I2C1_SCL
  PB7  → I2C1_SDA   (was LED, now I2C)

I2C2 (Encoder 2):  
  PB10 → I2C2_SCL
  PB11 → I2C2_SDA

Control:
  TIM2 → 1kHz interrupt for control loop
```

## Post-Generation Integration

### 1. Verify SSOT Compliance
After code generation, check that generated defines in `main.h` match:
- `src/config/hardware_config.h` pin assignments
- Peripheral instance selections
- Clock configurations

### 2. Update Build System
```bash
# Test build after CubeMX generation
cd /workspaces/code
cmake --build build

# Validate SSOT compliance
python3 scripts/validate_ssot.py
```

### 3. Integration with Instruction Files
The generated HAL initialization should align with:
- **SPI**: `stm32h7-spi-l6470.instructions.md`
- **I2C**: `stm32h7-i2c-as5600.instructions.md`  
- **Timer**: `time-handling.instructions.md`
- **Pins**: `hardware-pins.instructions.md`

## Priority Rationale

**Phase 1 (Motor Control)** provides the **minimum viable product**:
- SPI2: Communication with L6470 stepper drivers
- I2C1/I2C2: Position feedback from AS5600 encoders
- TIM2: Real-time control loop timing

**Phase 2 (Communication)** adds **advanced features**:
- FDCAN1: Multi-MCU coordination
- Ethernet: Network connectivity  
- Enhanced protocols

This approach gets the core motor control system operational first, then adds communication capabilities.

## Ready to Proceed

Your foundation work is excellent! The instruction files are comprehensive and the basic CubeMX project is properly set up. You're ready to add the critical motor control peripherals (SPI2, I2C1/I2C2, TIM2) to complete the core functionality.

Focus on **Phase 1 peripherals first** - they provide the essential motor control capabilities needed for the stepper motor system.
