# STM32H753ZI CubeMX Configuration Update

## Current Analysis Summary

After reviewing all `.instructions` files and current `code.ioc`, here's what needs to be integrated:

### ✅ Already Configured
- **System Clock**: HSE 25MHz → PLL → 480MHz system clock  
- **USART3**: Virtual COM Port on PD8/PD9
- **Basic GPIO**: LEDs (PB0, PB14, PB7) and user button (PC13)
- **Nucleo BSP**: Board support package enabled

### 🔄 Missing Critical Peripherals

## Required CubeMX Configuration Changes

### 1. SPI2 - L6470 Stepper Motor Drivers
**Reference**: `stm32h7-spi-l6470.instructions.md`

**Add to CubeMX:**
```ini
# SPI2 Configuration
Mcu.IP6=SPI2
SPI2.Mode=SPI_MODE_MASTER
SPI2.Direction=SPI_DIRECTION_2LINES
SPI2.DataSize=SPI_DATASIZE_8BIT
SPI2.CLKPolarity=SPI_POLARITY_HIGH
SPI2.CLKPhase=SPI_PHASE_2EDGE
SPI2.NSS=SPI_NSS_SOFT
SPI2.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_32
SPI2.FirstBit=SPI_FIRSTBIT_MSB

# SPI2 Pins (Arduino Shield Compatible)
PB13.Mode=Alternate
PB13.Alternate=GPIO_AF5_SPI2
PB13.Signal=SPI2_SCK
PB13.Locked=true

PB14.Mode=Alternate  
PB14.Alternate=GPIO_AF5_SPI2
PB14.Signal=SPI2_MISO
PB14.Locked=true

PB15.Mode=Alternate
PB15.Alternate=GPIO_AF5_SPI2  
PB15.Signal=SPI2_MOSI
PB15.Locked=true

# CS Pin (Software Controlled)
PA9.Mode=Output
PA9.Signal=GPIO_Output
PA9.Locked=true

# L6470 Control Pins
PA10.Mode=Input
PA10.Signal=GPIO_Input
PA10.Locked=true

PA11.Mode=Input
PA11.Signal=GPIO_Input  
PA11.Locked=true
```

### 2. I2C1 & I2C2 - AS5600 Magnetic Encoders
**Reference**: `stm32h7-i2c-as5600.instructions.md`

**Add to CubeMX:**
```ini
# I2C1 Configuration (Encoder 1)
Mcu.IP7=I2C1
I2C1.I2C_Mode=I2C_Fast
I2C1.I2C_Speed_Mode=I2C_Fast
I2C1.I2C_Analog_Filter=I2C_ANALOGFILTER_ENABLE

PB6.Mode=Alternate
PB6.Alternate=GPIO_AF4_I2C1
PB6.Signal=I2C1_SCL
PB6.Locked=true

PB7.Mode=Alternate
PB7.Alternate=GPIO_AF4_I2C1
PB7.Signal=I2C1_SDA
PB7.Locked=true

# I2C2 Configuration (Encoder 2)  
Mcu.IP8=I2C2
I2C2.I2C_Mode=I2C_Fast
I2C2.I2C_Speed_Mode=I2C_Fast
I2C2.I2C_Analog_Filter=I2C_ANALOGFILTER_ENABLE

PB10.Mode=Alternate
PB10.Alternate=GPIO_AF4_I2C2
PB10.Signal=I2C2_SCL
PB10.Locked=true

PB11.Mode=Alternate
PB11.Alternate=GPIO_AF4_I2C2
PB11.Signal=I2C2_SDA
PB11.Locked=true
```

### 3. TIM2 - Control Loop Timer
**Reference**: `time-handling.instructions.md`

**Add to CubeMX:**
```ini
# TIM2 Configuration
Mcu.IP9=TIM2
TIM2.IPParameters=Prescaler,Period
TIM2.Prescaler=12000-1
TIM2.Period=10-1
# This gives 1kHz: 120MHz / 12000 / 10 = 1000Hz

# Enable TIM2 interrupt
NVIC.TIM2_IRQn=true:1:0:false:false:true:true:true:true
```

### 4. FDCAN1 - Inter-MCU Communication  
**Reference**: `comm-protocols.instructions.md`

**Add to CubeMX:**
```ini
# FDCAN1 Configuration
Mcu.IP10=FDCAN1
FDCAN1.NominalBitRate=500000
FDCAN1.NominalSyncJumpWidth=1
FDCAN1.DataBitRate=2000000
FDCAN1.DataSyncJumpWidth=1

PD0.Mode=Alternate
PD0.Alternate=GPIO_AF9_FDCAN1
PD0.Signal=FDCAN1_RX
PD0.Locked=true

PD1.Mode=Alternate
PD1.Alternate=GPIO_AF9_FDCAN1
PD1.Signal=FDCAN1_TX
PD1.Locked=true

# Enable FDCAN1 interrupt
NVIC.FDCAN1_IT0_IRQn=true:1:0:false:false:true:true:true:true
```

### 5. Update IPNb Count
```ini
Mcu.IPNb=11
```

## Pin Conflicts Resolution

### Current Pin Usage Check
Based on `code.ioc` analysis:

**✅ No Conflicts Detected:**
- PB13, PB15: Currently unassigned → Available for SPI2
- PB6, PB10, PB11: Currently unassigned → Available for I2C  
- PA9, PA10, PA11: Currently unassigned → Available for SPI CS/control
- PD0, PD1: Currently unassigned → Available for FDCAN

**⚠️ Potential Conflicts:**
- PB14: Currently set as GPIO_Output → **CONFLICT** with SPI2_MISO
- PB7: Currently set as GPIO_Output → **CONFLICT** with I2C1_SDA

### Conflict Resolution Required

**Update existing pin assignments:**
```ini
# Remove PB14 as LED (reassign to different pin)
# Remove PB7 as LED (reassign to different pin)

# Reassign LEDs to avoid conflicts
PE1.Mode=Output      # Use PE1 for LED instead of PB14
PE1.Signal=GPIO_Output
PE1.Locked=true

PC1.Mode=Output      # Use PC1 for LED instead of PB7  
PC1.Signal=GPIO_Output
PC1.Locked=true
```

## Required Manual Actions

### 1. Open CubeMX Project
```bash
# Launch STM32CubeMX
stm32cubemx code.ioc
```

### 2. Add Peripherals in Order
1. **SPI2**: Configure master mode, pins PB13/PB14/PB15
2. **I2C1**: Configure fast mode, pins PB6/PB7  
3. **I2C2**: Configure fast mode, pins PB10/PB11
4. **TIM2**: Configure for 1kHz interrupt
5. **FDCAN1**: Configure 500kbps, pins PD0/PD1

### 3. Resolve Pin Conflicts
1. Reassign PB14 LED to PE1
2. Reassign PB7 LED to PC1
3. Verify no remaining conflicts

### 4. Generate Code  
1. Keep existing user code
2. Verify main.c initialization order
3. Check against SSOT configuration

### 5. Validate Configuration
```bash
# After code generation, validate SSOT compliance
python3 scripts/validate_ssot.py --check-pins

# Build and test
cmake --build build
```

## Integration Notes

### SSOT Compliance
All pin assignments must match `src/config/hardware_config.h`. After generation:
1. Verify generated pin defines match SSOT
2. Update SSOT if any discrepancies found
3. Ensure peripheral instances match configuration

### Instruction File Alignment
Generated code should align with:
- `hardware-pins.instructions.md` - Pin assignments
- `stm32h7-spi-l6470.instructions.md` - SPI implementation  
- `stm32h7-i2c-as5600.instructions.md` - I2C implementation
- `comm-protocols.instructions.md` - Protocol specifications

### Build System Integration
After CubeMX generation:
1. Update CMakeLists.txt if new source files added
2. Verify HAL library components included
3. Test build process with new peripherals

This configuration provides the complete peripheral set needed for the STM32H753ZI stepper motor control project while maintaining SSOT compliance and following the modular instruction system.
