# CubeMX Peripheral Integration Plan

## Current Status Analysis
Based on review of `.instructions` files and `code.ioc` analysis:

### ✅ Currently Configured in CubeMX
- **HSE Clock**: 25MHz external oscillator (PH0/PH1) ✅ 
- **System Clock**: 480MHz PLL configuration ✅
- **USART3**: ST-Link VCP (PD8/PD9) ✅
- **Basic GPIO**: LEDs and user button ✅
- **Nucleo BSP**: Board support package enabled ✅

### 🔄 Peripherals to Add/Configure

#### 1. SPI2 - L6470 Stepper Drivers (Priority: CRITICAL)
**Based on**: `.github/instructions/stm32h7-spi-l6470.instructions.md`
**Hardware**: X-NUCLEO-IHM02A1 shield with dual L6470 drivers
```
Configuration Required:
- SPI2 Master Mode
- Pins: PB13(SCK), PB14(MISO), PB15(MOSI)
- CS: PA9 (software controlled, daisy-chain)
- Additional pins: PA10(FAULT), PA11(BUSY)
- SPI Mode 3 (CPOL=1, CPHA=1)
- Max 5MHz clock (L6470 limitation)
```

#### 2. I2C1 & I2C2 - AS5600 Magnetic Encoders (Priority: CRITICAL)
**Based on**: `.github/instructions/stm32h7-i2c-as5600.instructions.md`
**Hardware**: Dual AS5600 encoders (same address 0x36, requires separate buses)
```
I2C1 Configuration (Encoder 1):
- Pins: PB6(SCL), PB7(SDA)
- Speed: 400kHz (Fast Mode)
- Address: 0x36

I2C2 Configuration (Encoder 2):
- Pins: PB10(SCL), PB11(SDA)  
- Speed: 400kHz (Fast Mode)
- Address: 0x36
```

#### 3. FDCAN1 - MCU-to-MCU Communication (Priority: HIGH)
**Based on**: `.github/instructions/comm-protocols.instructions.md`
**Hardware**: External CAN transceiver required
```
Configuration Required:
- FDCAN1 peripheral
- Pins: PD0(RX), PD1(TX)
- Bitrate: 500kbps standard, 1Mbps capable
- External transceiver: MCP2551 or SN65HVD230
```

#### 4. Ethernet RMII - Network Connectivity (Priority: MEDIUM)
**Based on**: `.github/instructions/comm-protocols.instructions.md`
**Hardware**: Built-in LAN8742 PHY on Nucleo
```
Configuration Required:
- ETH peripheral with RMII interface
- Multiple pins across PA, PC, PG ports
- LwIP stack integration
- On-board LAN8742 PHY (address 0x00)
```

#### 5. TIM2 - Control Loop Timer (Priority: HIGH)
**Based on**: `.github/instructions/time-handling.instructions.md`
```
Configuration Required:
- TIM2 for 1kHz control loop
- Interrupt-driven timing
- High priority NVIC setting
```

#### 6. DMA Configuration (Priority: MEDIUM)
**For high-performance transfers**:
- SPI2 TX/RX DMA streams
- I2C1/I2C2 DMA if needed
- UART3 DMA for command processing

#### 7. Additional GPIO (Priority: MEDIUM)
**Based on safety and user interface requirements**:
- Additional status LEDs
- Emergency stop inputs
- Motor fault monitoring pins

## Integration Priority Order

### Phase 1: Core Motor Control (IMMEDIATE)
1. **SPI2** - L6470 motor drivers
2. **I2C1/I2C2** - AS5600 encoders  
3. **TIM2** - Control loop timing

### Phase 2: Communication (NEXT)
4. **FDCAN1** - Inter-MCU communication
5. **Enhanced UART3** - Command protocol

### Phase 3: Advanced Features (LATER)
6. **Ethernet RMII** - Network connectivity
7. **DMA** - Performance optimization
8. **Additional GPIO** - Enhanced I/O

## CubeMX Configuration Steps

### Step 1: SPI2 Configuration
1. Enable SPI2 in Master mode
2. Set pins: PB13/PB14/PB15 with AF5_SPI2
3. Configure: Mode 3, 8-bit, ~5MHz
4. Add PA9 as GPIO Output (CS)
5. Add PA10/PA11 as GPIO Input (FAULT/BUSY)

### Step 2: I2C Configuration  
1. Enable I2C1: PB6/PB7 with AF4_I2C1
2. Enable I2C2: PB10/PB11 with AF4_I2C2
3. Set both to 400kHz Fast Mode
4. Configure pull-up resistors

### Step 3: Timer Configuration
1. Enable TIM2 for control loop
2. Configure for 1kHz interrupt
3. Set appropriate NVIC priority

### Step 4: FDCAN Configuration
1. Enable FDCAN1
2. Set pins: PD0/PD1 with AF9_FDCAN1
3. Configure bitrate and filters

## SSOT Integration Notes

All pin assignments and configurations must match `src/config/hardware_config.h`. After CubeMX regeneration, verify:

1. Generated pin assignments match SSOT
2. Clock configurations are preserved
3. Peripheral configurations align with instruction files
4. No conflicts with existing Nucleo BSP functions

## Next Actions Required

1. **Open CubeMX**: Load current `code.ioc`
2. **Add SPI2**: Configure for L6470 drivers
3. **Add I2C1/I2C2**: Configure for AS5600 encoders
4. **Add TIM2**: Configure control loop timing
5. **Generate Code**: Verify against SSOT configuration
6. **Validate**: Test peripheral initialization

## Reference Documentation

- Hardware pins: `.github/instructions/hardware-pins.instructions.md`
- SPI implementation: `.github/instructions/stm32h7-spi-l6470.instructions.md`
- I2C implementation: `.github/instructions/stm32h7-i2c-as5600.instructions.md`
- Communication protocols: `.github/instructions/comm-protocols.instructions.md`
- SSOT configuration: `src/config/hardware_config.h`

This plan ensures systematic integration of all required peripherals while maintaining SSOT compliance and following the modular instruction system.
